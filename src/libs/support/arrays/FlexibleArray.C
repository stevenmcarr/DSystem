/* $Id: FlexibleArray.C,v 1.1 1997/06/25 15:13:37 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#include <include/bstring.h>

#include <libs/support/arrays/FlexibleArray.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/support/strings/rn_string.h>

#define ELTSIZE(f)	(f->size)
#define LENGTH(f)	(f->length)
#define STORAGETYPE(f)	(f->type)

#define BIG_HOLE  (ELTSIZE(f) == 1 ? 256 : 10)	/* size to make hole if it is too small */

#define START(f)	(f->flex_union.SH.start)
#define END(f)		(f->flex_union.SH.endtext)
#define HOLE(f)		(f->flex_union.SH.hole)
#define HOLESIZE(f)	(f->flex_union.SH.hole_size)

#define TABLE(f)	(f->flex_union.BL.table)
#define TABLESIZE(f)	(f->flex_union.BL.tablesize)
#define CURBLOCK(f)	(f->flex_union.BL.current)
#define BLOCKOFFSET(f)	(f->flex_union.BL.offset)

#define BLOCK(f, id)		(f->flex_union.BL.table[id].block_id)
#define BLOCKUSAGE(f, id)	(f->flex_union.BL.table[id].usage)
#define BLOCKINFO(f, id)	(f->flex_union.BL.table[id].info)

# define COREDUMP() abort()
# define ASSERT(bool)	{if ( !(bool) ) COREDUMP();} 

STATIC(void, flex_hole_to,(Flex *f, int hole_offset));

/****** epsilon *******/
void flex_insert_general(Flex *f, int start, int n)
{

	register char *oldstart;
	register int newholesize;
	register int nb;
	register int diff;

	LENGTH(f) += n;

	if (STORAGETYPE(f) == Single_hole)
	{/* increase hole size to amount needed, moving its location to start */
	
		flex_hole_to (f, start * ELTSIZE(f));
		/*
		 * Return if the hole is already large enough to satisfy 
 		 * the request.
		 */
		if (HOLESIZE(f) <= n * ELTSIZE(f))
		{

		/*
		 * Get enough for the request, plus "BIG_HOLE" extra bytes
		 * for later requests.
		 */
			newholesize = (n + BIG_HOLE) * ELTSIZE(f);
			oldstart = START(f);
			nb = (END(f) - START(f)) + newholesize + 1;
	
			START(f) = (char *) reget_mem((void*)oldstart, (unsigned) nb, "flex_insert()" );
	
			diff = START(f) - oldstart;  /* may be negative, by the way */
			HOLE(f) += diff;
			END(f) += diff;
	
			if (HOLE(f) <= END(f))
			bcopy( HOLE(f) + HOLESIZE(f),
			       HOLE(f) + newholesize, 
			       END(f) - HOLE(f) + 1 );
	
			HOLESIZE(f) = newholesize;
			bzero( HOLE(f), HOLESIZE(f) );
			if ( HOLE(f) >= END(f) )
				*END(f) = ' ';	/* magical blank after true end of buffer */
		}
		HOLE(f)     += n * ELTSIZE(f);
		HOLESIZE(f) -= n * ELTSIZE(f);
		END(f)	    += n * ELTSIZE(f);
	}

}

/*
 * flex_insert (f, start, n) -insert n undefined elements at start
 */
/*ARGSUSED*/
void flex_insert (Flex *f, int start, int n)
{

	register char *oldstart;
	register int newholesize;
	register int nb;
	register int diff;

	if (ELTSIZE(f) == 1)
	{
		LENGTH(f) += n;

		if (STORAGETYPE(f) == Single_hole)
		{/* increase hole size to amount needed, moving its location to start */
	
			flex_hole_to (f, start);
			/*
			 * Return if the hole is already large enough to satisfy 
	 		 * the request.
			 */
			if (HOLESIZE(f) <= n)
			{

			/*
			 * Get enough for the request, plus "BIG_HOLE" extra bytes
			 * for later requests.
			 */
				newholesize = (n + BIG_HOLE);
				oldstart = START(f);
				nb = (END(f) - START(f)) + newholesize + 1;
	
				START(f) = (char *) reget_mem((void*)oldstart, (unsigned) nb, "flex_insert()" );

				diff = START(f) - oldstart;  /* may be negative, by the way */
				HOLE(f) += diff;
				END(f) += diff;
	
				if (HOLE(f) <= END(f))
				bcopy( HOLE(f) + HOLESIZE(f),
				       HOLE(f) + newholesize, 
				       END(f) - HOLE(f) + 1 );
	
				HOLESIZE(f) = newholesize;
				bzero( HOLE(f), HOLESIZE(f) );
				if ( HOLE(f) >= END(f) )
					*END(f) = ' ';	/* magical blank after true end of buffer */
			}
			HOLE(f)     += n;
			HOLESIZE(f) -= n;
			END(f)	    += n;
		}
	}
	else
	   flex_insert_general(f, start, n);
}


/*
 * flex_delete (f, start, n) -   delete n elements beginning at start.
 */
void flex_delete (Flex *f, int start, int n)
{
	register int nb = n * ELTSIZE(f);

	LENGTH(f) -= n;

	if (STORAGETYPE(f) == Single_hole)
	{
		flex_hole_to ( f, start * ELTSIZE(f));
		HOLESIZE(f) += nb;
		END(f) -= nb;
	}
}

/****** One ******/

/*
 * flex_set_one (f, offset, elem) - set element at offset to elem
 */
void flex_set_one (Flex *f, int offset, char *elem)
{
	if (offset >= LENGTH(f))
	{
		flex_insert (f, LENGTH(f), offset - LENGTH(f) + 1);
	}

	if (STORAGETYPE(f) == Single_hole)
	{
		register char *cp = START(f) + offset * ELTSIZE(f);
		cp += cp < HOLE(f) ?
			0:
			HOLESIZE(f);
		bcopy (elem, cp, ELTSIZE(f));
	}
}

/*
 * flex_get_one (f, offset) - return a pointer to the element at offset
 */
char *flex_get_one (Flex *f, int offset)
{
	register char *cp = START(f) + offset * ELTSIZE(f);
	register char *buf = (char *) get_mem (ELTSIZE(f) + 1, "flex_get_one");

	if (STORAGETYPE(f) == Single_hole)
	{
		cp += cp < HOLE(f) ?
			0:
			HOLESIZE(f);
		bcopy (cp, buf, ELTSIZE(f));
	}
	return buf;
}

/*
 * flex1_get_one (f, offset) - return the 1 byte element at offset
 */
char flex1_get_one (Flex *f, int offset)
{
	register char *cp = START(f) + offset * ELTSIZE(f);

	if (STORAGETYPE(f) == Single_hole)
	{
		cp += cp < HOLE(f) ?
			0:
			HOLESIZE(f);
		return *cp;
	}
	return '\0';	/* make lint happy */
}

/*
 * flex_insert_one (f, start, elem) - insert elem at start
 */
void flex_insert_one (Flex *f, int start, char *elem)
{
	if (STORAGETYPE(f) == Single_hole)
	{
		flex_insert (f, start, 1);
		
		bcopy (elem, START(f) + start * ELTSIZE(f), ELTSIZE(f));
	}
}

/*
 * flex_delete_one (f, start, *buf) - delete one element at start into buf
 *	If buf = 0 allocate it.
 */
char *flex_delete_one (Flex *f, int start, char *buf)
{
	if (buf == 0)
	{
		buf = (char *)get_mem (ELTSIZE(f) + 1, "flex_delete_one");
	}

	LENGTH(f) -= 1;

	if (STORAGETYPE(f) == Single_hole)
	{
		flex_hole_to ( f, start * ELTSIZE(f));
		bcopy (START(f) + start * ELTSIZE(f) + HOLESIZE(f), buf, ELTSIZE(f));
		HOLESIZE(f) += ELTSIZE(f);
		END(f)	    -= ELTSIZE(f);
	}
	if( ELTSIZE(f) == 1 )  buf[ELTSIZE(f)] = '\0';
	return buf;
}

/******* Buffer ******/

/*
 * flex_set_buffer (f, start, n, buf) - set n elements at start from buf
 *
 */
void  flex_set_buffer (Flex *f, int start, int n, char *buf)
{
	register int i;

	if (STORAGETYPE(f) == Single_hole)
	{
		for (i = 0; i < n; i++)
		{
			flex_set_one (f, start + i, buf + (i * ELTSIZE(f)));
		}
	}
}

/*
 * flex_get_buffer (f, start, n, buf) - copy n elements from start to buf
 * 	if buf == 0, alloc buf. Return it regardless.
 */
char *flex_get_buffer (Flex *f, int start, int n, char *buf)
{
	register char *offset;
	register char *buf_temp;
	register int nb = n * ELTSIZE(f);

	if (buf == 0)
	{
		buf = (char *)get_mem(nb + 1, "flex_get_buffer");
	}
	if( ELTSIZE(f) == 1 )  buf[nb] = '\0';
	buf_temp = buf;

	if (STORAGETYPE(f) == Single_hole)
	{	
		offset = START(f) + start * ELTSIZE(f);

		/*
		 * Copy the elements that are before the hole
		 */
		if ( HOLE(f) > offset )
		{
			register int before_hole = min( HOLE(f)-offset, nb );

			bcopy(offset, buf_temp, before_hole );
			nb -= before_hole;
			buf_temp += before_hole;
			offset += before_hole;
		}

		/*
		 * Copy the elements that are after the hole
		 */
		if (nb > 0)
		{
			/* Skip over hole */
			offset += HOLESIZE(f);

			bcopy(offset, buf_temp, nb);
		}
	}
	return buf;
}

/*
 * flex_insert_buffer (f, start, n, buf) - insert n elements at start from buf
 */
void  flex_insert_buffer (Flex *f, int start, int n, char *buf)
{
	if (STORAGETYPE(f) == Single_hole)
	{
		flex_insert (f, start, n);

		bcopy (buf, START(f) + start * ELTSIZE(f), n * ELTSIZE(f));
	}
}

/*
 * flex_delete_buffer (f, start, n, buf) delete n elements at start to buf
 *  	if buf == 0, alloc buf. Return it regardless. (Is this flex_delete?)
 */
char *flex_delete_buffer (Flex *f, int start, int n, char *buf)
{
	register int nb = n * ELTSIZE(f);

	if (buf == 0)
	{
		buf = (char *)get_mem (nb + 1, "flex_delete_buffer");
	}

	LENGTH(f) -= n;

	if (STORAGETYPE(f) == Single_hole)
	{
		flex_hole_to ( f, start * ELTSIZE(f));
		bcopy (START(f) + start * ELTSIZE(f) + HOLESIZE(f), buf, nb);
		HOLESIZE(f) += nb;
		END(f) -= nb;
	}
	if( ELTSIZE(f) == 1)  buf[nb] = '\0';
	return buf;
}

Flex* flex_create (short element)
{
	register Flex *f = (Flex *)get_mem (sizeof (Flex), "flex_create");
	
	ELTSIZE(f)	= element;
	LENGTH(f)	= 0;
	STORAGETYPE(f)  = Single_hole;
	START(f)	= ssave("");
	END(f)		= START(f);
	HOLE(f)		= END(f) + 1;
	HOLESIZE(f)	= 0;
	
	return f;
}

void flex_destroy(Flex *f)
{
	free_mem ((void*)START(f));
	free_mem ((void*)f);
}

int flex_get_file (Flex *f, char *fname)
{
	register int fd;/* file descriptor for the file that we are writing */

	if (STORAGETYPE(f) == Single_hole)
	{

		register int nb = END(f)-START(f); /* number of bytes of text we're returning */

		fd = open( fname, O_TRUNC | O_CREAT | O_WRONLY, 0666 );
		if (fd < 0)
		{/* can't open */
			return -1;
		}
		flex_hole_to (f, flex_size(f));

		(void) write (fd, START(f), nb);
		(void) close (fd);
	}
	return 0;
}

int flex_insert_file(Flex *f, int start, char *fname)
{
	register int fd;		/* file descriptor for the file that we are reading */
	register int size;		/* size of the file we are reading */
	register int rsize = UNUSED;	/* the size we read */
	struct stat statbuf;		/* to check for existence and size of the file */

	if (STORAGETYPE(f) == Single_hole)
	{
	/* Throw all the available space into the hole. */

		if ( stat( fname, &statbuf ) )
		{/* can't stat the file */
			return 0;
		}
	
		size = (int)statbuf.st_size;

		if ( size == 0 )
		{/* stat says it's empty */
			return 0;
		}
		/* Read in the file */
		fd = open( fname, O_RDONLY, 666 );
		if (fd < 0)
		{/* can stat, but can't open */
			return 0;
		}
		/* Get more space if we need it. */
		flex_insert (f, start, size);

		rsize = read( fd, START(f) + start, size );
		(void) close( fd );

		if (rsize < 0)
		{
			rsize = 0;
		}
		if (size > rsize)
		{/* if we read less than expected, delete the trash at the end of buffer */
			(void) flex_delete (f, start + rsize, size - rsize);
		}

		/* Shouldn't need this, but why not. */
		bzero( HOLE(f), HOLESIZE(f) );
	}
	return rsize;
}

int flex_size (Flex *f)
{
	if (STORAGETYPE(f) == Single_hole)
		return (END(f) - START(f));
	else return 0;
}


int flex_length (Flex *f)
{
	return LENGTH(f);
}


int flex_index(Flex *f, int start, char *elt)
{
	register int size = flex_size(f);
	register char *curr;
	register int i;

	i = start;
	curr = START(f) + start*ELTSIZE(f);

	for (; curr < HOLE(f); curr+=ELTSIZE(f),i++)
		 if (! bcmp(curr,elt,ELTSIZE(f)) )
			return i;

	curr += HOLESIZE(f);

	for (;i < size; curr+=ELTSIZE(f),i++)
		if (! bcmp(curr,elt,ELTSIZE(f)) )
			return i;
	return -1;

}

int flex1_index(Flex *f, int start, char elt)
{
	register char *curr;
	register char *end;

	/*
	 * Look for match in [start..HOLE]
	 */
	curr = START(f) + start;
	end = HOLE(f);

	for(; curr < end; curr++ )
		 if (*curr == elt)
			return curr-START(f);

	/*
	 * Look for match in [HOLE+HOLESIZE..END+HOLESIZE]
	 */
	curr += HOLESIZE(f);
	end = END(f) + HOLESIZE(f);

	for (;curr < end; curr++)
		if (*curr == elt)
			return curr-HOLESIZE(f)-START(f);
	return -1;

}

int flex_rindex(Flex *f, int start, char *elt)
{
	register char *curr;
	register int i;

	curr = START(f)+HOLESIZE(f) + (start-1)*ELTSIZE(f);
	i = start-1;

	for (; curr >= HOLE(f)+HOLESIZE(f); curr-=ELTSIZE(f),--i)
		 if (! bcmp(curr,elt,ELTSIZE(f)) )
			return i;

	curr -= HOLESIZE(f);

	for (;i >= 0; curr-=ELTSIZE(f),--i)
		if (! bcmp(curr,elt,ELTSIZE(f)) )
			return i;
	return -1;
}

int flex1_rindex(Flex *f, int start, char elt)
{
	register char *curr;
	register char *end;

	/*
	 * Look for match in [HOLE+HOLESIZE..start-1]
	 */
	curr = START(f)+HOLESIZE(f) + start-1;
	end = HOLE(f)+HOLESIZE(f);

	for(; curr >= end; --curr )
		 if (*curr == elt)
			return curr-HOLESIZE(f)-START(f);

	/*
	 * Look for match in [0..HOLE]
	 */
	curr -= HOLESIZE(f);
	end = START(f);

	for (;curr >= end; --curr)
		if (*curr == elt)
			return curr-START(f);
	return -1;
}

int flex_count_occurrences(Flex *f, int start, int finish, char *elt)
{
	register char *curr;
	register char *end;
	register int occurs = 0;

	start  *= ELTSIZE(f);
	finish *= ELTSIZE(f);

	/*
	 * Look for match in [start..HOLE]
	 */
	curr = START(f) + start;
	end  = MIN( START(f)+finish, HOLE(f) );

	for(; curr < end; curr+=ELTSIZE(f) )
		 if (! bcmp(curr,elt,ELTSIZE(f)) )
			occurs++;

	/*
	 * Look for match in [HOLE+HOLESIZE..END+HOLESIZE]
	 */
	curr = MAX( HOLE(f), start  + START(f) );
	end  = MIN( END(f),  finish + START(f) );
	curr += HOLESIZE(f);
	end  += HOLESIZE(f);

	for (;curr < end; curr+=ELTSIZE(f))
		if (! bcmp(curr,elt,ELTSIZE(f)) )
			occurs++;
	return occurs;
}


int flex1_count_occurrences(Flex *f, int start, int finish, char elt)
{
	register char *curr;
	register char *end;
	register int occurs = 0;

	/*
	 * Look for match in [start..HOLE]
	 */
	curr = START(f) + start;
	end  = MIN(START(f) + finish, HOLE(f));

	for(; curr < end; curr++ )
		 if (*curr == elt)
			occurs++;

	/*
	 * Look for match in [HOLE+HOLESIZE..END+HOLESIZE]
	 */
	curr = MAX( HOLE(f), start  + START(f) );
	end  = MIN( END(f),  finish + START(f) );
	curr += HOLESIZE(f);
	end  += HOLESIZE(f);

	for (;curr < end; curr++)
		if (*curr == elt)
			occurs++;
	return occurs;
}


static void flex_hole_to (Flex *f, int hole_offset)
{
	register char *n_hole     = START(f) + hole_offset;
	register char *n_hole_end = n_hole     + HOLESIZE(f);
	register char *o_hole     = HOLE(f);
	register char *o_hole_end = o_hole     + HOLESIZE(f);
	register int shift	  = n_hole - o_hole;

	ASSERT( START(f) <= HOLE(f) );
	ASSERT( o_hole <= END(f) + 1);

	if (shift == 0)
	{
		/* nothing to do! */
		return;
	}

	if ( shift > 0 )
	{
		/* push stuff in "shift" region down into old hole, leaving space after it */
		bcopy(o_hole_end, o_hole, shift );
	}
	else	/* shift < 0 */
	{
		/* push stuff in "shift" region up into old hole, leaving space before it */
		bcopy( n_hole, n_hole_end, -shift );
	}

	bzero( n_hole, HOLESIZE(f) );
	HOLE(f) = n_hole;
}


/******* Flex ********/

Flex* flex_get_flex(Flex *f, int start, int count)
{
  die_with_message("flex_get_flex not implemented.");
  return f;
}
