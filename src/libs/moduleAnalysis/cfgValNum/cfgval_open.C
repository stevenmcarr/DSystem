/* $Id: cfgval_open.C,v 1.10 1997/03/11 14:35:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <assert.h>

#include <libs/moduleAnalysis/cfgValNum/cfgval.i>
#include <libs/moduleAnalysis/cfg/tarjan.h>
#include <stdio.h>
#include <libs/moduleAnalysis/ssa/ssa_private.h>
#include <libs/support/arrays/ExtendingArray.h>
#include <libs/moduleAnalysis/valNum/val_ip.h>
#include <libs/moduleAnalysis/valNum/val_pass.h>
#include <libs/ipAnalysis/problems/symbolic/SymConstraints.h>

#include <libs/fileAttrMgmt/attributedFile/AttributedFile.h>
#include <libs/support/file/FormattedFile.h>

/*
 *  These are the global variables for certain saved value numbers;
 *  by virtue of being created first and in a certain order for all
 *  cfg/ssa instances, they should be the same for all.
 */
//  Now enums in val.h
//
//  ValNumber VAL_BOTTOM, VAL_TOP, VAL_ZERO, VAL_ONE,
//     VAL_M_ONE, VAL_TRUE, VAL_FALSE;

void cfgval_Open(CfgInfo cfgGlobals, Boolean useIpVals)
{
    cfgGlobals->valParms = (Generic)get_mem(sizeof(Boolean), "value numbering parms");

    *((Boolean *)(cfgGlobals->valParms)) = useIpVals;

    /*
     *  Note that we are using the valParms pointer both as a pointer
     *  to the parameters and as a handle to confirm which client
     *  of the cfg we're registering.
     */
    cfg_Register(cfgGlobals, cfgGlobals->valParms,
                 cfgval_OpenOne, cfgval_CloseOne, cfgval_Dump,
                 NULL, NULL, NULL,
                 NULL, NULL, NULL);
}

void cfgval_Close(CfgInfo cfgGlobals)
{
    CfgInstance cfg;

    for (cfg = cfgGlobals->firstInst;
         cfg;
         cfg = cfg->next)
    {
        cfgval_CloseOne(cfgGlobals->valParms, cfg);
    }

    cfg_Deregister(cfgGlobals, cfgGlobals->valParms);

    free_mem((void*)cfgGlobals->valParms);
}

extern char *D_sym_level;

void cfgval_OpenOne(Generic junk, CfgInstance cfg)
{
    int symLevel;
    sscanf(D_sym_level, "%d", &symLevel);
    int ssaSize, i;

    if (!SSA_nodes(cfg)) return;

    tarj_sort(cfg);

    /* VAL_matched(cfg) = 0; */ /* metric of amount of sharing */

    if (SSA_ipInfo(cfg) && (symLevel > 0))
    {
#ifdef  ELIM_LINK_HACK
	//  paco added this Oct 1993
	//
	ValIP *vipp = SymGetEntryCfgVals(SSA_ipInfo(cfg),
					 cfg_get_inst_name(cfg));
	V(cfg) = vipp->values;
	CFGVAL_stuff(cfg)->entryVals = vipp->pass;
	vipp->values = NULL;	// forestall destruction
	vipp->pass   = NULL;	//	on destruction of vipp
#else
	// commented out above to get minimal ped to link
	assert(0);
#endif
    }
    else
    {
	//  paco changed this 2/28/93
	//
	V(cfg) = val_Open();
	CFGVAL_stuff(cfg)->entryVals = NULL;
    }
    ValIP *vip = &(CFGVAL_stuff(cfg)->vip);
    vip->pass = NULL;

    CFGVAL_stuff(cfg)->passNodes = NULL;

    //  This initialization violates abstraction, in the interests
    //  of not missing any SSA nodes.
    //
    ssaSize = area_size(SSA_nodes(cfg));
    for (i = 0; i < ssaSize; i++) 
    {
	SSA_node(cfg, i)->value = VAL_NIL;
    }
}

void cfgval_CloseOne(Generic junk, CfgInstance cfg)
{
    if ((cfg->type == CFG_GLOBAL) ||
        (cfg->type == CFG_BLOCK_DATA))
        return;

    ValIP *vip = &(CFGVAL_stuff(cfg)->vip);

    if (vip->pass) delete vip->pass;
    if (CFGVAL_stuff(cfg)->passNodes) delete CFGVAL_stuff(cfg)->passNodes;
    val_Close(V(cfg));
}


/*---------------------------------------------------------------------------
 * -- cfgval_Dump
 *
 *             a debugging to show the contents of the value table 
 *--------------------------------------------------------------------------*/
void cfgval_Dump(Generic junk, CfgInstance cfg)
{
    val_Dump(V(cfg));
}

//
//  Saving to the database -- e.g. for interprocedural analysis
//
void cfgval_Save(CfgInstance cfg, Context m_context)
{
    File *tsf =  m_context->CreateExternalAttributeFile("values.initial");
    FormattedFile port(tsf);

    if (!(VIP(cfg)->pass)) VIP(cfg)->pass = cfgval_build_passmap(cfg);

    VIP(cfg)->Write(port);

    port.Close();
}

void cfgval_Restore(CfgInstance cfg, Context m_context)
{
    File *tsf = m_context->GetExternalAttributeFile("values.initial", 1);
    FormattedFile port(tsf);

    VIP(cfg)->Read(port);

    port.Close();
}
