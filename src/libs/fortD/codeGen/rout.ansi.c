/* $Id: rout.ansi.c,v 1.16 1997/03/11 14:28:19 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/



/*----------------------------------------------------------------

    rout.c      Utility routines for the Fortran D compiler

*/


#include <libs/fortD/codeGen/private_dc.h>

/********************************************************************/
/* like findadd(), but takes AST of subscripted ref instead of name */
/********************************************************************/
SNODE *
findadd2(AST_INDEX node, int flag, int level, Dist_Globals* dh)
{
  SNODE *sp, *sp2;

  if (is_subscript(node))
    node = gen_SUBSCRIPT_get_name(node);

  if (!is_identifier(node))
  {
    printf("findadd2(): node not identifier\n");
    return NULL;
  }

  if (!dh->in_ped)
  {
    sp = (SNODE *) get_info(dh->ped, node, type_fd);
    if (sp != (SNODE *) NO_DC_INFO) 
      return sp;
  }


  sp = findadd(gen_get_text(node), flag, level, dh->ihash);

  sp2 = (SNODE *) get_info(dh->ped, node, type_fd);
     
    if (sp2 == (SNODE *) NO_DC_INFO) {
      put_info(dh->ped, node, type_fd, (Generic)sp);
 		 }

  return sp;
}


/**************************************************************/
DIST_INFO *
get_dist_info(AST_INDEX name_id, int dim, Dist_Globals* dh)
{
  SNODE *sp;
  char *name;

  name = gen_get_text(name_id);
  sp = findadd(name, 0, 0, dh->ihash);

  if (!sp)
    die_with_message("syntax error %s not declared \n", name);

  return (sp->distinfo[dim]);
}

/**************************************************************/
/* returns the factorial of a positive number                 */
/**************************************************************/
int
fact(int n)
{
  int factval, i;

  if ((n == 1) || !n)
    return (1);

  factval = 1;
  for (i = 1; i <= n; ++i)
    factval = factval * i;

  return (factval);
}


/**************************************************************/
int
dc_rsd_upper(Rsd_section* rs, int dim)
{
  if (rs->subs[dim].type == RSD_RANGE)
    return (rs->subs[dim].up_b);
  else if (rs->subs[dim].type == RSD_CONSTANT)
    return (rs->subs[dim].constant);
  printf("dc_rsd_upper(): symbolic bound\n");
  return 0;
}

/**************************************************************/
int
dc_rsd_lower(Rsd_section* rs, int dim)
{
  if (rs->subs[dim].type == RSD_RANGE)
    return (rs->subs[dim].lo_b);
  else if (rs->subs[dim].type == RSD_CONSTANT)
    return (rs->subs[dim].constant);
  printf("dc_rsd_lower(): symbolic bound\n");
  return 0;
}


/*******************************************************************/
/* This routine checks to see if it can find an instance of the    */
/* string searchingfor in the AST node.  If it can, it returns true*/
/*******************************************************************/
Boolean
is_match_subscript2(AST_INDEX node, char* searchingfor)

{
  AST_INDEX term1;
  AST_INDEX term2;
  char *name;

  /*---------------------------------------------------*/
  /* constants */

  if (is_constant(node))
  {
    return (false);
  }

  else if (is_identifier(node))
  {
    name = gen_get_text(node);
    return NOT(strcmp(name, searchingfor));

  }

  /*---------------------------------------------------*/
  /* plus or minus */

  else if (is_binary_plus(node))
  {
    term1 = gen_BINARY_PLUS_get_rvalue1(node);
    term2 = gen_BINARY_PLUS_get_rvalue2(node);
    return BOOL(is_match_subscript2(term2, searchingfor) ||
                is_match_subscript2(term1, searchingfor));
  }

  /*---------------------------------------------------*/

  else if (is_binary_minus(node))
  {
    term1 = gen_BINARY_MINUS_get_rvalue1(node);
    term2 = gen_BINARY_MINUS_get_rvalue2(node);
    return BOOL(is_match_subscript2(term2, searchingfor) ||
                is_match_subscript2(term1, searchingfor));

  }

  /*---------------------------------------------------*/
  /* multiply */

  else if (is_binary_times(node))
  {
    term1 = gen_BINARY_TIMES_get_rvalue1(node);
    term2 = gen_BINARY_TIMES_get_rvalue2(node);

    return BOOL(is_match_subscript2(term2, searchingfor) ||
                is_match_subscript2(term1, searchingfor));

  }

  /*---------------------------------------------------*/
  else
    return true;

/*  return false;        found linear subscript */
}


/**********************************************************/
/* determine the type of RSD section              */
/**********************************************************/
enum RANGETYPE
dc_range_type(int l2, int u2)
{
  if ((l2 <= 0) && (u2 <= 0))
    return NEG_NEG;

  if ((l2 > 0) && (u2 > 0))
    return POS_POS;

  return NEG_POS;
}
