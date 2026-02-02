#ifndef CUSTOM_ALLOC_H
#define CUSTOM_ALLOC_H

#include <unistd.h>

void* sppool_malloc(size_t size);
void* sppool_realloc(void* ptr, size_t size);
void* sppool_calloc(size_t nmemb, size_t size);
void sppool_free(void* ptr);

#ifndef USE_STD_ALLOC
  #define malloc(SIZE)        sppool_malloc(SIZE)
  #define realloc(PTR, SIZE)  sppool_realloc(PTR, SIZE)
  #define calloc(NMEMB, SIZE) sppool_calloc(NMEMB, SIZE)
  #define free(PTR)           sppool_free(PTR)
#endif

#endif
