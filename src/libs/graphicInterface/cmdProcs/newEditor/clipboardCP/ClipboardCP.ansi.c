/* $Id: ClipboardCP.ansi.c,v 1.5 1997/03/11 14:30:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ned_cp/ClipboardCP/ClipboardCP.c				*/
/*									*/
/*	ClipboardCP -- window showing the Rn global scrap		*/
/*	Last edited: July 20, 1989 at 11:58 am				*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>
#include <libs/graphicInterface/cmdProcs/newEditor/clipboardCP/ClipboardCP.i>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




typedef struct
  {
    int nonzero;

  } cb_Repr;

#define	R(ob)		((cb_Repr *) ob)


static short cbcp_index = UNUSED;	/* set when first needed */






/*************************/
/*  Menus		 */
/*************************/


/* main menu buttons */

static
char *cbcp_buttons[] =
{
  "formats"
};

#define FORMATS_BUTTON		0

#define NUM_BUTTONS		1

static
Point cbcp_numButtons = {NUM_BUTTONS, 1};




/* "formats" submenu */

static
char *cbcp_formatButtons[] =
{
  "text"
};

#define FMT_TEXT		0

#define NUM_FMT_BUTTONS		1






/*************************/
/*  Miscellaneous	 */
/*************************/


typedef struct startup
  {
    Context     context;
    Generic avail;
  } Startup;


int cbcp_InitCount = 0;




/*************************/
/*  Forward declarations */
/*************************/


STATIC(Boolean,  cbcp_Start,(Generic cpm));
STATIC(Generic,  cbcp_CreateInstance,(Generic parent_id, Generic cp_id, 
                                      Startup *startup));
STATIC(Boolean,  cbcp_HandleInput,(ClipboardCP cbcp, Generic generator, Generic
                                   event_type, Point info, char *msg));
STATIC(void,     cbcp_DestroyInstance,(ClipboardCP cbcp, Boolean panicked));
STATIC(void,     cbcp_Finish,(void));






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Initialization	*/
/************************/




/* part of interface is exported as structures of "private" proc values */

aProcessor cb_Processor =
  {
    "clipboard",
    false,
    0,
    (cp_root_starter_func)cp_standard_root_starter,
    (cp_start_cp_func)cbcp_Start,
    (cp_create_instance_func)cbcp_CreateInstance,
    (cp_handle_input_func)cbcp_HandleInput,
    (cp_destroy_instance_func)cbcp_DestroyInstance,
    (cp_finish_cp_func)cbcp_Finish,
    CP_UNSTARTED
  };




void cbcp_NewCP(Generic parent, Context context, Generic avail)
{
  Generic cp;
  struct startup *startup;

  startup = (struct startup *) get_mem(sizeof(struct startup), "startup");
  startup->context = context;
  startup->avail = avail;

  cp = cp_new((anInstance*)parent, cbcp_index, (Generic) startup);
  if( cp == UNUSED )
    { message("Can't start the Clipboard");
      abort();
    }
}






/************************/
/*  Change notification	*/
/************************/


/*ARGSUSED*/
void cbcp_NoteChanges(ClipboardCP cbcp, Boolean autoscroll)
{
  /* nothing yet */
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/*ARGSUSED*/

static
Boolean cbcp_Start(Generic cpm)
{
  if( cbcp_InitCount++ == 0 )
    { /* initialize all submodules */

      /* initialize CP window appearance constants */
    }

  return true;
}




/*ARGSUSED*/
static
Generic cbcp_CreateInstance(Generic parent_id, Generic cp_id, Startup *startup)
{
  /* temporary */
  return UNUSED;
}




/*ARGSUSED*/

static
Boolean cbcp_HandleInput(ClipboardCP cbcp, Generic generator, Generic event_type, 
                         Point info, char *msg)
{
  return false;
}




/* ARGSUSED */

static
void cbcp_DestroyInstance(ClipboardCP cbcp, Boolean panicked)
{
}




static
void cbcp_Finish()
{
  if( --cbcp_InitCount == 0 )
    { 
      /* finalize all submodules */

      /* free our global storage */
    }
}



