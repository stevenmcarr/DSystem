/* $Id: integer.C,v 1.1 1997/06/25 14:48:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/* item/integer.c                                                       */
/*                                                                      */
/* integer item                                                         */
/* Last edited: 25 May 1990 at 3:35 pm                                  */
/*                                                                      */
/************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include <libs/graphicInterface/oldMonitor/include/mon/item_def.h>
#include <libs/support/strings/rn_string.h>
#include <string.h>
#include <libs/graphicInterface/oldMonitor/include/items/integer.h>
#include <libs/graphicInterface/oldMonitor/include/items/regex.h>

#define TEXT_SIZE 128
#define RE_INT  "^ *[+-]?[0-9]* *$"

static Boolean stringToInteger(char *s, int **ip, Boolean conforms)
{
    int newi;

    if (NOT(conforms))
    {/* Use 0 as the default value. */
        if (**ip == 0)
            return false;

        **ip = 0;
        return true;
    }

    newi = atoi(s);
    if (**ip != newi)
    {/* A new value of i */
        **ip = newi;
        return true;
    }
    else
        /* i unchanged */
        return false;
        
}

static Boolean integerToString(int *i, char **sp)
{
    char buf[TEXT_SIZE];

    (void) sprintf(buf,"%d",*i);
    if (strcmp(buf,*sp))
    {/* The string representation of i changed.  Update sp */
        sfree(*sp);
        *sp = ssave(buf);
        return true;
    }
    else
    {/* No change in the string representation of i */
        return false;
    }
}

/* Another fine piece of information hiding shot to hell by C.  But     */
/* code sharing is in the end a nobler goal.  We let the regex_item     */
/* routines do most of the grunt work for us, and they will call the    */
/* two conversion routines above to do the "tricky" stuff.              */

static  Item    integer_item = {        /* the integer item             */
                        "integer",
                        regex_item_get_size,
                        (item_initialize_func)regex_item_initialize,
                        regex_item_handle_event,
                        regex_item_set_focus,
                        regex_item_modified,
                        regex_item_destroy
                };

/************************* EXTERNAL ROUTINES ****************************/


/* Create a dialog descriptor for the integer item.                     */
DiaDesc *
item_integer(Generic item_id, char *title, short font, int *vptr)
{
    DiaDesc *dd;

    dd = item_gregex(item_id,
            title,
            font,
            (Generic)vptr,
            RE_INT,
            6,
            true,
            (item_gregex_toval_func)stringToInteger,
            (item_gregex_fromval_func)integerToString);
            
    /* Is this really necessary? */
    dd->item = &integer_item;
    return dd;
}
