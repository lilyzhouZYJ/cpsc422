// File system implementation.  Five layers:
//   + Blocks: allocator for raw disk blocks.
//   + Log: crash recovery for multi-step updates.
//   + Files: inode allocator, reading, writing, metadata.
//   + Directories: inode with special contents (list of other inodes!)
//   + Names: paths like /usr/fs.c for convenient naming.
//
// This file contains the low-level file system manipulation
// routines.  The (higher-level) system call implementations
// are in sysfile.c.

#include <kern/lib/types.h>
#include <kern/lib/debug.h>
#include <kern/lib/spinlock.h>
#include <thread/PTCBIntro/export.h>
#include <thread/PCurID/export.h>
#include <lib/string.h>
#include "inode.h"
#include "dir.h"
#include "log.h"

// Paths

/**
 * Copy the next path element from path into name.
 * If the length of name is larger than or equal to DIRSIZ, then only
 * (DIRSIZ - 1) # characters should be copied into name.
 * This is because you need to save '\0' in the end.
 * You should still skip the entire string in this case.
 * Return a pointer to the element following the copied one.
 * The returned path has no leading slashes,
 * so the caller can check *path == '\0' to see if the name is the last one.
 * If no name to remove, return 0.
 *
 * Examples :
 *   skipelem("a/bb/c", name) = "bb/c", setting name = "a"
 *   skipelem("///a//bb", name) = "bb", setting name = "a"
 *   skipelem("a", name) = "", setting name = "a"
 *   skipelem("", name) = skipelem("////", name) = 0
 */
static char *skipelem(char *path, char *name)
{
    // TODO
    int start = -1, end = -1;
    int i = 0;
    int finished_first_elt = 0;
    char c;
    while((c = path[i]) != '\0'){
        if(c == '/'){
            // ignore slash
            i++;

            // check if this is a trailing slash
            if(start != -1){
                finished_first_elt = 1;
            }
        } else {
            // check if this is the second path element
            if(finished_first_elt == 1){
                break;
            }

            // process non-slash
            if(start == -1){
                start = i;
            }
            end = i;
            i++;
        }
    }

    if(start < 0){
        // No element to remove
        name[0] = '\0';
        return 0;
    } else {
        // Check length of first element
        int len = (end - start) + 1;
        if(len >= DIRSIZ){
            len = DIRSIZ - 1;
        }

        // Copy first element into name
        strncpy(name, path + start, len);
        name[len] = '\0';
    }

    // Return remaining path
    return path + i;
}

/**
 * Look up and return the inode for a path name.
 * If nameiparent is true, return the inode for the parent and copy the final
 * path element into name, which must have room for DIRSIZ bytes.
 * Returns 0 in the case of error.
 */
static struct inode *namex(char *path, bool nameiparent, char *name)
{
    KERN_DEBUG("orig path: %s\n", path);

    struct inode *ip;

    // If path is a full path, get the pointer to the root inode. Otherwise get
    // the inode corresponding to the current working directory.
    if (*path == '/') {
        ip = inode_get(ROOTDEV, ROOTINO);
    } else {
        ip = inode_dup((struct inode *) tcb_get_cwd(get_curid()));
    }

    KERN_DEBUG("hi im here ish\n");


    inode_lock(ip);
    KERN_DEBUG("hi im here\n");

    while ((path = skipelem(path, name)) != 0) {
        KERN_DEBUG("name %s, path: %s\n", name, path);

        // TODO
        // (1) Each iteration begins by locking ip and checking that it is a directory. 
        //     If not, the lookup fails.
        //     If ip's type is not a directory, return 0.
        if(ip->type != T_DIR){
            // Not a directory
            inode_unlockput(ip);
            return 0;
        }

        // (3) Then, the loop has to look for the path element using next = dir_lookup(ip, name, 0) 
        //     and prepare for the next iteration by setting ip = next. When the loop runs out of 
        //     path elements, it returns ip.
        struct inode * next = dir_lookup(ip, name, 0);
        ip = next;
    }

    KERN_DEBUG("hi im done with loop\n");

    // If the call is for a parent inode (that is, when nameiparent is true) and this 
    // is the last path element (first character in path is '\0'), the loop stops early,
    // as per the definition of nameiparent. We need to copy the final path element into
    // name, so namex need only return the unlocked ip.
    if (nameiparent) {
        inode_unlockput(ip);
        return 0;
    }
    inode_unlockput(ip);
    return ip;
}

/**
 * Return the inode corresponding to path.
 */
struct inode *namei(char *path)
{
    char name[DIRSIZ];
    return namex(path, FALSE, name);
}

/**
 * Return the inode corresponding to path's parent directory and copy the final
 * element into name.
 */
struct inode *nameiparent(char *path, char *name)
{
    return namex(path, TRUE, name);
}
