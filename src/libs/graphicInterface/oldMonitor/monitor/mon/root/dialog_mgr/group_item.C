/* $Id: group_item.C,v 1.1 1997/06/25 14:50:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*                            group_item.c                              */
/*                             group item                               */
/*                                                                      */
/************************************************************************/

#include <stdarg.h>

#include <libs/graphicInterface/oldMonitor/include/mon/item_def.h>


    /*** LOCAL INFORMATION ***/

struct  group   {                       /* GROUP INFORMATION STRUCTURE  */
    Point               size;           /* our overall size             */
    aDialogDir          dir;            /* the direction of the group   */
    short               num_dds;        /* the number of dds            */
    DiaDesc             **dd_list;      /* the dd's that we manage      */
    Point               *size_list;     /* the size of each dd          */
    short               focus_index;    /* the index of the focus       */
                };
#define         BETWEEN 6               /* space between adjacent items */


/************************** ITEM DEFINITION *****************************/


/* Get the pixel size of the group item.                                */
static
Point
group_item_get_size(DiaDesc *dd)
{
struct  group   *info;                  /* the information parameter    */
short           i;                      /* the current child dd index   */

    info = (struct group *) dd->item_info;

    info->size = Origin;
    for (i = 0; i < info->num_dds; i++)
    {/* walk down the child list */
        info->size_list[i] = (info->dd_list[i]->item->get_size)(
                info->dd_list[i]
        );
        switch (info->dir)
        {/* add based on the direction */
            case DIALOG_HORIZ_TOP:
            case DIALOG_HORIZ_CENTER:
            case DIALOG_HORIZ_BOTTOM:   /* horizontal grouping */
                info->size.x += info->size_list[i].x + ((i== 0) ? 0 : BETWEEN);
                info->size.y = MAX(info->size.y, info->size_list[i].y);
                break;

            case DIALOG_VERT_LEFT:
            case DIALOG_VERT_CENTER:
            case DIALOG_VERT_RIGHT:     /* vertical grouping */
                info->size.x = MAX(info->size.x, info->size_list[i].x);
                info->size.y += info->size_list[i].y + ((i== 0) ? 0 : BETWEEN);
                break;
        }
    }
    return info->size;
}


/* Place the item and initialize the pane.                              */
static
void
group_item_initialize(DiaDesc *dd, Point start)
{
struct  group   *info;                  /* the information parameter    */
Point           ulc;                    /* where to put the item        */
short           i;                      /* the current child dd index   */

    info = (struct group *) dd->item_info;

    for (i = 0; i < info->num_dds; i++)
    {/* initialize each of the dd's */
        ulc = start;
        switch (info->dir)
        {/* figure the ulc of this dd */
            case DIALOG_HORIZ_CENTER:
                ulc.y += (info->size.y - info->size_list[i].y) / 2;
                break;
            case DIALOG_HORIZ_BOTTOM:
                ulc.y += info->size.y - info->size_list[i].y;
                break;
            case DIALOG_VERT_CENTER:
                ulc.x += (info->size.x - info->size_list[i].x) / 2;
                break;
            case DIALOG_VERT_RIGHT:
                ulc.x += info->size.x - info->size_list[i].x;
                break;
        }

        initDialogDescriptor(dd->owner, info->dd_list[i], ulc);

        switch (info->dir)
        {/* move to the next base position */
            case DIALOG_HORIZ_TOP:
            case DIALOG_HORIZ_CENTER:
            case DIALOG_HORIZ_BOTTOM:   /* horizontal grouping */
                start.x += info->size_list[i].x + BETWEEN;
                break;

            case DIALOG_VERT_LEFT:
            case DIALOG_VERT_CENTER:
            case DIALOG_VERT_RIGHT:     /* vertical grouping */
                start.y += info->size_list[i].y + BETWEEN;
                break;
        }
    }
}


/* Handle the current event.                                            */
/*ARGSUSED*/
static
FocusStatus
group_item_handle_event(DiaDesc *dd)
{
    die_with_message("group_item_handle_event():  why is this called?");
    return FocusOK;
}


/* Set the focus.                                                       */
static
DiaDesc *
group_item_set_focus(DiaDesc *dd, Boolean fs)
{
struct  group   *info;                  /* the information parameter    */
DiaDesc         *focus;                 /* the dialog descriptor        */

    info = (struct group *) dd->item_info;

    if (fs == FOCUS_RESET)
    {/* set the focus */
        if (info->focus_index == UNUSED)
        {/* we currently have no focus -- try to start one */
            info->focus_index = 0;
        }
        do
        {/* try to find a new focus */
            focus = (info->dd_list[info->focus_index]->item->set_focus)(
                    info->dd_list[info->focus_index],
                    FOCUS_RESET
            );
            if (focus == NULL_DIA_DESC)
            {/* set up for the next iteration */
                info->focus_index++;
            }
        } while (!focus && (info->focus_index < info->num_dds));
        if (!focus)
        {/* we have just gone to having no focus */
            info->focus_index = UNUSED;
        }
        return focus;
    }
    else if (info->focus_index != UNUSED)
    {/* remove the focus */
        (info->dd_list[info->focus_index]->item->set_focus)(
                info->dd_list[info->focus_index],
                FOCUS_CLEAR
        );
        info->focus_index = UNUSED;
    }
    return NULL_DIA_DESC;
}


/* Handle a modification of the item.                                   */
/*ARGSUSED*/
static
void
group_item_modified(DiaDesc *dd, Boolean user)
{
    die_with_message("group_item_modified():  why is this called?");    
}


/* Destroy item specific information.                                   */
/*ARGSUSED*/
static
void
group_item_destroy(DiaDesc *dd)
{
struct  group   *info;                  /* the information parameter    */
short           i;                      /* the current child dd index   */

    info = (struct group *) dd->item_info;

    for (i = 0; i < info->num_dds; i++)
    {/* free each dialog descriptor in the list */
        freeDialogDescriptor(info->dd_list[i]);
    }
    free_mem((void*) info->dd_list);
    free_mem((void*) info->size_list);
    free_mem((void*)dd->item_info);
}


static  Item    group_item = {          /* the group item               */
                        "group",
                        group_item_get_size,
                        (item_initialize_func)group_item_initialize,
                        group_item_handle_event,
                        group_item_set_focus,
                        group_item_modified,
                        group_item_destroy
                };

/************************* EXTERNAL ROUTINES ****************************/


/* Create a dialog descriptor for a group.                              */
/*VARARGS*/
DiaDesc* dialog_desc_group(aDialogDir dir, int num_dds, ...)
{
  va_list         arg_list;               /* argument list as per varargs */
  struct  group*  info;                   /* the group information        */
  short           i;                      /* the current child dd index   */

  va_start(arg_list, num_dds);
    {
           /* create and initialize the item structure */
        info = (struct group*) get_mem(
                sizeof(struct group),
                "group item group information");
        info->size        = Origin;
        info->dir         = dir;
        info->num_dds     = num_dds;
        info->dd_list     = (DiaDesc**)get_mem(sizeof(DiaDesc*) * info->num_dds,
                                               "dd list");
        info->size_list   = (Point*)get_mem(sizeof(Point) * info->num_dds,
                                            "size list");
        info->focus_index = UNUSED;

           /* set up for each dialog descriptor */
        for (i = 0; i < info->num_dds; i++)
        {
               /* get each of the dialog descriptors */
            info->dd_list[i]   = va_arg(arg_list, DiaDesc*);
            info->size_list[i] = Origin;
        }
    }
  va_end(arg_list);

  return (makeDialogDescriptor(&group_item, (Generic)info, DIALOG_UNUSED_ID));
}
