/* $Id: trans.C,v 1.1 1997/06/25 13:52:57 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ped_cp/dp/trans.c							*/
/*									*/
/*	trans.c -- List of PED transformations 	*/
/*									*/
/*									*/
/************************************************************************/

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped.h>
#include <libs/support/database/newdatabase.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/ast/groups.h>
#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/vanilla_sm.h>
#include <libs/graphicInterface/oldMonitor/include/sms/list_sm.h>
#include <libs/graphicInterface/oldMonitor/include/mon/dialog.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/ped_structs.h>
#include <libs/frontEnd/textTree/TextTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pedCP/PedPrivate.h>

#include <libs/moduleAnalysis/dependence/loopInfo/li_instance.h>
#include <libs/moduleAnalysis/dependence/loopInfo/private_li.h>
#include <libs/moduleAnalysis/dependence/edgeList/el_instance.h>
#include <libs/moduleAnalysis/dependence/edgeList/el_header.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dt.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/pt_util.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/align.h>

#ifdef CPROP_STUBBED_OFF
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/cprop.h>
#endif

STATIC(void, unsetParallel,(PedInfo ped));

/*---------------------------------------------------------------------
  
 */

void
pedDelete(Generic PD, PedInfo ped)
{
    int             edge;
    int             oldsel;
    
    if (PED_CURRENT_DEPENDENCE(ped) < 0)	/* no such edge */
	return;
    oldsel = PED_CURRENT_DEPENDENCE(ped);
    edge = get_dependence( PED_EL(ped), oldsel);
    if (edge == -1)
	return;
    
    /* is this edge active? */
    if (!edge_is_active( PED_EL(ped), PED_CURRENT_DEPENDENCE(ped)))
	return;
    dg_delete_free_edge( PED_DG(ped), edge);
    forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped));
    (void) pedDepSelect(ped, oldsel - 1);
    dialogUpdate(PD);
}

/*---------------------------------------------------------------------
  
 */

void
doInterchange(InterDia *ih, Generic handle)
{
    PedInfo     	ped;
    
    ped = (PedInfo) handle;
    
    ped->TreeWillChange(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped));
    
    pt_switch_loops(ped, PED_SELECTED_LOOP(ped), ih->loop_type,ih->tri_type);
    
    ped->TreeChanged(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped));
    
    ped->UpdateNodeInfo(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped), UPDATE_DTINFO);
    forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped));
}

/*---------------------------------------------------------------------
  
 */

void
doDistribution(DistrDia *dh, Generic handle, int type)
{
    PedInfo         ped;
    AST_INDEX       ur;
    AST_INDEX       oldloop;
    AST_INDEX       newloop;
    AST_INDEX       oldsel;
    AST_INDEX       newsel;
    int             level;
    
    ped     = (PedInfo) handle;
    ur      = out(PED_SELECTED_LOOP(ped));
    oldsel  = PED_SELECTION(ped);
    oldloop = PED_SELECTED_LOOP(ped);
    
    ped->TreeWillChange(PED_ED_HANDLE(ped), ur);
    
    level   = loop_level (oldloop);
    newloop = pt_do_distribution (ped, oldloop, type);

    ped->TreeChanged(PED_ED_HANDLE(ped), ur);
    
    /* find the selection */
    
    if (oldsel == oldloop)
	newsel = newloop;
    else
    {	/* the selection was inside the loop, find which loop it's in now */
	
	newsel = oldsel;
	for (newloop = out(newsel); ;newloop = out(newloop))
	{
	    if (is_loop(newloop))
		if (level == loop_level(newloop))
		    break;
	}
    }
    ped->UpdateNodeInfo (PED_ED_HANDLE(ped), newloop, UPDATE_DTINFO);
    forcePedUpdate (ped, newloop, newsel);
    unsetParallel( ped);
}

/*---------------------------------------------------------------------
  
 */

void
doScalarExpansion(PedInfo ped)
{
    AST_INDEX       scope;
    
    scope = find_scope(PED_SELECTED_LOOP(ped));
    ped->TreeWillChange(PED_ED_HANDLE(ped), scope);
    se_scalar_expand(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped));
    ped->TreeChanged(PED_ED_HANDLE(ped), scope);
    ped->UpdateNodeInfo(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped), 
			UPDATE_SUBS | UPDATE_DTINFO);
    forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTED_LOOP(ped));
}

/*---------------------------------------------------------------------
  
 */

void
doRenamingExpansion(PedInfo ped)
{
    AST_INDEX       scope;
    
    scope = find_scope(PED_SELECTED_LOOP(ped));
    ped->TreeWillChange(PED_ED_HANDLE(ped), scope);
    pt_rename(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped));
    ped->TreeChanged(PED_ED_HANDLE(ped), scope);
    ped->UpdateNodeInfo(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped),
			UPDATE_SUBS | UPDATE_DTINFO);
    forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTED_LOOP(ped));
}


/*---------------------------------------------------------------------
  
 */

void
doAddStmt(PedInfo ped, char *arg)
{
    AST_INDEX scope;
    
    scope = find_scope(PED_SELECTION(ped));
    ped->TreeWillChange(PED_ED_HANDLE(ped), scope);
    pt_add_stmt(ped, arg); 
    
    pedReinitialize(ped);	/* for now, since tree root damaged	*/
    
    ped->TreeChanged(PED_ED_HANDLE(ped), PED_ROOT(ped));
    forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped)); 
}

/*---------------------------------------------------------------------
  
 */

void
doDeleteStmt(PedInfo ped)
{
    AST_INDEX       scope, next;
    
    if ((next = list_next(PED_SELECTION(ped))) == AST_NIL)
	next = tree_out(PED_SELECTION(ped));
    scope = find_scope(PED_SELECTED_LOOP(ped));
    ped->TreeWillChange(PED_ED_HANDLE(ped), scope);
    pt_delete_stmt(ped, PED_SELECTION(ped));
    PED_SELECTION(ped) = next;
    ped->TreeChanged(PED_ED_HANDLE(ped), scope);
    forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped));
}

/*---------------------------------------------------------------------
  
 */

void
doStmtInter(PedInfo ped)
{
    ped->TreeWillChange(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped));
    PED_SELECTION(ped) = pt_reorder_stmts(PED_SELECTION(ped));
    ped->TreeChanged(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped));
    forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped));
}

/*---------------------------------------------------------------------
  
 */

void
doPeelIterations(PedInfo ped, Boolean iteration, char *iter)
{
    AST_INDEX       scope;
    
    scope = find_scope(PED_SELECTED_LOOP(ped));
    ped->TreeWillChange(PED_ED_HANDLE(ped), scope);
    pt_peel_iteration(ped, PED_SELECTED_LOOP(ped), iteration, atoi(iter));
    ped->TreeChanged(PED_ED_HANDLE(ped), scope);
    ped->UpdateNodeInfo(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped), 
			UPDATE_DTINFO);
    forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped));
}


/*---------------------------------------------------------------------
  
 */

void
doSplit(PedInfo ped, char *step)
{
    AST_INDEX       scope;
    
    scope = find_scope(PED_SELECTED_LOOP(ped));
    ped->TreeWillChange(PED_ED_HANDLE(ped), scope);
    pt_split(ped, PED_SELECTED_LOOP(ped), step);           
    ped->TreeChanged(PED_ED_HANDLE(ped), scope);
    /* update the NodeInfo for the original loop */
    ped->UpdateNodeInfo(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped),
			UPDATE_DTINFO);
    /* update the NodeInfo for the loop that was split off */
    ped->UpdateNodeInfo(PED_ED_HANDLE(ped), list_next(PED_SELECTED_LOOP(ped)), UPDATE_DTINFO);
    forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped));
}


/*---------------------------------------------------------------------
  
 */

void
doStripMine(PedInfo ped, char *step)
{
    AST_INDEX       scope;
    
    scope = find_scope(PED_SELECTED_LOOP(ped));
    ped->TreeWillChange(PED_ED_HANDLE(ped), scope);
    pt_strip_mine(ped, PED_SELECTED_LOOP(ped), 0, step);
    ped->TreeChanged(PED_ED_HANDLE(ped), scope);
    ped->UpdateNodeInfo(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped),
			UPDATE_SUBS | UPDATE_DTINFO);
    forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped));
}

/*---------------------------------------------------------------------
  
 */

#ifdef CPROP_STUBBED_OFF
void
doCProp(PedInfo ped)
{
    AST_INDEX       root;
    
    root = PED_ROOT(ped);
    ped->TreeWillChange(PED_ED_HANDLE(ped), root);
    
    if (PED_CPROP(ped))
    {
        cprop_change_ast(PED_CPROP(ped));
        cprop_free_globals(PED_CPROP(ped));
        PED_CPROP(ped) =
	    cprop_build_structs((Generic) NULL, PED_FT(ped),
				/* make_changes */	0,
				/* print */		0,
				/* kill_graphs */	0,
				/* doValueTable */	0,
				/* doExecAlg */		1);
    }
    else
    {
        PED_CPROP(ped) =
	    cprop_build_structs((Generic) NULL, PED_FT(ped),
				/* make_changes */	0,
				/* print */		0,
				/* kill_graphs */	0,
				/* doValueTable */	0,
				/* doExecAlg */		1);
        cprop_change_ast(PED_CPROP(ped));
        cprop_free_globals(PED_CPROP(ped));
        PED_CPROP(ped) = (Generic) NULL;
    }

    ped->TreeChanged(PED_ED_HANDLE(ped), root);
    ped->UpdateNodeInfo(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped),
			UPDATE_SUBS | UPDATE_DTINFO);
    forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped));

}
#endif

/*---------------------------------------------------------------------
  
 */


void
doSkew(PedInfo ped, char *skew_degree)
{
    AST_INDEX       scope;
    
    scope = find_scope(PED_SELECTED_LOOP(ped));
    ped->TreeWillChange(PED_ED_HANDLE(ped), scope);
    pt_skew(skew_degree, PED_SELECTED_LOOP(ped)); 
    ped->TreeChanged(PED_ED_HANDLE(ped), scope);
    dg_set_set_interchange(PED_DG(ped), true);	/* want to update interchange flags */
    ped->UpdateNodeInfo(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped),
			UPDATE_SUBS | UPDATE_DTINFO);
    dg_set_set_interchange(PED_DG(ped), false);
    forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped)); 
}

/*---------------------------------------------------------------------
  
 */


void
doReverse(PedInfo ped)
{
    AST_INDEX scope;
    
    scope = find_scope(PED_SELECTED_LOOP(ped));
    ped->TreeWillChange(PED_ED_HANDLE(ped), scope);
    pt_reverse(ped); 
    ped->TreeChanged(PED_ED_HANDLE(ped), scope);
    ped->UpdateNodeInfo(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped),
			UPDATE_SUBS | UPDATE_DG);
    forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped)); 
}

/*---------------------------------------------------------------------
  
 */


void
doAdjust(PedInfo ped, char *adjust)
{
    AST_INDEX scope;
    
    scope = find_scope(PED_SELECTED_LOOP(ped));
    ped->TreeWillChange(PED_ED_HANDLE(ped), scope);
    pt_adjust(ped, adjust); 
    ped->TreeChanged(PED_ED_HANDLE(ped), scope);
    ped->UpdateNodeInfo(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped),
			UPDATE_SUBS | UPDATE_DG);
    forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped)); 
}

/*---------------------------------------------------------------------
  
 */


void
doAlign(PedInfo ped, char *arg)
{
    AST_INDEX scope;
    
    scope = find_scope(PED_SELECTED_LOOP(ped));
    ped->TreeWillChange(PED_ED_HANDLE(ped), scope);
    pt_align(ped, arg); 
    ped->TreeChanged(PED_ED_HANDLE(ped), scope);
    ped->UpdateNodeInfo(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped),
			UPDATE_SUBS | UPDATE_DG);
    forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped));
}

/*---------------------------------------------------------------------
  
 */


void
doReplaceS(PedInfo ped)
{
    AST_INDEX scope;
    
    scope = find_scope(PED_SELECTED_LOOP(ped));
    ped->TreeWillChange(PED_ED_HANDLE(ped), scope);
    pt_rep_s(ped); 
    ped->TreeChanged(PED_ED_HANDLE(ped), scope);
    ped->UpdateNodeInfo(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped),
			UPDATE_SUBS);
    forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped)); 
}

/*---------------------------------------------------------------------
  
 */


void
doUnroll(PedInfo ped, char *arg)
{
    AST_INDEX scope;
    
    scope = find_scope(PED_SELECTED_LOOP(ped));
    ped->TreeWillChange(PED_ED_HANDLE(ped), scope);
    pt_unroll_jam(ped, arg, false); 
    ped->TreeChanged(PED_ED_HANDLE(ped), scope);
    ped->UpdateNodeInfo(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped),
			UPDATE_SUBS | UPDATE_DG);
    forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped)); 
}

/*---------------------------------------------------------------------
  
 */


void
doUnrollJam(PedInfo ped, char *arg)
{
    AST_INDEX scope;
    
    scope = find_scope(PED_SELECTED_LOOP(ped));
    ped->TreeWillChange(PED_ED_HANDLE(ped), scope);
    pt_unroll_jam(ped, arg, true); 
    ped->TreeChanged(PED_ED_HANDLE(ped), scope);
    ped->UpdateNodeInfo(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped),
			UPDATE_SUBS | UPDATE_DG);
    forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped)); 
}

/*---------------------------------------------------------------------
  
 */


void
doFusion(PedInfo ped)
{
    AST_INDEX scope;
    
    scope = find_scope(PED_SELECTED_LOOP(ped));
    ped->TreeWillChange(PED_ED_HANDLE(ped), scope);
    pt_fuse(ped,PED_SELECTED_LOOP(ped)); 
    ped->TreeChanged(PED_ED_HANDLE(ped), scope);
    ped->UpdateNodeInfo(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped),
			UPDATE_DG);
    forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped)); 
}


/*---------------------------------------------------------------------
  
 */


void
doUnswitch(PedInfo ped)
{
    AST_INDEX scope;
    
    scope = find_scope(PED_SELECTED_LOOP(ped));
    ped->TreeWillChange(PED_ED_HANDLE(ped), scope);
    pt_unswitch(ped); 
    ped->TreeChanged(PED_ED_HANDLE(ped), scope);
    pedReinitialize(ped);
    forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped)); 
}

/*---------------------------------------------------------------------
  
 */

static void
unsetParallel(PedInfo ped)
{
    AST_INDEX       node;
    Loop_info      *lptr;
    
    node = gen_DO_get_stmt_LIST(PED_SELECTED_LOOP(ped));
    for (node = list_first(node); node != AST_NIL; node = list_next(node))
    {
	if (is_do(node) || is_parallelloop(node))
	{
	    lptr = find_loop( PED_LI(ped), node);
	    lptr->parallelized = false;
	}
	
    }
    
    
}

/*---------------------------------------------------------------------
  
  makeParallel() - Parallelize the selected loop. 
  
  If it has any loop carried deps, complain first, then 
  allow user to override and parallelize anyway. Vas, Sept. 1988.
  
  */

void
makeParallel(PedInfo ped)
{
    AST_INDEX       loop;
    EL_Instance    *el;
    LI_Instance    *li;
    Boolean         ok;
    
    loop = PED_SELECTED_LOOP(ped);
    if (is_parallelloop(loop))
	return;
    
    /* check if there are any dependences in this loop */
    el = (EL_Instance *) PED_EL(ped);
    
    if( el_num_lc_shared(el) > 0 )
    {
	if (el_show_loopCarried(el) == false)
	{
	    yes_no("Caution: This loop may have parallelism inhibiting dependences.\nTo see these dependences use the type dialog and include loop carried.\nParallelize anyway?", &ok, false);
	}
	else
	{
	    yes_no("Caution: This loop has parallelism inhibiting dependences.\nParallelize anyway?", &ok, false);
	}
	if (ok == false)
	    return;
    }
    
    ped->TreeWillChange(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped));
    pt_make_parallel(ped, loop);
    
    li = (LI_Instance *) PED_LI(ped);
    loopUpdate(ped, li->cur_loop->loop_hdr_index);
    
    ped->TreeChanged(PED_ED_HANDLE(ped), PED_SELECTED_LOOP(ped));
    forcePedUpdate(ped, PED_SELECTED_LOOP(ped), PED_SELECTION(ped));
}


/*---------------------------------------------------------------------
  
  makeSequential() - make selected loop sequential 
  
  */

void
makeSequential(PedInfo ped)
{
    AST_INDEX       loop;
    AST_INDEX		parent;
    LI_Instance    *li;
    
    li = (LI_Instance *) PED_LI(ped);
    loop = PED_SELECTED_LOOP(ped);
    if (!is_parallelloop(loop))
	return;
    
    parent = tree_out(loop);
    ped->TreeWillChange(PED_ED_HANDLE(ped), parent);
    pt_make_sequential(ped, loop);
    
    loop = li->cur_loop->loop_hdr_index;
    PED_SELECTED_LOOP(ped) = loop;
    PED_SELECTION(ped) = loop;
    PED_PREV_LOOP(ped) = loop;
    
    ped->TreeChanged(PED_ED_HANDLE(ped), parent);
    forcePedUpdate(ped, loop, loop);
}







