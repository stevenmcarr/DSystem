/* $Id: tedprivate.h,v 1.14 1997/06/25 14:59:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/****************************************************************/
/*								*/
/*			tedprivate.h				*/
/*								*/
/****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/support/lists/list.h>
#include <libs/support/stacks/xstack.h>
#include <libs/graphicInterface/oldMonitor/include/sms/optim_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/scroll_sm.h>
#include <libs/graphicInterface/oldMonitor/monitor/keyboard/keymap.h>
#include <libs/support/strings/rn_string.h>
#include <string.h>
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/iflex.h>
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/undo.h>
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/undo_dialog.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/find.h>
#include <libs/support/patternMatch/bmsearch.h>
#include <libs/graphicInterface/oldMonitor/include/sms/ted_sm.h>
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/damage.h>


#ifdef DEBUG
# define COREDUMP() abort()
# define ASSERT(bool)	{if ( !(bool) ) COREDUMP();} 
#else
# define COREDUMP()
# define ASSERT(bool)
#endif

#define IN_RANGE(lb,i,ub)	( (lb) <= (i) && (i) <= (ub)  )		/* boolean */

/* a few macros for handling characters */

#define WHITE	(unsigned char)0x01
#define DIGIT	(unsigned char)0x02
#define UPPER	(unsigned char)0x04
#define LOWER	(unsigned char)0x08
#define CTRL	(unsigned char)0x10
#define PUNC	(unsigned char)0x20
#define TABCH	(unsigned char)0x40
#define META	(unsigned char)0x80

#define IS_WHITE(c)	((char_tab[(c)] & WHITE) != 0)
#define IS_DIGIT(c)	((char_tab[(c)] & DIGIT) != 0)
#define IS_ALPHA(c)	((char_tab[(c)] & (UPPER | LOWER)) != 0 )
#define IS_ALPHA_NUM(c)	((char_tab[(c)] & (UPPER | LOWER |DIGIT)) != 0 )
#define IS_UPPER(c)	((char_tab[(c)] & UPPER) != 0)
#define IS_LOWER(c)	((char_tab[(c)] & LOWER) != 0)
#define IS_META(c)	((char_tab[(c)] & META) != 0)
#define IS_CONTROL(c)	((char_tab[(c)] & CTRL) != 0)

#define UPPERCASE(c)	(char_tab[(c)] & LOWER ? (c) - ('a' - 'A') : (c) )
#define LOWERCASE(c)	(char_tab[(c)] & UPPER ? (c) + ('a' - 'A') : (c) )

#define TAB	(CTRL + WHITE + TABCH)
#define LF	(CTRL + WHITE)
#define RET	(CTRL)
#define SPACE	(WHITE)
#define DEL	(CTRL)

#define MTAB	(META + TAB)
#define MLF	(META + LF)
#define MRET	(META + RET)
#define MSPACE	(META + SPACE)
#define MDEL	(META + DEL)
#define MCTRL	(META + CTRL)
#define MDIGIT	(META + DIGIT)
#define MPUNC	(META + PUNC)
#define MUPPER	(META + UPPER)
#define MLOWER	(META + LOWER)

extern unsigned char char_tab[];

/* buffer types */
typedef enum {
	FILEBUFFER,
	SCRATCHBUFFER,
	DOCUMENTBUFFER,
	MESSAGEBUFFER
} Buftype;

/* styles for drawing ctrl/meta chars */
typedef enum {
	VERBATIM,
	CARET,
	OCTAL
} Ctrlstyle;

short	SM_TED_OPTIM_INDEX;

typedef struct TedBufMark {
	char *markname;		/* name of mark */
	int   location; 	/* offset into buffer of mark */
} TedMark;

typedef struct TedBufStruct {
	char *bufname;
	char *filename;
	char *pathname;
	char *ml_format;		/* mode line format string */
	sm_ted_buftype buftype;		/* type of this buffer */
	int modified;			/* number of modifications */
	
	Flex *buffer;			/* pointer to all buffer storage info */
	int num_lines;			/* number of lines in the buffer */

	int mark;			/* buffer position of mark */
	Flex *marklist;			/* list of named marks */
	int num_marks;			/* number of marks in list */
	char *copy_buffer;		/* the buffer used in cut and paste */
	char *delete_buffer;		/* the most recently deleted group of text */

	Stack history;			/* the undo stack */
	Undo  undo;
	Undia *und;			/* the undo dialog */

	Generic owner;			/* Information private to the owner of this buffer */
	UtilList *my_wins;		/* list of windows into this buffer */

	int	save_behavior;		/* used to signal saveas and saveacopy */
        sm_ted_modify_callback mod_function;           /* used to signal REPLACE mods */
} TedBuf;

typedef struct TedWinStruct {
	/* screen dimensions, framing, positions */
	Pane		*optim_pane;		/* The optim_sm slave pane */
	Pane		*my_pane;		/* The pane to which this TedWin is attached */
	ScrollBar	vert;			/* The vertical scrollbar for scrolling this pane */
	ScrollBar	horz;			/* The horizontal scrollbar for scrolling this pane */
	TextChar	*tcp;			/* One screen line, for efficient communication with sm_optim */
	Point		size;			/* size of this text pane */
	Boolean		active;			/* Show cursor, handle mouse events iff true */
	short		topskip;		/* number of lines at top of pane that aren't to be used */
	short		botskip;		/* number of lines at bottom of pane that aren't to be used */
	float		page_fraction;		/* fraction of screen by which to page */
	int		prefer_col;		/* column in which the cursor would like to be */
	int		bot_scrline;		/* line number of last used screen line */
	int		dot_line;		/* line # in buffer of dot in this pane */
	int		dot_offset;		/* buffer position of dot */
	int		old_dot_offset;		/* buffer position of dot after last redisplay */
	Point		dot_loc;		/* pane position of dot */
	Point		old_dot_loc;		/* pane position of dot after last redisplay */
	Point		last_click;		/* last click location */
	int		multiplier;		/* a multiplier for function calls */
	UtilList	*damage;		/* list of various damages to the window since last redisplay */
	sm_ted_message_callback message_handler;	/* the callback for message printing */
	Generic		owner_id;		/* the owner id for the message call back */
	BMS_private	*bms;			/* boyer-moyer searching info */
	char 		*rpattern;		/* a pattern to be inserted */
	int		rpat_length;		/* the length of that pattern */
	aFRDia		*frd;			/* the find/replace dialog */

	/* pane specific variables */
	short tsize;			/* maximum size of a tab character (usually 8, always > 0) */
	unsigned char style;		/* style that this pane uses {NORMAL, BOLD, UNDERLINED, INVERSE} */
	short font;			/* font to use in this pane */
	Ctrlstyle ctrlstyle;		/* style of ctrl chars (VERBATIM, CARET, OCTAL) */

	Keymap		*keymap;	/* key mapping information */
	Keymap		*eventmap;	/* event mapping information */
	UtilNode *owner;		/* node in list of panes owned by a buffer */
} TedWin;


/* a collection of one of each of the above structures, wrapped into a package, for passing around */
typedef struct TedStruct {
	TedBuf *buf;
	TedWin *win;
} Ted;


#define INFO(p)		((Ted *)((p)->pane_information))
#define BUF(p)		((INFO(p)->buf))
#define WIN(p)		((INFO(p)->win))

#define STORE(p)	(BUF(p)->buffer)

#define BUFNAME(p)	(BUF(p)->bufname)
#define FILENAME(p)	(BUF(p)->filename)
#define PATHNAME(p)	(BUF(p)->pathname)
#define MODS(p)		(BUF(p)->modified)
#define TYPE(p)		(BUF(p)->buftype)
#define CONTEXT(p)	(BUF(p)->cd)
#define FORMAT(p)	(BUF(p)->ml_format)

#define MY_PANE(p)	(WIN(p)->my_pane)
#define VERT(p)		(WIN(p)->vert)
#define HORZ(p)		(WIN(p)->horz)
#define OP(p)		(WIN(p)->optim_pane)

#define OPTIM_LINE(p)	(WIN(p)->tcp)
#define SIZE(p)		(WIN(p)->size)
#define PAGE_FRAC(p)	(WIN(p)->page_fraction)
#define ACTIVE(p)	(WIN(p)->active)
#define STYLE(p)	(WIN(p)->style)
#define FONT(p)		(WIN(p)->font)
#define DAMAGE(p)	(WIN(p)->damage)
#define BMS(p)		(WIN(p)->bms)
#define RPATTERN(p)	(WIN(p)->rpattern)
#define RLENGTH(p)	(WIN(p)->rpat_length)
#define FRD(p)		(WIN(p)->frd)
#define KEY_MAP(p)	(WIN(p)->keymap)
#define EVENT_MAP(p)	(WIN(p)->eventmap)
#define TOPSKIP(p)	(WIN(p)->topskip)
#define BOTSKIP(p)	(WIN(p)->botskip)
#define LASTLINE(p)	(WIN(p)->bot_scrline)
#define	MESG_HANDLER(p) (WIN(p)->message_handler)
#define OWNER_ID(p)	(WIN(p)->owner_id)
#define MARK(p)		(BUF(p)->mark)
#define MARKLIST(p)	(BUF(p)->marklist)
#define NUM_MARKS(p)	(BUF(p)->num_marks)
#define COPYBUF(p)	(BUF(p)->copy_buffer)
#define KILLBUF(p)	(BUF(p)->delete_buffer)
#define HISTORY(p)	(BUF(p)->history)
#define UNDO(p)		(BUF(p)->undo)
#define UND(p)		(BUF(p)->und)
#define DOT(p)		(WIN(p)->dot_offset)
#define DOTLINE(p)	(WIN(p)->dot_line)
#define LINES(p)	(BUF(p)->num_lines)
#define OLD_DOT(p)	(WIN(p)->old_dot_offset)
#define DOT_LOC(p)	(WIN(p)->dot_loc)
#define OLD_DOT_LOC(p)	(WIN(p)->old_dot_loc)
#define PREFER(p)	(WIN(p)->prefer_col)
#define CLICK_AT(p)	(WIN(p)->last_click)
#define MULTIPLIER(p)   (WIN(p)->multiplier)
#define TAB_SIZE(p)	(WIN(p)->tsize)
#define CTRL_STYLE(p)	(WIN(p)->ctrlstyle)
#define WIN_OWNER(p)	(WIN(p)->owner)
#define WIN_LIST(p)	(BUF(p)->my_wins)
#define BUF_OWNER(p)	(BUF(p)->owner)

#define SAVE_BEHAV(p)	(BUF(p)->save_behavior)

#define BEHAV_SAVEIT	0
#define BEHAV_SAVEAS	1
#define BEHAV_SAVEACOPY	2


#define NO_MESG_HANDLER ((sm_ted_message_callback) 0) 

#define EOL 	2147483647	/* set cw->prefer_col to this to track eol */
				/* 1024*1024*1024*2 - 1 == 2^31 - 1 == 2147483647 */

#define BUFNAMELEN 40		/* length of buffer name */
#define FILENAMELEN 40		/* length of a file name */

#define NOT_SET -1		/* mark's have no initial positions in the buffer */

extern int min_hole_size;		/* try not to let hole get smaller than this */
extern int new_hole_size;		/* size to make the hole, if it is too small */

#define NULLSTRING	(char *) -1


/* FUNCTIONS */

/* sm_ted.c */
EXTERN(void, sm_ted_inform,(Pane *p, char *format, ...));
EXTERN(void, sm_ted_bitch,(Pane *p, char *format, ...));
EXTERN(void, sm_ted_resize_internal,(Pane *p));
EXTERN(TedBuf*, sm_ted_buf,(Pane *p));
EXTERN(TedWin*, sm_ted_win,(Pane *p));


/* buffers.c */
EXTERN(char, sm_ted_buf_get_char,(register Pane *p, register int offset));
EXTERN(void, sm_ted_buf_set_char,(register Pane *p, register int offset, char ch));
EXTERN(void, sm_ted_buf_insert_char,(Pane *p, char ch));
EXTERN(void, sm_ted_buf_insert_n_chars,(Pane *p, char *str, int len));
EXTERN(char*, sm_ted_buf_delete_char,(Pane *p));
EXTERN(char*, sm_ted_buf_delete_nprev_chars,(Pane *p, int amount));
EXTERN(char*, sm_ted_buf_delete_n_chars,(Pane *p, int n));
EXTERN(void, sm_ted_buf_insert_buf,());
EXTERN(Boolean, sm_ted_empty_buf,());
EXTERN(void, sm_ted_buf_erase,(Pane *p));
EXTERN(void, sm_ted_print_info,(Pane *p));
EXTERN(Boolean, sm_ted_buf_save_doit,());
EXTERN(Boolean, sm_ted_buf_write_current_file,());
EXTERN(void, sm_ted_buf_set_mark,(Pane *p, int num_mark));
EXTERN(int, sm_ted_buf_name_unique_mark,(Pane *p, char *title));
EXTERN(void, sm_ted_buf_delete_mark_number,(Pane *p, int num));
EXTERN(Boolean, sm_ted_buf_repeat_mark_name,(Pane *p, char *name));
EXTERN(void, sm_ted_buf_mark_modify,(Pane *p));
EXTERN(char*, sm_ted_buf_complete_file_name,(Pane *p, char *name));
EXTERN(char*, sm_ted_buf_get_pname,(Pane *p));
EXTERN(Boolean, sm_ted_buf_is_empty,(Pane *P)); 

/* win.c */
/* create the structures associated with a pane */
EXTERN(void, sm_ted_win_create_internal,(Pane *p));
/* destroy the structures associated with a pane */
EXTERN(char, sm_ted_win_get_char,(register Pane *p));
EXTERN(void, sm_ted_win_adjust,(Pane *p, int at, int elts));
EXTERN(TedWin*, TedWin_of,(UtilNode *node));

/* window.c */
/*int sm_ted_dot_to_click();*/ 
EXTERN(void, sm_ted_refresh,(Pane *p));

/* move.c */
EXTERN(void, sm_ted_page_forward,(Pane *p));
EXTERN(void, sm_ted_page_back,(Pane *p));
EXTERN(void, sm_ted_line_to_top_of_window,(Pane *p));
EXTERN(void, sm_ted_top_of_window_to_line,(Pane *p));
EXTERN(void, sm_ted_scroll_up,(Pane *p));
EXTERN(void, sm_ted_scroll_down,(Pane *p));
EXTERN(void, sm_ted_prev_line,(Pane *p));
EXTERN(void, sm_ted_next_line,(Pane *p));
EXTERN(void, sm_ted_beginning_of_line,(Pane *p));
EXTERN(void, sm_ted_end_of_line,(Pane *p));
EXTERN(void, sm_ted_prev_char,(Pane *p));
EXTERN(void, sm_ted_next_char,(Pane *p));
EXTERN(void, sm_ted_beginning_of_buffer,(Pane *p));
EXTERN(void, sm_ted_end_of_buffer,(Pane *p));
EXTERN(void, sm_ted_scroll,(Pane *p));
EXTERN(void, sm_ted_forward_word,(Pane *p));
EXTERN(void, sm_ted_backward_word,(Pane *p));
EXTERN(void, sm_ted_win_goto_mark,(Pane *p, int num_mark));

/* insert.c */
EXTERN(void, sm_ted_insert_char,(Pane *p));
EXTERN(void, sm_ted_insert_line,(Pane *p));
EXTERN(void, sm_ted_newline,(Pane *p));
EXTERN(void, sm_ted_newline_and_backup,(Pane *p));
EXTERN(void, sm_ted_insert_copy_buffer,(Pane *p));
EXTERN(void, sm_ted_insert_kill_buffer,(Pane *p));

/* delete.c */
EXTERN(int, sm_ted_delete_next_char,(Pane *p));
EXTERN(int, sm_ted_delete_prev_char,(Pane *p));
EXTERN(int, sm_ted_delete_previous_word,(Pane *p));
EXTERN(int, sm_ted_delete_next_word,(Pane *p));
EXTERN(int, sm_ted_kill_to_end_of_line,(Pane *p));
EXTERN(int, sm_ted_kill_to_beginning_of_line,(Pane *p));
EXTERN(int, sm_ted_kill_line,(Pane *p));
EXTERN(int, sm_ted_delete_from_dot_to_mark,(Pane *p));
EXTERN(int, sm_ted_copy_from_dot_to_mark,(Pane *p));

/* misc.c */
EXTERN(void, sm_ted_unbound_key,(Pane *p));
EXTERN(void, sm_ted_transpose_chars,(Pane *p));
EXTERN(void, sm_ted_set_multiplier,(Pane *p));
EXTERN(void, sm_ted_bell,(Pane *p));
EXTERN(int, sm_ted_count_lines,(char *str));
EXTERN(void, sm_ted_case_word_upper,(Pane *p));
EXTERN(void, sm_ted_case_word_lower,(Pane *p));
EXTERN(void, sm_ted_case_word_capitilize,(Pane *p));

/* bind.c */
EXTERN(int, sm_ted_quote_char,(void));
EXTERN(void, sm_ted_default_bindings,(Pane *p));

/* hole.c */
EXTERN(void,sm_ted_hole_to,(void));
EXTERN(void,sm_ted_enlarge_hole,(void));

/* mode.c */
EXTERN(void, sm_ted_octal_mode,(Pane *p));
EXTERN(void, sm_ted_caret_mode,(Pane *p));
EXTERN(void, sm_ted_verbatim_mode,(Pane *p));

/* display.c */
EXTERN(void, sm_ted_rewrite_line,(register Pane *p, Point loc));
EXTERN(void, sm_ted_rewrite_line_to_bot,(register Pane *p, short lineno));
EXTERN(void, sm_ted_rewrite_win,());
/*EXTERN(void, sm_ted_damage_add,());*/
EXTERN(Boolean, sm_ted_dot_on_screen,(Pane *p));
EXTERN(void, sm_ted_remove_cursor,(Pane *p));
EXTERN(void, sm_ted_place_cursor,(Pane *p));
EXTERN(void, sm_ted_repair,(Pane *p));
EXTERN(int, sm_ted_screenline_of,(Pane *p, register int off));
EXTERN(Boolean, sm_ted_same_line,(Pane *p, int off1, int off2));
EXTERN(WinChange, *Damage_of,(UtilNode *node));
EXTERN(void, sm_ted_damaged_buffer,(Pane *p));
EXTERN(void, sm_ted_damaged_dot_row,(Pane *p));
EXTERN(void, sm_ted_damaged_dot_col,(Pane *p));
EXTERN(void, sm_ted_damaged_prefer,(Pane *p));
EXTERN(void, sm_ted_dot_to_click,(Pane *p));

/* putchar.c */
/*short sm_ted_width_of_text();*/
/*short sm_ted_width_of_text_preferred();*/


/* region.c */
int sm_ted_move_region();
int sm_ted_copy_region();
int sm_ted_prepend_region();
int sm_ted_append_region();
int sm_ted_delete_region();
/*int sm_ted_set_mark();*/
int sm_ted_exchange_dot_and_mark();

/* dot.c */
EXTERN(Boolean, sm_ted_goto_next_line,(Pane *p));
EXTERN(Boolean, sm_ted_goto_prev_line,(Pane *p));
EXTERN(Boolean, sm_ted_offset_on_screen,());
EXTERN(void, sm_ted_goto_mark,(Pane *p));
EXTERN(void, sm_ted_set_mark,(Pane *p));
EXTERN(void, sm_ted_set_dot,(Pane *p, int nd));
EXTERN(void, sm_ted_set_dot_relative,(Pane *p, int del));
EXTERN(void, sm_ted_goto_bol,(Pane *p));
EXTERN(int, sm_ted_whereis_bol,(Pane *p, int offset));
EXTERN(int, sm_ted_at_bol,(Pane *p));
EXTERN(void, sm_ted_goto_eol,(Pane *p));
EXTERN(int, sm_ted_whereis_eol,(Pane *p, int offset));
EXTERN(int, sm_ted_at_eol,(Pane *p));
EXTERN(void, sm_ted_goto_bob,(Pane *p));
EXTERN(int, sm_ted_at_bob,(Pane *p));
EXTERN(void, sm_ted_goto_eob,(Pane *p));
EXTERN(int, sm_ted_at_eob,(Pane *p));
EXTERN(int, sm_ted_whereis_next_line,(Pane *p, int offset));
EXTERN(int, sm_ted_whereis_prev_line,(Pane *p, int offset));
EXTERN(int, sm_ted_goto_line_rel,(Pane *p, int delta));
EXTERN(int, sm_ted_whereis_line_rel,(Pane *p, int delta, int offset));
EXTERN(int, sm_ted_line_number,(Pane *p));
EXTERN(int, sm_ted_lines,(Pane *p));
EXTERN(int, sm_ted_tab_size,(Pane *p, short col));
EXTERN(int, sm_ted_ctrl_size,(Pane *p, short col));
EXTERN(short, sm_ted_width_of_text,(Pane *p, int offset, register short coloff,
                                    register short bytes));
EXTERN(short, sm_ted_width_of_text_preferred,(Pane *p, int offset, int maxbytes,
                                              short coloff, int prefer_col,
                                              int *bytes));
EXTERN(void, sm_ted_find_dot_using_prefer,(Pane *p));
EXTERN(void, sm_ted_find_prefer_using_doffset,(Pane *p));
EXTERN(int, sm_ted_whereis_beginning_of_word,(Pane *p, int offset));
EXTERN(int, sm_ted_whereis_end_of_word,(Pane *p));

/* search.c */
EXTERN(Boolean, sm_ted_search,(Pane *p));
/*int sm_ted_search_alone();*/
EXTERN(int, sm_ted_search_forward,(Pane *p));
EXTERN(int, sm_ted_search_backward,(Pane *p));
EXTERN(void, sm_ted_new_replace_pattern,());
EXTERN(void, sm_ted_new_search_pattern,());
EXTERN(int, sm_ted_global_replace,(Pane *p));
EXTERN(int, sm_ted_forward_replace,(Pane *p));
EXTERN(int, sm_ted_backward_replace,(Pane *p));
EXTERN(Boolean, sm_ted_finder,(Pane *p, aFRDia *frd, char *what, Boolean dir,
                               Boolean case_fold));
EXTERN(void, sm_ted_replacer,(Pane *p, aFRDia *frd, char *what, 
                                 Boolean case_fold, char *replace));
EXTERN(int, sm_ted_global_replacer,(Pane *p, aFRDia *frd, char *what, 
                                    Boolean global, Boolean dir, 
                                    Boolean case_fold, char *replace));
EXTERN(void, sm_ted_find_dialog_run_find,(Pane *p));
EXTERN(void, sm_ted_find_dialog_run_replace,(Pane *p));
EXTERN(void, sm_ted_replace_one,(Pane *p));
EXTERN(void, sm_ted_find_dialog_dirty,(Pane *p));

/* undo.c */
EXTERN(void, sm_ted_dot_record,(Pane *p));
EXTERN(void, sm_ted_undo,(Pane *p));
EXTERN(void, sm_ted_undo_one,(Pane *p));
EXTERN(void, sm_ted_event_continuous,(Pane *p));
EXTERN(void, sm_ted_event_disjoint,(Pane *p));
EXTERN(Boolean, sm_ted_continue,(Pane *p, short curvature, int depth, int tos));
EXTERN(void, sm_ted_undo_damage,(Pane *p, Undo event, int old_dot));
EXTERN(void, sm_ted_undo_destroy,(Pane *p));

/*menu.c*/
EXTERN(Boolean, sm_ted_menu_copy,(Pane *p));
EXTERN(Boolean, sm_ted_menu_cut,(Pane *p));
EXTERN(Boolean, sm_ted_menu_find,(Pane *p));
EXTERN(Boolean, sm_ted_menu_replace,(Pane *p));

/*scroll.c*/
EXTERN(void, sm_ted_handle_scroll_horizontal,(Pane *p, ScrollBar sb, short h_or_v,
                                            int loc));
EXTERN(void, sm_ted_handle_scroll_vertical,(Pane *p, ScrollBar sb, short h_or_v,
                                            int newtop));
EXTERN(void, sm_ted_win_set_scroller_horizontal,(Pane *p, ScrollBar hs));
EXTERN(void, sm_ted_win_set_scroller_vertical,(Pane *p, ScrollBar vs));
EXTERN(void, sm_ted_scroll_reset,(Pane *p));





