/* $Id: log_msg_dlg.C,v 1.1 1997/06/25 15:17:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *  log message facility
 *
 *    includes:
 * --dialog for setting logging levels
 *  
 * R. Hood, Aug. 1988
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <libs/graphicInterface/oldMonitor/include/mon/dialog_def.h>

#include <libs/graphicInterface/oldMonitor/include/items/title.h>
#include <libs/graphicInterface/oldMonitor/include/items/radio_btns.h>
#include <libs/graphicInterface/oldMonitor/include/items/check_box.h>
#include <libs/graphicInterface/oldMonitor/include/items/button.h>

#include <libs/support/msgHandlers/log_msg.h>
#include <libs/support/strings/rn_string.h>

#define TRACE 3
#define WARNG 2
#define ERROR 1
#define NONE  0

struct ls {
    char*     title;
    Boolean** col;   /* the array of Boolean *'s that make up this row */
    Boolean*  bit;   /* the temporary copies of above */
};

typedef struct logdia  {      /* logging dialog structure      */
    char*       title;        /* title of whole dialog         */
    int         num_rows;     /* number of rows (cols) of ...  */
    int         num_cols;     /* ... Boolean vars being set    */
    struct ls*  row;          /* the array of rows             */
    char**      col_title;    /* headings for the columns      */
    Dialog*     dialog;       /* the actual dialogue           */
    short       smallFont;
    Boolean     do_it;
} aLogDia;

STATIC(Boolean, logging_dialog_handler, (Dialog* di, aLogDia* ld, Generic item));

/*  Handle changes to the logging dialogue. */
/*    di:   the dialog that has been mod.   */
/*    ld:   the logging dialog              */
/*    item: the item which has changed      */
static Boolean logging_dialog_handler(Dialog* di, aLogDia* ld, Generic item)
{
  switch(item) 
    {
       case DIALOG_DEFAULT_ID:  /* done--change logging levels   */
         ld->do_it = true;
         return DIALOG_QUIT;

       case DIALOG_CANCEL_ID:   /* done--don't change levels  */
         return DIALOG_QUIT;

       default:                 /* not done--keep going    */
         return DIALOG_NOMINAL;
    }
}


/*
 *  put up a logging level dialog
 *  
 *  args:  
 * title, n, line_1, line_2, ...
 *
 * where n is the number of lines that follow and each line has
 * four items:
 *    char  *title;     -- name of logging area
 *    Boolean *trace,      -- if tracing  is  enabled
 *       *warng,     -- if warnings are enabled
 *       *error;     -- if errors   are enabled
 * 
 */
void run_2D_checkbox_dialog (char* title, int columns, int rows, ...)
{
  va_list    arg_list;
  int        i, j;
  aLogDia   *ld;
  DiaDesc   *whole_dd, *old_dd, *new_dd, *new_whole;

  va_start(arg_list, columns);
    {
       ld = (aLogDia*)get_mem(sizeof(aLogDia), "logging dialogue");

          /* create and initialize the dialog instance structure */
       ld->smallFont  = fontOpen("screen.7.rnf");
       ld->do_it      = false;
       ld->title      = title;
       ld->num_cols   = columns;
       ld->num_rows   = rows;
       ld->row        = (struct ls*)get_mem(sizeof(struct ls) * ld->num_rows,
                                            "row array--logging dialog");
       ld->col_title  = (char**)get_mem(sizeof(char*) * ld->num_cols,
                                        "column title array--logging dialog");

          /* get the column titles */
       whole_dd = (DiaDesc *)0;
       for (j = 0; j < ld->num_cols; j++) 
         {
            ld->col_title[j] = va_arg(arg_list, char*);
         }

          /* get the rows */
       for ( i = 0; i < ld->num_rows; i++ ) 
         {     /* get row i-- a title and then num_cols Boolean *'s */
            ld->row[i].title = va_arg(arg_list, char*);
            ld->row[i].col   = (Boolean**)get_mem(sizeof(Boolean*) * ld->num_cols,
                                                  "1 of the col **arrays--logging dialog");
            ld->row[i].bit   = (Boolean*)get_mem(sizeof(Boolean) * ld->num_cols,
                                                 "1 of the col *arrays--logging dialog");
            for (j = 0; j < ld->num_cols; j++) 
              {
                 ld->row[i].col[j] = va_arg(arg_list, Boolean*);
                 ld->row[i].bit[j] = *(ld->row[i].col[j]);
              }
         }
    }
  va_end(arg_list);

     /* build up the dialog descriptor, column by column */
     /* first the row-title column */
  old_dd = dialog_desc_group(DIALOG_VERT_LEFT,
                             2, 
                             item_title(1, " ", DEF_FONT_ID),
                             item_title(1, ld->row[0].title, DEF_FONT_ID));
  for ( i = 1; i < ld->num_rows; i++ ) 
    {
       new_dd = dialog_desc_group(DIALOG_VERT_LEFT, 2, old_dd,
                                  item_title(1, ld->row[i].title, DEF_FONT_ID));
       old_dd = new_dd;
    }
  whole_dd = old_dd;

     /* now the columns--0 ... num_cols-1 */
  for ( j = 0; j < ld->num_cols; j++ ) 
    {     /* get the j-th col in old_dd */
       old_dd = dialog_desc_group(DIALOG_VERT_CENTER, 2, 
                                  item_title(1, ld->col_title[j], DEF_FONT_ID),
                                             item_check_box(1, "", DEF_FONT_ID,
                                             &(ld->row[0].bit[j])));
       for ( i = 1; i < ld->num_rows; i++ ) 
         {
            new_dd = dialog_desc_group(DIALOG_VERT_CENTER, 2, old_dd,
                                       item_check_box(1, "", DEF_FONT_ID, 
                                                      &(ld->row[i].bit[j])));
            old_dd = new_dd;
         }

          /* cons the j-th col to the rest */
       new_whole = dialog_desc_group(DIALOG_HORIZ_TOP, 2, whole_dd, old_dd);
       whole_dd = new_whole;
    }

  ld->dialog = 
     dialog_create(ld->title, (dialog_handler_callback)logging_dialog_handler, 
                   (dialog_helper_callback)0, (Generic)ld,
                   dialog_desc_group(DIALOG_VERT_CENTER, 2, whole_dd, 
                                     dialog_desc_expand(
                                          (DiaDesc*)item_button((Generic)DIALOG_DEFAULT_ID,
                                                                "  OK  ",
                                                                ld->smallFont,
                                                                (Boolean)true))));

     /* run the dialog, record the changes */
  dialog_modal_run(ld->dialog);
  if (ld->do_it)
    {     /* change the logging levels */
       for(i = 0; i < ld->num_rows; i++)
         for(j = 0; j < ld->num_cols; j++)
           *(ld->row[i].col[j]) = ld->row[i].bit[j];

       ld->do_it = false;
    }

     /* destroy the dialog */
  fontClose(ld->smallFont);
  dialog_destroy(ld->dialog);
  for(i = 0; i < ld->num_rows; i++)
     free_mem((void*)ld->row[i].col);
  free_mem((void*)ld->row);
  free_mem((void*)ld->col_title);
  free_mem((void*)ld);

  return;
}
