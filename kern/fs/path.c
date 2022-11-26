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
    // KERN_DEBUG("START NAMEX\n");
    // KERN_DEBUG("namex: path %s, nameiparent %d\n", path, nameiparent);

    struct inode *ip;

    // If path is a full path, get the pointer to the root inode. Otherwise get
    // the inode corresponding to the current working directory.
    if (*path == '/') {
        ip = inode_get(ROOTDEV, ROOTINO);
    } else {
        ip = inode_dup((struct inode *) tcb_get_cwd(get_curid()));
        // KERN_DEBUG("namex: ip is cwd with ip->nlink %d\n", ip->nlink);
    }

    while ((path = skipelem(path, name)) != 0) {
        // KERN_DEBUG("namex: top of while loop with path %s, name %s\n", path, name);

        // TODO
        // KERN_DEBUG("namex: ip->nlink %d\n", ip->nlink);
        inode_lock(ip);

        // (1) Each iteration begins by locking ip and checking that it is a directory. 
        //     If not, the lookup fails.
        //     If ip's type is not a directory, return 0.
        if(ip->type != T_DIR){
            // Not a directory
            // KERN_DEBUG("namex: NOT A DIRECTORY!\n");
            inode_unlockput(ip);
            return 0;
        }

        // (2) If the call is for a parent inode (that is, when nameiparent is true) and this 
        //     is the last path element (first character in path is '\0'), the loop stops early,
        //     as per the definition of nameiparent. We need to copy the final path element into
        //     name, so namex need only return the unlocked ip.
        if (nameiparent && *path == '\0') {
            // KERN_DEBUG("namex: exiting while loop for nameiparent and this is last path element\n");
            inode_unlock(ip);
            return ip;
        }

        // (3) Then, the loop has to look for the path element using next = dir_lookup(ip, name, 0) 
        //     and prepare for the next iteration by setting ip = next. When the loop runs out of 
        //     path elements, it returns ip.
        struct inode * next = dir_lookup(ip, name, 0);
        if(next == 0){
            // KERN_DEBUG("namex: name %s is not found via dir_lookup\n", name);
            inode_unlockput(ip);
            return 0;
        }
        if(next->inum == ip->inum){
            // Stuck on the same inode
            inode_unlockput(ip);
            break;
        }

        inode_unlockput(ip);
        ip = next;

        // KERN_DEBUG("namex: bottom of while loop, ip is %s with ip->nlink %d\n", name, ip->nlink);
    }

    if(nameiparent){
        inode_put(ip);
        return 0;
    }
    
    // KERN_DEBUG("END NAMEX\n");
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
