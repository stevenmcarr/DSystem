/* $Id: idfa.C,v 1.1 1997/06/25 15:10:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 *
 * -- idfa.new.c
 *
 *            The interface routines to the interprocedural data flow 
 *            information kept in the database.  (After all, this is 
 *            part of the Rn programming environment.)
 *
 */

#include <libs/moduleAnalysis/ssa/ssa_private.h>
#include <libs/ipAnalysis/problems/modRef/ScalarModRefQuery.h>
#include <libs/moduleAnalysis/ssa/idfa.h>

/*
 *  get_var_info
 *      Get data used in tagging references to interprocedural variables
 */
static void get_var_info(CfgInstance cfg, fst_index_t index, fst_index_t* name, 
                         int* offset, int* length, int* expType)
{
    *name = (fst_index_t) fst_GetFieldByIndex(cfg->symtab,
					      index, SYMTAB_PARENT);
    if (*name == SSA_NIL_NAME) *name = index;
    *offset = fst_GetFieldByIndex(cfg->symtab, index, SYMTAB_EQ_OFFSET);
    *length = fst_GetFieldByIndex(cfg->symtab, index, SYMTAB_SIZE);
    *expType = fst_GetFieldByIndex(cfg->symtab, index, SYMTAB_TYPE);
}

Boolean idfaInit(CfgInstance cfg)
{
    if (SSA_ipInfo(cfg))
	return true;
    else
	return false;
}

void idfaFini(CfgInstance cfg)
{
    return;
}

Boolean idfaIsPure(CfgInstance cfg, AST_INDEX site)
{
    char *name;
    fst_index_t id;

    if (is_null_node(site)) return false;

    name = gen_get_text(gen_INVOCATION_get_name(site));
    id = fst_QueryIndex(cfg->symtab, name);

    if (FS_IS_INTRINSIC_OR_GENERIC(cfg->symtab, id))
	return true;
    else if (!SSA_ipInfo(cfg))
	return false;
    /* 
     *  else do nothing still for now
     */
    return false;
}

Boolean idfaNameIsMod(CfgInstance  cfg, AST_INDEX site, fst_index_t id )
{
    if (idfaIsPure(cfg, site))
    {
	return false;
    }
    else if (!SSA_ipInfo(cfg))
    {
	return true;
    }
    else
    {
	fst_index_t name;
	int offset, length, expType;

	get_var_info(cfg, id, &name, &offset, &length, &expType);
	
	if (is_null_node(site)) /* no, it's a program entry */
	{
	    return IPQuery_IsScalarModNode((Generic)SSA_ipInfo(cfg),
					   cfg_get_inst_name(cfg),
					   SsaSymText(cfg, name), 
					   offset, length);
	}
	else
	{
	    return IPQuery_IsScalarMod((Generic)SSA_ipInfo(cfg),
				       cfg_get_inst_name(cfg),
				       ft_NodeToNumber(cfg->cfgGlobals->ft,
						       site),
				       SsaSymText(cfg, name), offset, length);
	}
    }
}

Boolean idfaNameIsRef(CfgInstance  cfg, AST_INDEX site, fst_index_t id )
{
    /* 
     *  This is too conservative for a pure function -- they only
     *  reference their explicit arguments -- but the only alternative
     *  is searching through the arguments for the name.
     */
    if (!SSA_ipInfo(cfg))
    {
	return true;
    }
    else
    {
	fst_index_t name;
	int offset, length, expType;

	get_var_info(cfg, id, &name, &offset, &length, &expType);
	
	if (is_null_node(site)) /* no, it's a program entry */
	{
	    return IPQuery_IsScalarRefNode((Generic)SSA_ipInfo(cfg),
					   cfg_get_inst_name(cfg),
					   SsaSymText(cfg, name), 
					   offset, length);
	}
	else 
	{
	    return IPQuery_IsScalarRef((Generic)SSA_ipInfo(cfg),
				       cfg_get_inst_name(cfg),
				       ft_NodeToNumber(cfg->cfgGlobals->ft, 
						       site),
				       SsaSymText(cfg, name), offset, length);
	}
    }
}

Boolean idfaNameIsAcc(CfgInstance cfg, AST_INDEX site, fst_index_t id)
{
    return BOOL(idfaNameIsRef(cfg, site, id) || idfaNameIsMod(cfg, site, id));
}


Boolean idfaArgIsMod(CfgInstance cfg, AST_INDEX site, AST_INDEX arg)
{
    if (idfaIsPure(cfg, site))
    {
	return false;
    }
    else if (!SSA_ipInfo(cfg))
    {
	return true;
    }
    else
    {
	return IPQuery_IsScalarModArg((Generic)SSA_ipInfo(cfg),
				      cfg_get_inst_name(cfg),
				      ft_NodeToNumber(cfg->cfgGlobals->ft,
						      site),
				      list_element(arg));
    }
}

Boolean idfaArgIsRef(CfgInstance cfg, AST_INDEX site, AST_INDEX arg)
{
    /*
     *  Can do better than this when "pure" means no MODs
     *  (but some arguments may be ignored).
     */
    if (!SSA_ipInfo(cfg))
    {
	return true;
    }
    else
    {
	return IPQuery_IsScalarRefArg((Generic)SSA_ipInfo(cfg),
				      cfg_get_inst_name(cfg),
				      ft_NodeToNumber(cfg->cfgGlobals->ft, 
						      site),
				      list_element(arg));
    }
}

Boolean idfaArgIsAcc(CfgInstance cfg, AST_INDEX site, AST_INDEX arg)
{
    return BOOL(idfaArgIsRef(cfg, site, arg) || idfaArgIsMod(cfg, site, arg));
}

Boolean idfaHiddenMods(CfgInstance  cfg, AST_INDEX site )
{
    return NOT(idfaIsPure(cfg, site));
}

Boolean idfaHiddenRefs(CfgInstance  cfg, AST_INDEX site )
{
    return NOT(idfaIsPure(cfg, site));
}

Boolean idfaHiddenIo(CfgInstance  cfg, AST_INDEX site )
{
    return NOT(idfaIsPure(cfg, site));
}

Boolean idfaNameIsConst(CfgInstance cfg, AST_INDEX site, fst_index_t id)
{
    return false;
}

int idfaNameGetConst(CfgInstance cfg, AST_INDEX site, fst_index_t id)
{
    assert(false);
    return 0;
}
