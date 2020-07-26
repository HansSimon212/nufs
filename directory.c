#define _GNU_SOURCE
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

#include "directory.h"
#include "util.h"
#include "inode.h"

#define ROOT_NODE 0

char*
skip_string(char* data)
{
    while (*data != 0) {
        data++;
    }
    return data + 1;
}

//void /// may not neeed it
//dirent_setup(dirent* dd, int inum, char* name)
//{
//	dd->inum = inum;
//	strcpy(&(dd->name), name);
//	memset(&(dd->_reserved), 0, sizeof(char) * 12);
//}

void
directory_init()
{
    int inum = alloc_inode();

    inode* rn = get_inode(inum);
    rn->mode = 040755;
    rn->refs = 0; // not yet initialised

    char* selfname = ".";
    char* parentname = "..";

    directory_put(rn, selfname, inum);
    directory_put(rn, parentname, inum);

    rn->ents = 2;
    rn->refs = 0;

//    char* loc = pages_get_page(rn->ptrs[0]);
//    memcpy(loc, selfname, strlen(selfname)+1);
//    loc = skip_string(loc);
//    memcpy(loc, &inum, sizeof(inum));
//    loc+= sizeof(inum);
//
//    memcpy(loc, parentname, strlen(parentname)+1);
//    loc = skip_string(loc);
//    memcpy(loc, &inum, sizeof(inum));
//    loc+= sizeof(inum);
//
//    rn->ents = 2;
}

// gets the inum of the given entry in the given directory
int
directory_lookup(inode* dd, const char* name)
{
    char* data = inode_get_page(dd, 0);
    char* text = data;

    for (int ii = 0; ii < dd->ents; ++ii) {
        printf(" ++ lookup '%s' =? '%s' (%p)\n", name, text, text);

        if (streq(text, name)) {
            text = skip_string(text);
            int* inum = (int*)(text);
            return *inum;
        }

        text = skip_string(text);
        text += sizeof(int);
    }
    return -ENOENT;
}


// gets the inum of the given element in the file system tree
int
tree_lookup(const char* path)
{
    assert(path[0] == '/');

    if (streq(path, "/")) {
        return 0;
    }

    path += 1;

    int dir = 0;
    slist* pathlist = s_split(path, '/');
    slist* tmp = pathlist;

    while(tmp) {
	inode* node = get_inode(dir);
	dir = directory_lookup(node, tmp->data);
	if (dir == -1) {
	    return -1;	 
	}
	tmp = tmp->next;
    }

    return dir;
}

// inserts given name/inum into directory
int
directory_put(inode* dd, const char* name, int inum)
{
    int nlen = strlen(name) + 1;
    if (dd->size + nlen + sizeof(inum) > 4096) {
        return -ENOSPC;
    }

    char* data = inode_get_page(dd, 0);
    memcpy(data + dd->size, name, nlen);
    dd->size += nlen;


    memcpy(data + dd->size, &inum, sizeof(inum));
    dd->size += sizeof(inum);
    dd->ents += 1;


    inode* sub = get_inode(inum);
    sub->refs += 1;


    return 0;
}

// deletes given name from directory
int
directory_delete(inode* dd, const char* name)
{
    printf(" + directory_delete(%s)\n", name);
    printf("%d\n", dd->size);

    char* data = inode_get_page(dd, 0);
    char* text = data;
    char* eend = 0;

    for (int ii = 0; ii < dd->ents; ++ii) {
        if (strcmp(text, name) == 0) {
            goto delete_found;
        }

        text = skip_string(text);
        text += 4;
    }

    return -ENOENT;

delete_found:

    eend = skip_string(text);
    int inum = *((int*)eend);
    eend += sizeof(int);

    int epos = (int)(eend - data);
    printf("text: %s | epos: %d | eend : %s | sub: %d\n", text, epos, eend, dd->size - epos);
    memmove(text, eend, dd->size - epos);
    int elen = (int)(eend - text);
    dd->size -= elen;
    dd->ents -= 1;

    inode* sub = get_inode(inum);

    sub->refs = sub->refs - 1;

    if (sub->refs < 1) {
        free_inode(inum);
    }

    return 0;
}

slist*
directory_list(const char* path)
{
    int inum = tree_lookup(path);
    inode* dd = get_inode(inum);
    char* data = inode_get_page(dd, 0);
    char* text = data;

    printf("+ directory_list()\n");
    slist* ys = 0;

    for (int ii = 0; ii < dd->ents; ++ii) {
        char* name = text;
        text = skip_string(text);
        int pnum = *((int*) text);
        text += sizeof(int);

        printf(" - %d: %s [%d]\n", ii, name, pnum);
        ys = s_cons(name, ys);
    }

    return ys;
}

void
print_directory(const char* path)
{
    printf("Contents:\n");
    slist* items = directory_list(path);
    for (slist* xs = items; xs != 0; xs = xs->next) {
        printf("- %s\n", xs->data);
    }
    printf("(end of contents)\n");
    s_free(items);
}
