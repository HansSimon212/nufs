// based on cs3650 starter code

#ifndef INODE_H
#define INODE_H

#include <time.h>
#include <stdio.h>

#include "pages.h"

typedef struct inode {
    int refs;
    int mode; // permission & type
    int size; // bytes
    int ents; // number of entries
    int ptrs[2]; // direct pointers
    int iptr; // single indirect pointer
    time_t ctime; // time created
    time_t atime; // time last accessed
    time_t mtime; // time last modified
} inode;

void print_inode(inode* node);
inode* get_inode(int inum);
int alloc_inode();
void free_inode();
int grow_inode(inode* node, int size);
int shrink_inode(inode* node, int size);
int inode_get_pnum(inode* node, int fpn);
void* inode_get_page(inode* node, int fpn);

#endif
