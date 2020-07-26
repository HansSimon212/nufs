#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>


#include "inode.h"

const int INODE_COUNT = 64;

inode*
get_inode(int inum)
{
    assert(inum < INODE_COUNT);
    uint8_t* base = (uint8_t*) pages_get_page(0);
    inode* nodes = (inode*)(base);
    return &(nodes[inum]);
}

// allocates first available inode slot
int
alloc_inode()
{
    for (int ii = 0; ii < INODE_COUNT; ++ii) {
        inode* node = get_inode(ii);
        if (node->mode == 0) {
            time_t now = time(NULL);
            memset(node, 0, sizeof(inode));
            node->refs = 0;
            node->mode = 010644;
            node->size = 0;
            node->ents = 0;
	        node->ptrs[0] = alloc_page();
	        node->ptrs[1] = 0;
	        node->iptr = 0;
	        node->ctime = now;
	        node->atime = now;
	        node->mtime = now;
            printf("+ alloc_inode() -> %d\n", ii);
            return ii;
        }
    }

    return -1;
}

// frees the given inode
void
free_inode(int inum) {
	printf("+ free_inode(%d)\n", inum);

	inode* node = get_inode(inum);

	if(node->refs > 0) {
	    puts("cannot free");
	    abort();
	}

	shrink_inode(node, 0);
	free_page(node->ptrs[0]);
	memset(node, 0, sizeof(inode));
}

// shrinks given inode to size
int
shrink_inode(inode* node, int size) {
	
	int numPages = (node->size / 4096) + 1;
	int newNumPages = (size / 4096) + 1;

	// while the number of pages is still too large, deletes a page
	for(;numPages > newNumPages; numPages--) {
		int pnum = inode_get_pnum(node, node->size);
		free_page(pnum);

		// indirect pointers exist
		if(node->iptr) {
			int* iptrs = pages_get_page(node->iptr);
			int iptrIdx = (node->size/4096) - 2;
			iptrs[iptrIdx] = 0;
		
			// if lowest indexed indirect pointer is freed, updates iptr
			if(iptrs[0] == 0) {
				node->iptr = 0;
			}	
		} else if (node->ptrs[1]) {
			node->ptrs[1] = 0;	
		} else {
			puts("Tried to remove last page in shrink");
			return -1;
		}
	}

	// update the size of the node
	node->size = size;
	return 0;
}

// grows given inode to specified size
int
grow_inode(inode* node, int size) {
	int numPages = (node->size / 4096) + 1;
	int newNumPages = (size / 4096) + 1;

	// while the number of pages is still too little, adds a page
	for(;numPages < newNumPages; numPages++) {
		if(node->ptrs[1] == 0) {
			node->ptrs[1] = alloc_page();
		} else {
			// if indirect block doesn't exist, allocates a page
			if(node->iptr == 0) {
				node->iptr = alloc_page();
			}

			int* iptrs = pages_get_page(node->iptr);
			iptrs[numPages - 1] = alloc_page();
		}
	}

	// updates the size of the node
	node->size = size;
	return 0;
}

	

void
print_inode(inode* node)
{
    if (node) {
        printf("node{refs: %d, mode: %04o, size: %d, ents: %d, ptrs[0]: %d, ptrs[1]: %d, iptr: %d}\n",
               node->refs, node->mode, node->size, node->ents, node->ptrs[0], node->ptrs[1], node->iptr);
    }
    else {
        printf("node{null}\n");
    }
}

// fpn is file page number; unused for hw10
int
inode_get_pnum(inode* node, int fpn)
{
    int pageIdx = fpn / 4096; // index of the last filed page???
   
    if (pageIdx == 0) {
	return node->ptrs[0];
    } else if (pageIdx == 1) {
    	return node->ptrs[1];
    } else {
	int* iptr = pages_get_page(node->iptr);
        return iptr[pageIdx - 2];
    }
}

void*
inode_get_page(inode* node, int fpn)
{
    return pages_get_page(inode_get_pnum(node, fpn));
}
