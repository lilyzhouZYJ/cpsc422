#include <lib/debug.h>
#include <pmm/MContainer/export.h>
#include <vmm/MPTOp/export.h>
#include <vmm/MPTNew/export.h>
#include "export.h"

int MPTNew_test1()
{
    unsigned int vaddr = 4096 * 1024 * 400;
    container_split(0, 100);
    if (get_ptbl_entry_by_va(1, vaddr) != 0) {
        dprintf("test 1.1 failed: (%d != 0)\n", get_ptbl_entry_by_va(1, vaddr));
        return 1;
    }
    if (get_pdir_entry_by_va(1, vaddr) != 0) {
        dprintf("test 1.2 failed: (%d != 0)\n", get_pdir_entry_by_va(1, vaddr));
        return 1;
    }
    alloc_page(1, vaddr, 7);
    if (get_ptbl_entry_by_va(1, vaddr) == 0) {
        dprintf("test 1.3 failed: (%d == 0)\n", get_ptbl_entry_by_va(1, vaddr));
        return 1;
    }
    if (get_pdir_entry_by_va(1, vaddr) == 0) {
        dprintf("test 1.4 failed: (%d == 0)\n", get_pdir_entry_by_va(1, vaddr));
        return 1;
    }
    dprintf("test 1 passed.\n");
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
int MPTNew_test_own()
{
    // TODO (optional)
	for(int proc_id = 1; proc_id < 5; proc_id++){
		alloc_mem_quota(proc_id, 50);
		unsigned int new_proc = container_split(proc_id, 10);
		unsigned int vaddr1 = 0x70000000;
		unsigned int alloced_page = alloc_page(new_proc, vaddr1, 7);
		if(alloced_page == 0){
			dprintf("error at: %u\n", new_proc);
			return 1;
		}
		dprintf("proc: %u, vaddr: %u, alloced_page: %u\n", new_proc, vaddr1, alloced_page);
	}
	
    dprintf("own test passed.\n");
    return 0;
}

int test_MPTNew()
{
    return MPTNew_test1() + MPTNew_test_own();
}
