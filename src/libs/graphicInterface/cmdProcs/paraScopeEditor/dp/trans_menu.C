/* $Id: trans_menu.C,v 1.1 1997/06/25 14:38:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ped_cp/PEditorCP/dp/trans_menu.c				*/
/*									*/
/*	trans_menu.c -- transformation selection			*/
/*									*/
/*	modified 3/93 by tseng to reorganize menus                  */
/*	modified 7/91 by kats to make hierarchal menus                  */
/*	modified 8/91 by kats to make static menus                      */
/*	modified 8/91 by tseng to add parallelize/sequentialize loop     */
/*									*/
/************************************************************************/

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_dialogs.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>
#include <libs/graphicInterface/oldMonitor/include/dialogs/message.h>


#define T_NO_SELECT		-1

/* Top level transformation menu selection, then choose one of */
/* reordering transformations           */
/* dependence breaking transformations  */
/* memory optimizing transformations    */
/* Miscellaneous transformations        */


/************************************************************************/
/* Top level transformation menu selection                              */
/************************************************************************/

#define T_REORDER               0
#define T_DBREAK                1
#define T_MISC1              2
#define T_TOP                   3 /* must be last */

static
anOptionDef transOptions[] =
{
    {"reordering",          (char *) 0},
    {"dependence breaking", (char *) 0},
    {"optional",             (char *) 0}
};

static
aChoiceDef transChoices[] =
{
    {T_REORDER, toKbChar(0),  1, &transOptions[T_REORDER] },
    {T_DBREAK,  toKbChar(0),  1, &transOptions[T_DBREAK]  },
    {T_MISC1,toKbChar(0),  1, &transOptions[T_MISC1]}
};

static
aMenuDef transMenuDef =
{
    "Transformations", 
    { 1, sizeof(transChoices) / sizeof(aChoiceDef) },
    T_NO_SELECT,
    transChoices	
};


/************************************************************************/
/* Reordering Transformations                                           */
/************************************************************************/

#define	R_LOOP_INTER 	0
#define	R_DISTRIB 		1
#define	R_FUSE			2
#define R_REVERSE		3
#define R_SKEW			4
#define R_STRIP			5
#define	R_S_INTER		6
#define R_PAR_LOOP		7
#define R_SEQ_LOOP		8

static
anOptionDef reorderOptions[] =
{
    {"loop interchange",      (char *) 0 },
    {"loop distribution",     (char *) 0 },
    {"loop fusion",           (char *) 0 },
    {"loop reversal",         (char *) 0 },
    {"loop skewing", 	      (char *) 0 },
    {"strip mine",            (char *) 0 },
    {"statement interchange", (char *) 0 },
    {"parallelize loop",      (char *) 0 },
    {"sequentialize loop",    (char *) 0 },
};

static
aChoiceDef reorderChoices[] =
{
    {R_LOOP_INTER, toKbChar(0),  1, &reorderOptions[R_LOOP_INTER] },
    {R_DISTRIB,    toKbChar(0),  1, &reorderOptions[R_DISTRIB] },
    {R_FUSE,       toKbChar(0),  1, &reorderOptions[R_FUSE] },
    {R_REVERSE,    toKbChar(0),  1, &reorderOptions[R_REVERSE] },
    {R_SKEW,       toKbChar(0),  1, &reorderOptions[R_SKEW] },
    {R_STRIP,      toKbChar(0),  1, &reorderOptions[R_STRIP] },
    {R_S_INTER,    toKbChar(0),  1, &reorderOptions[R_S_INTER] },
    {R_PAR_LOOP,   toKbChar(0),  1, &reorderOptions[R_PAR_LOOP] },
    {R_SEQ_LOOP,   toKbChar(0),  1, &reorderOptions[R_SEQ_LOOP] },
};

static
aMenuDef reorderMenuDef =
{
    "Reordering", 
    { 1, sizeof(reorderChoices) / sizeof(aChoiceDef) },
    T_NO_SELECT,
    reorderChoices
};


/************************************************************************/
/* Dependence Breaking Transformations                                  */
/************************************************************************/

#define D_PEEL			0
#define D_SPLIT			1
#define	D_EXPAND		2
#define D_RENAME		3
#define D_ALIGN			4

static
anOptionDef breakOptions[] =
{
    {"peel iterations",	      (char *) 0 },
    {"split loop",            (char *) 0 },
    {"scalar expansion",      (char *) 0 },
    {"rename an array",       (char *) 0 },
    {"align dependence",      (char *) 0 },
};

static
aChoiceDef breakChoices[] =
{
    {D_PEEL,	   toKbChar(0),  1, &breakOptions[D_PEEL] },
    {D_SPLIT,      toKbChar(0),  1, &breakOptions[D_SPLIT] },
    {D_EXPAND,     toKbChar(0),  1, &breakOptions[D_EXPAND] },
    {D_RENAME,     toKbChar(0),  1, &breakOptions[D_RENAME] },
    {D_ALIGN,      toKbChar(0),  1, &breakOptions[D_ALIGN] },
};

static
aMenuDef breakMenuDef =
{
    "Dependence Breaking", 
    { 1, sizeof(breakChoices) / sizeof(aChoiceDef) },
    T_NO_SELECT,
    breakChoices	
};

/************************************************************************/
/* Miscellaneous (Optional) Transformations                             */
/************************************************************************/

#define M_UNROLL		0
#define M_UNROLL_JAM	1
#define M_REPLACE_S		2
#define M_ADJUST		3
#define	M_DELETE_S		4
#define	M_ADD_S			5
#define	M_UNSWITCH		6
#define M_CPROP			7
#define M_PROTECT		8
#define M_INPUT_DEP		9

static
anOptionDef otherOptions[] =
{
    {"unroll loop",           (char *) 0 },
    {"unroll-and-jam",        (char *) 0 },
    {"scalar replacement",    (char *) 0 },
    {"adjust loop bounds",    (char *) 0 },
    {"delete statement",      (char *) 0 },
    {"add statement",         (char *) 0 },
    {"loop unswitching",      (char *) 0 },
    {"propagate constants",   (char *) 0 },
    {"preserved dependence?", (char *) 0 },
    {"input dependences",     (char *) 0 },
};

static
aChoiceDef otherChoices[] =
{
    {M_UNROLL,     toKbChar(0),  1, &otherOptions[M_UNROLL] },
    {M_UNROLL_JAM, toKbChar(0),  1, &otherOptions[M_UNROLL_JAM] },
    {M_REPLACE_S,  toKbChar(0),  1, &otherOptions[M_REPLACE_S] },
    {M_ADJUST,     toKbChar(0),  1, &otherOptions[M_ADJUST] },
    {M_DELETE_S,   toKbChar(0),  1, &otherOptions[M_DELETE_S] } ,
    {M_ADD_S,      toKbChar(0),  1, &otherOptions[M_ADD_S] },
    {M_UNSWITCH,   toKbChar(0),  1, &otherOptions[M_UNSWITCH] },
    {M_CPROP,      toKbChar(0),  1, &otherOptions[M_CPROP] },
    {M_PROTECT,    toKbChar(0),  1, &otherOptions[M_PROTECT] },
    {M_INPUT_DEP,  toKbChar(0),  1, &otherOptions[M_INPUT_DEP] },
};

static
aMenuDef otherMenuDef =
{
    "Miscellaneous", 
    { 1, sizeof(otherChoices) / sizeof(aChoiceDef) },
    T_NO_SELECT,
    otherChoices	
};



/*-------------------------------------------------------------------

	transform_menu_create()  Creates menu for PED transformations

*/

void
transform_menu_create(TransMenu *th)
{
    th->menu = (aMenu **) get_mem(sizeof(((aMenu *)0)) * (T_TOP + 1), "transform_menu_create");

    th->menu[T_TOP]     = create_menu (&transMenuDef);
    th->menu[T_REORDER] = create_menu (&reorderMenuDef);
    th->menu[T_DBREAK]  = create_menu (&breakMenuDef);
    th->menu[T_MISC1] = create_menu (&otherMenuDef);
}

void
transform_menu (TransMenu *th)
{
    Generic 	selection, lowerSel;
    AST_INDEX	loop;
    PedInfo	ped;
    int             flow_type;
	
    ped = (PedInfo)(th->ped);
    loop = ped->selected_loop;

    /*-----------------------------------------*/
	/* these options are stubbed off right now */

    modify_menu_choice (th->menu[T_MISC1], M_ADD_S,        false, false);
    modify_menu_choice (th->menu[T_MISC1], M_UNSWITCH,     false, false);
    modify_menu_choice (th->menu[T_MISC1], M_CPROP,	      false, false);
	modify_menu_choice (th->menu[T_DBREAK], D_ALIGN,      false, false);

    /*-----------------------------------------*/
	/* these options are always legal */

    modify_menu_choice (th->menu[T_MISC1], M_INPUT_DEP,	false, true);

    /*-----------------------------------------*/
	/* if edits, no loop or parallel loop is selected, transforms are off */

    if ((loop == AST_NIL) || is_parallelloop(loop) || ped->edit_performed)
    {
	modify_menu_choice (th->menu[T_REORDER], R_DISTRIB,    false, false);
	modify_menu_choice (th->menu[T_REORDER], R_LOOP_INTER, false, false);
	modify_menu_choice (th->menu[T_REORDER], R_SKEW,       false, false);
	modify_menu_choice (th->menu[T_REORDER], R_REVERSE,    false, false);
	modify_menu_choice (th->menu[T_REORDER], R_FUSE,       false, false);
	modify_menu_choice (th->menu[T_REORDER], R_S_INTER,    false, false);
	modify_menu_choice (th->menu[T_REORDER], R_STRIP,      false, false);
	modify_menu_choice (th->menu[T_REORDER], R_PAR_LOOP,   false, false);

	modify_menu_choice (th->menu[T_DBREAK], D_PEEL,       false, false);
	modify_menu_choice (th->menu[T_DBREAK], D_SPLIT,      false, false);
	modify_menu_choice (th->menu[T_DBREAK], D_EXPAND,     false, false);
	modify_menu_choice (th->menu[T_DBREAK], D_RENAME,     false, false);

	modify_menu_choice (th->menu[T_MISC1], M_UNROLL,       false, false);
	modify_menu_choice (th->menu[T_MISC1], M_UNROLL_JAM,   false, false);
	modify_menu_choice (th->menu[T_MISC1], M_REPLACE_S,    false, false);
	modify_menu_choice (th->menu[T_MISC1], M_ADJUST,       false, false);
	modify_menu_choice (th->menu[T_MISC1], M_DELETE_S,     false, false);
	modify_menu_choice (th->menu[T_MISC1], M_PROTECT,      false, false);

	/* if parallel loop selected, then R_SEQ_LOOP is permitted */
	modify_menu_choice (th->menu[T_REORDER], R_SEQ_LOOP, false, 
		BOOL((loop != AST_NIL) && NOT(ped->edit_performed)));
    }
    else 
    {
	LI_Instance	   *LI = (LI_Instance *) (PED_LI(ped));

		/* try to catch problems with NULL LI->cur_loop field	*/
	
	if( ! li_cur_loop(LI) )
	{
	    if (NOT(el_get_loop_info( PED_LI(ped), loop)))
	    {
		message("Loop info out of date, \n please reanalyze program.");
		return;
	    }
	}

	flow_type = el_cflow( PED_LI(ped));
	if (flow_type == STRUCT)
	{
	modify_menu_choice (th->menu[T_REORDER], R_DISTRIB,    false, true);
	modify_menu_choice (th->menu[T_REORDER], R_LOOP_INTER, false, false);
	modify_menu_choice (th->menu[T_REORDER], R_SKEW,       false, true);
	modify_menu_choice (th->menu[T_REORDER], R_REVERSE,    false, true);
	modify_menu_choice (th->menu[T_REORDER], R_FUSE,       false, true);
	modify_menu_choice (th->menu[T_REORDER], R_S_INTER,    false, true);
	modify_menu_choice (th->menu[T_REORDER], R_STRIP,      false, true);
	modify_menu_choice (th->menu[T_REORDER], R_PAR_LOOP,	  false, true);

	modify_menu_choice (th->menu[T_DBREAK], D_PEEL,       false, true);
	modify_menu_choice (th->menu[T_DBREAK], D_SPLIT,      false, true);
	modify_menu_choice (th->menu[T_DBREAK], D_EXPAND,     false, false);
	modify_menu_choice (th->menu[T_DBREAK], D_RENAME,     false, false);

	modify_menu_choice (th->menu[T_MISC1], M_UNROLL,     false, true);
	modify_menu_choice (th->menu[T_MISC1], M_UNROLL_JAM, false, false);
	modify_menu_choice (th->menu[T_MISC1], M_REPLACE_S,  false, false);
	modify_menu_choice (th->menu[T_MISC1], M_ADJUST,      false, true);
	modify_menu_choice (th->menu[T_MISC1], M_DELETE_S,    false, true);
	modify_menu_choice (th->menu[T_MISC1], M_PROTECT,    false, false);
    }
	else if (flow_type == UNSTRUCT)
	{

	modify_menu_choice (th->menu[T_REORDER], R_DISTRIB,    false, false);
	modify_menu_choice (th->menu[T_REORDER], R_LOOP_INTER, false, false);
	modify_menu_choice (th->menu[T_REORDER], R_SKEW,       false, true);
	modify_menu_choice (th->menu[T_REORDER], R_REVERSE,    false, true);
	modify_menu_choice (th->menu[T_REORDER], R_FUSE,       false, true);
	modify_menu_choice (th->menu[T_REORDER], R_S_INTER,    false, false);
	modify_menu_choice (th->menu[T_REORDER], R_STRIP,      false, true);
	modify_menu_choice (th->menu[T_REORDER], R_PAR_LOOP,	  false, true);

	modify_menu_choice (th->menu[T_DBREAK], D_PEEL,       false, true);
	modify_menu_choice (th->menu[T_DBREAK], D_SPLIT,      false, true);
	modify_menu_choice (th->menu[T_DBREAK], D_EXPAND,     false, false);
	modify_menu_choice (th->menu[T_DBREAK], D_RENAME,     false, false);

	modify_menu_choice (th->menu[T_MISC1], M_UNROLL,     false, true);
	modify_menu_choice (th->menu[T_MISC1], M_UNROLL_JAM, false, false);
	modify_menu_choice (th->menu[T_MISC1], M_REPLACE_S,  false, false);
	modify_menu_choice (th->menu[T_MISC1], M_ADJUST,      false, true);
	modify_menu_choice (th->menu[T_MISC1], M_DELETE_S,    false, true);
	modify_menu_choice (th->menu[T_MISC1], M_PROTECT,    false, false);
    }
	else if (flow_type == BACK_BRANCH)
	{

	modify_menu_choice (th->menu[T_REORDER], R_DISTRIB,    false, false);
	modify_menu_choice (th->menu[T_REORDER], R_LOOP_INTER, false, false);
	modify_menu_choice (th->menu[T_REORDER], R_SKEW,       false, false);
	modify_menu_choice (th->menu[T_REORDER], R_REVERSE,    false, false);
	modify_menu_choice (th->menu[T_REORDER], R_FUSE,       false, false);
	modify_menu_choice (th->menu[T_REORDER], R_S_INTER,    false, true);
	modify_menu_choice (th->menu[T_REORDER], R_STRIP,      false, false);
	modify_menu_choice (th->menu[T_REORDER], R_PAR_LOOP,	  false, true);

	modify_menu_choice (th->menu[T_DBREAK], D_PEEL,       false, false);
	modify_menu_choice (th->menu[T_DBREAK], D_SPLIT,      false, false);
	modify_menu_choice (th->menu[T_DBREAK], D_EXPAND,     false, false);
	modify_menu_choice (th->menu[T_DBREAK], D_RENAME,     false, false);

	modify_menu_choice (th->menu[T_MISC1], M_UNROLL,     false, false);
	modify_menu_choice (th->menu[T_MISC1], M_UNROLL_JAM, false, false);
	modify_menu_choice (th->menu[T_MISC1], M_REPLACE_S,  false, false);
	modify_menu_choice (th->menu[T_MISC1], M_ADJUST,      false, false);
	modify_menu_choice (th->menu[T_MISC1], M_DELETE_S,    false, true);
	modify_menu_choice (th->menu[T_MISC1], M_PROTECT,    false, false);
    }
	else
	{
	modify_menu_choice (th->menu[T_REORDER], R_DISTRIB,    false, true);
	modify_menu_choice (th->menu[T_REORDER], R_LOOP_INTER, false, true);
	modify_menu_choice (th->menu[T_REORDER], R_SKEW,       false, true);
	modify_menu_choice (th->menu[T_REORDER], R_REVERSE,    false, true);
	modify_menu_choice (th->menu[T_REORDER], R_FUSE,       false, true);
	modify_menu_choice (th->menu[T_REORDER], R_S_INTER,    false, true);
	modify_menu_choice (th->menu[T_REORDER], R_STRIP,      false, true);
	modify_menu_choice (th->menu[T_REORDER], R_PAR_LOOP,	  false, true);

	modify_menu_choice (th->menu[T_DBREAK], D_PEEL,       false, true);
	modify_menu_choice (th->menu[T_DBREAK], D_SPLIT,      false, true);
	modify_menu_choice (th->menu[T_DBREAK], D_EXPAND,     false, true);
	modify_menu_choice (th->menu[T_DBREAK], D_RENAME,     false, true);

	modify_menu_choice (th->menu[T_MISC1], M_UNROLL,     false, true);
	modify_menu_choice (th->menu[T_MISC1], M_UNROLL_JAM, false, true);
	modify_menu_choice (th->menu[T_MISC1], M_REPLACE_S,  false, true);
	modify_menu_choice (th->menu[T_MISC1], M_ADJUST,      false, true);
	modify_menu_choice (th->menu[T_MISC1], M_DELETE_S,    false, true);

    /*-----------------------------------------*/
	/* following options must have dependence selected	*/

	if (ped->current_dependence != -1)
	{
	    /* modify_menu_choice (th->menu[T_DBREAK], D_ALIGN,   false, true); */
	    modify_menu_choice (th->menu[T_MISC1], M_PROTECT, false, true);
	}
	else
	{
	    /* modify_menu_choice (th->menu[T_DBREAK], D_ALIGN,   false, false); */
	    modify_menu_choice (th->menu[T_MISC1], M_PROTECT, false, false);
	}
    }
    } /* end else ... */

    selection = select_from_menu (th->menu[T_TOP], false);

    switch (selection)	
    {
    case T_REORDER:

	lowerSel = select_from_menu (th->menu[T_REORDER], false);
	switch (lowerSel)
	{
	case R_DISTRIB:
	    distribute_dialog_run (th->dh);
	    dialogUpdate (th->PD);
	    break;
	    
	case R_LOOP_INTER:
	    ali_dialog_run (th->ih);
	    dialogUpdate (th->PD);
	    break;

	case R_SKEW:
	    if (loop_level(loop) > 1)
		skew_dialog_run(th->allh);
	    else
		message("Only inner loops may\n be skewed.");
	    dialogUpdate (th->PD);
	    break;

	case R_FUSE:
	    fusion_dialog_run(th->allh);
	    dialogUpdate(th->PD);
	    break;

	case R_REVERSE:
	    reverse_dialog_run(th->allh);
	    dialogUpdate(th->PD);
	    break;

	case R_S_INTER:
	    sinter_dialog_run (th->sih);
	    dialogUpdate (th->PD);
	    break;

	case R_STRIP:
	    if (pt_can_strip (ped))
		strip_dialog_run(th->smih);
	    else
		message ("Only loops with a step size of 1\n can be strip mined.");
	    dialogUpdate (th->PD);
	    break;
				
	case R_PAR_LOOP:
	    makeParallel(ped);
	    break;

	case R_SEQ_LOOP:
	    makeSequential(ped);
	    break;

	default:
	    break;
	}
	break;

    case T_DBREAK:
	lowerSel = select_from_menu (th->menu[T_DBREAK], false);
	switch (lowerSel)
	{
	case D_PEEL:
	    peel_dialog_run(th->smih);
	    dialogUpdate (th->PD);
	    break;

	case D_SPLIT:
	    split_dialog_run(th->smih);
	    dialogUpdate (th->PD);
	    break;

	case D_EXPAND:
	    scalar_dialog_run ((Generic)ped);
	    dialogUpdate (th->PD);
	    break;
		
	case D_RENAME:
	    renaming_dialog_run ((Generic)ped);
	    dialogUpdate (th->PD);
	    break;
		
	case D_ALIGN:
	    align_dialog_run(th->allh);
	    dialogUpdate(th->PD);
	    break;
			
	default:
	    break;
	}
	break;

    case T_MISC1:
	lowerSel = select_from_menu (th->menu[T_MISC1], false);
	switch (lowerSel)
	{

	case M_UNROLL:
	    unroll_dialog_run(th->allh);
	    dialogUpdate(th->PD);
	    break;

	case M_UNROLL_JAM:
	    unroll_jam_dialog_run(th->allh);
	    dialogUpdate(th->PD);
	    break;

	case M_REPLACE_S:
	    replace_s_dialog_run(th->allh);
	    dialogUpdate(th->PD);
	    break;

	case M_UNSWITCH:
	    unswitch_dialog_run(th->allh);
	    dialogUpdate(th->PD);
	    break;

	case M_PROTECT:
	    switch (pt_is_protected(ped, 
		    get_dependence( PED_EL(ped),PED_CURRENT_DEPENDENCE(ped)),
		    PED_SELECTED_LOOP(ped)))
	    {
	    case PROTECTED:
		message ("Dependence is preserved");
		break;
					
	    case NOT_PROTECTED:
		message ("Dependence is not preserved");
		break;
					
	    case PERROR:
		message ("Insufficient information");
		break;
	    }
	    break;

	case M_ADJUST:
	    adjust_dialog_run(th->allh);
	    dialogUpdate(th->PD);
	    break;

	case M_ADD_S:
	    add_stmt_dialog_run(th->allh);
	    dialogUpdate (th->PD);
	    break;
			
	case M_DELETE_S:
	    delete_stmt_dialog_run ((Generic)ped);
	    dialogUpdate (th->PD);
	    break;			

#ifdef CPROP_STUBBED_OFF
	case M_CPROP:
	    doCProp(ped);
	    break;
#endif

	case M_INPUT_DEP:
	  dg_set_input_dependences(PED_DG(ped), true);
	  break;
	  
	default:
	    break;
	}
	break;
		
    default:
	break;
    }
}

void
transform_menu_destroy (TransMenu *th)
{
	free_mem(th->menu);
}

