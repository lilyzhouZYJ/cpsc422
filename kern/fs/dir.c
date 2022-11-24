#include <kern/lib/types.h>
#include <kern/lib/debug.h>
#include <kern/lib/string.h>
#include "inode.h"
#include "dir.h"

// Directories

int dir_namecmp(const char *s, const char *t)
{
    return strncmp(s, t, DIRSIZ);
}

/**
 * Look for a directory entry in a directory.
 * If found, set *poff to byte offset of entry.
 */
struct inode *dir_lookup(struct inode *dp, char *name, uint32_t * poff)
{
    uint32_t off, inum;
    struct dirent de;

    if (dp->type != T_DIR)
        KERN_PANIC("dir_lookup not DIR");

    //TODO
    uint32_t de_size = sizeof(de);

    for(off = 0; off < dp->size; off += de_size){
        int read_size = inode_read(dp, (char*) &de, off, de_size);
        if(read_size == de_size){
            if(de.inum != 0 && strcmp((const char *) &(de.name), name) == 0){
                // Found target directory entry
                *poff = off;
                inum = de.inum;
                return inode_get(dp->dev, inum);
            }
        } else {
            KERN_PANIC("dir_lookup: error in inode_read size!\n");
        }
    }

    return 0;
}

// Write a new directory entry (name, inum) into the directory dp.
int dir_link(struct inode *dp, char *name, uint32_t inum)
{
    // TODO: Check that name is not present.
    uint32_t poff;
    struct inode * existing_inode = dir_lookup(dp, name, &poff);
    if(existing_inode != 0){
        // Directory entry already exists
        inode_put(existing_inode);
        return -1;
    }

    uint32_t off;
    struct dirent de;
    uint32_t de_size = sizeof(de);

    for(off = 0; off < dp->size; off += de_size){
        int read_size = inode_read(dp, (char*) &de, off, de_size);
        if(read_size == de_size){
            if(de.inum == 0){
                // Copy name and inum
                strncpy(de.name, name, sizeof(name));
                de.inum = inum;

                int write_size = inode_write(dp, (char*) &de, off, de_size);
                if(write_size != de_size){
                    KERN_PANIC("dir_link: %d bytes written by inode_write, expected %u\n", write_size, de_size);
                }

                return 0;
            }
        } else {
            KERN_PANIC("dir_lookup: error in inode_read size!\n");
        }
    }

    // If we reach here, that means we did not find an unallocated subdirectory entry
    KERN_PANIC("dir_link: no unallocated subdirectory entry found\n");
    return -1;
}
