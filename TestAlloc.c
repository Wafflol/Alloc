#include "cpen212alloc.h"
#include <stdlib.h>
#include <stdio.h>

extern void checkHeap(void* start, void* end);
extern void prettyPrint(void* start, void* end);
extern void printHeap(void* start, void* end);
void smallTest();
void bigTest();
void testCoalesceForwards();
void testCoalesceBackwards();
void testCoalesceBackFor();
void testCase0();
void testCase1a1();
void testCase1a2();
void testCase1b();
void testCase2a();
void testCase2b();
void testCase3a();
void testCase3b();
void testCase4a();
void testCase4b();
void testCase5();
void trace13SegFTest();

int main() {
    //trace13SegFTest();
    printf("Testing 0: \n");
    testCase0();
    printf("Testing 1a1: \n");
    testCase1a1();
    printf("Testing 1a2: \n");
    testCase1a2();
    printf("Testing 1b: \n");
    testCase1b();
    // printf("Testing 2a: \n");
    // testCase2a();
    // printf("Testing 2b: \n");
    // testCase2b();
    // printf("Testing 3a: \n");
    // testCase3a();
    // printf("Testing 3b: \n");
    // testCase3b();
    // printf("Testing 4a: \n");
    // testCase4a();
    // printf("Testing 4b: \n");
    // testCase4b();
    // printf("Testing 5: \n");
    // testCase5();
    // testCoalesceForwards();
    // smallTest();
    // bigTest();
    // testCoalesceForwards();
    // testCoalesceBackwards();
    // testCoalesceBackFor();
    return 0;
}

void trace13SegFTest() {
    int space = 400;
    void* start = malloc(space);
    void* end = start + space;
    void* heap = cpen212_init(start, end);
    void* b1 = cpen212_alloc(start, 32);
    printHeap(start, end);
    checkHeap(start, end);
    void* b2 = cpen212_alloc(start, 16);
    printHeap(start, end);
    b1 = cpen212_realloc(start, b1, 48);
    printHeap(start, end);
    void* b3 = cpen212_alloc(start, 32);
    printHeap(start, end);
    cpen212_free(heap, b2);
    printHeap(start, end);
    printf("testing realloc b1, 60 (will use 80 bytes)");
    b1 = cpen212_realloc(heap, b1, 60);
    printHeap(start, end);
    void* b4 = cpen212_alloc(heap, 20);
    printHeap(start, end);
    cpen212_free(heap, b3);
    printHeap(start, end);
    b1 = cpen212_realloc(heap, b1, 40);
    printHeap(start, end);
}

void testCase0() {
    int space = 96;
    void* start = malloc(space);
    void* end = start + space;
    cpen212_init(start, end);
    void* block1 = cpen212_alloc(start, 16);
    printHeap(start, end);
    cpen212_realloc(start, block1, 8);
    printHeap(start, end);
    cpen212_free(start, block1);
    printHeap(start, end);
    void* block2 = cpen212_alloc(start, 7);
    printHeap(start, end);
    cpen212_realloc(start, block1, 16);
    printHeap(start, end);
}
void testCase1a1() {
    int space = 64 + 64 + 32;
    void* start = malloc(space);
    void* end = start + space;
    cpen212_init(start, end);
    void* block1 = cpen212_alloc(start, 48);
    printHeap(start, end);
    void* block2 = cpen212_alloc(start, 16);
    printf("Expected: 2 alloc blocks\n");
    printHeap(start, end);
    block1 = cpen212_realloc(start, block1, 40);
    printf("Expected: no change");
    printHeap(start, end);
}
void testCase1a2() {
    int space = 64 + 64 + 32;
    void* start = malloc(space);
    void* end = start + space;
    cpen212_init(start, end);
    void* block1 = cpen212_alloc(start, 16+32);
    void* block2 = cpen212_alloc(start, 16);
    printf("Expected: 2 alloc blocks\n");
    printHeap(start, end);
    block1 = cpen212_realloc(start, block1, 16);
    printf("Expected: 32a, 32f, 32a");
    printHeap(start, end);
}
void testCase1b() {
    int space = 64 + 64 + 32;
    void* start = malloc(space);
    void* end = start + space;
    cpen212_init(start, end);
    void* block1 = cpen212_alloc(start, 16+32);
    void* block2 = cpen212_alloc(start, 16);
    printf("Expected: 2 alloc blocks\n");
    printHeap(start, end);
    cpen212_free(start, block2);
    printf("Expected: 1 free 1 alloc\n");
    printHeap(start, end);
    block1 = cpen212_realloc(start, block1, 16);
    printf("Expected: 32 alloc rest free");
    printHeap(start, end);
}
void testCase2a() {

}
void testCase2b() {

}
void testCase3a() {

}
void testCase3b() {

}
void testCase4a() {

}
void testCase4b() {

}
void testCase5() {

}

void testCoalesceBackwards() {
    int space = 176;
    printf("Testing Coalescing Backwards\n");
    void* start = malloc(space);
    void* end = start + space;
    cpen212_init(start, end);
    void* block1 = cpen212_alloc(start, 16);
    void* block2 = cpen212_alloc(start, 32);
    void* block3 = cpen212_alloc(start, 8);
    printf("3 Allocs: \n");
    printHeap(start, end);
    cpen212_free(start, block1);
    printf("1 free 32 block at start\n");
    printHeap(start, end);
    cpen212_free(start, block2);
    printf("1 free 80 block at start\n");
    printHeap(start, end);
    cpen212_free(start, block3);
    printf("1 free 112 block at start\n");
    printHeap(start, end);
}

void testCoalesceBackFor() {
    int space = 176;
    printf("Testing Coalescing Backwards and then Forwards\n");
    void* start = malloc(space);
    void* end = start + space;
    cpen212_init(start, end);
    void* block1 = cpen212_alloc(start, 16);
    void* block2 = cpen212_alloc(start, 32);
    void* block3 = cpen212_alloc(start, 8);
    printf("3 Allocs: \n");
    printHeap(start, end);
    cpen212_free(start, block1);
    printf("1 free 32 block at start\n");
    printHeap(start, end);
    cpen212_free(start, block3);
    printf("1 free 32 block at start and end\n");
    printHeap(start, end);
    cpen212_free(start, block2);
    printf("1 free 112 block\n");
    printHeap(start, end);
}

void testCoalesceForwards() {
    int space = 176;
    printf("Testing Coalescing Forwards\n");
    void* start = malloc(space);
    void* end = start + space;
    cpen212_init(start, end);
    void* block1 = cpen212_alloc(start, 16);
    void* block2 = cpen212_alloc(start, 32);
    void* block3 = cpen212_alloc(start, 8);
    printf("3 Allocs: \n");
    //checkHeap(start, end);
    printHeap(start, end);
    cpen212_free(start, block3);
    printf("1 free 32 block at end\n");
    //checkHeap(start, end);
    printHeap(start, end);
    cpen212_free(start, block2);
    printf("1 free 80 block at end\n");
    //checkHeap(start, end);
    printHeap(start, end);
    cpen212_free(start, block1);
    printf("1 free 112 block at end\n");
    //checkHeap(start, end);
    printHeap(start, end);
}

void smallTest(){
    int space = 96;
    printf("Testing test 1\n");
    void* start = malloc(space);
    void* end = start + space;
    cpen212_init(start, end);
    cpen212_alloc(start, 16);
    printf("Test 1 passed!\n");
    checkHeap(start, end);

    printf("Testing test 2\n");
    void* start1 = malloc(space);
    void* end1 = start1 + space;
    cpen212_init(start1, end1);
    cpen212_alloc(start1, 15);
    printf("Test 2 passed!\n");
    checkHeap(start1, end1);

    printf("Testing test 3\n");
    void* start2 = malloc(space);
    void* end2 = start2 + space;
    cpen212_init(start2, end2);
    cpen212_alloc(start2, 17);
    printf("Test 3 passed!\n");
    checkHeap(start2, end2);
}

void bigTest() {
    int space = 96+32+32;
    printf("Testing test 4\n");
    void* start = malloc(space);
    void* end = start + space;
    void* heap_start = cpen212_init(start, end);
    printf("Expected: 160 FREE\n");
    checkHeap(start, end);
    void* block1Start = cpen212_alloc(heap_start, 8);
    printf("Expected, 32 allocated block\n");
    checkHeap(start, end);
    void* block2Start = cpen212_alloc(heap_start, 16);
    printf("Expected, 2x32 allocated block\n");
    checkHeap(start, end);
    cpen212_free(heap_start, block1Start);
    printf("Expected, 1 allocated block in middle\n");
    checkHeap(start, end);
    void* block3Start = cpen212_alloc(heap_start, 8);
    printf("Expected, 2x32 allocated\n");
    checkHeap(start, end);
    cpen212_free(heap_start, block2Start);
    printf("Expected, 32 allocated\n");
    checkHeap(start, end);
    void* block4Start = cpen212_realloc(heap_start, block3Start, 8);
    printf("Expected, 32 allocated at 2nd spot\n");
    checkHeap(start, end);

    printf("\n\n-----------------------");
    space = 96+32;
    printf("Testing test 4\n");
    start = malloc(space);
    end = start + space;
    heap_start = cpen212_init(start, end);
    printf("Expected: 64 FREE\n");
    checkHeap(start, end);
    block1Start = cpen212_alloc(heap_start, 8);
    printf("Expected, 32 allocated block\n");
    checkHeap(start, end);
    block2Start = cpen212_alloc(heap_start, 16);
    printf("Expected, 2x32 allocated, full mem\n");
    checkHeap(start, end);

    printf("---------------\n");
    space = 12512;
    int free = space - 64;
    int allocSize = 0x7F8;
    start = malloc(space);
    end = start + space;
    heap_start = cpen212_init(start, end);
    printf("Expected: %d FREE\n", free);
    checkHeap(start, end);
    block1Start = cpen212_alloc(heap_start, allocSize);
    printf("Expected, %d allocated block\n", allocSize + 16);
    checkHeap(start, end);
    block2Start = cpen212_alloc(heap_start, allocSize);
    printf("Expected, 2x%d allocated, full mem\n", allocSize + 16);
    checkHeap(start, end);
    checkHeap(start, end);
}

