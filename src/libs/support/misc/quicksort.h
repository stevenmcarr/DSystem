#ifndef quicksort_h
#define quicksort_h

#include <libs/support/misc/general.h>

EXTERN(void, quicksort,(void *base,
			size_t nmemb,
			size_t size,
			int ((*compar)(const void *, const void *))));

#endif
