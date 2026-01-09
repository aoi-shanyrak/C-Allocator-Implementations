#ifndef CUSTOM_ALLOC_H
#define CUSTOM_ALLOC_H

#include <unistd.h>

void* kr_malloc(size_t size);
void* kr_realloc(void* ptr, size_t size);
void* kr_calloc(size_t nmemb, size_t size);
void kr_free(void* ptr);

#ifndef USE_STD_ALLOC
  #define malloc(SIZE)        kr_malloc(SIZE)
  #define realloc(PTR, SIZE)  kr_realloc(PTR, SIZE)
  #define calloc(NMEMB, SIZE) kr_calloc(NMEMB, SIZE)
  #define free(PTR)           kr_free(PTR)
#endif

#endif
