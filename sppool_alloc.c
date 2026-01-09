#include "sppool_alloc.h"
#undef malloc
#undef free

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define SPECIALIZED_ARENA_SIZE (10 * 1024 * 1024)
#define LARGE_ARENA_SIZE (300 * 1024 * 1024)

typedef char byte;

typedef struct block {
  struct block* next;
} block;

typedef struct {
  byte* start;
  byte* current;
  byte* end;
  block* free_list;
  size_t block_size;
} arena;

typedef struct large_block {
  struct large_block* next_block;
  size_t size;
  char is_free;
} large_block;

typedef struct {
  byte* start;
  byte* end;
  large_block* list_of_blocks;
} large_block_arena;

arena arenas[16];
large_block_arena large_arena;
int arenas_initialized = 0;

void arenas_init() {
  for (size_t i = 0; i < 16; i++) {
    size_t block_size = (i + 1) * 8;
    arenas[i].block_size = block_size;

    arenas[i].start = malloc(SPECIALIZED_ARENA_SIZE);
    if (!arenas[i].start)
      return;
    arenas[i].current = arenas[i].start;
    arenas[i].end = arenas[i].start + SPECIALIZED_ARENA_SIZE;
    arenas[i].free_list = NULL;
  }
  large_arena.start = malloc(LARGE_ARENA_SIZE);
  if (!large_arena.start)
    return;
  large_arena.end = large_arena.start + LARGE_ARENA_SIZE;

  large_block* first_block = (large_block*) large_arena.start;
  first_block->size = LARGE_ARENA_SIZE - sizeof(large_block);
  first_block->next_block = NULL;
  first_block->is_free = 1;

  large_arena.list_of_blocks = first_block;

  arenas_initialized = 1;
}

void arenas_deinit() {
  for (size_t i = 0; i < 16; i++) {
    free(arenas[i].start);
  }
  free(large_arena.start);
}

void ensure_arenas_initialized() {
  if (!arenas_initialized) {
    arenas_init();
    atexit(arenas_deinit);
  }
}

#define ALIGN(SIZE) (((((SIZE) - 1) / 8) + 1) * 8)
#define MIN_BLOCK_SIZE_IN_LARGE 128

void* sppool_malloc(size_t size) {
  ensure_arenas_initialized();
  if (!arenas_initialized)
    return NULL;
  if (size == 0)
    return NULL;
  if (size <= 128) {
    arena* arena_for_size = &arenas[(size - 1) / 8];
    if (arena_for_size->free_list) {
      block* freed_block = arena_for_size->free_list;
      arena_for_size->free_list = freed_block->next;
      return freed_block;
    }
    if (arena_for_size->current + arena_for_size->block_size < arena_for_size->end) {
      byte* ptr = arena_for_size->current;
      arena_for_size->current += arena_for_size->block_size;
      return ptr;
    }
  }
  size_t aligned_size = ALIGN(size);

  large_block* previous_block = NULL;
  large_block* current_block = large_arena.list_of_blocks;

  while (current_block) {
    if ((current_block->is_free) && (current_block->size >= aligned_size)) {
      current_block->is_free = 0;

      size_t remaining_size = current_block->size - aligned_size;
      if (remaining_size >= sizeof(large_block) + MIN_BLOCK_SIZE_IN_LARGE) {
        large_block* new_block = (large_block*) ((byte*) current_block + sizeof(large_block) + aligned_size);

        new_block->is_free = 1;
        new_block->size = current_block->size - sizeof(large_block) - aligned_size;
        new_block->next_block = large_arena.list_of_blocks;
        large_arena.list_of_blocks = new_block;

        current_block->size = aligned_size;
      }
      return (byte*) current_block + sizeof(large_block);
    }
    previous_block = current_block;
    current_block = previous_block->next_block;
  }
  return NULL;
}

#define IS_PTR_IN_ARENA(PTR, ARENA) (((ARENA).start <= (byte*) (PTR)) && ((byte*) (PTR) < (ARENA).end))
#define IS_ALIGNED_PTR(PTR, ARENA) (((size_t) ((byte*) (PTR) - (ARENA).start) % (ARENA).block_size) == 0)

void sppool_free(void* ptr) {
  ensure_arenas_initialized();
  if (!arenas_initialized)
    return;
  if (!ptr)
    return;
  for (size_t i = 0; i < 16; i++) {
    if ((IS_PTR_IN_ARENA(ptr, arenas[i])) && (IS_ALIGNED_PTR(ptr, arenas[i]))) {
      block* free_block = ptr;
      free_block->next = arenas[i].free_list;
      arenas[i].free_list = free_block;
      return;
    }
  }
  if (IS_PTR_IN_ARENA(ptr, large_arena)) {
    large_block* block = (large_block*) ((byte*) ptr - sizeof(large_block));

    block->is_free = 1;
    block->next_block = large_arena.list_of_blocks;
    large_arena.list_of_blocks = block;
  }
}

void* sppool_realloc(void* ptr, size_t size) {
  ensure_arenas_initialized();
  if (!arenas_initialized)
    return NULL;
  if (ptr == NULL)
    return sppool_malloc(size);
  if (size == 0) {
    sppool_free(ptr);
    return NULL;
  }
  size_t cur_bs = 0;
  for (size_t i = 0; i < 16; i++) {
    if (IS_PTR_IN_ARENA(ptr, arenas[i])) {
      cur_bs = arenas[i].block_size;
      break;
    }
  }
  if (!cur_bs) {
    large_block* large_cur_block = (large_block*) ((byte*) ptr - sizeof(large_block));
    cur_bs = large_cur_block->size;
  }
  if (cur_bs >= size)
    return ptr;
  byte* new_ptr = sppool_malloc(size);
  if (new_ptr)
    memcpy(new_ptr, ptr, cur_bs);
  sppool_free(ptr);
  return new_ptr;
}

void* sppool_calloc(size_t nmemb, size_t size) {
  ensure_arenas_initialized();
  if (!arenas_initialized)
    return NULL;
  if ((nmemb == 0) || (size == 0))
    return NULL;
  if (nmemb > SIZE_MAX / size)
    return NULL;
  size_t total_size = nmemb * size;
  byte* ptr = sppool_malloc(total_size);
  if (ptr)
    memset(ptr, 0, total_size);
  return ptr;
}
