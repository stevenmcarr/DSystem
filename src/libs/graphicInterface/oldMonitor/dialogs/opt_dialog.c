/* $Id: opt_dialog.c,v 1.2 1997/03/11 14:33:01 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * 			      dialog.c
 * 									
 *			argument list handling
 * 									
 *   FUNCTION:
 *	given a description of an abstract argument list:
 *	    provide a generic dialog for modifying an arg list
 *
 *   USAGE:
 *
 *	
 *	
 *   AUTHOR:
 *	Robert Hood
 */

#include <misc/options.h>

#include <libs/support/msgHandlers/log_msg.h>

#include <varargs.h>
#include <assert.h>
#include <stdio.h>


#include <libs/support/strings/rn_string.h>
#include <libs/graphicInterface/oldMonitor/include/mon/dialog_def.h>
#include <libs/graphicInterface/oldMonitor/include/items/title.h>
#include <libs/graphicInterface/oldMonitor/include/items/check_box.h>
#include <libs/graphicInterface/oldMonitor/include/items/radio_btns.h>
#include <libs/graphicInterface/oldMonitor/include/items/regex.h>

static char *get_optstr();

typedef struct optiondia {
  Generic	*value;

  option_table	 t;
  int		*p_argc;
  char	      ***p_argv;

  char          *exec_name;

  Dialog        *dialog;
  short		 smallFont;
  Boolean	 do_it;
} anOptionDia;


# define OK_BTN		59

static anOptionDia	*option_dlg_create();
static Boolean		 option_dlg_run();
static void		 option_dlg_destroy();

/*
 * if argv[1] starts with a . then launch the opt_dialog, otherwise
 * launch the opt parser directly...
 */
Boolean opt_parse_with_dialog(t, handle, argc, argv)
option_table	 t;
void            *handle;
int		 argc;
char	       **argv;
{
  if (argc == 1 || (argc == 2 && strcmp(argv[1], "...") == 0))
     if (!opt_dialog_ok(t, argv[0], &argc, &argv))
       return false;

  return (opt_parse_argv(t, handle, argc, argv) == 0);
}

/*
 * given a description of the arg list (in t) put up a dialog
 *	that allows the user to modify default values
 *
 * [p_argc, p_argv] are set to be the modified arg_list
 *
 * returns: true iff "OK" selected in dialog
 */
Boolean opt_dialog_ok ( t, exec_name, p_argc, p_argv )
option_table	 t;
char            *exec_name;
int		*p_argc;
char	      ***p_argv;
{
    anOptionDia	*option_dlg;
    Boolean	 okayed;

    option_dlg = option_dlg_create(t, exec_name, p_argc, p_argv);
    okayed = option_dlg_run(option_dlg);
    option_dlg_destroy(option_dlg);

    return okayed;
}



/*ARGSUSED*/
static Boolean option_dlg_handler ( di, od, item )
Dialog          *di;                    /* the dialog that has been mod.*/
anOptionDia     *od;                    /* the options dialog           */
Generic          item;                  /* the item which has changed   */
{
  switch ( item ) {
    case DIALOG_CANCEL_ID:
      return DIALOG_QUIT;
    
    case OK_BTN:
    case DIALOG_DEFAULT_ID:
      od->do_it = true;
      return DIALOG_QUIT;
    
    default:
      return DIALOG_NOMINAL;
  }
}



static anOptionDia *option_dlg_create ( t, exec_name, p_argc, p_argv )
option_table	 t;
char            *exec_name;
int		*p_argc;
char	      ***p_argv;
{
  anOptionDia   *od;
  DiaDesc	*dd, *dd1;
  int		 i, j;
  static char	*temp_str[10] = {"", "", "", "", "", "", "", "", "", ""};
  static Generic temp_hdl[10] = { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0};

  struct option_	*te;
  struct choice_	*ch;
  struct string_	*st;
  struct flag_		*fl;

  od = (anOptionDia*)get_mem(sizeof(anOptionDia), "option dialog");

  od->t		= t;
  od->p_argc	= p_argc;
  od->p_argv	= p_argv;
  od->exec_name = exec_name;

  od->smallFont = fontOpen("screen.7.rnf");
  od->do_it     = false;
  od->value	= (Generic *)get_mem(sizeof(Generic)*t->num_opts,
				     "option_dlg_create()");

  dd = item_title(UNUSED, "    ", DEF_FONT_ID);

  for ( i = 0; i < t->num_opts; i++ ) {
    te = &t->table[i];
    if ( te->in_dialog ) {
      switch ( te->t ) {
        case flag:
          fl = (struct flag_ *)te->f_c;
	  od->value[i] = te->init_value;
	  dd1 = item_check_box(
			       fl->callback,
			       fl->name,
			       DEF_FONT_ID,
			       &od->value[i]
			       );     
	  break;
	  
	case choice:
	  ch = (struct choice_ *)te->f_c;
	  for ( j = 0; j < ch->num_choices; j++ ) {
	    temp_str[j] = ch->choice[j].name;
	    temp_hdl[j] = ch->choice[j].choice_handle;
	  }
	  od->value[i] = te->init_value;
	  dd1 = item_radio_buttons(
				   ch->callback,
				   ch->name,
				   DEF_FONT_ID,
				   &od->value[i],
				   1, ch->num_choices,
				   temp_str[0], temp_hdl[0],
				   temp_str[1], temp_hdl[1],
				   temp_str[2], temp_hdl[2],
				   temp_str[3], temp_hdl[3],
				   temp_str[4], temp_hdl[4],
				   temp_str[5], temp_hdl[5],
				   temp_str[6], temp_hdl[6],
				   temp_str[7], temp_hdl[7],
				   temp_str[8], temp_hdl[8],
				   temp_str[9], temp_hdl[9]
				   );    
	  break;
	  
	case string:
	  st = (struct string_ *)te->f_c;
	  od->value[i] = (Generic)ssave(te->init_value);
	  dd1 = item_regex(st->callback,
			   st->name,
			   DEF_FONT_ID,
			   &od->value[i],
			   st->regex,
			   st->display_len
			   );     
	  break;
	  
	default:
	  log_msg(001, "error in options.c:opt_dialog_ok():  %s loc=%d\n",
		  "unknown arg type in table", i);
	  dd1 = item_title(UNUSED, "??? error in arg table ???", DEF_FONT_ID);
	  break;
	} /* switch */
      
      dd = dialog_desc_group(DIALOG_VERT_CENTER, 2, dd, dd1);
    } /* if in_dialog */
  } /* for */

  od->dialog = dialog_create(t->title,
			     option_dlg_handler,
			     (PFV) 0,
			     (Generic) od,
			     dialog_desc_group(DIALOG_VERT_CENTER,
					       2,
					       dd,
					       dialog_desc_expand(
						   item_button(OK_BTN,
							       "  OK  ",
							       od->smallFont,
							       true
							       )
								  ))
			     );
  return (od);
}


static void option_dlg_destroy ( od )
anOptionDia         *od;
{
  fontClose(od->smallFont);
  dialog_destroy(od->dialog);
  free_mem((Generic) od);
}


static Boolean option_dlg_run ( od )
anOptionDia         *od;
{
  int			 i, j, num_choices;
  char		       **argv;
  int			 argc;
  int			 nextarg;
  struct option_	*te;
  struct choice_	*ch;
  char			 buf[256];

  dialog_modal_run(od->dialog);
  if ( od->do_it ) {
    /* build up the arg list */
    num_choices = 0;
    for ( i = 0; i < od->t->num_opts; i++ )
      if ( od->t->table[i].t == choice || od->t->table[i].t == string )
	num_choices++;
    
    argc = od->t->num_opts+num_choices + 1;
    argv = (char **)get_mem(sizeof(char *) * (argc + 1), "option_dlg_run()");

    argv[0] = ssave(od->exec_name);
    nextarg = 1;
    for ( i = 0; i < od->t->num_opts; i++ ) {
      te = &od->t->table[i];
      if ( te->in_dialog ) {
	if ( (te->t == flag || te->t == choice) && od->value[i]!=te->init_value
	    || te->t == string && strcmp(od->value[i], te->init_value)
	    ) {
	  argv[nextarg] = ssave(sprintf(buf, "-%c", te->arg_char));
	  nextarg++;
	  if ( te->t == choice ) {
	    ch = (struct choice_ *)te->f_c;
	    j = 0;
	    while ( j < ch->num_choices &&
		   od->value[i] != ch->choice[j].choice_handle ) {
	      j++;
	    }
	    if ( od->value[i] == ch->choice[j].choice_handle ) {
	      argv[nextarg] = ssave(ch->choice[j].arg_str);
	      nextarg++;
	    } else {
	      log_msg(001, "error in options.c  %s loc=%d [0x%x]\n",
		      "unknown choice value",
		      i, ch->choice[j].choice_handle);
	    }
	  } else if ( te->t == string ) {
	    argv[nextarg] = ssave(od->value[i]);
	    nextarg++;
	  }
	}
      }
    }
    argv[nextarg] = 0;

    *od->p_argc = nextarg;
    *od->p_argv = argv;

    for ( i = 1; i < nextarg; i++ )
      fprintf(stderr, "%d:'%s' ", i, argv[i]);

    fprintf(stderr, "\n");

    return true;
  }
  return false;
}
