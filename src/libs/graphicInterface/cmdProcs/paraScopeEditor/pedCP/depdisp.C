/* $Id: depdisp.C,v 1.1 1997/06/25 14:42:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ped_cp/dp/depdisp.c						*/
/*									*/
/*	trans.c -- PED dependence display routines 	                */
/*									*/
/*									*/
/************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped.h>
#include <libs/support/database/newdatabase.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/vanilla_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/list_sm.h>
#include <libs/graphicInterface/oldMonitor/include/mon/dialog.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_structs.h>
#include <libs/frontEnd/textTree/TextTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/frontEnd/ast/groups.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dt.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pedCP/PedPrivate.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>

#define MAX_NAME 10

typedef struct struct_SubsWalkParams SubsWalkParams;

/* local functions */

STATIC(lsm_line_info, *dep_list_handler,(Generic pane, PedInfo ped, Boolean first,
                                         Generic id));
STATIC(char, *getLine,(PedInfo ped, DG_Edge *ea, int edge, int id));
STATIC(char, *getEmptyLine,(void));
STATIC(void, setDepTitle,(PedInfo ped, int num));
STATIC(AST_INDEX, findAbove,(AST_INDEX stmt));
STATIC(AST_INDEX, findBelow,(AST_INDEX stmt));
STATIC(int, attach_subscripts,(AST_INDEX stmt, int nesting_level, 
                               SubsWalkParams *subsWalkParams));

void edgeUpdate();




/*---------------------------------------------------------------------

	depDispInit()

*/

void
depDispInit(PedInfo ped)
{
    setDepTitle(ped, 0);
    
    /* initialize the header for the list */
    sm_vanilla_set_text(
			(Pane*)PED_DEP_HEADER_PANE(ped),
			getEmptyLine(),
			DEF_FONT_ID,
			STYLE_NORMAL,
			VSM_JUSTIFY_LEFT);
    
    /* initialize the list itself */
    sm_list_initialize((Pane*)PED_DEP_PANE(ped),
		       (Generic)ped,
		       (lsm_generator_callback) dep_list_handler,
		       DEF_FONT_ID,
		       LSM_SHIFT_AUTO,
		       LSM_NO_H_SCROLLBAR,
		       LSM_V_SCROLLBAR);
    
    sm_list_modified((Pane*)PED_DEP_PANE(ped), Origin, UNUSED);
}

/*---------------------------------------------------------------------

	loopUpdate()	Update loop selection & display

	Stores the old selected loop, sets the new selected loop and
	calls the appropriate handlers to emphasize the new loop header 
	and clear the emphasis in the old loop header.  kef-Jul. 89

*/

void
loopUpdate(PedInfo ped, AST_INDEX newloop)
{
	Generic         pedcp;

	/* store the old selection so it can be redrawn */
	PED_PREV_LOOP(ped) = PED_SELECTED_LOOP(ped);

	PED_SELECTED_LOOP(ped) = newloop;
	pedcp = PED_ED_HANDLE(ped);
	PED_CURRENT_DEPENDENCE(ped) = -1;

	/* clear old loop */
	ped->RedrawLoop(pedcp, PED_PREV_LOOP(ped));	

	/* emphasize new loop */
	ped->RedrawLoop(pedcp, PED_SELECTED_LOOP(ped));	
}

/*---------------------------------------------------------------------

*/

void
pedUpdate(PedInfo ped, AST_INDEX node)
{
	int             ndeps;	/* num of dependences found in selected loop */

	if (PED_EDIT_PERFORMED(ped))		/* no update if editing	*/
		return;

	if (PED_SELECTION(ped) != node)			/* a new selection */
	{
		PED_SELECTION(ped) = node;
		if (is_loop(node))
		{
			/* free the stack of EL structures */
			while (PED_STACK_DEPTH(ped) > 0)
			{
				/* free the current EL */
				el_destroy_instance(PED_EL(ped));
				PED_STACK_DEPTH(ped)--;
				stack_pop(PED_EL_STACK(ped), (Generic*)&PED_EL(ped));
			}
			/* bottom of stack will be overwritten by new_loop */

			loopUpdate(ped, node);
			ndeps = ped_el_new_loop(ped, node);

			/* update the title to reflect the new loop */
			setDepTitle(ped, ndeps);

			/* mark the loop in the fortran display */
			ped->SelectNode(PED_ED_HANDLE(ped), node);

			/* select the first dependence if possible */
			edgeUpdate(ped);

		}
	}
}
/*---------------------------------------------------------------------

*/

void
forcePedUpdate(PedInfo ped, AST_INDEX loop, AST_INDEX node)
{
	int             ndeps;	/* num of dependences found in selected loop */

	if (PED_EDIT_PERFORMED(ped))		/* no update if editing	*/
		return;
	/*
	 * This ensures that if the loop header changes, and the original
	 * selection was the header, the selection becomes the new header.
	 */

	if (node == PED_PREV_LOOP(ped))
		PED_SELECTION(ped) = loop;
	else
		PED_SELECTION(ped) = node;

	PED_SELECTED_LOOP(ped) = loop;

	if (is_loop(loop))
	{
		/* free the stack of EL structures */
		while (PED_STACK_DEPTH(ped) > 0)
		{
			/* free the current EL */
			el_destroy_instance(PED_EL(ped));
			PED_STACK_DEPTH(ped)--;
			stack_pop(PED_EL_STACK(ped), (Generic*)&PED_EL(ped));
		}
		/* bottom of stack will be overwritten by new_loop */

		loopUpdate(ped, loop);
		ndeps = ped_el_new_loop(ped, loop);

		/* update the title to reflect the new loop */
		setDepTitle(ped, ndeps);

		/* mark the loop in the fortran display */
		ped->SelectNode(PED_ED_HANDLE(ped), PED_SELECTION(ped));

		/* select the first dependence if possible */
		edgeUpdate(ped);
	}
	else			/* Selection is not a loop, mpal:910710	*/
	  {
	    /* Update the edges displayed in Dependence Pane	*/
	    edgeUpdate(ped);
	  }
}

/*---------------------------------------------------------------------

*/

Boolean
pedDepSelect(PedInfo ped, int newsel)
{
	int             edge;
	DG_Edge        *ea;

	if (newsel < 0)		/* no such edge */
		return false;

	if (PED_EDIT_PERFORMED(ped))	/* no viewing allowed if editing	*/
		return false;

	edge = get_dependence( PED_EL(ped), newsel);
	if (edge == -1)
		return false;

	/* ok, scroll it onto the screen, and select it for real */

	PED_CURRENT_DEPENDENCE(ped) = newsel;
	(void) sm_list_line_show((Pane*)ped->dep_pane, newsel, true, UNUSED);

	ea = dg_get_edge_structure( PED_DG(ped));
	ped->RedrawSrcSink(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped), ea[edge].src, ea[edge].sink, PED_PREV_SRC(ped), PED_PREV_SINK(ped));
	ped->prev_src = ea[edge].src;
	ped->prev_sink = ea[edge].sink;

	/* (void) sm_list_line_show(PED_DEP_PANE(ped), newsel, true, UNUSED); */
	return true;
}

/*---------------------------------------------------------------------

*/

void
edgeUpdate(PedInfo ped)
{
	if (!pedDepSelect(ped, 0))
	{
		/* Clear the dep list and clear the previous	*/
		/* dependence viewing in the text.				*/

		PED_CURRENT_DEPENDENCE(ped) = -1;

		ped->RedrawSrcSink(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped), AST_NIL, AST_NIL, PED_PREV_SRC(ped), PED_PREV_SINK(ped));

		sm_list_modified((Pane*)PED_DEP_PANE(ped), Origin, UNUSED);
	}
}

void
edgeClear(PedInfo ped)
{
  /* Clear the dep list and clear the previous	*/
  /* dependence viewing in the text.				*/

  PED_CURRENT_DEPENDENCE(ped) = -1;
  ped->RedrawSrcSink(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped), AST_NIL, AST_NIL, PED_PREV_SRC(ped), PED_PREV_SINK(ped));

  sm_list_modified((Pane*)PED_DEP_PANE(ped), Origin, UNUSED);
}

/*---------------------------------------------------------------------

*/

static char    *
getEmptyLine()
{
    return "type     src(___)         sink(bold)       vector           level     block";
}

/*---------------------------------------------------------------------

	setDepTitle()	-	Set title of dependence pane
*/

static void
setDepTitle(PedInfo ped, int num)
{
	int             i;
	char            buffer[100];
	char            open_p[MAX_NAME];
	char            close_p[MAX_NAME];

	for (i = 0; i < PED_STACK_DEPTH(ped) && i < 19; i++)
	{
		open_p[i] = '[';
		close_p[i] = ']';
	}

	open_p[PED_STACK_DEPTH(ped)] = '\0';
	close_p[PED_STACK_DEPTH(ped)] = '\0';

	sprintf(buffer, "%sDependences%s", open_p, close_p, num);

	sm_vanilla_set_inversion((Pane*)PED_DEP_TITLE_PANE(ped),
				 VSM_INVERT_BKGND, VSM_NORMAL_TRACK);

	sm_vanilla_set_text((Pane*)PED_DEP_TITLE_PANE(ped),
		       buffer, DEF_FONT_ID, STYLE_BOLD, VSM_JUSTIFY_CENTER);
}

/*---------------------------------------------------------------------

*/

static char    *
getLine(PedInfo ped, DG_Edge *ea, int edge, int id)
{
    DepType  	    type;
    char           *src;
    char           *sink;
    int             level;
    char           *block;
    char           *sink_sub_text;
    char           *src_sub_text;
    char           *dvect;
    static char     buffer[500];
    static char     srcbuf[100];
    static char     sinkbuf[100];
    
    src = gen_get_text(ea[edge].src);
    
    src_sub_text = (char *)get_info(ped, ea[edge].src, type_subscript);
    sink_sub_text = (char *)get_info(ped, ea[edge].sink, type_subscript);
    
    if (ea[edge].src_str)
    {
#ifdef OLD_DEP_FORMAT 
	sprintf(srcbuf, "%-s%-s", src, ea[edge].src_str);
#else /* JMC 1/93 fix display of ParaScope IP edges */
	sprintf(srcbuf, "%-s", ea[edge].src_str);
#endif
    }
    else if (src_sub_text)
    {
	sprintf(srcbuf, "%-s%-s", src, src_sub_text);
    }
    else
	sprintf(srcbuf, "%-s", src);
    
    sink = gen_get_text(ea[edge].sink);
    if (ea[edge].sink_str)
    {
#ifdef OLD_DEP_FORMAT 
	sprintf(sinkbuf, "%-s%-s", sink, ea[edge].sink_str);
#else /* JMC 1/93 fix display of ParaScope IP edges */
	sprintf(sinkbuf, "%-s", ea[edge].sink_str);
#endif
    }
    else if (sink_sub_text)
    {
	sprintf(sinkbuf, "%-s%-s", sink, sink_sub_text);
    }
    else
	sprintf(sinkbuf, "%-s", sink);
    
    level = ea[edge].level;
    type = ea[edge].type;
    block = ssave(el_get_block( PED_EL(ped), id));
    if (!strcmp(block, "_local_") || !strcmp(block, "_blank"))
    {
	sfree(block);
	block = ssave(" ");
    }
    
    /*----------------------------------------------*/
    /* create dependence vector string - cwt Feb 90 */
    
    if (ea[edge].dt_type == DT_UNKNOWN)
    {
	dvect = "";
    }
    else
    {
	dvect = ea[edge].dt_str;
    }
    
    switch (type)
    {
    case dg_true:
	if (level == LOOP_INDEPENDENT)
	{
	    sprintf(buffer, "True     %-16s %-16s %-13s independent %-8s", 
		    srcbuf, sinkbuf, dvect, block);
	}
	else
	{
	    sprintf(buffer, "True     %-16s %-16s %-13s %7d     %-8s", 
		    srcbuf, sinkbuf, dvect, level, block);
	}
	break;
	
    case dg_anti:
	if (level == LOOP_INDEPENDENT)
	{
	    sprintf(buffer, "Anti     %-16s %-16s %-13s independent %-8s", 
		    srcbuf, sinkbuf, dvect, block);
	}
	else
	{
	    sprintf(buffer, "Anti     %-16s %-16s %-13s %7d     %-8s", 
		    srcbuf, sinkbuf, dvect, level, block);
	}
	break;
	
    case dg_output:
	if (level == LOOP_INDEPENDENT)
	{
	    sprintf(buffer, "Output   %-16s %-16s %-13s independent %-8s", 
		    srcbuf, sinkbuf, dvect, block);
	}
	else
	{
	    sprintf(buffer, "Output   %-16s %-16s %-13s %7d     %-8s", 
		    srcbuf, sinkbuf, dvect, level, block);
	}
	break;
	
    case dg_input:
	if (level == LOOP_INDEPENDENT)
	{
	    sprintf(buffer, "Input    %-16s %-16s %-13s independent %-8s", 
		    srcbuf, sinkbuf, dvect, block);
	}
	else
	{
	    sprintf(buffer, "Input    %-16s %-16s %-13s %7d     %-8s", 
		    srcbuf, sinkbuf, dvect, level, block);
	}
	break;
	
    case dg_inductive:
	if (level == LOOP_INDEPENDENT)
	{
	    sprintf(buffer, "Induct   %-16s %-16s %-13s independent %-8s", 
		    srcbuf, sinkbuf, "", block);
	}
	else
	{
	    sprintf(buffer, "Induct   %-16s %-16s %-13s %7d     %-8s", 
		    srcbuf, sinkbuf, "", level, block);
	}
	break;
	
    case dg_exit:
	sprintf(buffer, "Exit %s ", 
		ped->GetLine(PED_ED_HANDLE(ped), ea[edge].src));
	break;
	
    case dg_io:
	sprintf(buffer, "IO   %s ", 
		ped->GetLine(PED_ED_HANDLE(ped), ea[edge].src));
	break;
	
    case dg_call:
	sprintf(buffer, "Call  %s ", 
		ped->GetLine(PED_ED_HANDLE(ped), ea[edge].src));
	break;
	
    case dg_control:
	switch(ea[edge].cdtype)
	{
	case CD_UNCONDITIONAL:
	    sprintf(buffer, "Control (must)  %s    %d", 
		    ped->GetLine(PED_ED_HANDLE(ped), ea[edge].src),
		    ea[edge].level);
	    break;
	    
	case CD_LOGICAL_IF:
	    if (ea[edge].cdlabel == CD_FALSE)
	    {
		sprintf(buffer, "Control (false) %s    %d", 
			ped->GetLine(PED_ED_HANDLE(ped), ea[edge].src),
			ea[edge].level);
	    }
	    else if (ea[edge].cdlabel == CD_TRUE)
	    {
		sprintf(buffer, "Control (true)  %s    %d", 
			ped->GetLine(PED_ED_HANDLE(ped), ea[edge].src),
			ea[edge].level);
	    }
	    break;
	    
	case CD_DO_LOOP:
	    if (ea[edge].cdlabel == CD_FALLTHROUGH)
	    {
		sprintf(buffer, "Control (fall through) %s    %d", 
			ped->GetLine(PED_ED_HANDLE(ped), ea[edge].src),
			ea[edge].level);
	    }
	    else
	    {
		sprintf(buffer, "Control (enter) %s    %d", 
			ped->GetLine(PED_ED_HANDLE(ped), ea[edge].src),
			ea[edge].level);
	    }
	    break;
	    
	case CD_ARITHMETIC_IF:
	    if (ea[edge].cdlabel == CD_NEGATIVE)
	    {
		sprintf(buffer, "Control (negative) %s    %d", 
			ped->GetLine(PED_ED_HANDLE(ped), ea[edge].src),
			ea[edge].level);
	    }
	    else if (ea[edge].cdlabel == CD_ZERO)
	    {
		sprintf(buffer, "Control (zero) %s    %d", 
			ped->GetLine(PED_ED_HANDLE(ped), ea[edge].src),
			ea[edge].level);
	    }
	    else
	    {
		sprintf(buffer, "Control (positive) %s    %d", 
			ped->GetLine(PED_ED_HANDLE(ped), ea[edge].src),
			ea[edge].level);
	    }
	    break;

	case CD_COMPUTED_GOTO:
	case CD_ASSIGNED_GOTO:
	case CD_OPEN_ASSIGNED_GOTO:
	case CD_CASE:
	case CD_ALTERNATE_RETURN:
	    sprintf(buffer, "Control (%d) %s    %d", ea[edge].cdlabel,
		    ped->GetLine(PED_ED_HANDLE(ped), ea[edge].src),
		    ea[edge].level);
	    break;

	case CD_I_O:
	    if (ea[edge].cdlabel == CD_FALLTHROUGH)
	    {
		sprintf(buffer, "Control (fall through) %s    %d", 
			ped->GetLine(PED_ED_HANDLE(ped), ea[edge].src),
			ea[edge].level);
	    }
	    else if (ea[edge].cdlabel == CD_IO_ERR)
	    {
		sprintf(buffer, "Control (error) %s    %d", 
			ped->GetLine(PED_ED_HANDLE(ped), ea[edge].src),
			ea[edge].level);
	    }
	    else
	    {
		sprintf(buffer, "Control (end) %s    %d", 
			ped->GetLine(PED_ED_HANDLE(ped), ea[edge].src),
			ea[edge].level);
	    }
	    break;
	}
	break;
    case dg_unknown:
	sprintf(buffer, "Unknown");
	break;
    }
    
    sfree(block);
    return buffer;
}

/*---------------------------------------------------------------------
  
  highLight()
  
  This routine is no longer used to emphasize selected material in the text.
  It has been superseded by depFortFilter which selectively resets the
  emphasis on the line of text that it is called with.
  
  */

static void
highLight(PedInfo ped, AST_INDEX src, AST_INDEX sink)
{
    if (PED_PREV_SRC(ped) != AST_NIL)
    {
	ped->TreeWillChange(PED_ED_HANDLE(ped), PED_PREV_SRC(ped));
	ft_SetEmphasis(PED_PREV_SRC(ped), false);
	ped->TreeChanged(PED_ED_HANDLE(ped), PED_PREV_SRC(ped));
    }
    
    if (PED_PREV_SINK(ped) != AST_NIL)
    {
	ped->TreeWillChange(PED_ED_HANDLE(ped), PED_PREV_SINK(ped));
	ft_SetEmphasis(PED_PREV_SINK(ped), false);
	ped->TreeChanged(PED_ED_HANDLE(ped), PED_PREV_SINK(ped));
    }
    
    if (src != AST_NIL)
    {
	ped->TreeWillChange(PED_ED_HANDLE(ped), src);
	ft_SetEmphasis(src, true);
	ped->TreeChanged(PED_ED_HANDLE(ped), src);
    }
    
    if (sink != AST_NIL)
    {
	ped->TreeWillChange(PED_ED_HANDLE(ped), sink);
	ft_SetEmphasis(sink, true);
	ped->TreeChanged(PED_ED_HANDLE(ped), sink);
    }
    
    PED_PREV_SRC(ped) = src;
    PED_PREV_SINK(ped) = sink;
}


/*---------------------------------------------------------------------
  
 */

void
pedPrevDep(PedInfo ped)
{
    if (PED_SELECTED_LOOP(ped) == AST_NIL)
	return;
    (void) pedDepSelect(ped, PED_CURRENT_DEPENDENCE(ped) - 1);
}

/*---------------------------------------------------------------------
  
 */

void
pedNextDep(PedInfo ped)
{
    if (PED_SELECTED_LOOP(ped) == AST_NIL)
	return;
    (void) pedDepSelect(ped, PED_CURRENT_DEPENDENCE(ped) + 1);
}

/*---------------------------------------------------------------------
  
 */

void
pedPrevLoop(PedInfo ped)
{
    AST_INDEX       stmt;
    
    if (PED_SELECTION(ped) == AST_NIL)
    {
	return;
    }
    
    stmt = PED_SELECTION(ped);
    
    /* walk out to a statement if necessary */
    if (!is_statement(stmt))
	stmt = out(stmt);
    
    stmt = findAbove(stmt);
    if (stmt != AST_NIL)
	pedUpdate(ped, stmt);
}

/*---------------------------------------------------------------------
  
 */

void
pedNextLoop(PedInfo ped)
{
    AST_INDEX       stmt;
    
    if (PED_SELECTION(ped) == AST_NIL)
    {
	return;
    }
    
    stmt = PED_SELECTION(ped);
    
    /* walk out to a statement if necessary */
    if (!is_statement(stmt))
	stmt = out(stmt);
    
    stmt = findBelow(stmt);
    if (stmt != AST_NIL)
	pedUpdate(ped, stmt);
}


/*---------------------------------------------------------------------
  
 */

static AST_INDEX
findNext(AST_INDEX stmt)
{
    /* move in if possible, list may be empty */
    if (is_compound(stmt) && in(stmt) != AST_NIL)
	return in(stmt);
    
    /* may have to move out to find a next */
    while ((stmt != AST_NIL) && (next(stmt) == AST_NIL))
	stmt = out(stmt);
    
    if (stmt == AST_NIL)
	return AST_NIL;
    
    return next(stmt);
}

/*---------------------------------------------------------------------
  
 */

static AST_INDEX
findPrev(AST_INDEX stmt)
{
    /* may have to move out to find a prev */
    if (prev(stmt) != AST_NIL)
    {
	stmt = prev(stmt);
	while ((is_compound(stmt)) && (in_to_end(stmt) != AST_NIL))
	    stmt = in_to_end(stmt);
	
	/* at a simple statement */
	return stmt;
    }
    else
	return out(stmt);
}

/*---------------------------------------------------------------------
  
 */

/* ARGSUSED */
static AST_INDEX
findAbove(AST_INDEX stmt)
{
    for (;;)
    {
	stmt = findPrev(stmt);
	
	if (stmt == AST_NIL)
	    return AST_NIL;
	
	if (is_loop(stmt))
	    return stmt;
    }
}

/*---------------------------------------------------------------------
  
 */

/* ARGUSED */
static AST_INDEX
findBelow(AST_INDEX stmt)
{
    for (;;)
    {
	stmt = findNext(stmt);
	if (stmt == AST_NIL)
	    return AST_NIL;
	
	if (is_loop(stmt))
	    return stmt;
    }
}

/*---------------------------------------------------------------------
  
 */

EL_Instance	*
edgePush(PedInfo ped)
{
    stack_push(PED_EL_STACK(ped), (Generic *) & PED_EL(ped));
    PED_EL(ped) = el_copy_active_edge_list_structure(PED_EL(ped));
    PED_STACK_DEPTH(ped)++;
    setDepTitle(ped, 0);
    edgeUpdate(ped);
    
    return PED_EL(ped);
}

/*---------------------------------------------------------------------
  
 */

EL_Instance	*
edgePop(PedInfo ped)
{
    Generic	newEL;
    
    if (PED_STACK_DEPTH(ped) > 0)
    {
	stack_pop(PED_EL_STACK(ped), &newEL);
	PED_STACK_DEPTH(ped)--;
	el_destroy_instance(PED_EL(ped));
	PED_EL(ped) = (EL_Instance *)newEL;
	setDepTitle(ped, 0);
	edgeUpdate(ped);
    }
    
    return PED_EL(ped);
}


/*---------------------------------------------------------------------

*/

/*** LIST_SM CALLBACK ***/
/* ARGSUSED */
static lsm_line_info *
dep_list_handler(Generic pane, PedInfo ped, Boolean first, Generic id)
{
	lsm_line_info  *line = 0;
	int             edge;
	DG_Edge        *ea;

	/*------------------------------------------------------*/
	/* if currently editing program, do not display deps 	*/

	if (PED_EDIT_PERFORMED(ped))
		return 0;

	if (first)
	{	
		edge = first_dependence(PED_EL(ped));	/* give the first line */

		if (edge == -1)
			line = 0;
		else
		{
			line = (lsm_line_info *) get_mem(sizeof(lsm_line_info), 
				"dep list line");
			ea = dg_get_edge_structure( PED_DG(ped));

			line->text = ssave(getLine(ped, ea, edge, 0));
			line->should_free = true;
			line->len = UNUSED;
			line->id = 0;
			line->selected = BOOL(PED_CURRENT_DEPENDENCE(ped) == 0);
			line->selectable = BOOL(edge_is_active( PED_EL(ped), 0));
		}
	}
	else
	{
		id++;
		edge = next_dependence(PED_EL(ped));

		if (edge == -1)
			line = 0;
		else
		{
			line = (lsm_line_info *) get_mem(sizeof(lsm_line_info), 
				"dep list line");
			ea = dg_get_edge_structure( PED_DG(ped));
			line->text = ssave(getLine(ped, ea, edge, id));
			line->should_free = true;
			line->len = UNUSED;
			line->id = id;
			line->selected = BOOL(PED_CURRENT_DEPENDENCE(ped) == id);
			line->selectable = BOOL(edge_is_active( PED_EL(ped), id));
		}
	}

	return line;
}


/* ****************************************************************
This structure contains the information needed while walking the AST
using the 
*******************************************************************	*/
struct struct_SubsWalkParams
{
  SideInfo	*infoPtr;
  FortTextTree	 ftt;
}; 


/************************************************************
This routine checks to see if an expression is a subscript and
if so copies the subscript text into the side array.
*************************************************************/
static int
put_subscript_text(AST_INDEX expr, SubsWalkParams *subsWalkParams)
{
  if (is_subscript(expr))
    {
      dg_put_info(subsWalkParams->infoPtr, 
		  gen_SUBSCRIPT_get_name(expr), 
		  type_subscript, 
		  (Generic)ftt_NodeToStr(subsWalkParams->ftt, gen_SUBSCRIPT_get_rvalue_LIST(expr)));
    }
  return(WALK_CONTINUE);
}


/****************************************************************
This routine attaches the subscripts found in the statement to the 
subscript side arary with the ast.
*****************************************************************/

static int 
attach_subscripts(AST_INDEX stmt, int nesting_level, SubsWalkParams *subsWalkParams)
{
  AST_INDEX  expr1;
  AST_INDEX  expr2;

  if (get_expressions(stmt, &expr1, &expr2) == UNKNOWN_STATEMENT)
    { 
      return(WALK_CONTINUE); 
    }

  if (expr1 != AST_NIL)
      walk_expression(expr1, NULL, (WK_EXPR_CLBACK)put_subscript_text,
                      (Generic)subsWalkParams);
  if (expr2 != AST_NIL)
      walk_expression(expr2, NULL, (WK_EXPR_CLBACK)put_subscript_text, 
                      (Generic)subsWalkParams);

  return(WALK_CONTINUE);
}

/*---------------------------------------------------------------------

*/

void
subs_attach(AST_INDEX root, FortTextTree ftt, SideInfo *infoPtr)
{
  SubsWalkParams	subsWalkParams;

  subsWalkParams.infoPtr	= infoPtr;
  subsWalkParams.ftt		= ftt;

  walk_statements(root, LEVEL1, NOFUNC, (WK_STMT_CLBACK)attach_subscripts, 
                  (Generic)&subsWalkParams);
}

/*---------------------------------------------------------------------

*/
void
subs_free(SideInfo *infoPtr)
{
    int	count;
    Info_elem	*info_array;
    
    info_array = SI_INFO_ARRAY(infoPtr);
    
/*    for (count=0; count < ((int *)info_array)[-1];)
    {
	if (info_array[count].subscript_index != NO_SUBSCRIPT)
	    free_mem((char *)info_array[count].subscript_index);
	count++;
    }
*/
}


/******************************/
/******************************/
/* SKW */

char * ped_DepToText(PedInfo ped, DG_Edge *ea, int edge, int id)
{
  return getLine(ped, ea, edge, id);
}
