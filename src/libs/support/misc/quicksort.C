#include <stdlib.h>
#include <libs/support/misc/quicksort.h>

/* Added to get around problem with DEC cxx and -xtaso_short */


void quicksort(void *base,
	       size_t nmemb,
	       size_t size,
	       int ((*compar)(const void *, const void *)))
	      
{

  qsort(base,nmemb,size,compar);
}

	 
