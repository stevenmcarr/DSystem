/* $Id: regex.C,v 1.1 1997/06/25 14:48:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*  item/regex.c                                                        */
/*                                                                      */
/*  regular expression item                                             */
/*  Last edited: 25 May 1990 at 3:25 pm                                 */
/*                                                                      */
/************************************************************************/

#include <libs/graphicInterface/oldMonitor/include/mon/item_def.h>
#include <libs/graphicInterface/oldMonitor/include/items/regex.h>
#include <libs/support/strings/rn_string.h>
#include <string.h>
#include <libs/graphicInterface/oldMonitor/include/sms/vanilla_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/ted_sm.h>
#include <libs/support/patternMatch/regexp.h>

EXTERN(void,sm_ted_bell,(Pane*));
EXTERN(void,sm_ted_end_of_buffer,(Pane*));

    /*** LOCAL INFORMATION ***/

static  char    ErrorMark = STYLE_BOLD ;
static  char    StandOut  = ATTR_UNDERLINE ;

typedef FUNCTION_POINTER(Boolean,fromValFunc,(Generic,char**));
typedef FUNCTION_POINTER(Boolean,toValFunc,(char*,Generic*,Boolean));

struct  regex   {                       /* ITEM STARTUP INFORMATION     */
    char            *title;         /* title of the regex           */
    short           font;           /* the font to use              */
    char            style;          /* the style for the title      */
    Pane            *title_pane;    /* the title pane of the layout */
    Pane            *edit_pane;     /* the ted pane of the layout   */
    char            *strClient;     /* client's regular string      */
    char            *strCurrent;    /* the current string           */
    regexp   	    *comp_reg_expr; /* compiled regular expression  */
    short           width;          /* width of edit pane in chars  */
    Generic         val;            /* a value, of unknown flavor   */
    toValFunc       toVal;          /* converts text to val         */
    fromValFunc             fromVal;        /* converts val to text         */
    ItemRegexEnum   check;          /* results of last re_check()   */
    Boolean         is_focus;       /* true if we are the focus     */
    Boolean         mustConform;    /* must string be in L(re)?     */
            };

/*
 * regex_set_emphasis - turn highlighting of the string on or off.
 *      Return true if the emphasis changed, or we were in an
 *      unknown (ie. error) state.
 */
static Boolean regex_set_emphasis(struct regex *info, Boolean emphasize)
{
    Boolean changed = true;
    Boolean nochange = info->check == itemRegexError ? true : false;

    if (emphasize)
    {/* Turn on emphasis */
        if (info->mustConform)
        {/* Complain.  Text will be corrected later. */
            sm_ted_bell(info->edit_pane);
        }

        if (info->style & ErrorMark)
        /* Already emphasized */
            return nochange;

        info->style |= ErrorMark;
    }
    else
    {/* Turn off emphasis */

        /* Emphasis already off */
        if (!(info->style & ErrorMark) )
            return nochange;

        info->style &= ~ErrorMark;
    }
    sm_ted_win_set_style(info->edit_pane, info->style);

    /* Emphasis changed */
    return changed;
}


/************************** ITEM DEFINITION *****************************/


/* Get the pixel size of the regex item.                                */
Point
regex_item_get_size(DiaDesc *dd)
{
struct  regex   *info;                  /* the information parameter    */
Point           title_size;             /* the size of the title        */
Point           edit_size;              /* the size of the edit pane    */
Point           size;                   /* the overall size             */

    info = (struct regex *) dd->item_info;

    edit_size  = sm_ted_pane_size(makePoint(info->width, 1), info->font);
    if (strlen(info->title) == 0)
        title_size = Origin;
    else
        title_size = sm_vanilla_pane_size(info->title, info->font);
    size.x = edit_size.x + title_size.x;
    size.y = MAX(edit_size.y, title_size.y);
    return (size);
}


/*
 *  Check to see of the current string matches the compiled regular expression.
 *  Return a status indicating whether or not the string was matched by the
 *  RE, and whether or not this represents a change from the previous result.
 */
static ItemRegexEnum re_check(struct regex *info)
{
    int ret_val;

    ret_val = regexec( info->comp_reg_expr, info->strCurrent );

    switch(ret_val)
    {
        case REGEX_NOMATCH:
            if (regex_set_emphasis(info, true ))
                return itemRegexUnmatched;
            else
                return itemRegexStillUnmatched;

        case REGEX_MATCH:
            if (regex_set_emphasis(info, false ))
                return itemRegexMatched;
            else
                return itemRegexStillMatched;

        case REGEX_ERROR:
            message("Error in compiled regular expression");
            (void) regex_set_emphasis(info, true );
            
        default:
            return itemRegexError;
    }
}


/* Place the item and initialize the pane.                              */
void
regex_item_initialize(DiaDesc *dd, Point ulc)
{
Point           title_size;             /* the size of the title        */
Point           edit_size;              /* the size of the edit pane    */
struct  regex   *info;                  /* the information parameter    */

    info = (struct regex *) dd->item_info;

    	/* get and adjust the pane sizes */
    edit_size   = sm_ted_pane_size(makePoint(info->width, 1), info->font);
    if (strlen(info->title) == 0)
        title_size = Origin;
    else
        title_size = sm_vanilla_pane_size(info->title, info->font);
    edit_size.y = title_size.y = MAX(edit_size.y, title_size.y);

	/* create and initialize the title pane */
    info->title_pane = dialog_item_make_pane(
        dd,
        sm_vanilla_get_index(),
        ulc,
        title_size,
        0
    );
    sm_vanilla_set_text(
        info->title_pane,
        info->title,
        info->font,
        STYLE_BOLD,
        VSM_JUSTIFY_CENTER
    );

    /* create and initialize the edit pane */
    ulc.x += title_size.x;
    info->edit_pane = dialog_item_make_pane(
        dd,
        sm_ted_get_index(),
        ulc,
        edit_size,
        1
    );
    sm_ted_win_change_font(info->edit_pane, info->font);

	/* Generate text representation of val, and put it into an editor */
    (info->fromVal)(info->val,&(info->strCurrent));
    sm_ted_buf_set_text(info->edit_pane, info->strCurrent, strlen(info->strCurrent));
    sm_ted_end_of_buffer(info->edit_pane);
    sm_ted_win_active(info->edit_pane, false);
    info->strClient = sm_ted_buf_get_text(info->edit_pane);

    info->style = STYLE_NORMAL;
    (void) re_check(info);
}


/* Handle the current event.                                            */
FocusStatus
regex_item_handle_event(DiaDesc *dd)
{
struct  regex   *info;                  /* the information parameter    */
ItemRegexEnum   check;                  /* result of re_check()         */

    info = (struct regex *) dd->item_info;

    if (mon_event.type == EVENT_SELECT)
    {/* a selection to our pane--move the focus here */
        if (mon_event.from == (Generic) info->edit_pane)
        {/* move the cursor at the same time */
            sm_ted_win_set_xy(info->edit_pane, mon_event.info);
            sm_ted_repair(info->edit_pane);
        }
        return FocusThis;
    }
    else if ((mon_event.type == EVENT_KEYBOARD) && info->is_focus)
    {/* a character to us */
        sm_ted_handle_keyboard(info->edit_pane, (KbChar) mon_event.info.x);
        smove(&(info->strCurrent),
            sm_ted_buf_get_text(info->edit_pane));

        if (strcmp(info->strCurrent,info->strClient) != 0)
        {/* New string is different somehow */
            check = re_check(info);
            switch (check)
            {
                case itemRegexUnmatched:
                case itemRegexStillUnmatched:
                case itemRegexError:
                    if (info->mustConform)
                    {
                        sm_ted_buf_set_text(info->edit_pane,
                            info->strClient,
                            strlen(info->strClient));
                        smove(&(info->strCurrent),info->strClient);

                        /* Reset highlighting */
                        check = re_check(info);
                    }
                    else
                    {
                case itemRegexMatched:
                case itemRegexStillMatched:
                        smove(&(info->strClient),info->strCurrent);
                    }
            }

            /* Produce a new value from the new string */
            if ( (info->toVal)(
                    info->strCurrent,
                    &(info->val),
                    BOOL(check == itemRegexMatched || check == itemRegexStillMatched))
               )
            {/* The value has changed also */
                info->check = check;
                dialog_item_user_change(dd);
            }
            /* else no change visible to the owner of this item. */
        }
        /* else no difference in old and new strings */

        sm_ted_repair(info->edit_pane);
    }
    return FocusOK;
}


/* Handle a focus modification.                                         */
DiaDesc *
regex_item_set_focus(DiaDesc *dd, Boolean fs)
{
struct  regex   *info;                  /* the information parameter    */

    info = (struct regex *) dd->item_info;

    if (info->is_focus)
    {/* we currently have the focus--turn it off */
        info->is_focus = false;
        sm_ted_win_active(info->edit_pane, false);
        sm_ted_repair(info->edit_pane);
        return NULL_DIA_DESC;
    }
    else if ((fs == FOCUS_RESET) && dd->able)
    {/* we don't have the focus now--turn it on */
        info->is_focus = true;
        sm_ted_win_active(info->edit_pane, true);
        sm_ted_repair(info->edit_pane);
        return dd;
    }
    else
    {/* do not change status */
        return NULL_DIA_DESC;
    }
}


/* Handle a modification of the item.                                   */
void
regex_item_modified(DiaDesc *dd, Boolean self)
{
struct  regex   *info;                  /* the information parameter    */

    info = (struct regex *) dd->item_info;

    if (self)
    {/* the user modified the value */
        /* Work done in regex_item_handle_event. */
    }
    else
    {/* client changed the value internally */
        if ((info->fromVal)(info->val,&(info->strCurrent)))
        {
            sm_ted_buf_set_text(info->edit_pane, info->strCurrent, strlen(info->strCurrent));
            info->strClient = sm_ted_buf_get_text(info->edit_pane);
            info->check = re_check(info);
            sm_ted_repair(info->edit_pane);
        }
    }
}

/* Destroy item specific information.                                   */
/*ARGSUSED*/
void
regex_item_destroy(DiaDesc *dd)
{
    struct regex *info;
    info = (struct regex *)(dd->item_info);

    sfree(info->title);
    sfree(info->strCurrent);
    sfree(info->strClient);
    regfree(info->comp_reg_expr);
    free_mem((void*)dd->item_info);
}

static  Item    regex_item = {          /* the regex item               */
                        "regex",
                        regex_item_get_size,
                        (item_initialize_func)regex_item_initialize,
                        regex_item_handle_event,
                        regex_item_set_focus,
                        regex_item_modified,
                        regex_item_destroy
                };

/************************* EXTERNAL ROUTINES ****************************/


/* Create a dialog descriptor for the regex item.                       */
DiaDesc *
item_gregex(Generic item_id, char *title, short font, Generic val, char *re, 
            short width, Boolean conform, toValFunc toVal, fromValFunc fromVal)
{
struct  regex   *info;                  /* the regex information        */

    info = (struct regex *) get_mem(
        sizeof(struct regex),
        "regex item regex information"
    );
    if (title == (char *) 0)
        title = "";
    info->title         = ssave(title);
    info->font          = font;
    info->title_pane    = NULL_PANE;
    info->edit_pane     = NULL_PANE;
    info->val           = val;
    info->strClient     = ssave("");
    info->strCurrent    = ssave("");
    info->toVal         = toVal;
    info->fromVal       = fromVal;
    info->comp_reg_expr = regcomp(re);
    info->width         = width;
    info->check         = itemRegexError;
    info->is_focus      = false;
    info->mustConform   = conform;

    return (
        makeDialogDescriptor(
                &regex_item,
                (Generic) info,
                item_id
        )
    );
}


/*
 * A function to install for toVal when val is
 * really just a character string.
 */
/*ARGSUSED*/
Boolean toText(char *otext, char ***ntext, Boolean conforms)
    /* coforms: Ignored.  Don't care if otext is in L(re) */
{
    if (strcmp(otext,**ntext))
    {
        smove(*ntext,otext);
        return true;
    }
    else
    {/* No change */
        return false;
    }
}

/*
 * A function to install for fromVal when val is
 * really just a character string.
 */
Boolean fromText(char **ntext, char **otext)
{
    if (strcmp(*otext,*ntext))
    {
        smove(otext,*ntext);
        return true;
    }
    else
    {/* No change */
        return false;
    }
}

DiaDesc *
item_regex(Generic item_id, char *title, short font, char **re_str_ptr, 
           char *re_ptr, short width)
{
    return item_gregex(item_id,
        title,
        font,
        (Generic)re_str_ptr,
        re_ptr,
        width,
        true,
        (item_gregex_toval_func)toText,
        (item_gregex_fromval_func)fromText);
}

/*
 * Return the matching status of the regular expression item.
 */
ItemRegexEnum item_regex_status(DiaDesc *dd)
{
    struct regex *info;

    info = (struct regex *)dd->item_info;
    return info->check;
}

/*
 * regex_set_standout - turn orthogonal highlighting of the string on or off.
 *      Return true if the emphasis changed, or we were in an
 *      unknown (ie. error) state.
 */
Boolean item_regex_set_standout(DiaDesc *dd, Boolean emphasize)
{
    struct regex *info;

    info = (struct regex *)dd->item_info;
    if (emphasize)
    {/* Turn on emphasis */
        if (info->style & StandOut)
        /* Already emphasized */
            return false ;

        info->style |= StandOut;
    }
    else
    {/* Turn off emphasis */

        /* Emphasis already off */
        if (!(info->style & StandOut) )
            return false ;

        info->style &= ~StandOut;
    }
    sm_ted_win_set_style(info->edit_pane, info->style);
    sm_ted_repair(info->edit_pane) ;    /* needed since style may change */
                                        /* without a text change...      */

    /* Emphasis changed */
    return true ;
}

