Compile: make / make all
Run tests: make clean && make TEST=1
Run in qemu: make qemu / make qemu-nox
Debug with gdb: make qemu-gdb / make qemu-nox-gdb
                (in another terminal) gdb

To use your solutions from lab 1: git merge lab1
To use sample lab 1 solutions: copy files in samples/ to appropriate directories

List here the following info:
1. who you have worked with
2. whether you coded this assignment together, and if not, who worked on which part
3. brief description of what you have implemented
4. and anything else you would like us to know

# Description

For this lab, I implemented **Project 7: Advanced Synchronization - File Sharing**.

## **Usage**

```
int flock(int fd, int operation);
```

There are three possible operations:
- `LOCK_SH`: acquire shared lock
- `LOCK_EX`: acquire exclusive lock
- `LOCK_UN`: release lock

Non-blocking operations can also specified using `LOCK_NB`.

Shared locks may be held by multiple processes at the same time. An exclusive lock can only be held by one process at the same time.

Moreover, if a process calls `open` twice on the same file, the two file descriptors are treated independently.  An attempt to lock the file using one of these file descriptors may be denied by a lock that the calling process has already placed via another file descriptor.

The lock that a file descriptor holds will be automatically released when the file descriptor is closed.

For a more detailed description, see the [flock manual page](https://man7.org/linux/man-pages/man2/flock.2.html).

## **Implementation**

### a. Structure of Flock

The structure of flock is defined in `flock.h`. It includes:

- `type`: Indictates the flock type, which is one of `FLOCK_NONE`, `FLOCK_SH`, or `FLOCK_EX`.
- `num_shared_locks`: Indicates the number of shared-lock holders; the shared lock is released when this number becomes 0.
- Two conditional variables for suspending and restarting threads waiting for locks: (1) `cv_shared_flock`, which includes threads waiting to acquire a shared lock, and (2) `cv_exclusive_flock`, which includes threads waiting to acquire an exclusive lock.

### b. Implementation of Flock

The implementation of the flock is contained in `flock.c`. It contains the following functions:

- `flock_init`: Initializes the flock.
- `flock_acquire`: Acquires the flock. If lock of the specified type cannot be acquired, then the thread is suspended using the corresponding conditional variable, if the operation is blocking, or it exits, if the operation is non-blocking.
- `flock_release`: Releases the flock if the file object holds the flock. Also signal to any relevant waiting threads to wake up.
- `flock_operation`: This is the "dispatcher" function, which is responsible for dispatching the flock operation to the relevant function (either `flock_acquire` or `flock_release`).

The flock acquire and release operations are also protected by a spinlock to ensure they are atomic.

### c. How the flock is integrated

- Each inode object (of type `struct inode`) is given a flock (of type `struct flock`).
- Each file object (of type `struct file`) is given a `hold_flock` flag, which tracks if it currently holds the flock. This flag is necessary to determine if a file descriptor actually holds a flock on a file, and if it doesn't, it cannot release the flock.

## **Testing**

The testing scripts are contained in the `fstest` directory, which includes the following files:

- `flocktest.c`: This file contains some single-thread testing functions (kind of like unit tests), and it also spawns multiple threads for multi-threaded testing (using `spawn`).
- `flock1.c`, `flock2.c`, `flock3.c`: These three files are the three threads for multi-threaded testing.

The three threads perform the following sequence of actions:

1. Open the same file using `open`
2. Acquire exclusive lock
3. Acquire (convert to) shared lock
4. Release shared lock
5. Acquire shared lock
6. Release shared lock

All the above flock operations are blocking, meaning that if a thread fails to acquire a lock due to lock conflict, it will be put to sleep and waken up later when it could try to acquire the lock again.

In addition to the print statements in the testing scripts, there are additional debug statements from `flock.c` to illustrate the process of flock acquisition/release.

## **Others**

During testing, I found that the `tqueue_enqueue` function incorrectly sets the pointers when it comes to enqueueing threads that are already on the queue, leading to lost threads. More specifically, the issue arises when `thread_wakeup` iterates through all threads and enqueues the relevant ones to the ready queue, without checking if they are already present (but sleeping) on the ready queue. To fix this, I modified `tqueue_enqueue` to first check if the thread is already present on the queue.