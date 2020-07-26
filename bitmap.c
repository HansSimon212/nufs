#include <stdio.h>
#include <stdlib.h>

// gets an element at position ii in the given bitmap
int
bitmap_get(void *bm, int ii) {

    // what bit number in the corresponding byte this index is
    int inByteIdx = ii % 8; // base zero indexing

    char *bmChars = (char *) bm;
    char correctByte = bmChars[ii / 8]; // the byte containing the bit

    //  right shift the byte inByteIdx amount
    int result =  (correctByte >> (7 - inByteIdx)) & 1;

    return result;
}

// places given value into the given bitmap
void
bitmap_put(void *bm, int ii, int vv) {

    // ensures given value is a 0 or 1
    int val;
    (vv) ? (val = 1) : (val = 0);

    int inByteIdx = ii % 8; // index in corresponding byte
    char *bmChars = (char *) bm;

    // if the given bit is already the correct value, leaves it
    if(bitmap_get(bm, ii) == val) {
        return;
    }
    // otherwise, xors with a left shifted one (to flip bit at correct position)
    else {
        bmChars[ii/8] = (bmChars[ii/8]) ^ (1 << (7 - inByteIdx));
    }
}

// prints information about a bitmap
void
bitmap_print(void* bm, int size) {
    puts("===== BITMAP DUMP =====");
    for(int ii = 0; ii < size; ii++) {
        printf("%d", bitmap_get(bm, ii));
    }
}