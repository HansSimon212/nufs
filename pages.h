// based on cs3650 starter code

#ifndef PAGES_H
#define PAGES_H

#include <stdio.h>

void pages_init(const char* path);
void pages_free();
void* pages_get_page(int pnum);
char* get_freemap();
void* get_inode_bitmap();
int alloc_page();
void free_page(int pnum);

#endif
