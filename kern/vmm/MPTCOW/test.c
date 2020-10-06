#include <lib/debug.h>
#include <lib/x86.h>
#include "export.h"

#define PT_PERM_PTU (PTE_P | PTE_W | PTE_U)

int MPTCOW_test1()
{
    unsigned int *addr = (unsigned int *) 0x50000000;
    unsigned int write = 0x12345678;
    unsigned int read1, read2, pid1, pid2;

    pid1 = container_split(0, 500);
    pid2 = container_split(0, 500);
    alloc_page(pid1, (unsigned int) addr, PT_PERM_PTU);

    set_pdir_base(pid1);
    *addr = write;
    read1 = *addr;

    set_pdir_base(0);
    map_cow(pid1, pid2);
    set_pdir_base(pid2);
    read2 = *addr;

    if (read1 != read2) {
        dprintf("test 1.1 failed: (%08x != %08x)\n", read1, read2);
        return 1;
    }

    set_pdir_base(0);
    map_decow(pid2, (unsigned int) addr);
    set_pdir_base(pid2);
    *addr = 0x87654321;
    read2 = *addr;

    set_pdir_base(pid1);
    read1 = *addr;

    if (read1 == read2) {
        dprintf("test 1.2 failed: (%08x == %08x)\n", read1, read2);
        return 1;
    }

    return 0;
}

/**
 * Write Your Own Test Script (optional)
 *
 * Come up with your own interesting test cases to challenge your classmates!
 * In addition to the provided simple tests, selected (correct and interesting) test functions
 * will be used in the actual grading of the lab!
 * Your test function itself will not be graded. So don't be afraid of submitting a wrong script.
 *
 * The test function should return 0 for passing the test and a non-zero code for failing the test.
 * Be extra careful to make sure that if you overwrite some of the kernel data, they are set back to
 * the original value. O.w., it may make the future test scripts to fail even if you implement all
 * the functions correctly.
 */
int MPTCOW_test_own()
{
    // TODO (optional)
    // dprintf("own test passed.\n");
    return 0;
}

int test_MPTCOW()
{
    return MPTCOW_test1() + MPTCOW_test_own();
}
