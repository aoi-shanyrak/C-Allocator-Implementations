#include <stdint.h>
#include <string.h>
#include <unistd.h>

typedef long Align;

union header {
  struct {
    union header* next;
    size_t size;
  } s;

  Align align;
};

typedef union header Header;

#define MIN_TO_SBRK_ALLOC 128

#define GET_HEADER(P)     ((Header*) (P) - 1)
#define ALIGN_UNITS(N)    (((N) + sizeof(Header) - 1) / sizeof(Header) + 1)
#define DATA_SIZE(H)      ((H)->s.size * sizeof(Header) - sizeof(Header))

static Header base;
static Header* free_list = NULL;

static Header* morecore(size_t nu) {
  char *cp, *sbrk(int);
  Header* up;

  if (nu < MIN_TO_SBRK_ALLOC)
    nu = MIN_TO_SBRK_ALLOC;

  if (nu > SIZE_MAX / sizeof(Header))
    return NULL;

  cp = sbrk(nu * sizeof(Header));
  if (cp == (char*) -1)
    return NULL;

  up = (Header*) cp;
  up->s.size = nu;

  void kr_free(void*);
  kr_free((void*) (up + 1));

  return free_list;
}

void* kr_malloc(size_t nbytes) {
  if (nbytes == 0)
    return NULL;
  if (nbytes > SIZE_MAX - 2 * sizeof(Header))
    return NULL;

  Header *prev, *cur;

  size_t nunits = ALIGN_UNITS(nbytes);

  if (free_list == NULL) {
    base.s.next = free_list = &base;
    base.s.size = 0;
  }
  for (prev = free_list, cur = prev->s.next;; prev = cur, cur = cur->s.next) {
    if (cur->s.size >= nunits) {
      if (cur->s.size == nunits)
        prev->s.next = cur->s.next;
      else {
        cur->s.size -= nunits;
        cur += cur->s.size;
        cur->s.size = nunits;
      }
      free_list = prev;
      return (void*) (cur + 1); // skip header
    }
    if (cur == free_list)
      if ((cur = morecore(nunits)) == NULL)
        return NULL;
  }
}

void kr_free(void* ptr) {
  if (ptr == NULL)
    return;

  Header* cur;
  Header* block = GET_HEADER(ptr);
  for (cur = free_list; !(block > cur && block < cur->s.next); cur = cur->s.next) {
    if (cur >= cur->s.next && (block > cur || block < cur->s.next))
      break;
  }
  if (block + block->s.size == cur->s.next) {
    block->s.size += cur->s.next->s.size;
    block->s.next = cur->s.next->s.next;
  } else
    block->s.next = cur->s.next;
  if (cur + cur->s.size == block) {
    cur->s.size += block->s.size;
    cur->s.next = block->s.next;
  } else
    cur->s.next = block;

  free_list = cur;
}

void* kr_realloc(void* ptr, size_t size) {
  if (ptr == NULL)
    return kr_malloc(size);

  if (size == 0) {
    kr_free(ptr);
    return NULL;
  }
  Header* block = GET_HEADER(ptr);
  size_t nunits = ALIGN_UNITS(size);
  size_t block_size = DATA_SIZE(block);

  if (block->s.size >= nunits)
    return ptr;

  void* n_ptr = kr_malloc(size);
  if (n_ptr == NULL)
    return NULL;

  size_t cp_size = (block_size < size) ? block_size : size;
  memcpy(n_ptr, ptr, cp_size);

  kr_free(ptr);

  return n_ptr;
}

void* kr_calloc(size_t nmemb, size_t size) {
  if (nmemb == 0 || size == 0)
    return NULL;

  if (nmemb > SIZE_MAX / size)
    return NULL;

  size_t total = nmemb * size;
  void* ptr = kr_malloc(total);

  if (ptr != NULL)
    memset(ptr, 0, total);

  return ptr;
}
