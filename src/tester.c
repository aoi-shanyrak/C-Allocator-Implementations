#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef TEST_KR_ALLOC
  #include "kr_alloc.h"
  #define ALLOC_NAME "K&R Allocator"
#else
  #include "sppool_alloc.h"
  #define ALLOC_NAME "Pool allocator with specialized arenas"
#endif

#ifdef TEST_KR_ALLOC
  #define MALLOC(SIZE)     kr_malloc(SIZE)
  #define FREE(PTR)        kr_free(PTR)
  #define REALLOC(PTR, SZ) kr_realloc(PTR, SZ)
  #define CALLOC(NM, SZ)   kr_calloc(NM, SZ)
#else
  #define MALLOC(SIZE)     sppool_malloc(SIZE)
  #define FREE(PTR)        sppool_free(PTR)
  #define REALLOC(PTR, SZ) sppool_realloc(PTR, SZ)
  #define CALLOC(NM, SZ)   sppool_calloc(NM, SZ)
#endif

#define TEST(name, expr)   \
  do {                     \
    printf("[%s] ", name); \
    if (expr) {            \
      printf("OK\n");      \
      passed++;            \
    } else {               \
      printf("FAIL\n");    \
      failed++;            \
    }                      \
    total++;               \
  } while (0)

#define TEST_NULL(name, ptr)     TEST(name, (ptr) == NULL)
#define TEST_NOT_NULL(name, ptr) TEST(name, (ptr) != NULL)
#define TEST_MEM(name, ptr, value, size)            \
  do {                                              \
    int ok = 1;                                     \
    for (size_t i = 0; i < (size); i++) {           \
      if (((unsigned char*) (ptr))[i] != (value)) { \
        ok = 0;                                     \
        break;                                      \
      }                                             \
    }                                               \
    TEST(name, ok);                                 \
  } while (0)

int passed = 0, failed = 0, total = 0;

void test_basics() {
  printf("\n=== testing %s ===\n", ALLOC_NAME);

  void* p1 = MALLOC(100);
  TEST_NOT_NULL("malloc(100) returns not-NULL", p1);

  FREE(p1);
  TEST("free() is alright", 1);

  TEST_NULL("malloc(0) returns NULL", MALLOC(0));

  void* p2 = CALLOC(10, 20);
  TEST_NOT_NULL("calloc(10,20) returns not-NULL", p2);
  TEST_MEM("calloc zeros memory", p2, 0, 200);
  FREE(p2);
}

void test_realloc_basic() {
  int* p = (int*) MALLOC(5 * sizeof(int));
  TEST_NOT_NULL("malloc as realloc", p);

  for (int i = 0; i < 5; i++)
    p[i] = i;

  p = (int*) REALLOC(p, 10 * sizeof(int));
  TEST_NOT_NULL("realloc increases", p);

  int ok = 1;
  for (int i = 0; i < 5; i++) {
    if (p[i] != i)
      ok = 0;
  }
  TEST("realloc saves data", ok);

  FREE(p);
}

void test_multiple_allocs() {
  void* pointers[100];
  int ok = 1;

  for (int i = 0; i < 100; i++) {
    pointers[i] = MALLOC((i % 50) + 1);
    if (!pointers[i])
      ok = 0;
  }
  TEST("100 allocations", ok);

  for (int i = 0; i < 100; i++) {
    FREE(pointers[i]);
  }
  TEST("100 releases", 1);
}

void test_calloc_edge() {
  TEST_NULL("calloc(0, 100) -> NULL", CALLOC(0, 100));
  TEST_NULL("calloc(100, 0) -> NULL", CALLOC(100, 0));

  TEST_NULL("calloc overflow", CALLOC(SIZE_MAX / 2, 3));
}

int main() {
  printf("testing for %s\n", ALLOC_NAME);

  test_basics();
  test_realloc_basic();
  test_multiple_allocs();
  test_calloc_edge();

  printf("\n result: %d/%d tests are passed\n", passed, total);
  printf("%s: %s\n", ALLOC_NAME, failed == 0 ? "All tests are passed" : "sth wrong");

  return failed > 0 ? 1 : 0;
}