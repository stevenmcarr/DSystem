/* $Id: ssa_ip_refs.C,v 1.1 1997/06/25 15:10:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- ssa_ip_refs.c
 *
 *           Build interprocedural references.
 */

#include <libs/moduleAnalysis/ssa/ssa_private.h>
#include <libs/moduleAnalysis/ssa/idfa.h>

#include <libs/ipAnalysis/ipInfo/iptypes.h>

#define poss_gen_inf_exp(nt) ((nt == CFG_DO_CND) || (nt == CFG_DO_IND) || \
                              (nt == CFG_SPREAD) || (nt == CFG_FORK))

/*
 *  forward declarations
 */
STATIC(void, add_entry_formals, (CfgInstance cfg, CfgNodeId cfgId));
STATIC(void, enlist_ip_var, (SymDescriptor symtab, fst_index_t var, 
			     Generic passCfg));
STATIC(void, add_from_list, (CfgInstance cfg, CfgNodeId cfgParent, 
			     SsaNodeId ssaParent, 
                             fst_index_t list[], SsaType type));
STATIC(int, nameIsActual, (CfgInstance cfg, fst_index_t var, 
			   AST_INDEX invoc_node));

typedef struct three_lists_struct 
{
    fst_index_t *formals;
    fst_index_t *commons;
    fst_index_t *statics;
} ThreeLists;

static ThreeLists ipVars;

#define INIT_LISTS 12

/****************************************************************************
 ***			  PRIVATE FUNCTIONS                               ***
 ****************************************************************************/
static void add_entry_formals(CfgInstance cfg, CfgNodeId cfgId)
{
    AST_INDEX arg;
    fst_index_t var;
    AST_INDEX entry, formals;

    entry = CFG_node(cfg, cfgId)->astnode;

    if (is_subroutine(entry))
	formals = gen_SUBROUTINE_get_formal_arg_LIST(entry);
    else if (is_function(entry))
	formals = gen_FUNCTION_get_formal_arg_LIST(entry);
    else if (is_entry(entry))
        formals = gen_ENTRY_get_formal_arg_LIST(entry);
    else
	return;

    for (arg = list_last(formals);
	 !is_null_node(arg);
	 arg = list_prev(arg)) 
    {
	/*
	 *  Only track formals which contain data...
	 *  Track arrays only if asked...
	 *  Don't add pseudo-refs unless there were real refs before.
	 */
	if (is_identifier(arg))
	{
	    var = SsaGetSym(cfg, arg);

	    if (((fst_GetFieldByIndex(cfg->symtab, var, SYMTAB_OBJECT_CLASS))
		 & OC_IS_DATA) &&
		(SSA_doArrays(cfg) || !FS_IS_ARRAY(cfg->symtab, var)))
	    {
		(void) ssa_init_node(cfg, cfgId, SSA_NIL, arg, SSA_IP_IN, var);
	    }
	}
    }
} /* end of add_entry_formals() */






/* 
 * -- ssa_add_ip_refs
 *
 *      add references for formals at entries and exit, and
 *	for globals at entries, call sites, and exit
 *
 *	also, references for static (SAVE) variables on entry and exit
 *
 *	-- foreach definition of DUMMY_GLOBAL (a placeholder variable)
 *		-- if at an entry point, add formal/global/static defs
 *		-- if at a call, add global refs & defs
 *	-- foreach variable
 *		-- if a formal/global/static, add a ref at END
 *
 *	-- Note that if recursion is possible, call sites need to be treated
 *		as modifying any statics with local, non-DATA modifications.
 */
void ssa_add_ip_refs(CfgInstance cfg)
{
    SsaNodeId def;
    SsaDefVar *slot;
    SsaNodeId parent;
    CfgNodeId stmt;

    ipVars.formals = (fst_index_t *) f_alloc(INIT_LISTS, sizeof(fst_index_t),
					     "ssa formals", NULL);
    ipVars.commons = (fst_index_t *) f_alloc(INIT_LISTS, sizeof(fst_index_t),
					     "ssa common vars ", NULL);
    ipVars.statics = (fst_index_t *) f_alloc(INIT_LISTS, sizeof(fst_index_t),
					     "ssa statics", NULL);
    
    fst_ForAll(cfg->symtab, enlist_ip_var, (Generic) cfg);

    /*
     *  Examine each definition of DUMMY_GLOBAL.  These correspond to
     *  procedure entry points and call sites.
     */
    slot = ssa_find_ref_slot(cfg, DUMMY_GLOBAL(cfg));

    for (def = slot->defList;
	 def != SSA_NIL;
	 def = SSA_def(cfg, def)->nextDef)
    {
	stmt = SSA_node(cfg, def)->cfgParent;

	if (SSA_node(cfg, def)->type == SSA_IP_IN)
	{
	    /*
	     *  Add entry defs for commons, formals, statics
	     *
	     *  ... only if the variable also has a direct reference, or
	     *  we have IP info and there is an indirect reference.
	     */

	    add_from_list(cfg, stmt, SSA_NIL, ipVars.commons, SSA_IP_IN);
	    add_from_list(cfg, stmt, SSA_NIL, ipVars.statics, SSA_IP_IN);

	    add_entry_formals(cfg, stmt);
	}
	else
	{
	    if (SSA_node(cfg, def)->type != SSA_IP_MOD)
		die_with_message("ssa_add_ip_refs: neither IP_MOD nor IP_IN\n");

	    /*
	     *  Add call-site mods for globals
	     *
	     *  ... only if the variable also has a direct reference,
	     *  or we have IP info and there is an indirect reference.
	     *
	     *  If recursion is allowed, add mods only if there is a direct
	     *  modification (indirect can't happen for local statics)
	     */
	    parent = SSA_node(cfg, def)->ssaParent;

	    add_from_list(cfg, stmt, parent, ipVars.commons, SSA_IP_MOD);
	}
    }

    /*
     *  Simulated uses at the end for everything whose last def needs 
     *  to be preserved... globals, formals, static variables...
     */
    add_from_list(cfg, cfg->end, SSA_NIL, ipVars.commons, SSA_USE);
    add_from_list(cfg, cfg->end, SSA_NIL, ipVars.statics, SSA_USE);
    add_from_list(cfg, cfg->end, SSA_NIL, ipVars.formals, SSA_USE);

    /*
     *  May need DUMMY_GLOBAL here as a template for hidden vars
     */
    (void) ssa_init_node(cfg, cfg->end, SSA_NIL, AST_NIL,
			 SSA_USE, DUMMY_GLOBAL(cfg));

    /*
     *  Handle scalar function value
     */
    if (cfg->type == CFG_FUNCTION)
    {
	fst_index_t self = fst_QueryIndex(cfg->symtab,
					  cfg_get_inst_name(cfg));

	fst_index_t equiv = fst_GetFieldByIndex(cfg->symtab, 
						self, SYMTAB_PARENT);
	if (equiv == SSA_NIL_NAME)
	    equiv = self;

	(void) ssa_init_node(cfg, cfg->end, SSA_NIL, AST_NIL,
			     SSA_USE, equiv);
    }

    f_free((Generic) (ipVars.formals));
    f_free((Generic) (ipVars.commons));
    f_free((Generic) (ipVars.statics));

} /* end of ssa_add_ip_refs() */



/* 
 * -- enlist_ip_var
 *
 *	add variable to list of formals, statics, or COMMON block vars
 */
static void enlist_ip_var(SymDescriptor symtab, fst_index_t var, Generic passCfg)
{
    fst_index_t rep;
    CfgInstance cfg = (CfgInstance) passCfg;

    if (var == DUMMY_GLOBAL(cfg)) return;

    /*
     *  Add only one representative of each set of overlapping
     *  equivalenced variables.
     */
    rep = ssa_var_rep(cfg, var);

    if (rep != var) return;

    if (!SSA_doArrays(cfg))
    {
	/*
	 *  Don't always track arrays...
	 */
	if (FS_IS_ARRAY(symtab, var)) return;
	/*
	 *  Treat sets of equivalenced vars larger than one variable 
	 *  as arrays for now...
	 */
	if (!ssa_var_covers_eq(cfg, var))
	    return;
    }

    /*
     *  Interesting if formal, global, or static...
     *		(mostly on exit, but globals interesting at call sites
     *		-- statics too, if recursion is allowed)
     */
    if ((fst_GetFieldByIndex(symtab, var, SYMTAB_OBJECT_CLASS)) & OC_IS_DATA)
    {
	fst_index_t **list = NULL;

	if (!idfaNameIsAcc(cfg, /* site = */ AST_NIL, var)) return;

	if (FS_IS_DUMMY_PARAM_FOR_ENTRY_OR_PROCEDURE(symtab, var))
	{
	    list = &(ipVars.formals);
	}
	else
	{
	    /*
	     *  Now we're down to a unique representative for each set.
	     */
	    int sc;
	    sc = fst_GetFieldByIndex(symtab, var, SYMTAB_STORAGE_CLASS);

	    if (sc & SC_GLOBAL)
	    {
		/*
		 *  Don't add pseudo-refs unless there were defs before...
		 *  or unless we got here in the presence of ip info
		 */
		if (!ssa_has_defs(cfg, var) && !SSA_ipInfo(cfg))
		    return;

		list = &(ipVars.commons);
	    }
	    else if (sc & SC_STATIC)
	    {
		list = &(ipVars.statics);
	    }
	}
	if (list)
	{
	    int index;

	    index = f_new((Generic *) list);
	    (*list)[index] = var;
	}
    }
} /* end of enlist_ip_var() */


/*
 *  -- add_from_list
 *
 *	add references from "list", subordinate to the cfg/ssa parents
 */
static void add_from_list(CfgInstance cfg, CfgNodeId cfgParent, SsaNodeId ssaParent, 
                          fst_index_t list[], SsaType type)
{
    int i, size;
    SsaNodeId New;
    AST_INDEX site;

    size = f_curr_size((Generic) list);

    switch(type)
    {
      case SSA_IP_MOD:
	site = SSA_node(cfg,ssaParent)->refAst;

	for (i = 0; i < size; i++)
	{
	    /*  Do we want to track globals here if also passed as actuals?
	     *
	     *  if (nameIsActual(cfg,list[i],SSA_node(cfg,ssaParent)->refAst))
	     *      continue;
	     */

	    /*
	     *  IP_MOD gets edges as if REF and KILL
	     */
	    if (idfaNameIsMod(cfg, site, list[i]))
	    {
		New = ssa_init_node(cfg, cfgParent, ssaParent,
				    AST_NIL, SSA_IP_MOD, list[i]);	
	    }
	    /*
	     *  Have to add IP_REF second so that it's earlier in the
	     *  push-down list of SsaNodes under this cfgParent.
	     */
	    if (idfaNameIsRef(cfg, site, list[i]))
	    {
		New = ssa_init_node(cfg, cfgParent, ssaParent,
				    AST_NIL, SSA_IP_REF, list[i]);
	    }
	}
	break;

      case SSA_IP_IN:
	for (i = 0; i < size; i++)
	{
	    New = ssa_init_node(cfg, cfgParent, ssaParent, AST_NIL,
				type, list[i]);
	}
	break;

      case SSA_USE: /* at cfg->end */
	for (i = 0; i < size; i++)
	{
	    if (!idfaNameIsMod(cfg, /* site = */ AST_NIL, list[i])) continue;

	    New = ssa_init_node(cfg, cfgParent, ssaParent, AST_NIL,
				type, list[i]);
	}
	break;
    }
}

static int nameIsActual(CfgInstance cfg, fst_index_t var, AST_INDEX invoc_node)
{
  AST_INDEX tmp_node;
  int isActual=0;
  tmp_node = gen_INVOCATION_get_actual_arg_LIST(invoc_node);
  tmp_node = list_first(tmp_node);
  
  while(!isActual && tmp_node != ast_null_node){
    if(SsaGetSym(cfg,tmp_node) == var)
      isActual = 1;
    tmp_node = list_next(tmp_node);
  }

  return isActual;
}
