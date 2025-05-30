#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "cpen212alloc.h"
#include "cpen212common.h"

// My heap consists of 64 bytes of overhead, with the first 8 bytes storing the address to the end of the heap
// The heap is a bidirectional implicit freelist, and alloc uses first fit implementation
// Each block has a footer and header, which are identical, and hold the size of the block and an allocated bit
// Realloc implementation:
// There are many cases in this non-trivial implementation of realloc, and thus, each case has a comment before
// it outlining the conditions of the case, and what happens in that case
// Realloc first checks if the realloc is a shrink, and then does a shrink if so
// If it needs more space than currently given, then realloc first checks the next block, as it is simpler to reallocate forwards
// If the forward adjacent free block is not enough, then it checks for a prev adjacent free block, and reallocs if available and sufficient
// If neither are sufficient alone, it checks if they have enough space combined
// If there is, it will combine all 3 blocks
// If the combination is still not enough space, it resorts to the trivial realloc where it searches uses first fit for 
// an available free block of sufficient size

static void coalesce(void* block_header);
static void* getFooter(void* header);
static void* getHeader(void* footer);
static void combineBlocks(void* blockOneHead, void* blockOneFoot,
                          void* blockTwoHead, void* blockTwoFoot, int alloc);
static void* resizeBlocks(void* blockOneHead, void* blockTwoHead, int cpyDataBackwards,
                         size_t aligned_sz, int nbytesCpy);
static void* getNext(void* header);

static int isAlloc(BoundaryTag* tag) {
    size_t boundTag = tag->data;
    return ((boundTag & 1) == 1) ? 1 : 0;
}

static size_t getSize(BoundaryTag* tag) {
    return (tag->data) >> 1;
}

static void* getNext(void* header){
    return header + getSize(header);
}

void *cpen212_init(void *heap_start, void *heap_end) {
    *((void **) heap_start) = heap_end;
    BoundaryTag firstBlock = {heap_end - heap_start - 64};
    BoundaryTag* temp = &firstBlock;
    temp->data = (temp->data << 1);
    *((void **) heap_start + 8) = (void *) temp->data;
    *((void **) heap_end - 1) = (void *) temp->data;
    return heap_start;
}

void *cpen212_alloc(void *heap_handle, size_t nbytes) {
    if (nbytes == 0) {
        return NULL;
    }
    size_t aligned_sz = (nbytes + 7) & ~7;
    aligned_sz = aligned_sz < 16 ? 16 : aligned_sz;
    void* heap_end = *((void **) heap_handle);
    void* p_heap = heap_handle; // p is pointer to the start of heap
    BoundaryTag* p_blocks = (BoundaryTag *)((char *) p_heap + 64);
    short found = 0;
    while (found != 1) {
        if ((char *) p_blocks + 16 + aligned_sz > (char *) heap_end) {
            return NULL;
        }
        if (isAlloc(p_blocks) == 0 && getSize(p_blocks) >= aligned_sz + 16) {
            found = 1; // found first suitable block
            break;
        }
        p_blocks = (BoundaryTag *) ((char *) p_blocks + getSize(p_blocks));
    }

    if (getSize(p_blocks) < (aligned_sz + 2 * 8) + 32) {
        //sets a bit of entire block to 1
        p_blocks->data = p_blocks->data | 1;
        BoundaryTag* temp = (BoundaryTag *) ((char*) p_blocks + getSize(p_blocks) - 8);
        temp->data = temp->data | 1;
    }
    else {
        size_t sizeOfAllocBlock  = aligned_sz + (2 * 8);
        size_t leftOverSpace     = getSize(p_blocks) - sizeOfAllocBlock;
        BoundaryTag* newTagStart = (BoundaryTag *) ((char *) p_blocks + sizeOfAllocBlock);
        BoundaryTag* newTagEnd   = (BoundaryTag *) ((char *) newTagStart + leftOverSpace - 8);
        BoundaryTag* allocEndTag = (BoundaryTag *) ((char *) p_blocks + sizeOfAllocBlock - 8);
        p_blocks->data    = (sizeOfAllocBlock << 1) | 1;
        allocEndTag->data = (sizeOfAllocBlock << 1) | 1;
        newTagStart->data = leftOverSpace << 1;
        newTagEnd->data   = leftOverSpace << 1;
    }
    return (void *) p_blocks + 8;
}

static void* getFooter(void* header) {
    return (void *) ((char *) header + getSize(header) - 8);
}

static void* getHeader(void* footer) {
    return footer - (getSize(footer) - 8);
}

// precondition: blockOneHead and blockTwoHead have correct tag data (the footers do not)
static void combineBlocks(void* blockOneHead, void* blockOneFoot,
                          void* blockTwoHead, void* blockTwoFoot, int alloc) {
    size_t blockTwoSize = getSize(blockTwoHead);
    //delete middle header+footer
    *((void **) blockOneFoot) = (void *) 0;
    *((void **) blockTwoHead) = (void *) 0;

    //set new tags
    size_t newTag = (getSize(blockOneHead) + blockTwoSize) << 1;
    if (alloc != 0) {
        newTag++;
    }
    ((BoundaryTag *) blockOneHead)->data = newTag;
    ((BoundaryTag *) blockTwoFoot)->data = newTag;
}

static void* resizeBlocks(void* blockOneHead, void* blockTwoHead, int cpyDataBackwards,
                         size_t aligned_sz, int nbytesCpy) {
    BoundaryTag* blockOneFoot = getFooter(blockOneHead);
    BoundaryTag* blockTwoFoot = getFooter(blockTwoHead);
    BoundaryTag* newHead = (BoundaryTag *) ((char *) blockTwoFoot - 8 - aligned_sz);
    BoundaryTag* newFoot = (BoundaryTag *) ((char *) newHead - 8);

    //clear middle tags
    ((BoundaryTag*) blockTwoHead)->data = 0;
    blockOneFoot->data = 0;

    if (cpyDataBackwards) memcpy((char *) newHead+8, (char *) blockTwoHead+8, nbytesCpy);
    newHead->data = ((aligned_sz + 16) << 1) + 1;
    blockTwoFoot->data = ((aligned_sz + 16) << 1) + 1;
    newFoot->data = ((char *) newFoot - (char *) blockOneHead + 8) << 1;
    ((BoundaryTag*) blockOneHead)->data = ((char *) newFoot - (char *) blockOneHead + 8) << 1;
    return (char *) newHead + 8;
}

static void coalesce(void* block_header){
    //coalesce forwards
    void* footer = getFooter(block_header);
    void* nextHeader = (void *) ((char *) footer + 8);
    void* nextFooter = getFooter(nextHeader);

    // if next block is also free
    if (((BoundaryTag *) nextHeader)->data % 2 == 0) {
        combineBlocks(block_header, footer, nextHeader, nextFooter, 0);
    }

    //coalesce backwards
    footer = getFooter(block_header);
    void* prevFooter = (void *) ((char *) block_header - 8);
    void* prevHeader = prevFooter - getSize(prevFooter) + 8;

    // if prev block is free
    if (((BoundaryTag *) prevFooter)->data % 2 == 0) {
        combineBlocks(prevHeader, prevFooter, block_header, footer, 0);
    }
}

void cpen212_free(void *s, void *p) {
    BoundaryTag* header = (BoundaryTag *) ((char *) p - 8);
    BoundaryTag* footer = (BoundaryTag *) ((char *) header + getSize(header)-8);
    header->data = (header->data >> 1) << 1;
    footer->data = (footer->data >> 1) << 1;
    coalesce(header);
}

void *cpen212_realloc(void *s, void *prev, size_t nbytes) {
    if (nbytes == 0) {
        return NULL;
    }
    if (prev == NULL) {
        return cpen212_alloc(s, nbytes);
    }
    BoundaryTag* heapDataBorder = s + 64;
    BoundaryTag* heap_end = (BoundaryTag*) *((void **) s);
    size_t aligned_sz = (nbytes + 7) & ~7;
    aligned_sz = aligned_sz < 16 ? 16 : aligned_sz;
    BoundaryTag* currHead = prev - 8;

    //case 0: aligned+16 = prev size
    if (aligned_sz + 16 == getSize(currHead)) {
        return prev;
    }

    BoundaryTag* nextHead = getNext(currHead);
    //case 1: aligned+16 < prevSize, requires shrink
    if (aligned_sz + 16 < getSize(currHead)) {
        //case A: next blcok is allocated or if heap full
        if (nextHead >= heap_end || isAlloc(nextHead)){
            //case 1: prevsize-(aligned+16) < 32
            //no shrink, nothing to do
            if (getSize(currHead) < aligned_sz + 48) {
                return prev;
            }
            //case 2: prevsize-(aligned+16) >= 32
            //shrink, create new free block
            else {
                BoundaryTag* currFooter = getFooter(currHead);
                BoundaryTag* newFooter = (BoundaryTag *) ((char *) currHead + aligned_sz + 8);
                BoundaryTag* newFreeHead = (BoundaryTag *) ((char *) newFooter + 8);
                currHead->data = ((aligned_sz + 16) << 1) + 1;
                newFooter->data = ((aligned_sz + 16) << 1) + 1;
                size_t newFreeSize = (char *) currFooter - (char *) newFreeHead + 8;
                currFooter->data = newFreeSize << 1;
                newFreeHead->data = newFreeSize << 1;
                return prev;
            }
        }
        //case B: next block is free
        //shrink block and increase next block size
        else {
            BoundaryTag* currFooter = getFooter(currHead);
            BoundaryTag* nextFooter = getFooter(nextHead);
            BoundaryTag* newCurrFooter = (BoundaryTag *) ((char *) currHead + aligned_sz + 8);
            BoundaryTag* newFreeHead = (BoundaryTag *) ((char *) newCurrFooter + 8); 
            currFooter->data = 0;
            nextHead->data = 0;
            currHead->data = ((aligned_sz + 16) << 1) + 1;
            newCurrFooter->data = ((aligned_sz + 16) << 1) + 1;
            size_t newFreeSize = (char *) nextFooter - (char *) newFreeHead + 8;
            newFreeHead->data = newFreeSize << 1;
            nextFooter->data = newFreeSize << 1;
            return (char *) currHead + 8;
        }
    }

    size_t currBlockSize = getSize((BoundaryTag *) ((char *) prev - 8)) - 16;
    BoundaryTag* prevFoot = (BoundaryTag *) ((char *) currHead - 8);
    //case 2: just next block is sufficient
    if ((nextHead < heap_end) && (!isAlloc(nextHead)) && currBlockSize + getSize(nextHead) >= aligned_sz) {
        //case 2a: if nextBlockSize - amountToExpand  <= 32
        //combine both blocks
        size_t amountToExpand = aligned_sz - currBlockSize;
        if (getSize(nextHead) <= 32 + amountToExpand) {
            combineBlocks(currHead, getFooter(currHead), nextHead, getFooter(nextHead), 1);
            return prev;
        }
        //case 2b: else
        //decrease free block and increase alloc block
        else {
            BoundaryTag* currentFoot = (BoundaryTag *) getFooter(currHead);
            BoundaryTag* nextFoot = (BoundaryTag *) getFooter(nextHead);
            BoundaryTag* newFoot = (BoundaryTag *)((char *) currHead + 8 + aligned_sz);
            BoundaryTag* newHead = (BoundaryTag*) ((char *) newFoot + 8);
            currentFoot->data = 0;
            nextHead->data = 0;
            currHead->data = ((aligned_sz + 16) << 1) + 1;
            newFoot->data = ((aligned_sz + 16) << 1) + 1;
            newHead->data = ((char *)nextFoot - (char *) newHead + 8) << 1;
            nextFoot->data = ((char *) nextFoot - (char *) newHead + 8) << 1;
            return prev;
        }
    }

    //case 3: just prev block is sufficient
    if ((prevFoot > heapDataBorder) && (!isAlloc(prevFoot)) && currBlockSize + getSize(prevFoot) >= aligned_sz) {
        BoundaryTag* prevHead = getHeader(prevFoot);
        BoundaryTag* currFoot = getFooter(currHead);
        //case 3a: if prevBlockSize - amountToExpand <= 32
        //combine both blocks
        //memcpy
        size_t amountToExpand = aligned_sz - currBlockSize;
        if (getSize(prevFoot) <= 32 + amountToExpand) {
            combineBlocks(prevHead, prevFoot, currHead, currFoot, 1);
            memcpy((char *) prevHead+8, (char *) currHead+8, currBlockSize);
            return (char *) prevHead + 8;
        }
        //case 3b: else
        //clear middle tags, memcpy, add new tags
        else {
            return resizeBlocks(prevHead, currHead, 1, aligned_sz, currBlockSize);
        }
    }

    //case 4: next+prev is sufficient
    if ((prevFoot > heapDataBorder) && (nextHead < heap_end) && (!isAlloc(prevFoot) && (!isAlloc(nextHead)) &&
        currBlockSize + getSize(prevFoot) + getSize(nextHead) >= aligned_sz)) {
        //case 4a: prevBlockSize + nextBlockSize - amountToExpand <= 32
        //combine all blocks
        //memcpy
        size_t amountToExpand = aligned_sz - currBlockSize;
        if (getSize(prevFoot) + getSize(nextHead) <= 32 + amountToExpand) {
            combineBlocks(currHead, getFooter(currHead), nextHead, getFooter(nextHead), 1);
            combineBlocks(getHeader(prevFoot), prevFoot, currHead, getFooter(nextHead), 1);
            memcpy(getHeader(prevFoot) + 8, (char *) currHead + 8, currBlockSize);
            return getHeader(prevFoot) + 8;
        }
        //case 4b:
        else {
            combineBlocks(currHead, getFooter(currHead), nextHead, getFooter(nextHead), 1);
            return resizeBlocks(getHeader(prevFoot), currHead, 1, aligned_sz, currBlockSize);
        }
    }

    //case 5: last resort realloc (prev and next are not enough)
    void* useableSpace = cpen212_alloc(s, nbytes);
    if (!useableSpace) {
        return NULL;
    }
    void* returnVal = useableSpace;
    void* toCpy = (void *) ((char *) prev);
    size_t bytesToCpy = aligned_sz < currBlockSize ? aligned_sz : currBlockSize;
    memcpy(useableSpace, prev, bytesToCpy);
    if (useableSpace != prev) {
        cpen212_free(s, prev);
    }
    return returnVal;
}
