/* $Id: constraints.C,v 1.4 1997/03/11 14:35:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/moduleAnalysis/cfgValNum/cfgval.i>
#include <libs/frontEnd/fortTree/FortTree.h>

static int constants, idents, offsets, linears;

typedef struct passvalinfostruct 
{
    Boolean	isActual;	/* false => common block variable */
    int		parmNo;		/* parameter index for actual */
    fst_index_t	globalVar;	/* symtable index for common block var */
    ValNumber	base;		/* linear basis for passed value */
    int		coeff, add;	/* passed value == coeff*base + add */
} PassValInfo;

static void init_pvi(void *id)
{
    PassValInfo *elem = (PassValInfo *) id;
    elem->isActual	= true;
    elem->parmNo	= VAL_NIL;	/* -1 */
    elem->globalVar	= SSA_NIL_NAME;
    elem->base		= VAL_BOTTOM;
    elem->coeff		= 1;
    elem->add		= 0;
}

static void print_info(CfgInstance cfg, PassValInfo *info)
{
    if (info->isActual)
    {
	printf("(#%d, ", info->parmNo);
    }
    else /* common or formal */
    {
	printf("(%s, ", SsaSymText(cfg, info->globalVar));
    }

    if (ve_type(VE(cfg, info->base)) == VAL_CONST)
    {
	printf("%d)", ve_const(VE(cfg, info->base)));
    }
    else /* relation */
    {
	printf("%d[%d] + %d)", info->coeff, info->base, info->add);
    }
}

static void check_info(CfgInstance cfg, PassValInfo **info, 
		       PassValInfo *temp, SsaNodeId parent)
{
    int i;
    char *place = NULL;
    int sz = f_curr_size((Generic) *info);

    if (temp->globalVar == DUMMY_GLOBAL(cfg))
    {
	return;
    }

    if (SSA_node(cfg, parent)->type == SSA_CALL)
    {
	place = SsaSymText(cfg,
		   SsaGetSym(cfg,
		      gen_INVOCATION_get_name(SSA_node(cfg,
						       parent)->refAst)));
    }
    /* else return point */

    if (ve_type(VE(cfg, temp->base)) == VAL_CONST)
    {
	constants++;

	if (place) /* call site */
	{
	    printf("\tConstant @ call to %s: ", place);
	}
	else /* return point */
	{
	    printf("\tConstant @ return point: ");
	}
	print_info(cfg, temp);
	printf("\n");
	return;
    }
    
    for (i = 0;
	 i < sz;
	 i++)
    {
	if (temp->base == (*info)[i].base)
	{
	    char *reltype;

	    if (temp->coeff != (*info)[i].coeff)
	    {
		linears++;
		reltype = "Linear";
	    }
	    else if (temp->add != (*info)[i].add)
	    {
		offsets++;
		reltype = "Offset";
	    }
	    else
	    {
		idents++;
		reltype = "Identity";
	    }
       
	    /*
	     *  Found related pair -- print out
	     */
	    if (SSA_node(cfg, parent)->type == SSA_CALL)
	    {
		printf("\t%s @ call to %s: ", reltype, place);
	    }
	    else /* return point */
	    {
		printf("\t%s @ return point: ", reltype);
	    }
	    print_info(cfg, &((*info)[i]));
	    print_info(cfg, temp);
	    printf("\n");
	    break;
	}
    }
    if (i == sz)
    {
	/* ran completely through without finding relation */
	/*
	 *  Add unrelated thing to list
	 */
	int nuevo = f_new((Generic *) info);
	(*info)[nuevo] = *temp;
    }
}

static void extract(CfgInstance cfg, ValNumber *valp, 
		    int *constp, ValOpType opType)
{
    ValEntry *v = &(VE(cfg, *valp));

    if ((ve_type(*v) == VAL_OP) &&
	(ve_opType(*v) == opType))
    {
	ValNumber left, right;
	left = ve_left(*v);
	right = ve_right(*v);

	/*
	 *  constants now guaranteed to be on left
	 */
	if (ve_type(VE(cfg,left)) == VAL_CONST)
	{
	    *constp = ve_const(VE(cfg,left));
	    *valp   = right;
	}
    }
}

static void parse(CfgInstance cfg, PassValInfo **info,
		  ValNumber val, SsaNodeId sn, 
		  Boolean isActual, int parmNo, fst_index_t globalVar)
{
    PassValInfo temp;

    init_pvi(&temp);

    temp.base = val;

    if ((temp.base == VAL_BOTTOM) ||
	(temp.base == VAL_TOP) ||
	(temp.base == VAL_NIL))
    {
	return;
    }
    temp.isActual  = isActual;
    temp.parmNo    = parmNo;
    temp.globalVar = globalVar;
		
    /*
     *  Extract constant additive part if possible
     */
    extract(cfg, &temp.base, &temp.add, VAL_OP_PLUS);
    /*
     *  Extract constant coefficient if possible
     */
    extract(cfg, &temp.base, &temp.coeff, VAL_OP_TIMES);

    check_info(cfg, info, &temp, sn);
}

void cfgval_constraints(CfgInstance cfg)
{
    SsaNodeId	sn;
    SsaNodeId	kid;
    SsaEdgeId	in;

    PassValInfo *info = (PassValInfo *) f_alloc(10, sizeof(PassValInfo),
						"testing constraints",
						init_pvi);
    constants = 0;
    idents    = 0;
    offsets   = 0;
    linears   = 0;

    /*
     *  Walk in topological Tarjan order (see ssa_util.c for details)
     */
    for (sn = ssa_get_first_node(cfg);
	 sn != SSA_NIL;
	 sn = ssa_get_next_node(cfg, sn))
    {
	if (SSA_node(cfg,sn)->type == SSA_CALL)
	{
	    /*
	     *  Go through the subordinate references -- 
	     *  	SSA_ACTUALs for actual parms and 
	     *  	SSA_USEs for globals 
	     *
	     *  -- looking for linearly related values.
	     */
	    int anum = 0;
	    f_reset((Generic) info);

	    for (kid = SSA_node(cfg, sn)->subUses;
		 kid != SSA_NIL;
		 kid = SSA_node(cfg, kid)->nextSsaSib)
	    {
		ValNumber val = cfgval_build(cfg, kid);

		if (SSA_node(cfg,kid)->type == SSA_ACTUAL)
		{
		    parse(cfg, &info, val, sn, true, 
			  list_element(SSA_node(cfg,kid)->refAst), 
			  SSA_NIL_NAME);
		}
		else /* global */
		{
		    parse(cfg, &info, val, sn, false, VAL_NIL,
			  SSA_node(cfg,kid)->name);
		}
	    }
	}
    }
    /*
     *  Look at SSA_USEs at cfg->end
     */
    f_reset((Generic) info);

    for (sn = ssa_first_cfg_kid(cfg, cfg->end);
	 sn != SSA_NIL;
	 sn = ssa_next_cfg_kid(cfg, sn))
    {
	if (SSA_node(cfg, sn)->type == SSA_USE)
	{
	    ValNumber val = cfgval_build(cfg, sn);
	    in = SSA_node(cfg, sn)->defsIn;

	    parse(cfg, &info, val, sn, false, VAL_NIL,
		  SSA_node(cfg, SSA_edge(cfg, in)->source)->name);
	}
    }
    if (constants)
	printf("\tconstants: %d", constants);
    if (idents)
	printf("\tidents: %d", idents);
    if (offsets)
	printf("\toffsets: %d", offsets);
    if (linears)
	printf("\tlinears: %d", linears);

    printf("\n");
}
