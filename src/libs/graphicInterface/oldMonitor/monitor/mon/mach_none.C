/* $Id: mach_none.C,v 1.1 1997/06/25 14:53:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 		      mach_none.c			*/
		/*  Handles "no window system" machine specific calls.	*/
		/* 							*/
		/********************************************************/

#include <include/bstring.h>
#include <libs/support/misc/general.h>

#include <libs/graphicInterface/support/graphics/point.h>
#include <libs/graphicInterface/support/graphics/rect.h>
#include <libs/support/memMgmt/mem.h>

#include <libs/graphicInterface/oldMonitor/include/mon/gfx.h>
#include <libs/graphicInterface/oldMonitor/include/mon/event_codes.h>
#include <libs/graphicInterface/oldMonitor/include/mon/event.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/mach.h>

        /* LOCAL BITMAP DEFINITION */

typedef struct          {                       /* PRIVATE BITMAP STRUCTURE
        */
        Point           size;                   /* the size of the image
        */
        BITMAPM_UNIT    *data;                  /* the image data
        */
                        } PBitmap;

STATIC(void,		mm_blt,(PBitmap *src, PBitmap *dst, Rectangle srcArea, 
       Point dstOrigin, short fun));            /* general memory to memory bltter      */

STATIC(void,mm_blt_copy,(PBitmap *, PBitmap *, Rectangle , Point ));
#if 0  /** DEBUG 06 May 11.1 */
Color			default_foreground_color; /* foreground Color of root window	*/
Color			default_background_color; /* background Color of root window	*/
Color			default_border_color;	/* border color of root window		*/

Boolean			monochrome_screen;	/* true if should use only black & white*/
Pixmap			screen_pixmap;
#endif  /** 0 DEBUG 06 May 11.1 */

/* Create a new white bitmap of size 'size'.						*/
Bitmap
makeBitmap(Point size, char *who)
{
PBitmap			*pb;			/* the new bitmap			*/

	pb = (PBitmap *) get_mem(sizeof(PBitmap), "new bitmap by %s", who);
	pb->size = size;
	pb->data = (BITMAPM_UNIT *) get_mem(BITMAPM_BYTES_PER_IMAGE(size.x, size.y), "display (%d x %d) by %s", size.x, size.y, who);
	(void) bzero((char *) pb->data, BITMAPM_BYTES_PER_IMAGE(size.x, size.y));
	return ((Bitmap) pb);
}


/* Create a new initialized bitmap of size 'size'.					*/
Bitmap
makeBitmapFromData(Point size, BITMAPM_UNIT *data, char *who)
{
PBitmap			*pb;			/* the new bitmap			*/

	pb = (PBitmap *) get_mem(sizeof(PBitmap), "new data bitmap by %s", who);
	pb->size = size;
	pb->data = (BITMAPM_UNIT *) get_mem(BITMAPM_BYTES_PER_IMAGE(size.x, size.y), "initialized display (%d x %d) by %s", size.x, size.y, who);
	(void) bcopy((char *) data, (char *) pb->data, BITMAPM_BYTES_PER_IMAGE(size.x, size.y));
	return ((Bitmap) pb);
}


/* Free a created memory bitmap 'b'.							*/
void
freeBitmap(Bitmap b)
{
PBitmap			*pb = (PBitmap *) b;	/* the bitmap				*/

	free_mem((void*) pb->data);
	pb->data = 0;
	free_mem((void*) pb);
}


/* Get the size of the bitmap 'b'.							*/
Point
getBitmapSize(Bitmap b)
{
PBitmap			*pb = (PBitmap *) b;	/* the bitmap				*/

	if (b == NULL_BITMAP)
	{/* bogus request */
		die_with_message("getBitmapSize():  null source size requested");
	}
	return pb->size;
}


/* Get the data from the bitmap 'b'.							*/
void
getBitmapData(Bitmap b, BITMAPM_UNIT *data)
{
PBitmap			*pb = (PBitmap *) b;	/* the bitmap				*/

	if (b == NULL_BITMAP)
	{/* bogus request */
		die_with_message("getBitmapData():  null source data requested");
	}
	(void) bcopy((char *) pb->data, (char *) data, BITMAPM_BYTES_PER_IMAGE(pb->size.x, pb->size.y));
}


/* Set the data into the bitmap 'b'.							*/
void
setBitmapData(Bitmap b, BITMAPM_UNIT *data)
{
PBitmap			*pb = (PBitmap *) b;	/* the bitmap				*/

	if (b == NULL_BITMAP)
	{/* bogus request */
		die_with_message("setBitmapData():  null source data requested to be set");
	}
	(void) bcopy((char *) data, (char *) pb->data, BITMAPM_BYTES_PER_IMAGE(pb->size.x, pb->size.y));
}


/* Generalized low level graphics operation.						*/
void
BLTBitmap(Bitmap src, Bitmap dst, Rectangle srcArea, Point dstOrigin, 
          int fun, Boolean clip)
{
Point			save;			/* saved difference between origins	*/

	if (dst == NULL_BITMAP)
	{/* blt to a null destination */
		die_with_message("BLT():  null destination");
	}
	else if (clip)
	{/* clip against the destination */
		save = subPoint(srcArea.ul, dstOrigin);
		srcArea = interRect(srcArea, makeRectFromSize(save, getBitmapSize(dst)));
		dstOrigin = subPoint(srcArea.ul, save);
	}

	if (src == NULL_BITMAP)
	{/* adjust the graphics function */
		fun = (fun >> 2) * 5;	/* black source */
	}
	else if (clip)
	{/* clip against the source */
		save = subPoint(srcArea.ul, dstOrigin);
		srcArea = interRect(srcArea, makeRectFromSize(Origin, getBitmapSize(src)));
		dstOrigin = subPoint(srcArea.ul, save);
	}

	if (positiveArea(srcArea))
	{/* there is work to do */
		if (fun == BITMAP_COPY)
			mm_blt_copy((PBitmap *) src, (PBitmap *) dst, srcArea, dstOrigin);
		else if (fun != BITMAP_NOOP)
			mm_blt((PBitmap *) src, (PBitmap *) dst, srcArea, dstOrigin, fun);
	}
}


/* Create a brand new MouseCursor from the information provided.			*/
/*ARGSUSED*/
MouseCursor
makeMouseCursorFromData(Point size, 
                        BITMAPM_UNIT *single_data, short single_op, 
                        BITMAPM_UNIT *primary_data, short primary_op, 
                        BITMAPM_UNIT *secondary_data, short secondary_op, 
                        Point hot_spot, char *who)
{
	return (MouseCursor) 0;
}


/* Free a created MouseCursor 'mc'.							*/
/*ARGSUSED*/
void
freeMouseCursor(MouseCursor mc)
{
}


/* Dummy cursor play for computeBound handling.						*/
MouseCursor
CURSOR(MouseCursor New)
{
	return(New);
}


	/* "WINDOW" CALLS */

/* Start up the window system and events.						*/
/*ARGSUSED*/
void
startScreenEvents(ResizeFunc resize, RedrawFunc redraw, CharHandlerFunc handler)
{
	start_gfx();
}


/* Stop the flow of events from the window.						*/
void
stopScreenEvents()
{
	finish_gfx();
}


/* Return true if there is an event waiting.						*/
Boolean
readyScreenEvent()
{
	return false;
}


/* Get an important mouse/keyboard event into mon_event.				*/
void
getScreenEvent()
{
}


/* Flush the queued screen events.							*/
Boolean
flushScreenEvents()
{
	return true;
}


/* Handle a fully clipped memory to memory (or null to memory) BLT.			*/
static
void
mm_blt(PBitmap *src, PBitmap *dst, Rectangle srcArea, Point dstOrigin, short fun)
{
register BITMAPM_UNIT	*current_dst_ptr;	/* the destination pointer		*/
register BITMAPM_UNIT	*current_src_ptr;	/* the source pointer			*/
register BITMAPM_UNIT	*start_dst_ptr;		/* the initial destination pointer	*/
register short		src_shift;		/* the src pixel shift amount		*/
register BITMAPM_UNIT	tmp;			/* temporary for shifting		*/
register int		src_other_shift;	/* the shift in the other direction	*/
register BITMAPM_UNIT	*end_dst_ptr;		/* the final destination pointer	*/
register BITMAPM_UNIT	*start_src_ptr;		/* the initial source pointer		*/
register int		y;			/* the current source coordinate	*/
register Boolean	dir;			/* direction to do the blt (true = fwd)	*/
register BITMAPM_UNIT	start_dst_mask;		/* the clobbered bits at begin. of line	*/
register BITMAPM_UNIT	end_dst_mask;		/* the clobbered bist at end of line	*/
register int		increment_dst_ptr;	/* the increment of the destination ptr	*/
register int		increment_src_ptr;	/* the increment of the source ptr	*/
register BITMAPM_UNIT	start_dst_unit;		/* the saved value of the starting unit	*/
register BITMAPM_UNIT	end_dst_unit;		/* the saved value of the ending unit	*/
Point			srcSize;		/* the size of the source area		*/
Rectangle		dstArea;		/* the destination area			*/
static	PBitmap		null_pbitmap = {	/* the empty PBitmap for null source	*/
				{ 0, 0 },
				(BITMAPM_UNIT *) 0
			};

	if (src == (PBitmap *) 0)
	{/* sanitize the source for the algorithm */
		src = &null_pbitmap;
	}

	/* dstArea = relocRect(srcArea, dstOrigin); */
	   dstArea.lr.x = srcArea.lr.x + dstOrigin.x - srcArea.ul.x;
	   dstArea.lr.y = srcArea.lr.y + dstOrigin.y - srcArea.ul.y;
	   dstArea.ul   = dstOrigin;

	/* srcSize = sizeRect(srcArea); */
	   srcSize.x = srcArea.lr.x - srcArea.ul.x + 1;
	   srcSize.y = srcArea.lr.y - srcArea.ul.y + 1;

	start_dst_mask = (BITMAPM_UNIT) ((1 << (BITMAPM_BITS_PER_UNIT - (dstArea.ul.x & 0xF))) - 1);
	end_dst_mask   = (BITMAPM_UNIT) ((1 << (BITMAPM_BITS_PER_UNIT - 1 - (dstArea.lr.x &0xF))) - 1);

	if (srcArea.ul.y == dstArea.ul.y)
	{/* the direction really matters on the relative x values */
		dir = (Boolean)(srcArea.ul.x > dstArea.ul.x);
	}
	else
	{/* the direction matters on the y values */
		dir = (Boolean)(srcArea.ul.y > dstArea.ul.y);
	}

	if (dir)
	{ /* set up for forward direction */
		increment_dst_ptr = BITMAPM_UNITS_PER_LINE(dst->size.x);
		start_dst_ptr     = dst->data + dstArea.ul.y * BITMAPM_UNITS_PER_LINE(dst->size.x) 
					+ (dstArea.ul.x  >> 4);
		src_shift	  = (srcArea.ul.x &0xF) - (dstArea.ul.x & 0xF);
		increment_src_ptr = BITMAPM_UNITS_PER_LINE(src->size.x);
		start_src_ptr     = src->data + srcArea.ul.y * BITMAPM_UNITS_PER_LINE(src->size.x) 
					+ (srcArea.ul.x >> 4);
	}
	else
	{ /* set up for the reverse direction */
		increment_dst_ptr = -BITMAPM_UNITS_PER_LINE(dst->size.x);
		start_dst_ptr     = dst->data + dstArea.lr.y * BITMAPM_UNITS_PER_LINE(dst->size.x) 
					+ (dstArea.ul.x >> 4);
		src_shift 	  = (srcArea.lr.x & 0xF) - (dstArea.lr.x & 0xF);
		increment_src_ptr = -BITMAPM_UNITS_PER_LINE(src->size.x);
		start_src_ptr     = src->data + srcArea.lr.y * BITMAPM_UNITS_PER_LINE(src->size.x) 
					+ (srcArea.lr.x >> 4);
	}

	if (src_shift < 0)
	{/* adjust the source shift (we miscalculated) */
		src_shift += BITMAPM_BITS_PER_UNIT;
		start_src_ptr--;
	}
	src_other_shift = BITMAPM_BITS_PER_UNIT - src_shift;
	end_dst_ptr = start_dst_ptr + (dstArea.lr.x >> 4) - (dstArea.ul.x >> 4);

	switch (fun)
	{/* set up the result */
		case BITMAP_CLR:		/* make dest white			*/
#ifdef TRACING
	(void) fprintf(stderr, "CLR\n");
#endif 
			for (y = srcSize.y; y; y--)
			{/* transfer a line of data */
				/* save the ends which will be partially clobbered */
					start_dst_unit = *start_dst_ptr;
					end_dst_unit   = *end_dst_ptr;

				/* do the bulk of the work */
					for (
						current_dst_ptr = (dir) ? start_dst_ptr : end_dst_ptr;
						(start_dst_ptr <= current_dst_ptr) && (current_dst_ptr <= end_dst_ptr);
						(dir) ? current_dst_ptr++ : current_dst_ptr--
					)
					{/* do a BITMAPM_UNIT in the destination */
						*current_dst_ptr = 0;
					}

				/* fix the partially clobbered ends */
					*start_dst_ptr = (~start_dst_mask & start_dst_unit) | ( start_dst_mask & *start_dst_ptr);
					*end_dst_ptr   = ( end_dst_mask   & end_dst_unit  ) | (~end_dst_mask   & *end_dst_ptr);

				/* get ready for the next line */
					start_dst_ptr += increment_dst_ptr;
					end_dst_ptr   += increment_dst_ptr;
			}
			break;
		case BITMAP_NOR:		/* nor of src and dest			*/
#ifdef TRACING
	(void) fprintf(stderr, "NOR\n");
#endif 
			for (y = srcSize.y; y; y--)
			{/* transfer a line of data */
				/* save the ends which will be partially clobbered */
					start_dst_unit = *start_dst_ptr;
					end_dst_unit   = *end_dst_ptr;

				/* do the bulk of the work */
					current_src_ptr = start_src_ptr;
					for (
						current_dst_ptr = (dir) ? start_dst_ptr : end_dst_ptr;
						(start_dst_ptr <= current_dst_ptr) && (current_dst_ptr <= end_dst_ptr);
						(dir) ? current_dst_ptr++ : current_dst_ptr--
					)
					{/* do a BITMAPM_UNIT in the destination */
						*current_dst_ptr = ~*current_dst_ptr & ~((current_src_ptr[0] << src_shift) | (current_src_ptr[1]>>src_other_shift));
						(dir) ? current_src_ptr++ : current_src_ptr--;
					}

				/* fix the partially clobbered ends */
					*start_dst_ptr = (~start_dst_mask & start_dst_unit) | ( start_dst_mask & *start_dst_ptr);
					*end_dst_ptr   = ( end_dst_mask   & end_dst_unit  ) | (~end_dst_mask   & *end_dst_ptr);

				/* get ready for the next line */
					start_dst_ptr += increment_dst_ptr;
					end_dst_ptr   += increment_dst_ptr;
					start_src_ptr += increment_src_ptr;
			}
			break;
		case BITMAP_INV_AND:	/* not src and dest			*/
#ifdef TRACING
	(void) fprintf(stderr, "IAND\n");
#endif
			for (y = srcSize.y; y; y--)
			{/* transfer a line of data */
				/* save the ends which will be partially clobbered */
					start_dst_unit = *start_dst_ptr;
					end_dst_unit   = *end_dst_ptr;

				/* do the bulk of the work */
					current_src_ptr = start_src_ptr;
					for (
						current_dst_ptr = (dir) ? start_dst_ptr : end_dst_ptr;
						(start_dst_ptr <= current_dst_ptr) && (current_dst_ptr <= end_dst_ptr);
						(dir) ? current_dst_ptr++ : current_dst_ptr--
					)
					{/* do a BITMAPM_UNIT in the destination */
						*current_dst_ptr &= ~((current_src_ptr[0] << src_shift) | (current_src_ptr[1] >> (src_other_shift)));
						(dir) ? current_src_ptr++ : current_src_ptr--;
					}

				/* fix the partially clobbered ends */
					*start_dst_ptr = (~start_dst_mask & start_dst_unit) | ( start_dst_mask & *start_dst_ptr);
					*end_dst_ptr   = ( end_dst_mask   & end_dst_unit  ) | (~end_dst_mask   & *end_dst_ptr);

				/* get ready for the next line */
					start_dst_ptr += increment_dst_ptr;
					end_dst_ptr   += increment_dst_ptr;
					start_src_ptr += increment_src_ptr;
			}
			break;
		case BITMAP_INV_COPY:	/* copy from src to dest inverting	*/
#ifdef TRACING
	(void) fprintf(stderr, "ICOPY\n");
#endif
			for (y = srcSize.y; y; y--)
			{/* transfer a line of data */
				/* save the ends which will be partially clobbered */
					start_dst_unit = *start_dst_ptr;
					end_dst_unit   = *end_dst_ptr;

				/* do the bulk of the work */
					current_src_ptr = start_src_ptr;
					for (
						current_dst_ptr = (dir) ? start_dst_ptr : end_dst_ptr;
						(start_dst_ptr <= current_dst_ptr) && (current_dst_ptr <= end_dst_ptr);
						(dir) ? current_dst_ptr++ : current_dst_ptr--
					)
					{/* do a BITMAPM_UNIT in the destination */
						*current_dst_ptr = ~((current_src_ptr[0] << src_shift) | (current_src_ptr[1] >> src_other_shift));
						(dir) ? current_src_ptr++ : current_src_ptr--;
					}

				/* fix the partially clobbered ends */
					*start_dst_ptr = (~start_dst_mask & start_dst_unit) | ( start_dst_mask & *start_dst_ptr);
					*end_dst_ptr   = ( end_dst_mask   & end_dst_unit  ) | (~end_dst_mask   & *end_dst_ptr);

				/* get ready for the next line */
					start_dst_ptr += increment_dst_ptr;
					end_dst_ptr   += increment_dst_ptr;
					start_src_ptr += increment_src_ptr;
			}
			break;
		case BITMAP_AND_INV:	/* and of src and not dest		*/
#ifdef TRACING
	(void) fprintf(stderr, "ANDI\n");
#endif
			for (y = srcSize.y; y; y--)
			{/* transfer a line of data */
				/* save the ends which will be partially clobbered */
					start_dst_unit = *start_dst_ptr;
					end_dst_unit   = *end_dst_ptr;

				/* do the bulk of the work */
					current_src_ptr = start_src_ptr;
					for (
						current_dst_ptr = (dir) ? start_dst_ptr : end_dst_ptr;
						(start_dst_ptr <= current_dst_ptr) && (current_dst_ptr <= end_dst_ptr);
						(dir) ? current_dst_ptr++ : current_dst_ptr--
					)
					{/* do a BITMAPM_UNIT in the destination */
						*current_dst_ptr = ~*current_dst_ptr & ((current_src_ptr[0] << src_shift) | (current_src_ptr[1] >> src_other_shift));
						(dir) ? current_src_ptr++ : current_src_ptr--;
					}

				/* fix the partially clobbered ends */
					*start_dst_ptr = (~start_dst_mask & start_dst_unit) | ( start_dst_mask & *start_dst_ptr);
					*end_dst_ptr   = ( end_dst_mask   & end_dst_unit  ) | (~end_dst_mask   & *end_dst_ptr);

				/* get ready for the next line */
					start_dst_ptr += increment_dst_ptr;
					end_dst_ptr   += increment_dst_ptr;
					start_src_ptr += increment_src_ptr;
			}
			break;
		case BITMAP_INV:		/* inverse of destination		*/
#ifdef TRACING
	(void) fprintf(stderr, "INV\n");
#endif 
			for (y = srcSize.y; y; y--)
			{/* transfer a line of data */
				/* save the ends which will be partially clobbered */
					start_dst_unit = *start_dst_ptr;
					end_dst_unit   = *end_dst_ptr;

				/* do the bulk of the work */
					for (
						current_dst_ptr = (dir) ? start_dst_ptr : end_dst_ptr;
						(start_dst_ptr <= current_dst_ptr) && (current_dst_ptr <= end_dst_ptr);
						(dir) ? current_dst_ptr++ : current_dst_ptr--
					)
					{/* do a BITMAPM_UNIT in the destination */
						*current_dst_ptr = ~*current_dst_ptr;
					}

				/* fix the partially clobbered ends */
					*start_dst_ptr = (~start_dst_mask & start_dst_unit) | ( start_dst_mask & *start_dst_ptr);
					*end_dst_ptr   = ( end_dst_mask   & end_dst_unit  ) | (~end_dst_mask   & *end_dst_ptr);

				/* get ready for the next line */
					start_dst_ptr += increment_dst_ptr;
					end_dst_ptr   += increment_dst_ptr;
			}
			break;
		case BITMAP_XOR:		/* exclusive or of src and dest		*/
#ifdef TRACING
	(void) fprintf(stderr, "XOR\n");
#endif
			for (y = srcSize.y; y; y--)
			{/* transfer a line of data */
				/* save the ends which will be partially clobbered */
					start_dst_unit = *start_dst_ptr;
					end_dst_unit   = *end_dst_ptr;

				/* do the bulk of the work */
					current_src_ptr = start_src_ptr;
					for (
						current_dst_ptr = (dir) ? start_dst_ptr : end_dst_ptr;
						(start_dst_ptr <= current_dst_ptr) && (current_dst_ptr <= end_dst_ptr);
						(dir) ? current_dst_ptr++ : current_dst_ptr--
					)
					{/* do a BITMAPM_UNIT in the destination */
						*current_dst_ptr ^= ((current_src_ptr[0] << src_shift) | (current_src_ptr[1] >> src_other_shift));
						(dir) ? current_src_ptr++ : current_src_ptr--;
					}

				/* fix the partially clobbered ends */
					*start_dst_ptr = (~start_dst_mask & start_dst_unit) | ( start_dst_mask & *start_dst_ptr);
					*end_dst_ptr   = ( end_dst_mask   & end_dst_unit  ) | (~end_dst_mask   & *end_dst_ptr);

				/* get ready for the next line */
					start_dst_ptr += increment_dst_ptr;
					end_dst_ptr   += increment_dst_ptr;
					start_src_ptr += increment_src_ptr;
			}
			break;
		case BITMAP_NAND:		/* nand of src and dest			*/
#ifdef TRACING
	(void) fprintf(stderr, "NAND\n");
#endif
			for (y = srcSize.y; y; y--)
			{/* transfer a line of data */
				/* save the ends which will be partially clobbered */
					start_dst_unit = *start_dst_ptr;
					end_dst_unit   = *end_dst_ptr;

				/* do the bulk of the work */
					current_src_ptr = start_src_ptr;
					for (
						current_dst_ptr = (dir) ? start_dst_ptr : end_dst_ptr;
						(start_dst_ptr <= current_dst_ptr) && (current_dst_ptr <= end_dst_ptr);
						(dir) ? current_dst_ptr++ : current_dst_ptr--
					)
					{/* do a BITMAPM_UNIT in the destination */
						*current_dst_ptr = ~*current_dst_ptr | ~((current_src_ptr[0] << src_shift) | (current_src_ptr[1]>>src_other_shift));
						(dir) ? current_src_ptr++ : current_src_ptr--;
					}

				/* fix the partially clobbered ends */
					*start_dst_ptr = (~start_dst_mask & start_dst_unit) | ( start_dst_mask & *start_dst_ptr);
					*end_dst_ptr   = ( end_dst_mask   & end_dst_unit  ) | (~end_dst_mask   & *end_dst_ptr);

				/* get ready for the next line */
					start_dst_ptr += increment_dst_ptr;
					end_dst_ptr   += increment_dst_ptr;
					start_src_ptr += increment_src_ptr;
			}
			break;
		case BITMAP_AND:		/* and of src and dest			*/
#ifdef TRACING
	(void) fprintf(stderr, "AND\n");
#endif
			for (y = srcSize.y; y; y--)
			{/* transfer a line of data */
				/* save the ends which will be partially clobbered */
					start_dst_unit = *start_dst_ptr;
					end_dst_unit   = *end_dst_ptr;

				/* do the bulk of the work */
					current_src_ptr = start_src_ptr;
					for (
						current_dst_ptr = (dir) ? start_dst_ptr : end_dst_ptr;
						(start_dst_ptr <= current_dst_ptr) && (current_dst_ptr <= end_dst_ptr);
						(dir) ? current_dst_ptr++ : current_dst_ptr--
					)
					{/* do a BITMAPM_UNIT in the destination */
						*current_dst_ptr &= ((current_src_ptr[0] << src_shift) | (current_src_ptr[1] >> src_other_shift));
						(dir) ? current_src_ptr++ : current_src_ptr--;
					}

				/* fix the partially clobbered ends */
					*start_dst_ptr = (~start_dst_mask & start_dst_unit) | ( start_dst_mask & *start_dst_ptr);
					*end_dst_ptr   = ( end_dst_mask   & end_dst_unit  ) | (~end_dst_mask   & *end_dst_ptr);

				/* get ready for the next line */
					start_dst_ptr += increment_dst_ptr;
					end_dst_ptr   += increment_dst_ptr;
					start_src_ptr += increment_src_ptr;
			}
			break;
		case BITMAP_EQ:		/* eq of scr and dest			*/
#ifdef TRACING
	(void) fprintf(stderr, "EQ\n");
#endif
			for (y = srcSize.y; y; y--)
			{/* transfer a line of data */
				/* save the ends which will be partially clobbered */
					start_dst_unit = *start_dst_ptr;
					end_dst_unit   = *end_dst_ptr;

				/* do the bulk of the work */
					current_src_ptr = start_src_ptr;
					for (
						current_dst_ptr = (dir) ? start_dst_ptr : end_dst_ptr;
						(start_dst_ptr <= current_dst_ptr) && (current_dst_ptr <= end_dst_ptr);
						(dir) ? current_dst_ptr++ : current_dst_ptr--
					)
					{/* do a BITMAPM_UNIT in the destination */
						*current_dst_ptr ^= ~((current_src_ptr[0] << src_shift) | (current_src_ptr[1] >> src_other_shift));
						(dir) ? current_src_ptr++ : current_src_ptr--;
					}

				/* fix the partially clobbered ends */
					*start_dst_ptr = (~start_dst_mask & start_dst_unit) | ( start_dst_mask & *start_dst_ptr);
					*end_dst_ptr   = ( end_dst_mask   & end_dst_unit  ) | (~end_dst_mask   & *end_dst_ptr);

				/* get ready for the next line */
					start_dst_ptr += increment_dst_ptr;
					end_dst_ptr   += increment_dst_ptr;
					start_src_ptr += increment_src_ptr;
			}
			break;
		case BITMAP_NOOP:		/* do nothing				*/
#ifdef TRACING
	(void) fprintf(stderr, "NOOP\n");
#endif
			break;
		case BITMAP_INV_OR:	/* not src or dest			*/
#ifdef TRACING
	(void) fprintf(stderr, "IOR\n");
#endif
			for (y = srcSize.y; y; y--)
			{/* transfer a line of data */
				/* save the ends which will be partially clobbered */
					start_dst_unit = *start_dst_ptr;
					end_dst_unit   = *end_dst_ptr;

				/* do the bulk of the work */
					current_src_ptr = start_src_ptr;
					for (
						current_dst_ptr = (dir) ? start_dst_ptr : end_dst_ptr;
						(start_dst_ptr <= current_dst_ptr) && (current_dst_ptr <= end_dst_ptr);
						(dir) ? current_dst_ptr++ : current_dst_ptr--
					)
					{/* do a BITMAPM_UNIT in the destination */
						*current_dst_ptr |= ~((current_src_ptr[0] << src_shift) | (current_src_ptr[1] >> src_other_shift));
						(dir) ? current_src_ptr++ : current_src_ptr--;
					}

				/* fix the partially clobbered ends */
					*start_dst_ptr = (~start_dst_mask & start_dst_unit) | ( start_dst_mask & *start_dst_ptr);
					*end_dst_ptr   = ( end_dst_mask   & end_dst_unit  ) | (~end_dst_mask   & *end_dst_ptr);

				/* get ready for the next line */
					start_dst_ptr += increment_dst_ptr;
					end_dst_ptr   += increment_dst_ptr;
					start_src_ptr += increment_src_ptr;
			}
			break;

		/* THIS CASE SHOULD NEVER GET EXECUTED.  WE THINK WE HAVE MODIFIED ALL	*/
		/* THE CALLS TO mm_blt() TO CHECK THE FUNCTION AND CALL THE ROUTINE	*/
		/* mm_blt_copy() INSTEAD.  THAT ROUTINE SPECIAL CASES THE COMMON ONES	*/
		/* AND IS OVERALL MORE EFFICIENT!	- kdc				*/
		case BITMAP_COPY:		/* copy the image from src to dest	*/
#ifdef TRACING
	(void) fprintf(stderr, "COPY(blt)\n");
#endif
			if (dir)
			{/* generalized forward case */
				for (y = srcSize.y; y; y--)
				{/* blt a line */
					/* save the ends which will be partially clobbered */
						start_dst_unit = *start_dst_ptr;
						end_dst_unit   = *end_dst_ptr;

					current_src_ptr = start_src_ptr;
					for (current_dst_ptr = start_dst_ptr; current_dst_ptr <= end_dst_ptr; current_dst_ptr++)
					{/* do a BITMAPM_UNIT in the destination */
						tmp = *current_src_ptr++ << src_shift;
						*current_dst_ptr = tmp | (*current_src_ptr >> src_other_shift);
					}

					/* fix the partially clobbered ends */
						*start_dst_ptr = (~start_dst_mask & start_dst_unit) | ( start_dst_mask & *start_dst_ptr);
						*end_dst_ptr   = ( end_dst_mask   & end_dst_unit  ) | (~end_dst_mask   & *end_dst_ptr);

					/* get ready for the next line */
						start_dst_ptr += increment_dst_ptr;
						end_dst_ptr   += increment_dst_ptr;
						start_src_ptr += increment_src_ptr;
				}
			}
			else
			{ /* generalized backwards case */
				for (y = srcSize.y; y; y--)
				{/* blt a line */
					/* save the ends which will be partially clobbered */
						start_dst_unit = *start_dst_ptr;
						end_dst_unit   = *end_dst_ptr;

					current_src_ptr = start_src_ptr + 1;
					for (current_dst_ptr = end_dst_ptr;	start_dst_ptr <= current_dst_ptr; current_dst_ptr--)
					{/* do a BITMAPM_UNIT in the destination */
						tmp = *current_src_ptr -- >> src_other_shift;
						*current_dst_ptr = tmp | (*current_src_ptr << src_shift);
					}

					/* fix the partially clobbered ends */
						*start_dst_ptr = (~start_dst_mask & start_dst_unit) | ( start_dst_mask & *start_dst_ptr);
						*end_dst_ptr   = ( end_dst_mask   & end_dst_unit  ) | (~end_dst_mask   & *end_dst_ptr);

					/* get ready for the next line */
						start_dst_ptr += increment_dst_ptr;
						end_dst_ptr   += increment_dst_ptr;
						start_src_ptr += increment_src_ptr;
				}
			}
			break;
		case BITMAP_OR_INV:	/* or of src and not dest		*/
#ifdef TRACING
	(void) fprintf(stderr, "ORI\n");
#endif
			for (y = srcSize.y; y; y--)
			{/* transfer a line of data */
				/* save the ends which will be partially clobbered */
					start_dst_unit = *start_dst_ptr;
					end_dst_unit   = *end_dst_ptr;

				/* do the bulk of the work */
					current_src_ptr = start_src_ptr;
					for (
						current_dst_ptr = (dir) ? start_dst_ptr : end_dst_ptr;
						(start_dst_ptr <= current_dst_ptr) && (current_dst_ptr <= end_dst_ptr);
						(dir) ? current_dst_ptr++ : current_dst_ptr--
					)
					{/* do a BITMAPM_UNIT in the destination */
						*current_dst_ptr = ~*current_dst_ptr | ((current_src_ptr[0] << src_shift) | (current_src_ptr[1] >> src_other_shift));
						(dir) ? current_src_ptr++ : current_src_ptr--;
					}

				/* fix the partially clobbered ends */
					*start_dst_ptr = (~start_dst_mask & start_dst_unit) | ( start_dst_mask & *start_dst_ptr);
					*end_dst_ptr   = ( end_dst_mask   & end_dst_unit  ) | (~end_dst_mask   & *end_dst_ptr);

				/* get ready for the next line */
					start_dst_ptr += increment_dst_ptr;
					end_dst_ptr   += increment_dst_ptr;
					start_src_ptr += increment_src_ptr;
			}
			break;
		case BITMAP_OR:		/* or of src and dest			*/
#ifdef TRACING
	(void) fprintf(stderr, "OR\n");
#endif
			for (y = srcSize.y; y; y--)
			{/* transfer a line of data */
				/* save the ends which will be partially clobbered */
					start_dst_unit = *start_dst_ptr;
					end_dst_unit   = *end_dst_ptr;

				/* do the bulk of the work */
					current_src_ptr = start_src_ptr;
					for (
						current_dst_ptr = (dir) ? start_dst_ptr : end_dst_ptr;
						(start_dst_ptr <= current_dst_ptr) && (current_dst_ptr <= end_dst_ptr);
						(dir) ? current_dst_ptr++ : current_dst_ptr--
					)
					{/* do a BITMAPM_UNIT in the destination */
						*current_dst_ptr |= ((current_src_ptr[0] << src_shift) | (current_src_ptr[1] >> src_other_shift));
						(dir) ? current_src_ptr++ : current_src_ptr--;
					}

				/* fix the partially clobbered ends */
					*start_dst_ptr = (~start_dst_mask & start_dst_unit) | ( start_dst_mask & *start_dst_ptr);
					*end_dst_ptr   = ( end_dst_mask   & end_dst_unit  ) | (~end_dst_mask   & *end_dst_ptr);

				/* get ready for the next line */
					start_dst_ptr += increment_dst_ptr;
					end_dst_ptr   += increment_dst_ptr;
					start_src_ptr += increment_src_ptr;
			}
			break;
		case BITMAP_SET:		/* make dest black			*/
#ifdef TRACING
	(void) fprintf(stderr, "SET\n");
#endif
			tmp = ~0;
			for (y = srcSize.y; y; y--)
			{/* transfer a line of data */
				/* save the ends which will be partially clobbered */
					start_dst_unit = *start_dst_ptr;
					end_dst_unit   = *end_dst_ptr;

				/* do the bulk of the work */
					for (
						current_dst_ptr = (dir) ? start_dst_ptr : end_dst_ptr;
						(start_dst_ptr <= current_dst_ptr) && (current_dst_ptr <= end_dst_ptr);
						(dir) ? current_dst_ptr++ : current_dst_ptr--
					)
					{/* do a BITMAPM_UNIT in the destination */
						*current_dst_ptr = tmp /* ~0 */;
					}

				/* fix the partially clobbered ends */
					*start_dst_ptr = (~start_dst_mask & start_dst_unit) | ( start_dst_mask & *start_dst_ptr);
					*end_dst_ptr   = ( end_dst_mask   & end_dst_unit  ) | (~end_dst_mask   & *end_dst_ptr);

				/* get ready for the next line */
					start_dst_ptr += increment_dst_ptr;
					end_dst_ptr   += increment_dst_ptr;
			}
			break;
	}
}


/* Special case of mm_blt() for BITMAP_COPY */
static
void
mm_blt_copy(PBitmap *src, PBitmap *dst, Rectangle srcArea, Point dstOrigin)
{
register BITMAPM_UNIT	*current_dst_ptr;	/* the destination pointer		*/
register BITMAPM_UNIT	*current_src_ptr;	/* the source pointer			*/
register BITMAPM_UNIT	*start_dst_ptr;		/* the initial destination pointer	*/
register short		src_shift;		/* the src pixel shift amount		*/
register BITMAPM_UNIT	tmp;			/* temporary for shifting		*/
register int		src_other_shift;	/* the shift in the other direction	*/
register BITMAPM_UNIT	*end_dst_ptr;		/* the final destination pointer	*/
register BITMAPM_UNIT	*start_src_ptr;		/* the initial source pointer		*/
register int		y;			/* the current source coordinate	*/
register Boolean	dir;			/* direction to do the blt (true = fwd)	*/
register BITMAPM_UNIT	start_dst_mask;		/* the clobbered bits at begin. of line	*/
register BITMAPM_UNIT	end_dst_mask;		/* the clobbered bist at end of line	*/
register int		increment_dst_ptr;	/* the increment of the destination ptr	*/
register int		increment_src_ptr;	/* the increment of the source ptr	*/
register BITMAPM_UNIT	start_dst_unit;		/* the saved value of the starting unit	*/
register BITMAPM_UNIT	end_dst_unit;		/* the saved value of the ending unit	*/
Point			srcSize;		/* the size of the source area		*/
Rectangle		dstArea;		/* the destination area			*/
static	PBitmap		null_pbitmap = {	/* the empty PBitmap for null source	*/
				{ 0, 0 },
				(BITMAPM_UNIT *) 0
			};

#ifdef TRACING
(void) fprintf(stderr, "COPY(sa)");
#endif

	if (src == (PBitmap *) 0)
	{/* sanitize the source for the algorithm */
		src = &null_pbitmap;
	}


	/* dstArea = relocRect(srcArea, dstOrigin); */
	   dstArea.lr.x = srcArea.lr.x + dstOrigin.x - srcArea.ul.x;
	   dstArea.lr.y = srcArea.lr.y + dstOrigin.y - srcArea.ul.y;
	   dstArea.ul   = dstOrigin;

	/* srcSize = sizeRect(srcArea); */
	   srcSize.x = srcArea.lr.x - srcArea.ul.x + 1;
	   srcSize.y = srcArea.lr.y - srcArea.ul.y + 1;

	start_dst_mask = (BITMAPM_UNIT) ((1 << (BITMAPM_BITS_PER_UNIT - (dstArea.ul.x & 0xF))) - 1);
	end_dst_mask   = (BITMAPM_UNIT) ((1 << (BITMAPM_BITS_PER_UNIT - 1 - (dstArea.lr.x & 0xF))) - 1);

	if (srcArea.ul.y == dstArea.ul.y)
	{/* the direction really matters on the relative x values */
		dir = (Boolean)(srcArea.ul.x > dstArea.ul.x);
	}
	else
	{/* the direction matters on the y values */
		dir = (Boolean)(srcArea.ul.y > dstArea.ul.y);
	}

	if (dir)
	{ /* set up for forward direction */
		increment_dst_ptr = BITMAPM_UNITS_PER_LINE(dst->size.x);
		start_dst_ptr     = dst->data + dstArea.ul.y * BITMAPM_UNITS_PER_LINE(dst->size.x) 
					+ (dstArea.ul.x >> 4);
		src_shift 	  = (srcArea.ul.x & 0xF) -  (dstArea.ul.x  & 0xF);
		increment_src_ptr = BITMAPM_UNITS_PER_LINE(src->size.x);
		start_src_ptr     = src->data + srcArea.ul.y * BITMAPM_UNITS_PER_LINE(src->size.x) 
					+ (srcArea.ul.x >> 4);
	}
	else
	{ /* set up for the reverse direction */
		increment_dst_ptr = -BITMAPM_UNITS_PER_LINE(dst->size.x);
		start_dst_ptr     = dst->data + dstArea.lr.y * BITMAPM_UNITS_PER_LINE(dst->size.x) 
					+ (dstArea.ul.x >> 4);
		src_shift 	  = (srcArea.lr.x & 0xF) - (dstArea.lr.x & 0xF);
		increment_src_ptr = -BITMAPM_UNITS_PER_LINE(src->size.x);
		start_src_ptr     = src->data + srcArea.lr.y * BITMAPM_UNITS_PER_LINE(src->size.x) 
					+ (srcArea.lr.x >> 4);
	}

	if (src_shift < 0)
	{/* adjust the source shift (we miscalculated) */
		src_shift += BITMAPM_BITS_PER_UNIT;
		start_src_ptr--;
	}
	src_other_shift = BITMAPM_BITS_PER_UNIT - src_shift;
	end_dst_ptr = start_dst_ptr + (dstArea.lr.x >> 4) - (dstArea.ul.x >> 4);

  if (dir)
  {
	if (start_dst_ptr == end_dst_ptr)
	{/* special forward case */
#ifdef TRACING
(void) fprintf(stderr, "SF1\n");
#endif
		for (y = srcSize.y; y; y--)
		{/* blt a line */
			/* save the ends which will be partially clobbered */
				start_dst_unit = *start_dst_ptr;

				current_src_ptr = start_src_ptr;
				tmp = *current_src_ptr++ << src_shift;
				*start_dst_ptr = tmp | (*current_src_ptr >> src_other_shift);

			/* fix the partially clobbered ends */
				tmp = (~start_dst_mask & start_dst_unit) | ( start_dst_mask & *start_dst_ptr);
				*start_dst_ptr   = ( end_dst_mask   & start_dst_unit  ) | (~end_dst_mask   & tmp );

			/* get ready for the next line */
				start_dst_ptr += increment_dst_ptr;
				start_src_ptr += increment_src_ptr;
		}
	}
	else
	{/* generalized forward case */
#ifdef TRACING
(void) fprintf(stderr, "GF\n");
#endif
		for (y = srcSize.y; y; y--)
		{/* blt a line */
			/* save the ends which will be partially clobbered */
				start_dst_unit = *start_dst_ptr;
				end_dst_unit   = *end_dst_ptr;

			current_src_ptr = start_src_ptr;
			for (current_dst_ptr = start_dst_ptr; current_dst_ptr <= end_dst_ptr; current_dst_ptr++)
			{/* do a BITMAPM_UNIT in the destination */
				tmp = *current_src_ptr++ << src_shift;
				*current_dst_ptr = tmp | (*current_src_ptr >> src_other_shift);
			}

			/* fix the partially clobbered ends */
				*start_dst_ptr = (~start_dst_mask & start_dst_unit) | ( start_dst_mask & *start_dst_ptr);
				*end_dst_ptr   = ( end_dst_mask   & end_dst_unit  ) | (~end_dst_mask   & *end_dst_ptr);

			/* get ready for the next line */
				start_dst_ptr += increment_dst_ptr;
				end_dst_ptr   += increment_dst_ptr;
				start_src_ptr += increment_src_ptr;
		}
	}
  }
  else
  {
	if (start_dst_ptr == end_dst_ptr)
	{/* special case -- length 1 short */
#ifdef TRACING
(void) fprintf(stderr, "SB1\n");
#endif
		for (y = srcSize.y; y; y--)
		{/* blt a line */
		/* save the ends which will be partially clobbered */
			start_dst_unit = *start_dst_ptr;

			current_src_ptr = start_src_ptr + 1;
			tmp = *current_src_ptr -- >> src_other_shift;
			*start_dst_ptr = tmp | (*current_src_ptr << src_shift);

		/* fix the partially clobbered ends */
			tmp = (~start_dst_mask & start_dst_unit) | ( start_dst_mask & *start_dst_ptr);
			*start_dst_ptr   = ( end_dst_mask   & start_dst_unit  ) | (~end_dst_mask   & tmp );

		/* get ready for the next line */
			start_dst_ptr += increment_dst_ptr;
			start_src_ptr += increment_src_ptr;
		}
	}
	else if (end_dst_ptr - start_dst_ptr == 1)
	{/* special case - 2 shorts */
#ifdef TRACING
(void) fprintf(stderr, "SB2\n");
#endif
		for (y = srcSize.y; y; y--)
		{/* blt a line */
			/* save the ends which will be partially clobbered */
				start_dst_unit = *start_dst_ptr;
				end_dst_unit   = *end_dst_ptr;

			current_src_ptr = start_src_ptr + 1;

			/* the unrolled, CSE'd loop */
			current_dst_ptr = end_dst_ptr;	
			tmp = *current_src_ptr -- >> src_other_shift;
			*current_dst_ptr-- = tmp | (*current_src_ptr << src_shift);
			tmp = *current_src_ptr -- >> src_other_shift;
			*current_dst_ptr = tmp | (*current_src_ptr << src_shift);

			/* fix the partially clobbered ends */
				*start_dst_ptr = (~start_dst_mask & start_dst_unit) | ( start_dst_mask & *start_dst_ptr);
				*end_dst_ptr   = ( end_dst_mask   & end_dst_unit  ) | (~end_dst_mask   & *end_dst_ptr);

			/* get ready for the next line */
				start_dst_ptr += increment_dst_ptr;
				end_dst_ptr   += increment_dst_ptr;
				start_src_ptr += increment_src_ptr;
		}
	}
	else
	{/* generalized backwards case */
#ifdef TRACING
(void) fprintf(stderr, "GB\n");
#endif
		for (y = srcSize.y; y; y--)
		{/* blt a line */
			/* save the ends which will be partially clobbered */
				start_dst_unit = *start_dst_ptr;
				end_dst_unit   = *end_dst_ptr;

			current_src_ptr = start_src_ptr + 1;
			for (current_dst_ptr = end_dst_ptr;	start_dst_ptr <= current_dst_ptr; current_dst_ptr--)
			{/* do a BITMAPM_UNIT in the destination */
				tmp = *current_src_ptr -- >> src_other_shift;
				*current_dst_ptr = tmp | (*current_src_ptr << src_shift);
			}

			/* fix the partially clobbered ends */
				*start_dst_ptr = (~start_dst_mask & start_dst_unit) | ( start_dst_mask & *start_dst_ptr);
				*end_dst_ptr   = ( end_dst_mask   & end_dst_unit  ) | (~end_dst_mask   & *end_dst_ptr);

			/* get ready for the next line */
				start_dst_ptr += increment_dst_ptr;
				end_dst_ptr   += increment_dst_ptr;
				start_src_ptr += increment_src_ptr;
		}
	}
  }
}


void
ColorWithPattern(Pixmap dst, Rectangle dstArea, Bitmap pattern, Point offset, 
                 Boolean clip, Color foreground, Color background)
{}

void
invertPixmap(Pixmap dst, Rectangle dstArea, Boolean clip)
{}

#if 0  /** DEBUG 06 May 11.3 */
Pixmap
makePixmap(Point size, char *who)
{}

void
freePixmap(Pixmap b)
{}

Bitmap
flattenPixmap(Pixmap b, Rectangle r, Color foreground, Color background)
{}

void
pointColor(Pixmap dst, Point p, Rectangle clipper, Boolean clip, Color c)
{}

void
polyColorPoint(Pixmap dst, Point origin, short n, Point *pp, 
               Rectangle clipper, Boolean clip, Color c)
{}

void
lineColor(Pixmap dst, Point p1, Point p2, short width, LineStyle *style, 
          Rectaangle clipper, Boolean clip, Color c)
{}

void
polyColorLine(Pixmap dst, Point origin, short n, Point *pp, short width, 
              LineStyle *style, Rectangle clipper, Boolean clip, Color c)
{}

void
polygonColor(Pixmap dst, Point origin, short n, Point *pp, Bitmap pattern, 
             Point offset, Rectangle clipper, Boolean clip, Color fg, Color bg)
{}

void
arcLine(Pixmap dst, Rectangle* r, double a1, double a2, short width, 
        LineStyle *style, Rectangle clipper, Boolean clip, Color c)
{}

void
arcColor(Pixmap dst, Rectangle* r, double a1, double a2, Bitmap pattern, 
         Point offset, Rectangle clipper, Boolean clip, Color fg, Color bg)
{}

Color
getColorFromName(char *name)
{}

int
getRGBFromName(char *name, int *red, int *green, int *blue)
{}

void
BLTColorCopy(Pixmap src, Pixmap dst, Rectangle srcArea, Point dstOrigin, 
             Bitmap m, Point offset, Boolean clip)
{}
#endif  /** 0 DEBUG 06 May 11.3 */
