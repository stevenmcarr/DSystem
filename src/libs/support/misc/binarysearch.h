#ifndef binarysearch_h
#define binarysearch_h

#include <libs/support/misc/general.h>

EXTERN(void*, binarysearch,(const void *key,
			   const void *base,
			   size_t nmemb,
			   size_t size,
			   int (*compar)(const void *, const void *)));

#endif
