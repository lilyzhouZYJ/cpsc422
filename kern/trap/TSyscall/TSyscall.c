#include <lib/debug.h>
#include <lib/pmap.h>
#include <lib/types.h>
#include <lib/x86.h>
#include <lib/trap.h>
#include <lib/syscall.h>
#include <dev/intr.h>
#include <pcpu/PCPUIntro/export.h>
#include <dev/console.h>
#include <lib/string.h>

#include "import.h"

static char sys_buf[NUM_IDS][PAGESIZE];

/* System call for reading a line of input from console. */
void sys_readline(tf_t *tf)
{
    // Get syscall arguments
    uintptr_t user_prompt = syscall_get_arg2(tf);
    size_t prompt_len = syscall_get_arg3(tf);
    uintptr_t user_buffer = syscall_get_arg4(tf);

    // Check user_buffer pointer
    if (!(VM_USERLO <= user_prompt && user_prompt + prompt_len <= VM_USERHI)) {
        // Outside user address space
        syscall_set_errno(tf, E_INVAL_ADDR);
        syscall_set_retval1(tf, -1);
        return;
    }
    if (!(VM_USERLO <= user_buffer && user_buffer + BUFLEN <= VM_USERHI)) {
        // Outside user address space
        syscall_set_errno(tf, E_INVAL_ADDR);
        syscall_set_retval1(tf, -1);
        return;
    }

    // Read in user prompt
    char sys_prompt[prompt_len];
    if (pt_copyin(get_curid(), user_prompt, sys_prompt, prompt_len) != prompt_len) {
        syscall_set_errno(tf, E_MEM);
        syscall_set_retval1(tf, -1);
        return;
    }
    sys_prompt[prompt_len] = '\0';

    // KERN_DEBUG("in sys_readline with prompt %s\n", sys_prompt);

    char * line = readline((const char *) sys_prompt);
    int line_len = strnlen(line, BUFLEN);

    // KERN_DEBUG("in sys_readline with line %s and line_len %d\n", line, line_len);

    // Copy to user buffer
    size_t n_copied = pt_copyout(line, get_curid(), user_buffer, line_len+1); // copy null terminator as well
    if(n_copied != line_len){
        syscall_set_errno(tf, E_MEM);
        return;
    }

    // KERN_DEBUG("in sys_readline with user buffer %s and length %d\n", (char*)user_buffer, strnlen((char*)user_buffer, line_len));

    // Set up return value
    syscall_set_errno(tf, E_SUCC);
}

/**
 * Copies a string from user into buffer and prints it to the screen.
 * This is called by the user level "printf" library as a system call.
 */
void sys_puts(tf_t *tf)
{
    unsigned int cur_pid;
    unsigned int str_uva, str_len;
    unsigned int remain, cur_pos, nbytes;

    cur_pid = get_curid();
    str_uva = syscall_get_arg2(tf);
    str_len = syscall_get_arg3(tf);

    if (!(VM_USERLO <= str_uva && str_uva + str_len <= VM_USERHI)) {
        syscall_set_errno(tf, E_INVAL_ADDR);
        return;
    }

    remain = str_len;
    cur_pos = str_uva;

    while (remain) {
        if (remain < PAGESIZE - 1)
            nbytes = remain;
        else
            nbytes = PAGESIZE - 1;

        if (pt_copyin(cur_pid, cur_pos, sys_buf[cur_pid], nbytes) != nbytes) {
            syscall_set_errno(tf, E_MEM);
            return;
        }

        sys_buf[cur_pid][nbytes] = '\0';
        KERN_INFO("%s", sys_buf[cur_pid]);

        remain -= nbytes;
        cur_pos += nbytes;
    }

    syscall_set_errno(tf, E_SUCC);
}

extern uint8_t _binary___obj_user_pingpong_ping_start[];
extern uint8_t _binary___obj_user_pingpong_pong_start[];
extern uint8_t _binary___obj_user_pingpong_shell_start[];
extern uint8_t _binary___obj_user_fstest_fstest_start[];

/**
 * Spawns a new child process.
 * The user level library function sys_spawn (defined in user/include/syscall.h)
 * takes two arguments [elf_id] and [quota], and returns the new child process id
 * or NUM_IDS (as failure), with appropriate error number.
 * Currently, we have three user processes defined in user/pingpong/ directory,
 * ping, pong, and ding (changed to shell).
 * The linker ELF addresses for those compiled binaries are defined above.
 * Since we do not yet have a file system implemented in mCertiKOS,
 * we statically load the ELF binaries into the memory based on the
 * first parameter [elf_id].
 * For example, ping, pong, and ding correspond to the elf_ids
 * 1, 2, 3, and 4, respectively.
 * If the parameter [elf_id] is none of these, then it should return
 * NUM_IDS with the error number E_INVAL_PID. The same error case apply
 * when the proc_create fails.
 * Otherwise, you should mark it as successful, and return the new child process id.
 */
void sys_spawn(tf_t *tf)
{
    unsigned int new_pid;
    unsigned int elf_id, quota;
    void *elf_addr;
    unsigned int curid = get_curid();

    elf_id = syscall_get_arg2(tf);
    quota = syscall_get_arg3(tf);

    if (!container_can_consume(curid, quota)) {
        syscall_set_errno(tf, E_EXCEEDS_QUOTA);
        syscall_set_retval1(tf, NUM_IDS);
        return;
    }
    else if (NUM_IDS < curid * MAX_CHILDREN + 1 + MAX_CHILDREN) {
        syscall_set_errno(tf, E_MAX_NUM_CHILDEN_REACHED);
        syscall_set_retval1(tf, NUM_IDS);
        return;
    }
    else if (container_get_nchildren(curid) == MAX_CHILDREN) {
        syscall_set_errno(tf, E_INVAL_CHILD_ID);
        syscall_set_retval1(tf, NUM_IDS);
        return;
    }

    switch (elf_id) {
    case 1:
        elf_addr = _binary___obj_user_pingpong_ping_start;
        break;
    case 2:
        elf_addr = _binary___obj_user_pingpong_pong_start;
        break;
    case 3:
        elf_addr = _binary___obj_user_pingpong_shell_start;
        break;
    case 4:
        elf_addr = _binary___obj_user_fstest_fstest_start;
        break;
    default:
        syscall_set_errno(tf, E_INVAL_PID);
        syscall_set_retval1(tf, NUM_IDS);
        return;
    }

    new_pid = proc_create(elf_addr, quota);

    if (new_pid == NUM_IDS) {
        syscall_set_errno(tf, E_INVAL_PID);
        syscall_set_retval1(tf, NUM_IDS);
    } else {
        syscall_set_errno(tf, E_SUCC);
        syscall_set_retval1(tf, new_pid);
    }
}

/**
 * Yields to another thread/process.
 * The user level library function sys_yield (defined in user/include/syscall.h)
 * does not take any argument and does not have any return values.
 * Do not forget to set the error number as E_SUCC.
 */
void sys_yield(tf_t *tf)
{
    thread_yield();
    syscall_set_errno(tf, E_SUCC);
}
