/* $Id: bind.C,v 1.1 1997/06/25 14:59:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/tedprivate.h>

/* some unprincipled holes in the abstraction 
 *
 * These are functions provided in menu.c in order to ensure that
 * the keyboard bound functions behave like the menu driven functions.
 *
 *						- kdc 30 Sept 88
 *
 */

void sm_ted_default_bindings (Pane *p);

Keymap *verbatim_map;	/* Initialized by sm_ted_start() */

static KbString esc_prefix(KbChar kbc)
{
	KbString kbs;

	kbs = getKbString(2,"esc_prefix");
	kbs.ephemeral = true;
	kbs.kc_ptr[0] = KB_Escape;
	kbs.kc_ptr[1] = kbc;

	return kbs;
}

void sm_ted_default_bindings(Pane *p)
{
	Keymap *map;

	map = keymap_create_fancy( (Generic)sm_ted_bell, (Generic)sm_ted_insert_char );
	KEY_MAP(p) = map;


	/*
	 * Set up the binding for quote character (hack)
	 */
	keymap_bind_range(map, (Generic) sm_ted_insert_char, makeKbString("\021", "sm_ted_default_bindings"), KB_first_ascii, KB_last_ascii);

	/*
	 * Do all the coke-bottle style (oxymoron?) bindings.
	 */
#define BIND(func,str)	keymap_bind_KbString(map, (Generic)func, makeKbString(str, "sm_ted_default_bindings"))

	BIND( sm_ted_beginning_of_line,		"\001"		);	/* ^A */
	BIND( sm_ted_prev_char,			"\002"		);	/* ^B */
	BIND( sm_ted_delete_next_char,		"\004"		);	/* ^D */
	BIND( sm_ted_end_of_line,		"\005"		);	/* ^E */
	BIND( sm_ted_next_char,			"\006"		);	/* ^F */
	BIND( sm_ted_kill_to_end_of_line,	"\013"		);	/* ^K */
	BIND( sm_ted_refresh,			"\014"		);	/* ^L */
	BIND( sm_ted_next_line,			"\016"		);	/* ^N */
	BIND( sm_ted_newline_and_backup,	"\017"		);	/* ^O */
	BIND( sm_ted_prev_line,			"\020"		);	/* ^P */
	BIND( sm_ted_search_backward,		"\022"		);	/* ^R */
	BIND( sm_ted_search_forward,		"\023"		);	/* ^S */
	BIND( sm_ted_transpose_chars,		"\024"		);	/* ^T */

	BIND( sm_ted_page_forward,		"\026"		);	/* ^V */
	BIND( sm_ted_insert_kill_buffer,	"\031"		);	/* ^Y */
	BIND( sm_ted_scroll_up,			"\032"		);	/* ^Z */

	BIND( sm_ted_buf_erase,			"\030\005"	);      /* ^X^E */


	/*
	 * Do some fancier hard-wired bindings to give named keys meaning.
	 */
#undef BIND
#define BIND(func,kbc)	keymap_bind_KbChar(map, (Generic)func, kbc)

	BIND( sm_ted_bell,			KB_Bell		);
	if (kb_swap_bs_del)
	{
		BIND( sm_ted_delete_prev_char,		KB_Delete	);
		BIND( sm_ted_delete_next_char,		KB_Backspace	);
	}
	else
	{
		BIND( sm_ted_delete_prev_char,		KB_Backspace	);
		BIND( sm_ted_delete_next_char,		KB_Delete	);
	}
	BIND( sm_ted_insert_char,		KB_Tab		);
	BIND( sm_ted_insert_line,		KB_Linefeed	);
	BIND( sm_ted_newline,			KB_Enter	);

	BIND( sm_ted_beginning_of_line,		KB_right(1)	);
	BIND( sm_ted_set_mark,			KB_right(2)	);
	BIND( sm_ted_end_of_line,		KB_right(3)	);
	BIND( sm_ted_search_backward,		KB_right(4)	);
	BIND( sm_ted_copy_from_dot_to_mark,	KB_right(5)	);
	BIND( sm_ted_search_forward,		KB_right(6)	);
	BIND( sm_ted_backward_word,		KB_right(7)	);
	BIND( sm_ted_forward_word,		KB_right(9)	);
	BIND( sm_ted_insert_copy_buffer,	KB_right(11)	);
	BIND( sm_ted_page_back,			KB_right(13)	);
	BIND( sm_ted_page_forward,		KB_right(15)	);
	
	BIND( sm_ted_menu_copy,			KB_top(2)	);
	BIND( sm_ted_menu_cut,			KB_top(3)	);
	BIND( sm_ted_insert_copy_buffer,	KB_top(4)	); /* paste */
	BIND( sm_ted_set_mark,			KB_top(5)	);

	BIND( sm_ted_prev_char,			KB_ArrowL	);
	BIND( sm_ted_next_char,			KB_ArrowR	);
	BIND( sm_ted_prev_line,			KB_ArrowU	);
	BIND( sm_ted_next_line,			KB_ArrowD	);

#undef BIND
#define BIND(func,kbs)	keymap_bind_KbString(map, (Generic)func, kbs)

	BIND( sm_ted_backward_word,		esc_prefix(toKbChar('b'))	);
	BIND( sm_ted_case_word_capitilize,	esc_prefix(toKbChar('c'))	);
	BIND( sm_ted_delete_next_word,		esc_prefix(toKbChar('d'))	);
	BIND( sm_ted_delete_from_dot_to_mark,	esc_prefix(toKbChar('e'))	);
	BIND( sm_ted_forward_word,		esc_prefix(toKbChar('f'))	);
	BIND( sm_ted_goto_mark,			esc_prefix(toKbChar('g'))	);
	BIND( sm_ted_delete_previous_word,	esc_prefix(toKbChar('h'))	);
	BIND( sm_ted_set_mark,			esc_prefix(toKbChar('i'))	);
	BIND( sm_ted_kill_to_beginning_of_line,	esc_prefix(toKbChar('k'))	);
	BIND( sm_ted_case_word_lower,		esc_prefix(toKbChar('l'))	);
	BIND( sm_ted_kill_line,			esc_prefix(toKbChar('n'))	);
	BIND( sm_ted_case_word_upper,		esc_prefix(toKbChar('u'))	);
	BIND( sm_ted_page_back,			esc_prefix(toKbChar('v'))	);
	BIND( sm_ted_copy_from_dot_to_mark,	esc_prefix(toKbChar('w'))	);
	BIND( sm_ted_scroll_down,		esc_prefix(toKbChar('z'))	);
	BIND( sm_ted_beginning_of_buffer,	esc_prefix(toKbChar('<'))	);
	BIND( sm_ted_end_of_buffer,		esc_prefix(toKbChar('>'))	);

	BIND( sm_ted_kill_to_beginning_of_line,	esc_prefix(KB_right(1))		);
	BIND( sm_ted_kill_to_end_of_line,	esc_prefix(KB_right(3))		);
	BIND( sm_ted_delete_previous_word,	esc_prefix(KB_right(7))		);
	BIND( sm_ted_delete_next_word,		esc_prefix(KB_right(9))		);
	BIND( sm_ted_beginning_of_buffer,	esc_prefix(KB_right(13))	);
	BIND( sm_ted_end_of_buffer,		esc_prefix(KB_right(15))	);
}

static void sm_ted_ted_cp_bindings(Pane *p)
{
	Keymap *map;

	map = KEY_MAP(p);

#undef  BIND
#define BIND(func,str)	keymap_bind_KbString(map,(Generic)func, makeKbString(str, "sm_ted_default_bindings"))

	BIND( sm_ted_set_multiplier,		"\025"		);	/* ^U */
	BIND( sm_ted_print_info,		"\030\020"	);	/* ^X^P */
	BIND( sm_ted_undo,			"\030\025"	);	/* ^X^U */

	/*
	 * Do some fancier hard-wired bindings to give named keys meaning.
	 */
#undef BIND
#define BIND(func,kbc)	keymap_bind_KbChar(map, (Generic)func, kbc)

	BIND( sm_ted_undo,			KB_top(1)	);
	BIND( sm_ted_buf_mark_modify,		KB_top(6)	);

#undef BIND
#define BIND(func,kbs)	keymap_bind_KbString(map, (Generic)func, kbs)

	BIND( sm_ted_line_to_top_of_window, 	esc_prefix(KB_ArrowU)		);

	BIND( sm_ted_undo_one,			esc_prefix(KB_top(1))		);
	BIND( sm_ted_menu_find,			esc_prefix(KB_top(2))		); 
	BIND( sm_ted_menu_replace,		esc_prefix(KB_top(3))		);
	BIND( sm_ted_print_info,		esc_prefix(KB_top(9))		);
}


