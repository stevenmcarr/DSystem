#include <stdlib.h>
#include <libs/support/misc/binarysearch.h>

/* Added to get around problem with DEC cxx and -xtaso_short */

void *binarysearch(const void *key,
		   const void *base,
		   size_t nmemb,
		   size_t size,
		   int (*compar)(const void *, const void *))
	      
{

  return(bsearch(key,base,nmemb,size,compar));
}

	 
