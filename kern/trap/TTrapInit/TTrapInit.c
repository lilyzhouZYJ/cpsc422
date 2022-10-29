#include <lib/trap.h>
#include <lib/debug.h>
#include <lib/string.h>
#include <dev/intr.h>
#include <trap/BBQueue/export.h>
#include "import.h"

#define KERN_INFO_CPU(str, idx) \
    if (idx == 0) KERN_INFO("[BSP KERN] " str); \
    else KERN_INFO("[AP%d KERN] " str, idx);

int inited = FALSE;

trap_cb_t TRAP_HANDLER[NUM_CPUS][256];

void trap_init_array(void)
{
    KERN_ASSERT(inited == FALSE);
    memzero(&TRAP_HANDLER, sizeof(trap_cb_t) * 8 * 256);
    inited = TRUE;
}

void trap_handler_register(int cpu_idx, int trapno, trap_cb_t cb)
{
    KERN_ASSERT(0 <= cpu_idx && cpu_idx < 8);
    KERN_ASSERT(0 <= trapno && trapno < 256);
    KERN_ASSERT(cb != NULL);

    TRAP_HANDLER[cpu_idx][trapno] = cb;
}

void trap_init(unsigned int cpu_idx)
{
    if (cpu_idx == 0) {
        trap_init_array();
    }

    // Initialize BBQueue for sys_consume and sys_produce
    BBQ_init();

    KERN_INFO_CPU("Register trap handlers...\n", cpu_idx);

    // TODO: for CPU # [cpu_idx], register appropriate trap handler for each trap number,
    // with trap_handler_register function defined above.

	// Exceptions: 0-31
	for(unsigned int exception_no = 0; exception_no <= 31; exception_no++){
		trap_handler_register(cpu_idx, exception_no, exception_handler);
	}
	// Interrupts: 32-47
	for(unsigned int int_no = 32; int_no <= 47; int_no++){
		trap_handler_register(cpu_idx, int_no, interrupt_handler);
	}
	// System call
	trap_handler_register(cpu_idx, T_SYSCALL, syscall_dispatch);

	// TODO: what about other trap numbers

    KERN_INFO_CPU("Done.\n", cpu_idx);
    KERN_INFO_CPU("Enabling interrupts...\n", cpu_idx);

    /* enable interrupts */
    intr_enable(IRQ_TIMER, cpu_idx);
    intr_enable(IRQ_KBD, cpu_idx);
    intr_enable(IRQ_SERIAL13, cpu_idx);

    KERN_INFO_CPU("Done.\n", cpu_idx);
}
