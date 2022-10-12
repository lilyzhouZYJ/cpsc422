#include <lib/elf.h>
#include <lib/debug.h>
#include <lib/gcc.h>
#include <lib/seg.h>
#include <lib/trap.h>
#include <lib/x86.h>

#include "import.h"

extern tf_t uctx_pool[NUM_IDS];
extern char STACK_LOC[NUM_IDS][PAGESIZE];

void proc_start_user(void)
{
    unsigned int cur_pid = get_curid();
    tss_switch(cur_pid);
    set_pdir_base(cur_pid);

    trap_return((void *) &uctx_pool[cur_pid]);
}

unsigned int proc_create(void *elf_addr, unsigned int quota)
{
    unsigned int pid, id;

    id = get_curid();
    pid = thread_spawn((void *) proc_start_user, id, quota);

    if (pid != NUM_IDS) {
        elf_load(elf_addr, pid);

        uctx_pool[pid].es = CPU_GDT_UDATA | 3;
        uctx_pool[pid].ds = CPU_GDT_UDATA | 3;
        uctx_pool[pid].cs = CPU_GDT_UCODE | 3;
        uctx_pool[pid].ss = CPU_GDT_UDATA | 3;
        uctx_pool[pid].esp = VM_USERHI;
        uctx_pool[pid].eflags = FL_IF;
        uctx_pool[pid].eip = elf_entry(elf_addr);
    }

    return pid;
}

// Copies the current process. 
// Allocate half of the quota size remaining to the child.
// Be really careful when you setup the user context of the child 
// (think of the return value of fork in that context).
unsigned int proc_fork()
{
	unsigned int curr_pid = get_curid();
	unsigned int child_quota = (container_get_quota(curr_pid) - container_get_usage(curr_pid)) / 2;
	unsigned int child_pid = thread_spawn((void *) proc_start_user, curr_pid, child_quota);

	if (child_pid != NUM_IDS){
		uctx_pool[child_pid].es = uctx_pool[curr_pid].es;
        uctx_pool[child_pid].ds = uctx_pool[curr_pid].ds;
        uctx_pool[child_pid].cs = uctx_pool[curr_pid].cs;
        uctx_pool[child_pid].ss = uctx_pool[curr_pid].ss;
        uctx_pool[child_pid].esp = uctx_pool[curr_pid].esp;
        uctx_pool[child_pid].eflags = uctx_pool[curr_pid].eflags;
        uctx_pool[child_pid].eip = uctx_pool[curr_pid].eip;

		uctx_pool[child_pid].regs.ebx = 0; // return value for the child
	}

	uctx_pool[curr_pid].regs.ebx = child_pid;

	return child_pid;
}