
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <alloca.h>
#include <string.h>
#include <libgen.h>
#include <bsd/string.h>
#include <stdint.h>
#include <stdlib.h>

#include "storage.h"
#include "slist.h"
#include "util.h"
#include "pages.h"
#include "inode.h"
#include "directory.h"

// initializes storage
void
storage_init(const char *path) {
    pages_init(path);
    directory_init();
}

int
storage_stat(const char *path, struct stat *st) {
    int inum = tree_lookup(path);
    if (inum < 0) {
        printf("+ storage_stat(%s) -> %d\n", path, inum);
        return inum;
    }

    inode *node = get_inode(inum);
    printf("+ storage_stat(%s) -> 0; inode %d\n", path, inum);
    print_inode(node);

    // sets up stat structure
    memset(st, 0, sizeof(struct stat));
    st->st_uid = getuid();
    st->st_mode = node->mode;
    st->st_size = node->size;
    st->st_nlink = node->refs;
    return 0;
}

// reads size bytes from specified path into buffer
int
storage_read(const char *path, char *buf, size_t size, off_t offset) {
    int inum = tree_lookup(path);
    if (inum < 0) {
        return inum;
    }
    inode *node = get_inode(inum);
    printf("+ storage_read(%s); inode %d\n", path, inum);
    print_inode(node);

    // ensures offset is accurate
    if (offset >= node->size) {
        return 0;
    }

    if (offset + size >= node->size) {
        size = node->size - offset;
    }

    int numRead = 0;

    // iterates until enough bytes are read
    while (numRead < size) {
        int pnum = inode_get_pnum(node, offset + numRead);
        char *pagestart = pages_get_page(pnum);
        char *readPtr = pagestart + ((offset + numRead) % 4096);

        int bytesToRead = 0;
        int leftOnPage = size - numRead;
        int pageRem = pagestart + 4096 - readPtr;
        (leftOnPage <= pageRem) ? (bytesToRead = leftOnPage) : (bytesToRead = pageRem);
        printf(" + reading from page: %d\n", pnum);
        memcpy(buf + numRead, readPtr, bytesToRead);
        numRead += bytesToRead;
    }

    return size;
}

// writes size bytes from path starting at offset into buf
int
storage_write(const char *path, const char *buf, size_t size, off_t offset) {
    int trv = storage_truncate(path, offset + size);
    if (trv < 0) {
        return trv;
    }

    int inum = tree_lookup(path);
    if (inum < 0) {
        return inum;
    }

    inode *node = get_inode(inum);
    printf("+ storage_read(%s); inode %d\n", path, inum);
    print_inode(node);

    int numWrit = 0;

    // iterates until enough bytes are written
    while (numWrit < size) {
        int pnum = inode_get_pnum(node, offset + numWrit);
        char *pagestart = pages_get_page(pnum);
        char *writPtr = pagestart + ((offset + numWrit) % 4096);

        int bytesToWrit = 0;
        int leftOnPage = size - numWrit;
        int pageRem = pagestart + 4096 - writPtr;
        (leftOnPage <= pageRem) ? (bytesToWrit = leftOnPage) : (bytesToWrit = pageRem);
        printf("+ writing to page: %d\n", pnum);
        memcpy(writPtr, buf + numWrit, bytesToWrit); ///
        numWrit += bytesToWrit;
    }

    return size;
}

// truncates file to given length
int
storage_truncate(const char *path, off_t size) {
    int inum = tree_lookup(path);
    if (inum < 0) {
        return inum;
    }

    inode *node = get_inode(inum);
    if (size >= node->size) {
        int rv = grow_inode(node, size);
        return rv;
    } else {
        int rv = shrink_inode(node, size);
        return rv;
    }
}

// given the full path, parses the parent directory of it
void
parse_parent(const char *fullpath, char *dir) {
    strcpy(dir, fullpath);

    int wordLen = 0;
    for (int ii = strlen(fullpath) - 1; fullpath[ii] != '/'; ii--) {
        wordLen++;
    }

    if (wordLen == strlen(fullpath) - 1) {
        dir[1] = '\0';
    } else {
        dir[strlen(fullpath) - wordLen - 1] = '\0';
    }
}

// given the full path, parses the child of it
char *
parse_child(const char *fullpath, char *sub) {
    strcpy(sub, fullpath);

    int wordLen = 0;
    for (int ii = strlen(fullpath) - 1; fullpath[ii] != '/'; ii--) {
        wordLen++;
    }

    sub += strlen(fullpath) - wordLen;
    return sub;
}

// makes a file/directory at the specified path
int
storage_mknod(const char *path, int mode) {

    if (tree_lookup(path) > -1) {
        printf("mknod fail: already exist\n");
        return -EEXIST;
    }

    char *builtPath = malloc(strlen(path) + 10);
    memset(builtPath, 0, strlen(path) + 10);
    builtPath[0] = '/'; // root

    slist *items = s_split(path + 1, '/');

    // iterate until no path elements remain
    for (slist *xs = items; xs != NULL; xs = xs->next) {
        int subNum = directory_lookup(get_inode(tree_lookup(builtPath)), xs->data);
        int dirNum = tree_lookup(builtPath);

        // if last element has been reached
        if (xs->next == NULL && subNum < 0) {
            int newInum = alloc_inode();
            inode *newNode = get_inode(newInum); // boring
            newNode->mode = mode;

            if (newNode->mode == 040755) {
                char *selfname = ".";
                char *parentname = "..";

                directory_put(newNode, parentname, tree_lookup(builtPath));
                directory_put(newNode, selfname, newInum);
            }

            int rv = directory_put(get_inode(tree_lookup(builtPath)), xs->data, newInum);
            if (rv != 0) {
                free(builtPath);
                return -1;
            }

            newNode->refs = 1;
            free(builtPath);
            return 0;
        } else if (subNum > -1 && get_inode(subNum) -> mode == 040755) {
            if (strcmp(builtPath, "/") == 0) {
                strcpy(builtPath + strlen(builtPath), xs->data);
                continue;
            } else {
                strcpy(builtPath + strlen(builtPath), "/");
                strcpy(builtPath + strlen(builtPath), xs->data);
                continue;
            }
        }
        // sub element doesnt exist
            else {

            int newInum = alloc_inode();
            inode *newNode = get_inode(newInum); // boring
            newNode->mode = 040755;

            char *selfname = ".";
            char *parentname = "..";

            directory_put(newNode, parentname, dirNum);
            directory_put(newNode, selfname, newInum);
            directory_put(get_inode(dirNum), xs->data, newInum);
            newNode->refs = 1;

            if (strcmp(builtPath, "/") == 0) {
                strcat(builtPath, xs->data);
                continue;
            } else {
                strcat(builtPath, "/");
                strcat(builtPath, xs->data);
                continue;
            }
        }

    }


}

// list all elements at the given path
slist *
storage_list(const char *path) {
    return directory_list(path);
}

// UNUSED
void
storage_deref_all(inode *dd) {
    char *data = pages_get_page(dd->ptrs[0]);
    char *text = data;
    int numEnt = dd->ents;
    for (int ii = 0; ii < numEnt; ii++) {
        text = skip_string(text);
        int inum = *((int *) text);
        inode *node = get_inode(inum);
        node->refs--;
        text += 4;
    }
}


// unlinks file/dir at given path from its parent
int
storage_unlink(const char *path) {
    char *dir = alloca(strlen(path) + 1);
    char *sub = alloca(strlen(path) + 1);

    parse_parent(path, dir);
    sub = parse_child(path, sub);

    int dirnum = tree_lookup(dir);
    int subnum = tree_lookup(path);

    inode *dirnode = get_inode(dirnum);
    inode *subnode = get_inode(subnum);

    int rv = directory_delete(dirnode, sub);
    return rv;
}

// creates link from from to to
int
storage_link(const char *from, const char *to) {

    int toNum = tree_lookup(to);
    if (toNum < 0) {
        return toNum;
    }

    char *fromParent = alloca(strlen(from) + 1);
    char *fromChild = alloca(strlen(from) + 1);
    parse_parent(from, fromParent);
    fromChild = parse_child(from, fromChild);

    int fromParentNum = tree_lookup(fromParent); // directory of link
    inode *fromParentNode = get_inode(fromParentNum);


    int rv = directory_put(fromParentNode, fromChild, toNum);

    return 0;
}

// renames the from file to to
int
storage_rename(const char *from, const char *to) {
    storage_link(to, from);
    storage_unlink(from);
    return 0;
}

int
storage_set_time(const char *path, const struct timespec ts[2]) {
    int inum = tree_lookup(path);
    inode *node = get_inode(inum);
    time_t time = ts->tv_sec;
    node->mtime = time;
}

int
storage_access(const char *path) {
    int inum = tree_lookup(path);

    if (inum == -1) {
        return -1;
    }

    inode *node = get_inode(inum);
    node->atime = time(NULL);

    return 0;
}