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
    return uctx_pool[get_curid()].regs.eax;
}

unsigned int syscall_get_arg2(void)
{
    return uctx_pool[get_curid()].regs.ebx;
}

unsigned int syscall_get_arg3(void)
{
    return uctx_pool[get_curid()].regs.ecx;
}

unsigned int syscall_get_arg4(void)
{
    return uctx_pool[get_curid()].regs.edx;
}

unsigned int syscall_get_arg5(void)
{
    return uctx_pool[get_curid()].regs.esi;
}

unsigned int syscall_get_arg6(void)
{
    return uctx_pool[get_curid()].regs.edi;
}

/**
 * Sets the error number in uctx_pool that gets passed
 * to the current running process when we return to it.
 */
void syscall_set_errno(unsigned int errno)
{
    uctx_pool[get_curid()].regs.eax = errno;
}

/**
 * Sets the return values in uctx_pool that get passed
 * to the current running process when we return to it.
 */
void syscall_set_retval1(unsigned int retval)
{
    uctx_pool[get_curid()].regs.ebx = retval;
}

void syscall_set_retval2(unsigned int retval)
{
    uctx_pool[get_curid()].regs.ecx = retval;
}

void syscall_set_retval3(unsigned int retval)
{
    uctx_pool[get_curid()].regs.edx = retval;
}

void syscall_set_retval4(unsigned int retval)
{
    uctx_pool[get_curid()].regs.esi = retval;
}

void syscall_set_retval5(unsigned int retval)
{
    uctx_pool[get_curid()].regs.edi = retval;
}
