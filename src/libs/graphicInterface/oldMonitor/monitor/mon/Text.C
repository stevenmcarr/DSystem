/* $Id: Text.C,v 1.1 1997/06/25 14:53:48 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
		/********************************************************/
		/* 			font.c				*/
		/*   		    Font functions.			*/
		/* 							*/
		/********************************************************/


#include <libs/support/misc/general.h>
#include <include/bstring.h>
#include <libs/graphicInterface/support/graphics/point.h>
#include <libs/graphicInterface/support/graphics/rect.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/graphicInterface/oldMonitor/include/mon/gfx.h>
#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/include/mon/font.h>
#include <string.h>
#include <sys/file.h>
#include <fcntl.h>

TextString		emptyTextString = 	/* an empty text string			*/
				{ 0, 0, false};


/* Convert a char and style into the corresponding TextChar				*/
TextChar
makeTextChar(unsigned char ch,unsigned char style)
// unsigned char		ch;			/* the character			*/
// unsigned char		style;			/* the style				*/
{
TextChar		tch;			/* the returned TextChar		*/

	tch.ch    = ch;
	tch.style = style;
	return (tch);
}


/* Return true if two TextChar's are exactly the same and false otherwise.		*/
/* Boolean
 * equalTextChar(tc1, tc2)
 * TextChar		tc1;
 * TextChar		tc2;
 * {
 *	return (BOOL(tc1.ch == tc2.ch && tc1.style == tc2.style));
 * }
 */

/* Return an ephemeral text string representing a regular string and a style.		*/
TextString
makeTextString(char *s, unsigned char style, char *who)
// char			*s;			/* the string to be converted		*/
// unsigned char		style;			/* the style parameter			*/
// char			*who;			/* the id of the caller			*/
{
TextString		ts;			/* the text string being made		*/
register short		i;			/* the current character		*/

	ts.num_tc    = strlen(s);
	ts.tc_ptr    = (TextChar *) ((ts.num_tc == 0) ? 0 : get_mem(ts.num_tc * sizeof(TextChar), "%s (makeTextString)", who));
	ts.ephemeral = true;
	for (i = 0; i < ts.num_tc; i++)
	{/* transfer a character to a TextChar */
		ts.tc_ptr[i].ch    = s[i];
		ts.tc_ptr[i].style = style;
	}
	return ts;
}


/* Return an ephemeral text string representing a regular string and a style.		*/
TextString
makePartialTextString(char *s, unsigned char style, short len, char *who)
// char			*s;			/* the string to be converted		*/
// unsigned char		style;			/* the style parameter			*/
// short			len;			/* the usable length of the string	*/
// char			*who;			/* the id of the caller			*/
{
TextString		ts;			/* the text string being made		*/
register short		i;			/* the current character		*/

	ts.num_tc    = len;
	ts.tc_ptr    = (TextChar *) ((len == 0) ? 0 : get_mem(len * sizeof(TextChar), "%s (makePartialTextString)", who));
	ts.ephemeral = true;
	for (i = 0; i < len; i++)
	{/* transfer a character to a TextChar */
		ts.tc_ptr[i].ch    = s[i];
		ts.tc_ptr[i].style = style;
	}
	return ts;
}


/* Make a TextString of a known size.							*/
TextString
createTextString(short size, char *who)
// short			size;			/* the number of TextChars needed	*/
// char			*who;			/* the memory identification string	*/
{
TextString		ts;			/* the resulting TextString		*/

	ts.num_tc    = size;
	ts.ephemeral = false;
	ts.tc_ptr    = (TextChar *) get_mem(size * sizeof(TextChar), "createTextString for %s", who);
	return ts;
}


/* Make a duplicate of a TextString.							*/
TextString
copyTextString(TextString ts)
  // TextString		ts;			/* the TextString to copy		*/
{
	return subTextString(ts, 0, ts.num_tc);
}


/* Duplicate a portion of a TextString.							*/
TextString
subTextString(TextString ts, short start, short count)
// TextString		ts;			/* the source of the copy		*/
// short			start;			/* the starting index of the copy	*/
// short			count;			/* the number of TextChars to copy	*/
{
TextString		result;			/* the resulting TextString		*/
short			bytes;			/* the number of bytes involved		*/

	result.num_tc = count;
	result.ephemeral = false;
	if (count == 0)
	{/* don't bother allocating space */
		result.tc_ptr = (TextChar *) 0;
	}
	else
	{/* allocate space and copy */
		bytes = count * sizeof(TextChar);
		result.tc_ptr = (TextChar *) get_mem(bytes,"subTextString");
		bcopy((char *) (ts.tc_ptr + start), (char *) result.tc_ptr, bytes);
	}

	return result;
}


/* Destroy the storage associated with a TextString.					*/
void
moveTextString(TextString src, TextString *dst)
// TextString		src;			/* the source of the move		*/
// TextString		*dst;			/* the destination of the move		*/
{
	dst->num_tc = src.num_tc;
	bcopy((char *) src.tc_ptr, (char *) dst->tc_ptr, src.num_tc * sizeof(TextChar));
}


/* Destroy the storage associated with a TextString.					*/
void
destroyTextString(TextString ts)
  // TextString		ts;			/* the TextString to free		*/
{
	if (ts.tc_ptr)
	{/* the pointer had been allocated */
		free_mem((void*) ts.tc_ptr);
	}
}

