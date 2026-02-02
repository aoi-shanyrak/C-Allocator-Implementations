# C-Allocator-Implementations

## Allocator Descriptions
### 1. K&R Allocator (kr_alloc.c)

Classical allocator from "The C Programming Language" by Kernighan & Ritchie

    Principle: Circular free list, first-fit search

    Features: Automatic coalescing of adjacent free blocks

    Overhead: 16 bytes per block (header)

    Complexity: O(n) search, O(1) free


### 2. Pool Allocator with Specialized Arenas (sppool_alloc.c)

Modern allocator with size-based separation

    Principle: 16 arenas for 8-128 byte blocks + shared arena for large blocks

    Features: O(1) allocation for small objects, predictable behavior

    Overhead: 0 bytes for small objects, 16+ bytes for large

    Complexity: O(1) for small objects, O(n) for large

## Usage
Including an Allocator
```c

// Option 1: Use K&R allocator
#include "kr_alloc.h"
// Now malloc/free/calloc/realloc are redirected to kr_*

// Option 2: Use pool allocator  
#include "sppool_alloc.h"
// Now malloc/free/calloc/realloc are redirected to sppool_*

// Option 3: Use standard allocator
#define USE_STD_ALLOC
#include "kr_alloc.h"  // or sppool_alloc.h
// Macros won't override functions, standard malloc remains
```
## Testing
```bash
make test
```
