#include <lib/debug.h>
#include "import.h"

#define PAGESIZE     4096
#define VM_USERLO    0x40000000
#define VM_USERHI    0xF0000000
#define VM_USERLO_PI (VM_USERLO / PAGESIZE)
#define VM_USERHI_PI (VM_USERHI / PAGESIZE)

/**
 * Allocate a physical page.
 *
 * 1. First, implement a naive page allocator that scans the allocation table (AT)
 *    using the functions defined in import.h to find the first unallocated page
 *    with normal permissions.
 *    (Q: Do you have to scan the allocation table from index 0? Recall how you have
 *    initialized the table in pmem_init.)
 *    Then mark the page as allocated in the allocation table and return the page
 *    index of the page found. In the case when there is no available page found,
 *    return 0.
 * 2. Optimize the code using memoization so that you do not have to
 *    scan the allocation table from scratch every time.
 */

// Use global variable to store last-allocated page index
unsigned int last_allocated_idx = VM_USERLO_PI;

unsigned int palloc()
{
    // TODO
    // Scan AT for non-kernel-reserved pages
    for(unsigned int idx = last_allocated_idx; idx <= VM_USERHI_PI - 1; idx++){
        if(at_is_norm(idx) == 1 && at_is_allocated(idx) == 0){
            // Found page with normal permissions + is not allocated
            at_set_allocated(idx, 1);
            last_allocated_idx = idx;
            return idx;
        }
    }
    // No such page found
    return 0;
}

/**
 * Free a physical page.
 *
 * This function marks the page with given index as unallocated
 * in the allocation table.
 *
 * Hint: Simple.
 */
void pfree(unsigned int pfree_index)
{
    // TODO
    if(at_is_norm(pfree_index) && pfree_index >= VM_USERLO_PI && pfree_index < VM_USERHI_PI){
        at_set_allocated(pfree_index, 0);
        if(pfree_index < last_allocated_idx){
            last_allocated_idx = pfree_index;
        }
    }
}
