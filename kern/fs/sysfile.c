// File-system system calls.

#include <kern/lib/types.h>
#include <kern/lib/debug.h>
#include <kern/lib/pmap.h>
#include <kern/lib/string.h>
#include <kern/lib/trap.h>
#include <kern/lib/syscall.h>
#include <kern/thread/PTCBIntro/export.h>
#include <kern/thread/PCurID/export.h>
#include <kern/trap/TSyscallArg/export.h>
#include <lib/spinlock.h>

#include "dir.h"
#include "path.h"
#include "file.h"
#include "fcntl.h"
#include "log.h"

// Global buffers
#define BUFFER_SIZE 10000
char kernel_buffer[BUFFER_SIZE];
struct file_stat kernel_st;
spinlock_t kernel_buffer_lock;

/**
 * This function is not a system call handler, but an auxiliary function
 * used by sys_open.
 * Allocate a file descriptor for the given file.
 * You should scan the list of open files for the current thread
 * and find the first file descriptor that is available.
 * Return the found descriptor or -1 if none of them is free.
 */
static int fdalloc(struct file *f)
{
    // TODO
    // Get the list of open files for the current thread
    unsigned int curid = get_curid();
    struct file ** openfiles = tcb_get_openfiles(curid);

    // Scan the list for available file descriptor
    for(int i = 0; i < NOFILE; i++){
        struct file * f = openfiles[i];
        if(f == NULL || f->type == FD_NONE){
            // Found available file descriptor
            return i;
        }
    }

    // No free file descriptor; return -1
    return -1;
}

/**
 * From the file indexed by the given file descriptor, read n bytes and save them
 * into the buffer in the user. As explained in the assignment specification,
 * you should first write to a kernel buffer then copy the data into user buffer
 * with pt_copyout.
 * Return Value: Upon successful completion, read() shall return a non-negative
 * integer indicating the number of bytes actually read. Otherwise, the
 * functions shall return -1 and set errno E_BADF to indicate the error.
 */
void sys_read(tf_t *tf)
{
    // TODO
    // Read input from syscall
    int fd = syscall_get_arg2(tf); // ebx
    uintptr_t user_buffer = syscall_get_arg3(tf); // ecx
    size_t n = syscall_get_arg4(tf); // edx

    // Check size
    if(n > BUFFER_SIZE){
        // Exceeds max buffer length
        syscall_set_errno(tf, E_INVAL_ID);
        return;
    }

    // Check valid fd
    if(fd >= NOFILE){
        // Invalid fd
        syscall_set_errno(tf, E_BADF);
        return;
    }

    // Check user_buffer pointer
    if (!(VM_USERLO <= user_buffer && user_buffer + n <= VM_USERHI)) {
        // Outside user address space
        syscall_set_errno(tf, E_INVAL_ADDR);
        return;
    }

    // Read file fd into global buffer
    unsigned int curid = get_curid();
    struct file * f = tcb_get_openfiles(curid)[fd];
    spinlock_acquire(&kernel_buffer_lock);
    if(file_read(f, kernel_buffer, n) == -1){
        // Error in file_read
        syscall_set_errno(tf, E_BADF);
        spinlock_release(&kernel_buffer_lock);
        return;
    }

    // Copy using pt_copyout
    size_t n_read = pt_copyout(kernel_buffer, get_curid(), user_buffer, n);
    if(n_read < 0){
        // Error in copying
        syscall_set_errno(tf, E_BADF);
        spinlock_release(&kernel_buffer_lock);
        return;
    }
    spinlock_release(&kernel_buffer_lock);
}

/**
 * Write n bytes of data in the user's buffer into the file indexed by the file descriptor.
 * You should first copy the data info an in-kernel buffer with pt_copyin and then
 * pass this buffer to appropriate file manipulation function.
 * Upon successful completion, write() shall return the number of bytes actually
 * written to the file associated with f. This number shall never be greater
 * than nbyte. Otherwise, -1 shall be returned and errno E_BADF set to indicate the
 * error.
 */
void sys_write(tf_t *tf)
{
    // TODO
    // Read input from syscall
    int fd = syscall_get_arg2(tf); // ebx
    uintptr_t user_buffer = syscall_get_arg3(tf); // ecx
    size_t n = syscall_get_arg4(tf); // edx

    // Check size
    if(n > BUFFER_SIZE){
        // Exceeds max buffer length
        syscall_set_errno(tf, E_INVAL_ID);
        return;
    }

    // Check valid fd
    if(fd >= NOFILE){
        // Invalid fd
        syscall_set_errno(tf, E_BADF);
        return;
    }

    // Check user_buffer pointer
    if (!(VM_USERLO <= user_buffer && user_buffer + n <= VM_USERHI)) {
        // Outside user address space
        syscall_set_errno(tf, E_INVAL_ADDR);
        return;
    }

    // Copy from user_buffer to kernel_buffer using pt_copyin
    spinlock_acquire(&kernel_buffer_lock);
    int n_write = pt_copyin(get_curid(), user_buffer, kernel_buffer, n);
    if(n_write < 0){
        // Error in copying
        syscall_set_errno(tf, E_BADF);
        spinlock_release(&kernel_buffer_lock);
        return;
    }

    // Write to file from kernel_buffer
    unsigned int curid = get_curid();
    struct file * f = tcb_get_openfiles(curid)[fd];
    n_write = file_write(f, kernel_buffer, n);
    if(n_write == -1){
        // Error in file_write
        syscall_set_errno(tf, E_BADF);
        spinlock_release(&kernel_buffer_lock);
        return;
    }
    spinlock_release(&kernel_buffer_lock);
}

/**
 * Return Value: Upon successful completion, 0 shall be returned; otherwise, -1
 * shall be returned and errno E_BADF set to indicate the error.
 */
void sys_close(tf_t *tf)
{
    // TODO
    // Read input from syscall
    int fd = syscall_get_arg2(tf); // ebx

    // Check valid fd
    if(fd >= NOFILE){
        // Invalid fd
        syscall_set_errno(tf, E_BADF);
        return;
    }

    // Find file and close it
    unsigned int curid = get_curid();
    struct file * f = tcb_get_openfiles(curid)[fd];
    file_close(f);
}

/**
 * Return Value: Upon successful completion, 0 shall be returned. Otherwise, -1
 * shall be returned and errno E_BADF set to indicate the error.
 */
void sys_fstat(tf_t *tf)
{
    // TODO
    // Read input from syscall
    int fd = syscall_get_arg2(tf); // ebx
    uintptr_t user_st = syscall_get_arg3(tf); // ecx

    // Check valid fd
    if(fd >= NOFILE){
        // Invalid fd
        syscall_set_errno(tf, E_BADF);
        return;
    }

    // Check st pointer
    if (!(VM_USERLO <= user_st && user_st + sizeof(user_st) <= VM_USERHI)) {
        // Outside user address space
        syscall_set_errno(tf, E_INVAL_ADDR);
        return;
    }

    // Find file and copy its metadata to kernel_st
    unsigned int curid = get_curid();
    struct file * f = tcb_get_openfiles(curid)[fd];
    spinlock_acquire(&kernel_buffer_lock);
    if(file_stat(f, &kernel_st) == -1){
        // Error in file_stat
        syscall_set_errno(tf, E_BADF);
        spinlock_release(&kernel_buffer_lock);
        return;
    }

    // Copy kernel_st to user_st
    int n_read = pt_copyout(&kernel_st, get_curid(), user_st, sizeof(kernel_st));
    if(n_read < 0){
        // Error in copying
        syscall_set_errno(tf, E_BADF);
        spinlock_release(&kernel_buffer_lock);
        return;
    }
    spinlock_release(&kernel_buffer_lock);
}

/**
 * Create the path new as a link to the same inode as old.
 */
void sys_link(tf_t * tf)
{
    char name[DIRSIZ], new[128], old[128];
    struct inode *dp, *ip;

    unsigned int old_len = syscall_get_arg4(tf); // edx
    unsigned int new_len = syscall_get_arg5(tf); // esi
    pt_copyin(get_curid(), syscall_get_arg2(tf), old, old_len + 1);
    pt_copyin(get_curid(), syscall_get_arg3(tf), new, new_len + 1);
    old[old_len] = '\0';
    new[new_len] = '\0';

    if ((ip = namei(old)) == 0) {
        syscall_set_errno(tf, E_NEXIST);
        return;
    }

    begin_trans();

    inode_lock(ip);
    if (ip->type == T_DIR) {
        inode_unlockput(ip);
        commit_trans();
        syscall_set_errno(tf, E_DISK_OP);
        return;
    }

    ip->nlink++;
    inode_update(ip);
    inode_unlock(ip);

    if ((dp = nameiparent(new, name)) == 0)
        goto bad;
    inode_lock(dp);
    if (dp->dev != ip->dev || dir_link(dp, name, ip->inum) < 0) {
        inode_unlockput(dp);
        goto bad;
    }
    inode_unlockput(dp);
    inode_put(ip);

    commit_trans();

    syscall_set_errno(tf, E_SUCC);
    return;

bad:
    inode_lock(ip);
    ip->nlink--;
    inode_update(ip);
    inode_unlockput(ip);
    commit_trans();
    syscall_set_errno(tf, E_DISK_OP);
    return;
}

/**
 * Is the directory dp empty except for "." and ".." ?
 */
static int isdirempty(struct inode *dp)
{
    int off;
    struct dirent de;

    for (off = 2 * sizeof(de); off < dp->size; off += sizeof(de)) {
        if (inode_read(dp, (char *) &de, off, sizeof(de)) != sizeof(de))
            KERN_PANIC("isdirempty: readi");
        if (de.inum != 0)
            return 0;
    }
    return 1;
}

void sys_unlink(tf_t *tf)
{
    struct inode *ip, *dp;
    struct dirent de;
    char name[DIRSIZ], path[128];
    uint32_t off;

    unsigned int path_len = syscall_get_arg3(tf);
    pt_copyin(get_curid(), syscall_get_arg2(tf), path, path_len + 1);
    path[path_len] = '\0';

    if ((dp = nameiparent(path, name)) == 0) {
        syscall_set_errno(tf, E_DISK_OP);
        return;
    }

    begin_trans();

    inode_lock(dp);

    // Cannot unlink "." or "..".
    if (dir_namecmp(name, ".") == 0 || dir_namecmp(name, "..") == 0)
        goto bad;

    if ((ip = dir_lookup(dp, name, &off)) == 0)
        goto bad;
    inode_lock(ip);

    if (ip->nlink < 1)
        KERN_PANIC("unlink: nlink < 1");
    if (ip->type == T_DIR && !isdirempty(ip)) {
        inode_unlockput(ip);
        goto bad;
    }

    memset(&de, 0, sizeof(de));
    if (inode_write(dp, (char *) &de, off, sizeof(de)) != sizeof(de))
        KERN_PANIC("unlink: writei");
    if (ip->type == T_DIR) {
        dp->nlink--;
        inode_update(dp);
    }
    inode_unlockput(dp);

    ip->nlink--;
    inode_update(ip);
    inode_unlockput(ip);

    commit_trans();

    syscall_set_errno(tf, E_SUCC);
    return;

bad:
    inode_unlockput(dp);
    commit_trans();
    syscall_set_errno(tf, E_DISK_OP);
    return;
}

static struct inode *create(char *path, short type, short major, short minor)
{
    uint32_t off;
    struct inode *ip, *dp;
    char name[DIRSIZ];

    if ((dp = nameiparent(path, name)) == 0)
        return 0;
    inode_lock(dp);

    if ((ip = dir_lookup(dp, name, &off)) != 0) {
        inode_unlockput(dp);
        inode_lock(ip);
        if (type == T_FILE && ip->type == T_FILE)
            return ip;
        inode_unlockput(ip);
        return 0;
    }

    if ((ip = inode_alloc(dp->dev, type)) == 0)
        KERN_PANIC("create: ialloc");

    inode_lock(ip);
    ip->major = major;
    ip->minor = minor;
    ip->nlink = 1;
    inode_update(ip);

    if (type == T_DIR) {  // Create . and .. entries.
        dp->nlink++;      // for ".."
        inode_update(dp);
        // No ip->nlink++ for ".": avoid cyclic ref count.
        if (dir_link(ip, ".", ip->inum) < 0
            || dir_link(ip, "..", dp->inum) < 0)
            KERN_PANIC("create dots");
    }

    if (dir_link(dp, name, ip->inum) < 0)
        KERN_PANIC("create: dir_link");

    inode_unlockput(dp);
    return ip;
}

void sys_open(tf_t *tf)
{
    char path[128];
    int fd, omode;
    struct file *f;
    struct inode *ip;

    unsigned int path_len = syscall_get_arg4(tf);
    pt_copyin(get_curid(), syscall_get_arg2(tf), path, path_len + 1);
    path[path_len] = '\0';

    omode = syscall_get_arg3(tf);

    if (omode & O_CREATE) {
        begin_trans();
        ip = create(path, T_FILE, 0, 0);
        commit_trans();
        if (ip == 0) {
            syscall_set_retval1(tf, -1);
            syscall_set_errno(tf, E_CREATE);
            return;
        }
    } else {
        if ((ip = namei(path)) == 0) {
            syscall_set_retval1(tf, -1);
            syscall_set_errno(tf, E_NEXIST);
            return;
        }
        inode_lock(ip);
        if (ip->type == T_DIR && omode != O_RDONLY) {
            inode_unlockput(ip);
            syscall_set_retval1(tf, -1);
            syscall_set_errno(tf, E_DISK_OP);
            return;
        }
    }

    if ((f = file_alloc()) == 0 || (fd = fdalloc(f)) < 0) {
        if (f)
            file_close(f);
        inode_unlockput(ip);
        syscall_set_retval1(tf, -1);
        syscall_set_errno(tf, E_DISK_OP);
        return;
    }
    inode_unlock(ip);

    f->type = FD_INODE;
    f->ip = ip;
    f->off = 0;
    f->readable = !(omode & O_WRONLY);
    f->writable = (omode & O_WRONLY) || (omode & O_RDWR);
    syscall_set_retval1(tf, fd);
    syscall_set_errno(tf, E_SUCC);
}

void sys_mkdir(tf_t *tf)
{
    char path[128];
    struct inode *ip;

    unsigned int path_len = syscall_get_arg3(tf);
    pt_copyin(get_curid(), syscall_get_arg2(tf), path, path_len + 1);
    path[path_len] = '\0';

    begin_trans();
    if ((ip = (struct inode *) create(path, T_DIR, 0, 0)) == 0) {
        commit_trans();
        syscall_set_errno(tf, E_DISK_OP);
        return;
    }
    inode_unlockput(ip);
    commit_trans();
    syscall_set_errno(tf, E_SUCC);
}

void sys_chdir(tf_t *tf)
{
    char path[128];
    struct inode *ip;
    int pid = get_curid();

    unsigned int path_len = syscall_get_arg3(tf);
    pt_copyin(get_curid(), syscall_get_arg2(tf), path, path_len + 1);
    path[path_len] = '\0';

    if ((ip = namei(path)) == 0) {
        syscall_set_errno(tf, E_DISK_OP);
        return;
    }
    inode_lock(ip);
    if (ip->type != T_DIR) {
        inode_unlockput(ip);
        syscall_set_errno(tf, E_DISK_OP);
        return;
    }
    inode_unlock(ip);
    inode_put(tcb_get_cwd(pid));
    tcb_set_cwd(pid, ip);
    syscall_set_errno(tf, E_SUCC);
}
