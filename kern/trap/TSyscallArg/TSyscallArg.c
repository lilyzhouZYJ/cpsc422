#include <lib/trap.h>
#include <lib/x86.h>

#include "import.h"

extern tf_t uctx_pool[NUM_IDS];

/**
 * Retrieves the system call arguments from uctx_pool that get
 * passed in from the current running process' system call.
 */
unsigned int syscall_get_arg1(void)
{
    // TODO
	int curr_pid = get_curid();
    return uctx_pool[curr_pid].regs.eax; // system call number in %eax
}

unsigned int syscall_get_arg2(void)
{
    // TODO
    int curr_pid = get_curid();
    return uctx_pool[curr_pid].regs.ebx; // arg1: ebx
}

unsigned int syscall_get_arg3(void)
{
    // TODO
    int curr_pid = get_curid();
    return uctx_pool[curr_pid].regs.ecx; // arg2: ecx
}

unsigned int syscall_get_arg4(void)
{
    // TODO
    int curr_pid = get_curid();
    return uctx_pool[curr_pid].regs.edx; // arg3: edx
}

unsigned int syscall_get_arg5(void)
{
    // TODO
    int curr_pid = get_curid();
    return uctx_pool[curr_pid].regs.esi; // arg4: esi
}

unsigned int syscall_get_arg6(void)
{
    // TODO
    int curr_pid = get_curid();
    return uctx_pool[curr_pid].regs.edi; // arg5: edi
}

/**
 * Sets the error number in uctx_pool that gets passed
 * to the current running process when we return to it.
 */
void syscall_set_errno(unsigned int errno)
{
    // TODO
	int curr_pid = get_curid();
	uctx_pool[curr_pid].err = errno;
}

/**
 * Sets the return values in uctx_pool that get passed
 * to the current running process when we return to it.
 */
void syscall_set_retval1(unsigned int retval)
{
    // TODO
	int curr_pid = get_curid();
    uctx_pool[curr_pid].regs.ebx = retval; // arg1: ebx
}

void syscall_set_retval2(unsigned int retval)
{
    // TODO
	int curr_pid = get_curid();
    uctx_pool[curr_pid].regs.ecx = retval; // arg2: ecx
}

void syscall_set_retval3(unsigned int retval)
{
    // TODO
	int curr_pid = get_curid();
    uctx_pool[curr_pid].regs.edx = retval; // arg3: edx
}

void syscall_set_retval4(unsigned int retval)
{
    // TODO
	int curr_pid = get_curid();
    uctx_pool[curr_pid].regs.esi = retval; // arg4: esi
}

void syscall_set_retval5(unsigned int retval)
{
    // TODO
	int curr_pid = get_curid();
    uctx_pool[curr_pid].regs.edi = retval; // arg5: edi
}
