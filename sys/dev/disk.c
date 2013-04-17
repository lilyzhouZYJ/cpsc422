#include <sys/context.h>
#include <sys/debug.h>
#include <sys/intr.h>
#include <sys/pcpu.h>
#include <sys/proc.h>
#include <sys/queue.h>
#include <sys/sched.h>
#include <sys/spinlock.h>
#include <sys/string.h>
#include <sys/trap.h>
#include <sys/types.h>

#include <dev/disk.h>

#ifdef DEBUG_DISK

#define DISK_DEBUG(fmt, ...)				\
	do {						\
		KERN_DEBUG("DISK: "fmt, ##__VA_ARGS__);	\
	} while (0)

#else

#define DISK_DEBUG(fmt, ...)			\
	do {					\
	} while (0)

#endif

static bool disk_mgmt_inited = FALSE;

static TAILQ_HEAD(all_disk_dev, disk_dev) all_disk_devices;
static spinlock_t disk_mgmt_lock;

int
disk_init(void)
{
	if (disk_mgmt_inited == TRUE || pcpu_onboot() == FALSE)
		return 0;

	TAILQ_INIT(&all_disk_devices);
	spinlock_init(&disk_mgmt_lock);
	disk_mgmt_inited = TRUE;

	return 0;
}

/*
 * XXX: disk_add_device() should always be invoked by the kernel. No user
 *      behavior can invoke this function. Therefore, it's safe to use
 *      spinlock_acquire(&existing_dev->lk) without worring about the dead
 *      lock.
 */
int
disk_add_device(struct disk_dev *dev)
{
	KERN_ASSERT(disk_mgmt_inited == TRUE);
	KERN_ASSERT(dev != NULL);

	struct disk_dev *existing_dev;

	spinlock_acquire(&disk_mgmt_lock);

	TAILQ_FOREACH(existing_dev, &all_disk_devices, entry) {
		if (existing_dev == dev) {
			DISK_DEBUG("Cannot add existing devices.\n");
			spinlock_release(&disk_mgmt_lock);
			return E_DISK_DUPDEV;
		}
	}

	if (sched_add_slpq(dev)) {
		DISK_DEBUG("Cannot add a sleep queue.\n");
		return E_DISK_ERROR;
	}

	spinlock_acquire(&dev->lk);
	dev->status = XFER_SUCC;
	memzero(&dev->last_req, sizeof(dev->last_req));
	TAILQ_INSERT_TAIL(&all_disk_devices, dev, entry);
	DISK_DEBUG("Add a disk device 0x%08x.\n", dev);
	spinlock_release(&dev->lk);

	spinlock_release(&disk_mgmt_lock);
	return E_DISK_SUCC;
}

/*
 * XXX: disk_remove_device() should always be invoked by the kernel. No user
 *      hehavior can invoke this function. Therefore, it's safe to use
 *      spinlock_acquire(&existing_dev->lk) without worring about the dead
 *      lock.
 */
int
disk_remove_device(struct disk_dev *dev)
{
	KERN_ASSERT(disk_mgmt_inited == TRUE);
	KERN_ASSERT(dev != NULL);

	struct disk_dev *existing_dev;

	if (sched_remove_slpq(dev)) {
		DISK_DEBUG("Device 0x%08x is busy.\n", dev);
		return E_DISK_ERROR;
	}

	spinlock_acquire(&disk_mgmt_lock);

	TAILQ_FOREACH(existing_dev, &all_disk_devices, entry) {
		if (existing_dev == dev) {
			spinlock_acquire(&dev->lk);
			TAILQ_REMOVE(&all_disk_devices, dev, entry);
			spinlock_release(&dev->lk);
			spinlock_release(&disk_mgmt_lock);
			/* TODO: remove interrupt handler */
			return E_DISK_SUCC;
		}
	}

	spinlock_release(&disk_mgmt_lock);
	return E_DISK_NODEV;
}

static int
disk_xfer_helper(struct disk_dev *dev,
		 uint64_t lba, uintptr_t pa, uint16_t nsect, bool write)
{
	KERN_ASSERT(spinlock_holding(&dev->lk) == TRUE);
	KERN_ASSERT(dev->status != XFER_PROCESSING);

	int rc;

	if (dev->last_req.retry > 1) {
		DISK_DEBUG("Exceed maximum retries.\n");
		return 1;
	}

	dev->last_req.write = write;
	dev->last_req.lba = lba;
	dev->last_req.buf_pa = pa;
	dev->last_req.nsect = nsect;

	rc = (write == TRUE) ? dev->dma_write(dev, lba, nsect, pa) :
		dev->dma_read(dev, lba, nsect, pa);
	if (rc)
		return 1;

	/* sleep to wait for the completion of the transfer */
	dev->status = XFER_PROCESSING;

	return 0;
}

int
disk_xfer(struct disk_dev *dev, uint64_t lba, uintptr_t pa, uint16_t nsect,
	  bool write)
{
	KERN_ASSERT(disk_mgmt_inited == TRUE);
	KERN_ASSERT(dev != NULL);
	KERN_ASSERT((write == TRUE && dev->dma_write != NULL) ||
		    (write == FALSE && dev->dma_read != NULL));
	KERN_ASSERT(dev->intr_handler != NULL);

	int rc;
	struct proc *caller = proc_cur();

	KERN_ASSERT(caller != NULL);

	/* if others are using the device, ... */
	while (spinlock_try_acquire(&dev->lk))
		proc_yield();

	if ((rc = disk_xfer_helper(dev, lba, pa, nsect, write))) {
		DISK_DEBUG("disk_xfer() error %d.\n", rc);
		dev->status = XFER_FAIL;
		memzero(&dev->last_req, sizeof(dev->last_req));
		spinlock_release(&dev->lk);
		return E_DISK_XFER;
	}

	/*
	 * XXX: The process caller is sleeping with the spinlock dev->lk. It's
	 *      risky to do so which may cause dead locks.
	 */
	DISK_DEBUG("Process %d is sleeping for device 0x%08x...\n",
		   caller->pid, dev);
	proc_sleep(caller, dev, NULL);

	KERN_ASSERT(dev->status != XFER_PROCESSING);
	KERN_ASSERT(spinlock_holding(&dev->lk) == TRUE);

	memzero(&dev->last_req, sizeof(dev->last_req));

	/* transfer is accomplished */
	if (dev->status == XFER_SUCC) {
		spinlock_release(&dev->lk);
		return E_DISK_SUCC;
	} else {
		spinlock_release(&dev->lk);
		return E_DISK_XFER;
	}
}

static int
disk_intr_handler(uint8_t trapno, struct context *ctx, int guest)
{
	KERN_ASSERT(disk_mgmt_inited == TRUE);

	uint8_t irq = trapno;
	struct disk_dev *dev;

	intr_eoi();

	if (guest)
		vmm_cur_vm()->exit_handled = TRUE;

	spinlock_acquire(&disk_mgmt_lock);

	TAILQ_FOREACH(dev, &all_disk_devices, entry) {
		if (dev->irq == irq) {
			if (dev->intr_handler)
				dev->intr_handler(dev);

			if (dev->status == XFER_FAIL) {
				DISK_DEBUG("Retry request (%s, lba 0x%llx, "
					   "nsect %d, buf 0x%08x) ... \n",
					   dev->last_req.write == TRUE ? "write" : "read",
					   dev->last_req.lba,
					   dev->last_req.nsect,
					   dev->last_req.buf_pa);
				dev->last_req.retry++;
				if (disk_xfer_helper(dev, dev->last_req.lba,
						     dev->last_req.buf_pa,
						     dev->last_req.nsect,
						     dev->last_req.write)) {
					dev->status = XFER_FAIL;
					DISK_DEBUG("Wake process(s) waiting fo "
						   "device 0x%08x.\n", dev);
					proc_wake(dev);
				}
			} else if (dev->status != XFER_PROCESSING) {
				DISK_DEBUG("Wake process(s) waiting fo "
					   "device 0x%08x.\n", dev);
				proc_wake(dev);
			}
		}
	}

	spinlock_release(&disk_mgmt_lock);
	return 0;
}

void
disk_register_intr(void)
{
	KERN_ASSERT(disk_mgmt_inited == TRUE);
	struct disk_dev *dev = NULL;
	spinlock_acquire(&disk_mgmt_lock);
	TAILQ_FOREACH(dev, &all_disk_devices, entry) {
		trap_handler_register(dev->irq, disk_intr_handler);
		DISK_DEBUG("Register handler 0x%08x for IRQ 0x%x.\n",
			   disk_intr_handler, dev->irq);
	}
	spinlock_release(&disk_mgmt_lock);
}

void
disk_intr_enable(void)
{
	KERN_ASSERT(disk_mgmt_inited == TRUE);
	struct disk_dev *dev = NULL;
	spinlock_acquire(&disk_mgmt_lock);
	TAILQ_FOREACH(dev, &all_disk_devices, entry) {
		intr_enable(dev->irq, 0);
		DISK_DEBUG("IRQ 0x%x is enabled.\n", dev->irq);
	}
	spinlock_release(&disk_mgmt_lock);
}

size_t
disk_capacity(struct disk_dev *dev)
{
	KERN_ASSERT(disk_mgmt_inited == TRUE);
	KERN_ASSERT(dev != NULL);
	return dev->capacity;
}

struct disk_dev *
disk_get_dev(int nr)
{
	int i = 0;
	struct disk_dev *dev = NULL;

	if (nr < 0)
		return NULL;

	TAILQ_FOREACH(dev, &all_disk_devices, entry) {
		if (i == nr)
			return dev;
		i++;
	}

	return NULL;
}
