#ifndef CUSTOM_ALLOC_H
#define CUSTOM_ALLOC_H

#include <stdlib.h>

void* custom_malloc(size_t size);
void* custom_realloc(void* ptr, size_t size);
void* custom_calloc(size_t nmemb, size_t size);
void custom_free(void* ptr);

#ifndef USE_STD_ALLOC
  #define malloc(SIZE) custom_malloc(SIZE)
  #define realloc(PTR, SIZE) custom_realloc(PTR, SIZE)
  #define calloc(NMEMB, SIZE) custom_calloc(NMEMB, SIZE)
  #define free(PTR) custom_free(PTR)
#endif

#endif
