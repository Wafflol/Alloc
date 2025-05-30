#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "cpen212alloc.h"
#include "cpen212common.h"

// YOUR CODE HERE

void prettyPrint(void* heapStart, void* heapEnd);
void checkHeap(void* heapStart, void* heapEnd);
void printHeap(void* heapStart, void* heapEnd);
void printBlock(BoundaryTag* tag);

static int isAlloc(BoundaryTag* tag) {
    size_t boundTag = tag->data;
    return ((boundTag & 1) == 1) ? 1 : 0;
}

static size_t getSize(BoundaryTag* tag) {
    return (tag->data) >> 1;
}

void printBlock(BoundaryTag* tag) {
    printf("Block at %14p | Size: %4lu | Allocated: %3s\n", 
           (void*)tag, 
           getSize(tag), 
           isAlloc(tag) ? "Yes" : "No");
}

int cpen212_debug(void *alloc_state, int op) {
    return 0;
}

void checkHeap(void* heapStart, void* heapEnd) {
    printf("\n\nBEGIN HEAP CHECK\n");
    void* head = (void *) (((char *) heapStart) + 64);
    BoundaryTag* tags = (BoundaryTag*) head;
    while((void*) tags < heapEnd) {
        if ((tags->data >> 1) == 0) {
            printf("Block at %p has size 0!\n", tags);
        }
        size_t footerData = ((BoundaryTag *)((char *) tags + getSize(tags) - 8))->data;
        if (tags->data != footerData) {
            printf("Block at %p has data mismatch with footer\n", tags);
        }
        tags = (BoundaryTag *) ((char *) tags + getSize(tags));
    }
    printf("-------------\nExpected Blocks: \n");
    prettyPrint(heapStart, heapEnd);
    printf("-------------\nActual Block Headers: \n");
    int isHeader = 0;
    for (void* traverser = head; traverser < heapEnd; traverser = (void*) ((char*)traverser + 8)) {
        if (getSize(traverser) == 0){
            continue;
        }
        if (isHeader % 2 == 0) {
            printf("HEADER!\n");
        }
        else {
            printf("FOOTER!\n");
        }
        printBlock(traverser);
        isHeader++;
    }
    printf("END HEAP CHECK\n\n");
}

void printHeap(void* heapStart, void* heapEnd) {
    void* head = (void *) (((char *) heapStart) + 64);
    printf("\n\nBEGIN HEAP PRINT\n");
    for (void* traverser = head; traverser < heapEnd; traverser = (void*) ((char*)traverser + 8)) {
        printBlock(traverser);
    }
    printf("END HEAP PRINT\n\n");

}

void prettyPrint(void* heapStart, void* heapEnd) {
    void* head = (void *) (((char *) heapStart) + 64);
    BoundaryTag* tags = (BoundaryTag*) head;
    printf("=== HEAP DUMP ===\n");
    printf("Heap Start: %p, Heap End: %p\n", heapStart, heapEnd);
    while((void*) tags < heapEnd) {
        printBlock(tags);
        tags = (BoundaryTag *) ((char *)tags + getSize(tags));
    }
    printf("=== END HEAP DUMP ===\n");
}
