/* $Id: menu_mgr.C,v 1.1 1997/06/25 14:52:22 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/ 
/*                                                                      */ 
/*                                  menu_mgr.c                          */
/*                                 Menu manager                         */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <libs/graphicInterface/oldMonitor/monitor/mon/root/desk_sm.h>
#include <libs/graphicInterface/oldMonitor/monitor/mon/root/menu_mgr.h>

#include <libs/support/strings/rn_string.h>

#include <libs/graphicInterface/oldMonitor/include/mon/sm_def.h>
#include <libs/graphicInterface/oldMonitor/include/mon/manager.h>

#include <libs/graphicInterface/oldMonitor/include/sms/button_sm.h>


    /*** MENU REGISTRATION ***/

struct  menu_list   {                   /* MENU REGISTRATION            */
    struct  _menu       *menu;          /* the menu structure pointer   */
    struct  menu_list   *next;          /* next registration            */
            };


    /*** KEYBOARD EQUIVALENCE  ***/

typedef struct equiv {			/* KB EQUIVALENCE ENTRY		*/
    KbChar		kb_code;	/* the keyboard code		*/
    Generic		id;		/* the corresponding id		*/
    Boolean		enabled;	/* true if equiv is enabled	*/
		} Equivalent;


    /*** WINDOW INFORMATION ***/

struct  _menu   {                       /* MENU INFORMATION STRUCTURE   */
    aLayoutDef          *ld;            /* the button layout definition */
    Generic             deflt;          /* the default choice           */
    Point               prefered_size;  /* the prefered button size     */
    Window              *w;             /* the menu window              */
    Boolean		active;		/* true if menu is "in use"	*/
    short		num_equivs;	/* size of keyboard equiv. list	*/
    Equivalent		*equiv_list;	/* keyboard equivalent list	*/
    Pane                *button_pane;   /* the button pane              */
    Generic             layout;         /* the menu layout              */
    Boolean             binding;        /* true if the menu is binding  */
    Generic             result;         /* the last result              */
                };


    /*** MANAGER INFORMATION ***/

struct  menu_mgr {                      /* MANAGER INFORMATION STRUCT.  */
    Generic             manager_id;     /* the id of the manager        */
    struct  menu_list   *menu_list;     /* registered menus             */
                };

STATIC(void,    menu_mgr_start,(void));       
     /* manager start routine        */
STATIC(Generic, menu_mgr_create,(Generic manager_id));      
     /* manager create routine       */
STATIC(void,    menu_mgr_event,(aMenuMgr *mm));       
     /* manager event handler        */
STATIC(Point,   menu_mgr_window_tile,(aMenuMgr *mm, Window *w, aMenu *menu, 
                                      Point ulc, Boolean New)); 
     /* manager window tiler         */
STATIC(void,    menu_mgr_destroy,(aMenuMgr *mm));     
     /* manager destroy routine      */
STATIC(void,    menu_mgr_finish,(void));      
     /* manager finish routine       */

aManager        menu_manager = {        /* MANAGER DECLARATION          */
                        "menu manager",
                        0,
                        menu_mgr_start,
                        menu_mgr_create,
                        (ManagerEventFunc)menu_mgr_event,
                        (ManagerWindowTileFunc)menu_mgr_window_tile,
                        (ManagerDestroyFunc)menu_mgr_destroy,
                        menu_mgr_finish
                };


        /*** LOCAL SYMBOLS ***/

static  aMenuMgr    *current_mm;        /* the id of the menu manager   */
static  MouseCursor menu_cursor;        /* menu cursor                  */
static  short       button_sm;          /* the index of the button sm   */
static  short       menu_font;          /* the font of the menus        */



/************************** UTILITY FUNCTIONS ***************************/


/* Return true if this is an skw weird string.				*/
static Boolean
skw(char *s)
{
    if (strlen(s) == 0)
	return false;
    else
	return BOOL((s[1] & 0xC0) == 0xC0);
}



/************************ MANAGER ENTRY POINTS **************************/


/* Start the menu manager.                                              */
static void
menu_mgr_start()
{
static
BITMAPM_UNIT    arrow_data[] = {        /* a menu left arrow image data */
                        0x0000, 0x0600, 0x0C00, 0x1C00,
                        0x3FFE, 0x7FFE, 0x3FFE, 0x1C00,
                        0x0C00, 0x0600, 0x0000, 0x0000,
                        0x0000, 0x0000, 0x0000, 0x0000
                };
static
BITMAPM_UNIT    arrow_outline_data[] = {/* menu left arrow outline data */
			0x0700,	0x0F00,	0x1F00,	0x3FFF,
			0x7FFF,	0xFFFF,	0x7FFF,	0x3FFF,
			0x1F00,	0x0F00,	0x0700,	0x0000,
			0x0000,	0x0000,	0x0000,	0x0000
                };


    menu_cursor = makeMouseCursorFromData(makePoint(16, 16),
				          arrow_data, BITMAP_OR,
				          arrow_data, 0,
				          arrow_outline_data, 0,
				          makePoint(2, 8), "menu_mgr.c: menu_mgr_start()"
    );
    button_sm = sm_button_get_index();
    menu_font = DEF_FONT_ID;
}


/* Finish the menu manager.                                             */
static void
menu_mgr_finish()
{
    freeMouseCursor(menu_cursor);
}


/* Create an instance of the menu manager.                              */
static Generic
menu_mgr_create(Generic manager_id)
{
aMenuMgr        *mm;                    /* menu manager being created   */

    mm = (aMenuMgr *) get_mem(sizeof(aMenuMgr), "menu_mgr_create() new inst.");
    mm->manager_id = manager_id;
    mm->menu_list  = (struct menu_list *) 0;
    return ((Generic) mm);
}


/* Destroy an instance of the menu manager.                             */
static void
menu_mgr_destroy(aMenuMgr *mm)
{
    while (mm->menu_list)
    {/* free the remaining menus */
        destroy_menu(mm->menu_list->menu);
    }
    free_mem((void*) mm);
}


/* Handle an event to one of the instances.                             */
static void
menu_mgr_event(aMenuMgr *mm)
{
aMenu           *menu = NULL_MENU;      /* the menu of the event        */
struct menu_list *current;              /* the current menu list item   */
Boolean		found;			/* keyboard equivalent is found	*/
short		i;			/* equivalent list index	*/

    /* figure which menu we are talking about */
        for (current = mm->menu_list; current; current = current->next)
        {/* walk down the list of menus */
            if (
                (current->menu->w == (Window *) mon_event.from) ||
                (current->menu->w == ((Pane *) mon_event.from)->parent)
            )
            {/* the event is from this menu window or a pane in this window */
                menu = current->menu;
                break;
            }
        }

    if (mon_event.from == (Generic) menu->button_pane)
    {/* a button event */
        if (mon_event.type == EVENT_SELECT)
        {/* a button has been picked -- hide the window & save result */
            menu->result = mon_event.msg;
            sm_desk_win_release(menu->w);
            sm_desk_win_hide(menu->w);
        }
    }
    else if (mon_event.from == (Generic) menu->w)
    {/* a window event */
	switch (mon_event.type)
	{/* what kind of event */
	    case EVENT_HELP:
		message("Select one of the items%s.%s",
                        (menu->binding)
                                ? ""
                                : " or select the quit box",
                        (sm_button_visible(menu->button_pane))
                                ? ""
                                : "\nThe menu items may be moved."
                );
		break;
	    case EVENT_KILL:
	    case EVENT_SELECT:
                if (NOT(menu->binding))
	        {/* return the default result */
		    menu->result = menu->deflt;
		    sm_desk_win_release(menu->w);
		    sm_desk_win_hide(menu->w);
	        }
	        else
	        {/* can't do anything -- flash obnoxiously */
		    flashPane(menu->button_pane);
	        }
		break;
	    case MOUSE_KEYBOARD:
		found = false;
		for (i = 0; !found && i < menu->num_equivs; i++)
		{/* check each keyboard equivalent */
		    if (menu->equiv_list[i].kb_code == toKbChar(mon_event.info.x) && menu->equiv_list[i].enabled)
		    {/* we found a match */
			menu->result = menu->equiv_list[i].id;
			sm_desk_win_release(menu->w);
			sm_desk_win_hide(menu->w);
			found = true;
		    }
		}
		if (!found && NOT(menu->binding) && toKbChar(mon_event.info.x) == KB_Enter)
		{/* null selection from the keyboard */
		    menu->result = menu->deflt;
		    sm_desk_win_release(menu->w);
		    sm_desk_win_hide(menu->w);
		    found = true;
	        }
	        if (!found)
	        {/* can't do anything -- flash obnoxiously */
		    flashPane(menu->button_pane);
                }
		break;
	}
    }
}


/* Handle the tiling of a menu window.                                  */
/*ARGSUSED*/
static Point
menu_mgr_window_tile(aMenuMgr *mm, Window *w, aMenu *menu, Point ulc, Boolean New)
{
Point           button_size;            /* the button layout size       */

    /* make sure the menu window doesn't overflow the screen */
        button_size.x = MIN(menu->prefered_size.x, w->parent->size.x * 3 / 4);
        button_size.y = MIN(menu->prefered_size.y, w->parent->size.y * 3 / 4);

    if (New)
    {/* set up for a new window */
        menu->button_pane = newPane(w, button_sm, ulc, button_size, 1);
        menu->layout = sm_button_layout_create(
                menu->button_pane,
                menu->ld,
                menu_font,
                true
        );
        sm_button_layout_show(menu->button_pane, (Window*)menu->layout);
    }
    else
    {/* change the button size */
        menu->button_pane->position = ulc;
        menu->button_pane->size     = button_size;
    }
    return (button_size);
}



/********************** OLD STYLE MENU CALLBACKS ************************/


/* Create a popup window and return the event selected.                 */
Point
general_menu_select(char *title, Point size, char *labels[], Boolean binding)
{
anOptionDef     *options;               /* the option definition list   */
aMenuDef        md;                     /* the current menu definition  */
aMenu           *menu;                  /* the created menu             */
Generic         choice;                 /* the selected choice          */
short           i;                      /* the current choice index     */

    /* fabricate a menu descriptor */
        md.title       = title;
        md.size        = size;
        md.def         = UNUSED;
        md.choice_list = (aChoiceDef *) get_mem(
                                    sizeof(aChoiceDef) * size.x * size.y,
                                    "general_menu_select() choice definition."
        );
        options = (anOptionDef *) get_mem(
                                    sizeof(anOptionDef) * size.x * size.y,
                                    "general_menu_select() option definition."
        );
        for (i = size.x * size.y - 1; i >= 0; i--)
        {/* define the i'th choice */
            md.choice_list[i].id          = i;
            md.choice_list[i].kb_code     = toKbChar(0);
            md.choice_list[i].num_options = 1;
            md.choice_list[i].option_list = &options[i];
            options[i].displayed_text     = labels[i];
            options[i].help_text          = (char *) 0;
        }

    /* run the menu */
        menu = create_menu(&md);
        choice = select_from_menu(menu, binding);
        destroy_menu(menu);

    /* clean up the menu descriptor */
        free_mem((void*) md.choice_list);
        free_mem((void*) options);

    return (
        (choice == UNUSED) 
            ? makePoint(UNUSED, UNUSED)
            : makePoint(choice % size.x, choice / size.x)
    );
}


/* Select from a non-binding list of entries.                           */
short
menu_select(char *title, short num, char *labels[])
{
Point           val;                    /* the return value             */  

    val = general_menu_select(title, makePoint(1, num), labels, false);
    return (val.y);
}


/* Select from a binding list of entries.                               */
short
binding_menu_select(char *title, short num, char *labels[])
{
Point           val;                    /* the return value             */  

    val = general_menu_select(title, makePoint(1, num), labels, true);
    return (val.y);
}



/********************** NEW STYLE MENU CALLBACKS ************************/


/* Create a menu build from a list of arguments.                        */
/* (see menu.h for the argument list).                                  */
/*VARARGS*/
aMenu* make_menu (char* title, Generic def, int rows, int columns, ...)
{
  va_list         arg_list;               /* argument list as per varargs */
  aMenuDef        md;                     /* the menu definition          */
  short           num_choices;            /* total number of choices      */
  short           choice;                 /* current button index         */
  short           opt;                    /* current option of cur. choice*/
  aMenu           *menu;                  /* the created menu             */

  va_start(arg_list, columns);
    {
           /* build the menu descriptor */
        md.title       = title;
        md.def         = def;
        md.size.x      = rows;
        md.size.y      = columns;
        num_choices    = md.size.x * md.size.y;
        md.choice_list = (aChoiceDef*)get_mem(num_choices * sizeof(aChoiceDef),
                                              "temporary choice list for make_menu");

        for (choice = 0; choice < num_choices; choice++)
        {/* get and install the ith choice */
            md.choice_list[choice].id          = va_arg(arg_list, Generic);
            md.choice_list[choice].kb_code     = toKbChar(va_arg(arg_list, int));
            md.choice_list[choice].num_options = va_arg(arg_list, int);
            md.choice_list[choice].option_list = (anOptionDef*) get_mem(
                    md.choice_list[choice].num_options * sizeof(anOptionDef),
                    "an option list for this choice");
            for (opt = 0; opt < md.choice_list[choice].num_options; opt++)
            {/* get and install the faces */
                md.choice_list[choice].option_list[opt].displayed_text =
                                                    va_arg(arg_list, char *);
                md.choice_list[choice].option_list[opt].help_text =
                                                    va_arg(arg_list, char *);
            }
        }

        menu = create_menu(&md);

           /* destroy the menu descriptor */
        for (choice = 0; choice < num_choices; choice++)
        {
              /* destroy the ith button */
           free_mem((void*) md.choice_list[choice].option_list);
        }

        free_mem((void*)md.choice_list);
    }
  va_end(arg_list);

    return (menu);
}


/* Create a menu for later use.                                         */
aMenu *
create_menu(aMenuDef *md)
{
aMenu           *menu;                  /* the menu being created       */
struct menu_list *ml;                   /* new menu list entry          */
short           title_width;            /* the width of the title       */
short		menu_width;		/* width of each menu entry	*/
KbChar		kb_code;		/* the current kb code		*/
char		*kb_name;		/* the current kb equiv. name	*/
short		kb_width;		/* the width of the current kb	*/
short		kb_max_width;		/* width of widest kb equivalent*/
short		text_width;		/* width of the current choice	*/
short		text_max_width;		/* width of widest menu name	*/
short		num_faces;		/* the sum of all faces		*/
aFaceDef	*faces;			/* saved face definitions	*/
short		current_face;		/* the current face number	*/
short		num_buttons;		/* the number of buttons	*/
aButtonDef	*buttons;		/* saved button definitions	*/
short		num_kb_equivalents;	/* the number of kb choices	*/
aLayoutDef	ld;			/* the complete button layout	*/
Equivalent	*equivs;		/* list of kb/choice pairs	*/
short		current_equiv;		/* the current equivalent	*/
short		i, j, k;		/* loop iterators		*/
char		*text, *s;		/* temporaries			*/
TextChar	*tcp;			/* skw string temporary		*/

    /* create a button layout definition & kb equivalent list for the menu items */
	kb_max_width = 0;
	text_max_width = 0;
	num_faces = 0;
	num_buttons = md->size.x * md->size.y;
	num_kb_equivalents = 0;
        for (i = 0; i < num_buttons; i++)
	{/* walk over all menu choices */
	    kb_code = md->choice_list[i].kb_code;
	    if (kb_code != toKbChar(0))
	    {/* look up the equivalent kb_code */
		kb_name = actualFromKbChar(kb_code);
		if (kb_name != KB_bogus_name)
		{/* we have an actual kb definition */
		    num_kb_equivalents++;
		    kb_width = strlen(kb_name);
		    kb_max_width = MAX(kb_max_width, kb_width);
		}
	    }
	    num_faces += md->choice_list[i].num_options;
	    for (j = 0; j < md->choice_list[i].num_options; j++)
	    {/* calculate a running maximum over each option */
		text_width = strlen(md->choice_list[i].option_list[j].displayed_text);
		if (skw(md->choice_list[i].option_list[j].displayed_text))
		{/* skw's weird strings */
		    text_width /= 2;
		}
		text_max_width = MAX(text_max_width, text_width);
	    }
	}
	menu_width = text_max_width + kb_max_width + ((kb_max_width) ? 2 : 0);
	faces    = (aFaceDef *) get_mem(num_faces * sizeof(aFaceDef), "menu_mgr.c:  button faces");
        buttons  = (aButtonDef *) get_mem(num_buttons * sizeof(aButtonDef), "menu_mgr.c:  menu buttons");
	equivs   = (Equivalent *) get_mem(MAX(num_kb_equivalents, 1) * sizeof(Equivalent), "menu_mgr.c:  keyboard equivalents");
	current_face = 0;
	current_equiv = 0;
        for (i = 0; i < num_buttons; i++)
	{/* walk over all buttons installing information */
	    buttons[i].id        = md->choice_list[i].id;
	    buttons[i].face_list = &faces[current_face];
	    buttons[i].num_faces = md->choice_list[i].num_options;
	    kb_code = md->choice_list[i].kb_code;
	    kb_name = "";
	    if (kb_code != toKbChar(0))
	    {/* look up the equivalent kb_code */
		kb_name = actualFromKbChar(kb_code);
		if (kb_name == KB_bogus_name)
		{/* we have a bad equivalent string */
		    kb_name = "";
		}
		else
		{/* install this equivalent */
		    equivs[current_equiv].kb_code  = md->choice_list[i].kb_code;
		    equivs[current_equiv].id       = md->choice_list[i].id;
		    equivs[current_equiv].enabled  = true;
		    current_equiv++;
		}
	    }
	    kb_width = strlen(kb_name);
	    for (j = 0; j < buttons[i].num_faces; j++)
	    {/* install the processesed text for each face */
		text = md->choice_list[i].option_list[j].displayed_text;
		if (skw(text))
		{/* create & pad a weird skw string */
		    tcp = (TextChar *) get_mem(sizeof(TextChar) * (menu_width + 1), "menu_mgr.c: skw menu text template");
		    text_width = strlen(text) / (size_t)2;
		    for (k = 0; k < text_width; k++)
		    {/* copy the name */
			tcp[k] = ((TextChar *) text)[k];
		    }
		    for (k = text_width; k < menu_width - kb_width; k++)
		    {/* pad between the text and the kb_name */
			tcp[k] = makeTextChar(' ', STYLE_NORMAL | 0xC0);
		    }
		    s = kb_name;
		    for (k = menu_width - kb_width; k < menu_width; k++)
		    {/* copy the keyboard equivalent */
			tcp[k] = makeTextChar((unsigned char) *s++, STYLE_NORMAL | 0xC0);
		    }
		    tcp[menu_width] = makeTextChar('\0', 0);
		    buttons[i].face_list[j].displayed_text = (char *) tcp;
		}
		else
		{/* a normal string */
		    s = (char *) get_mem(menu_width + 1, "menu_mgr.c: menu text template.");
		    text_width = strlen(text);
		    (void) strcpy(s, text);
		    for (k = text_width; k < menu_width - kb_width; k++)
		    {/* pad between the text and the kb_name */
			s[k] = ' ';
		    }
		    (void) strcpy(s + menu_width - kb_width, kb_name);
		    buttons[i].face_list[j].displayed_text = s;
		}
		buttons[i].face_list[j].help_text = md->choice_list[i].option_list[j].help_text;
		current_face++;
	    }
	}
	ld.size    = md->size;
	ld.buttons = buttons;

    title_width = sm_desk_title_width(md->title, menu_font);
    menu = (aMenu *) get_mem(sizeof(aMenu), "new menu definition");
    menu->ld              = &ld;
    menu->deflt           = md->def;
    menu->num_equivs      = num_kb_equivalents;
    menu->equiv_list      = equivs;
    menu->prefered_size   = sm_button_layout_size(&ld, menu_font);
    menu->prefered_size.x = MAX(menu->prefered_size.x, title_width);
    menu->active          = false;
    menu->w               = sm_desk_win_create(
            (aMgrInst*)current_mm->manager_id,
            (Generic) menu,
            menu_font,
            DSM_WIN_NO_RESIZE
    );
    sm_desk_win_title(menu->w, md->title);

    /* clean up button layout storage */
        menu->ld = (aLayoutDef *) 0;
	for (i = 0; i < num_buttons; i++)
	{/* walk over all buttons installing information */
	    for (j = 0; j < buttons[i].num_faces; j++)
	    {/* free the text for each face */
		sfree(buttons[i].face_list[j].displayed_text);
	    }
	}
        free_mem((void*) faces);
        free_mem((void*) buttons);

    /* install in global menu list */
	ml = (struct menu_list *) get_mem(
		sizeof(struct menu_list),
		"menu regstiration"
	);
	ml->menu = menu;
	ml->next = current_mm->menu_list;
	current_mm->menu_list = ml;

    return (menu);
}


/* Destroy a menu.                                                      */
void
destroy_menu(aMenu *menu)
{
struct menu_list *current;              /* current menu registration    */
struct menu_list **mrpp;                /* menu registration ptr ptr    */

    for (mrpp = &current_mm->menu_list; *mrpp; mrpp = &(*mrpp)->next)
    {/* walk down the menu registration list */
        current = *mrpp;
        if (current->menu == menu)
        {/* current is the registration node--delete it */
            *mrpp = current->next;
            sm_desk_win_destroy(menu->w);
	    free_mem((void*) menu->equiv_list);
            free_mem((void*) menu);
            free_mem((void*) current);
            return;
        }
    }
    die_with_message("destroy_menu():  Bogus menu value.");
}


/* Ask the user to make a choice from the menu.                         */
Generic
select_from_menu(aMenu *menu, Boolean binding)
{
    menu->binding = binding;
    sm_desk_win_modify(menu->w, (Generic) menu);
    menu->active = true;
    sm_desk_win_force(menu->w, menu_cursor);
    menu->active = false;
    return (menu->result);
}


/* Modify the selectability/selectedness of a menu.                     */
void
modify_menu_choice(aMenu *menu, Generic id, Boolean selectable, Boolean selected)
{
short		i;			/* the equivalent index		*/

    sm_button_modify_button(
            menu->button_pane,
            (Window*)menu->layout,
            id,
            selectable,
            selected
    );
    for (i = 0; i < menu->num_equivs; i++)
    {/* search for kb equivalent to change status */
	if (menu->equiv_list[i].id == id)
	{/* this is the one to change */
	    menu->equiv_list[i].enabled = selectable;
	    break;
	}
    }
}


/* Set all choices back to the default selectedness/selectablity.       */
void
default_menu_choices(aMenu *menu)
{
short			i;		/* the equivalent index		*/

    sm_button_modify_all(menu->button_pane, (Window*)menu->layout, false, true);
    for (i = 0; i < menu->num_equivs; i++)
    {/* "turn on" all kb equivalents */
	menu->equiv_list[i].enabled = true;
    }
}


/* Display an arbitrary option for a choice in a menu.                  */
void
switch_menu_option(aMenu *menu, Generic id, short num)
{
    sm_button_switch_face(menu->button_pane, (Window*)menu->layout, id, num);
}


/* Display the next option for a choice in a menu.                      */
void
toggle_menu_option(aMenu *menu, Generic id)
{
    sm_button_toggle_face(menu->button_pane, (Window*)menu->layout, id);
}




/************************** MANAGER CALLBACKS ***************************/


/* Set the manager to use for subsequent menu creation calls.           */
Generic
menu_mgr_use(aMenuMgr *mm)
{
aMenuMgr        *saved;                 /* the saved handle             */

    saved = current_mm;
    current_mm = mm;
    return ((Generic) saved);
}


/* The parent pane has been resized.					*/
/*ARGSUSED*/
void
menu_mgr_resize_notify(aMenuMgr *mm)
{
struct menu_list *current;              /* current menu registration    */

    for (current = current_mm->menu_list; current; current = current->next)
    {/* walk down the menu registration list */
        if (current->menu->active)
        {/* this active window should be resized */
            sm_desk_win_modify(current->menu->w, (Generic) current->menu);
	}
    }
}
