# Custom `malloc` Implementation in C
This project implements a memory allocator in C that mimics the behavior of `malloc`, `free`, and `realloc`

## Overview
This allocator uses an **implicit bidirectional free list** with **in-place front and back coalescing**, a **first-fit strategy**, and a non-trivial `realloc`

## Heap Structure
- The heap begins with 64 bytes of overhead, including an 8-byte field pointing to the current end of the heap.
- Each block contains:
  - A **header and footer**, both storing the block size and an allocation bit.
- All free blocks are **implicitly linked**; no explicit pointers are stored.
- Allocation uses **first-fit** strategy.

## Features

### Coalescing
- **Bidirectional coalescing** is performed during `free` and `realloc`.
- Coalescing merges with:
  - The previous block (footer-based lookup)
  - The next block (header-based lookup)
  - Or both, if adjacent blocks are free

### `realloc` Strategy
`realloc` uses a ranked approach in its strategy:

1. **Shrink in place** if the new size is smaller.
2. **Expand forward** if the next block is free and sufficient.
3. **Expand backward** if the previous block is free and sufficient.
4. **Merge both adjacent blocks** if combined size suffices.
5. **Fallback**: allocate a new block via first-fit, copy contents, free the old block.

Each case is clearly documented in the code with condition-specific logic.

## Notes
- No external libraries or system calls are used
