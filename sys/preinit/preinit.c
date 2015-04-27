#include <lib/trap.h>
#include <lib/x86.h>
#include <lib/sysenter.h>

#include <preinit/dev/console.h>
#include <preinit/dev/disk.h>
#include <preinit/dev/intr.h>
#include <preinit/dev/pci.h>
#include <preinit/dev/tsc.h>
#include <preinit/dev/svm_drv.h>
#include <preinit/dev/vmx_drv.h>

#include <preinit/lib/mboot.h>
#include <preinit/lib/seg.h>
#include <preinit/lib/x86.h>
#include <preinit/lib/types.h>

#include <preinit/lib/debug.h>
#include <preinit/lib/timing.h>



cpu_vendor cpuvendor;

void
set_vendor()
{
    cpuvendor = vendor();
}


void
preinit (uintptr_t mbi_addr)
{
	FENCE();
    seg_init ();

    FENCE();
    enable_sse ();

    FENCE();
    cons_init ();
    KERN_DEBUG("cons initialized.\n");

    FENCE();
    tsc_init ();
    KERN_DEBUG("tsc initialized.\n");

#ifdef PROFILING
    FENCE();
    profiling_init ();
    KERN_DEBUG("profiling initialized.\n");
#endif

    FENCE();
    sysenter_init();
    KERN_DEBUG("sysenter initialized.\n");

    FENCE();
    intr_init ();
    KERN_DEBUG("intr initialized.\n");

    /* 	ide_init(); */
    /* KERN_DEBUG("ide initialized.\n"); */

    FENCE();
    disk_init ();

    FENCE();
    pci_init ();

    FENCE();
    set_vendor ();
    if (cpuvendor == AMD)
    {
        KERN_DEBUG("vendor detected: AMD.\n");
        svm_hw_init ();
        KERN_DEBUG("svm hw initialized.\n");
    }
    else if (cpuvendor == INTEL)
    {
        KERN_DEBUG("vendor detected: INTEL.\n");
        vmx_hw_init ();
    }
    else
    {
        KERN_PANIC("unknown cpu vendor.\n");
    }

    /* enable interrupts */
    FENCE();
    intr_enable (IRQ_TIMER);
    intr_enable (IRQ_KBD);
    intr_enable (IRQ_SERIAL13);

    FENCE();
    pmmap_init (mbi_addr);
}
