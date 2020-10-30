/* $Id: util_dg.C,v 1.1 1997/06/25 15:06:11 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/*	----------------- I N C L U D E     F I L E S  ------------	*/

#include <stdio.h>
#include <stdlib.h>

#include <libs/moduleAnalysis/dependence/dependenceGraph/private_dg.h>

#include <libs/support/memMgmt/mem.h>
#include <libs/support/strings/rn_string.h>

#include <include/bstring.h>

#include <libs/frontEnd/ast/ast.h>
#include <include/frontEnd/astnode.h>
#include <include/frontEnd/astrec.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/include/walk.h>

#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTree/fortsym.h>

#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dep_dg.h>

#include <libs/Memoria/include/mem_util.h>

/*	------------------ L O C A L    S T R U C T U R E S --------	*/

struct deps_parm
{
	DG_Instance	* dg;		/* handle for dependence graph	*/
	SideInfo	* infoPtr;	/* handle to side-array		*/
	int level;              /* level of loop carrying dependence    */
	Carried_deps *deps;     /* structure to store dependences found */
	DG_Edge *Earray;		/* array of all DG edges	*/
};

/*	This struct is for use of the print_deps routing during walks
	along the AST
*/
typedef struct	print_params_struct
{
  DG_Instance	*dg;
  SideInfo	*infoPtr;
  FortTree      ft;
  SymDescriptor symtab;     
} print_params;


STATIC(int, print_deps,(AST_INDEX id, print_params *params));
STATIC(int, carried_deps,(AST_INDEX stmt, int level, struct deps_parm *parm));

/*---------------------------------------------------------------------
 
  dg_print_deps()   Prints all dependences in program on REF lists

  Uses print_deps() as helper function for walk_expressions().

*/
int
dg_print_deps(AST_INDEX root, FortTree ft,DG_Instance *dg, SideInfo *infoPtr)
{
  print_params	 params;

  params.dg	= dg;
  params.infoPtr= infoPtr;
  params.ft = ft;


  walk_expression( root, (WK_EXPR_CLBACK)print_deps, NULL, (Generic )&params);
  
  return WALK_CONTINUE;
}

/*---------------------------------------------------------------------
 
  print_deps()   Helper function for dg_print_deps()

*/

static int
print_deps(AST_INDEX id, print_params *params)
{
  int	ref;
  AST_INDEX	 id2;
  EDGE_INDEX	 edge;
  DG_Edge	*Earray;      /* array of all DG edges    */
  DG_Instance	*dg	= params->dg;
  SideInfo	*infoPtr= params->infoPtr;


  if (is_program(id))
	params->symtab = ft_SymGetTable(params->ft,gen_get_text(gen_PROGRAM_get_name(id)));
  else if (is_subroutine(id))
	params->symtab=ft_SymGetTable(params->ft,gen_get_text(gen_SUBROUTINE_get_name(id)));
  else if (is_function(id))
	params->symtab = ft_SymGetTable(params->ft,gen_get_text(gen_FUNCTION_get_name(id)));

  /* if no dependences, then create RSD at outermost loop */
  if (!is_identifier(id))
    return WALK_CONTINUE;


  if ((ref = dg_get_info(infoPtr, id, type_levelv)) == NO_LEVELV)
    return WALK_CONTINUE;

  Earray = dg_get_edge_structure( dg);

  /* look at all dependences edges with id as src */
  if (dg_first_src_ref(dg,ref) >= 0 || dg_first_sink_ref( dg, ref) >= 0)
        if (is_subscript(tree_out(id))) {
                char text[80];
                ut_GetSubscriptText(tree_out(id),text);
                printf("Array Reference %s :\n", text);
        }
        else
             printf("Scalar Reference %s :\n", gen_get_text(id));
  else
          return (WALK_CONTINUE);

  for (edge = dg_first_src_ref( dg, ref);
       edge >= 0;
       edge = dg_next_src_ref( dg, edge))
  {
    id2 = Earray[edge].sink;

    switch (Earray[edge].type)
    {
      case dg_true:
        if (is_subscript(tree_out(id2))) {
                char subscript[80];
                ut_GetSubscriptText(tree_out(id2),subscript);
                printf("  True dep to %s, level = %d, dvec = %s\n",
                        subscript, Earray[edge].level,Earray[edge].dt_str);
        } else
                printf("  True dep to %s, level = %d, dvec = %s\n",
                        gen_get_text(id2), Earray[edge].level,Earray[edge].dt_str);
        break;

      case dg_anti:
        if (is_subscript(tree_out(id2))) {
                char subscript[80];
                ut_GetSubscriptText(tree_out(id2),subscript);
                printf("  Anti dep to %s, level = %d, dvec = %s\n",
                        subscript, Earray[edge].level,Earray[edge].dt_str);
        } else
                printf("  Anti dep to %s, level = %d, dvec = %s\n",
                        gen_get_text(id2), Earray[edge].level,Earray[edge].dt_str);
        break;

      case dg_output:
        if (is_subscript(tree_out(id2))) {
                char subscript[80];
                ut_GetSubscriptText(tree_out(id2),subscript);
                printf("  Output dep to %s, level = %d, dvec = %s\n",
                        subscript, Earray[edge].level,Earray[edge].dt_str);
        } else
                printf("  Output dep to %s, level = %d, dvec = %s\n",
                        gen_get_text(id2), Earray[edge].level,Earray[edge].dt_str);
        break;

    }
  }

  /* look at all dependences edges with id as sink */

  for (edge = dg_first_sink_ref( dg, ref);
       edge >= 0;
       edge = dg_next_sink_ref( dg, edge))
  {
    id2 = Earray[edge].src;

    switch (Earray[edge].type)
    {
      case dg_true:
        if (is_subscript(tree_out(id2))) {
                char subscript[80];
                ut_GetSubscriptText(tree_out(id2),subscript);
                printf("  True dep from %s, level = %d, dvec = %s\n",
                        subscript, Earray[edge].level,Earray[edge].dt_str);
        } else
                printf("  True dep from %s, level = %d, dvec = %s\n",
                        gen_get_text(id2), Earray[edge].level,Earray[edge].dt_str);
        break;

      case dg_anti:
        if (is_subscript(tree_out(id2))) {
                char subscript[80];
                ut_GetSubscriptText(tree_out(id2),subscript);
                printf("  Anti dep from %s, level = %d, dvec = %s\n",
                        subscript, Earray[edge].level,Earray[edge].dt_str);
        } else
                printf("  Anti dep from %s, level = %d, dvec = %s\n",
                        gen_get_text(id2), Earray[edge].level,Earray[edge].dt_str);
        break;

      case dg_output:
        if (is_subscript(tree_out(id2))) {
                char subscript[80];
                ut_GetSubscriptText(tree_out(id2),subscript);
                printf("  Output dep from %s, level = %d, dvec = %s\n",
                        subscript, Earray[edge].level,Earray[edge].dt_str);
        } else
                printf("  Output dep from %s, level = %d, dvec = %s\n",
                        gen_get_text(id2), Earray[edge].level,Earray[edge].dt_str);
        break;

    }
  }

  return WALK_CONTINUE;
}

/*---------------------------------------------------------------------
 
  dg_carried_deps()   Returns all dependences carried by loop

*/

Carried_deps *
dg_carried_deps(DG_Instance *dg, SideInfo *infoPtr, AST_INDEX loop)
{
  struct deps_parm parm;

  parm.dg	= dg;
  parm.infoPtr	= infoPtr;
  parm.level	= loop_level(loop);
  parm.Earray	= dg_get_edge_structure(dg);
  parm.deps	= (Carried_deps *) get_mem(sizeof(Carried_deps),"dg_carried_deps");
  bzero((char*)parm.deps, sizeof(Carried_deps));

  walk_statements(loop, LEVEL1, (WK_STMT_CLBACK)carried_deps, NULL, (Generic)&parm);

  parm.deps->all_num = parm.deps->true_num + 
                       parm.deps->anti_num + parm.deps->out_num;

  return parm.deps;
}


/*---------------------------------------------------------------------
 
  carried_deps()   Helper function for dg_carried_deps()

*/

static int
carried_deps(AST_INDEX stmt, int level, struct deps_parm *parm)
{
  int	vector;
  int	edge_idx;
  DG_Edge	*Edge;
  DG_Instance	*dg	= parm->dg;
  SideInfo	*infoPtr= parm->infoPtr;

  if ((vector = dg_get_info( infoPtr, stmt, type_levelv)) == UNUSED)
    return WALK_CONTINUE;

  edge_idx = dg_first_src_stmt( dg, vector, parm->level);
  while (edge_idx != UNUSED)
  {
    Edge = parm->Earray + edge_idx;
    
    switch (Edge->type)
    {
    case dg_true:
      parm->deps->true_deps[parm->deps->true_num++] = Edge;
      break;

    case dg_anti:
      parm->deps->anti_deps[parm->deps->anti_num++] = Edge;
      break;

    case dg_output:
      parm->deps->out_deps[parm->deps->out_num++] = Edge;
      break;
    }

    edge_idx = dg_next_src_stmt( dg, edge_idx);
  }

  return WALK_CONTINUE;
}


