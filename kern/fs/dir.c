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
// In dir_lookup, you are given an inode pointer (dp) for the current directory, 
// the name of the subdirectory to look up, and a pointer (poff) to write found 
// byte offset to. You should loop through all the subdirectory entries of dp by 
// calling inode_read, and find the one that is allocated (the inum is not 0) and 
// has the matching name. Then you write the offset of the found inode to the address 
// poff and return the pointer to the inode that the subdirectory entry corresponds 
// to by calling inode_get with the inode number in the found subdirectory entry. If 
// not found, it should return 0 (null pointer). Note that the size attribute in the 
// inode structure represents the total sizes, in bytes, of the subdirectory entries, 
// not the number of subdirectory entries. Also, the inode_read takes an offset in bytes, 
// not the index of the subdirectory entry.
struct inode *dir_lookup(struct inode *dp, char *name, uint32_t * poff)
{
    // KERN_DEBUG("START DIR_LOOKUP\n");
    // KERN_DEBUG("dir_lookup: looking up name %s\n", name);

    uint32_t off, inum;
    struct dirent de;

    if (dp->type != T_DIR)
        KERN_PANIC("dir_lookup not DIR");

    //TODO
    uint32_t de_size = sizeof(de);

    // Loop through all the subdirectory entries of dp by calling inode_read
    for(off = 0; off < dp->size; off += de_size){
        int read_size = inode_read(dp, (char*) &de, off, de_size);
        if(read_size != de_size){
            KERN_PANIC("dir_lookup: error in inode_read size!\n");
        }
        
        // KERN_DEBUG("dir_lookup: offset %d, de.inum %d, de.name %s\n", off, de.inum, de.name);

        // Find the one that is allocated (the inum is not 0) and has the matching name.
        if(de.inum != 0 && strcmp((const char *) de.name, name) == 0){
            // Write the offset of the found inode to the address poff and return the 
            // pointer to the inode that the subdirectory entry corresponds to by calling 
            // inode_get with the inode number in the found subdirectory entry. 
            *poff = off;
            inum = de.inum;
            return inode_get(dp->dev, inum);
        }
    }

    // Not found: return 0
    return 0;
}

/* Write a new directory entry (name, inum) into the directory dp. */
// In dir_link, you are given an inode pointer (dp) for the current directory, and 
// the name and inode number for the new subdirectory you are going to link. The first thing
// you should do is to check whether that subdirectory already exists in the current directory
// (with dir_lookup above), and if so, call inode_put on the inode returned by the dir_lookup
// and return -1. Recall that dir_lookup calls inode_get in the end, which creates a reference
// to the in-memory inode structure and increases the reference counter of that inode. Thus,
// in the case of error, we should call inode_put to drop that unnecessary reference we have
// created. After the error check, you can loop through the subdirectory structures of dp to
// find the first unallocated inode (one with the inum field being 0). Then copy the name and
// inum to the subdirectory structure found and issue a inode_write with the new subdirectory
// entry and the offset found. You can assume there will always be an unallocated subdirectory
// entry (or you can also call  KERN_PANIC with appropriate error message if not). The function
// should return 0 in the case of success.
int dir_link(struct inode *dp, char *name, uint32_t inum)
{
    int name_len = strnlen(name, DIRSIZ);
    // KERN_DEBUG("dir_link: name is %s with length %d\n", name, name_len);

    // TODO
    // Check whether that subdirectory already exists in the current directory
    // (with dir_lookup above), and if so, call inode_put on the inode returned 
    // by the dir_lookup and return -1.
    uint32_t poff;
    struct inode * existing_inode = dir_lookup(dp, name, &poff);
    if(existing_inode != 0){
        // Directory entry already exists:
        // Recall that dir_lookup calls inode_get in the end, which creates a reference
        // to the in-memory inode structure and increases the reference counter of that
        // inode. Thus, in the case of error, we should call inode_put to drop that unnecessary
        // reference we have created. 
        inode_put(existing_inode);
        return -1;
    }

    uint32_t off;
    struct dirent de;
    uint32_t de_size = sizeof(de);
    // struct file_stat st;

    // Loop through the subdirectory structures of dp to find the first unallocated 
    // inode (one with the inum field being 0)
    for(off = 0; off < dp->size; off += de_size){
        int read_size = inode_read(dp, (char*) &de, off, de_size);
        if(read_size != de_size){
            KERN_PANIC("dir_lookup: error in inode_read size!\n");
        }

        if(de.inum == 0){
            // Found available dirent
            break;
        }
    }

    // Copy the name and inum to the subdirectory structure found and issue
    // a inode_write with the new subdirectory entry and the offset found
    strncpy(de.name, name, name_len);
    de.name[name_len] = '\0';
    de.inum = inum;
    // KERN_DEBUG("dir_link: added %s to offset %d\n", de.name, off);

    // Write to dp
    int write_size = inode_write(dp, (char*) &de, off, de_size);
    if(write_size != de_size){
        KERN_PANIC("dir_link: %d bytes written by inode_write, expected %u\n", write_size, de_size);
    }

    return 0;
}
