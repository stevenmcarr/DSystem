/* $Id: decomp.ansi.c,v 1.22 1997/03/11 14:28:16 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/



/*-----------------------------------------------------------------

	decomp.c	Fortran D data decomposition parse/store routines

	author : Seema Hiranandani                                    

	The functions in this file store all the information in the   
	symbol table including details of data decomposition specs    

    Fortran D statements are declared using comment directives.  
    For example:

           program foo
            real x(100,100)
            parameter (n$proc = 4)
    C       decomposition d(100,100)
    C       align x with d
    C       distribute d(block, :)
            ...code...
           end

*/


#include <libs/fortD/codeGen/private_dc.h>

/*------------------------- local structures -------------------------*/

#define DCSTR 32

struct dc_expr
{
   char str[DCSTR];
   int val;
   enum dc_expr_type type;
};

struct dc_subs
{
   int num;
   struct dc_expr expr[DCSTR];
};

struct dc_id
{
   char str[DCSTR];
   struct dc_subs subs;
   Boolean ident;
};

struct dc_list
{
   int num;
   struct dc_id id[DCSTR];
};

/*------------------------- extern definitions -------------------------*/

EXTERN(Boolean, ast_eval,(AST_INDEX node, int *iptr));
EXTERN(void, decomp_local_bounds,(SNODE *sp, int dim, int* dupper, int* dlower));
EXTERN(void, init_var_snode,(SNODE *sp, int dim, enum FORM type));

/*------------------------- global definitions -------------------------*/

void dc_analyze(Dist_Globals *dh);
Dist_type is_part(AST_INDEX name_id, int dim, Dist_Globals *dh);

/*------------------------- local definitions -------------------------*/

STATIC(int, store_all,(AST_INDEX stmt, int level, Dist_Globals *dh));
STATIC(int, store_decl, (AST_INDEX stmt, Dist_Globals *dh));
STATIC(void, store_sym_table,(AST_INDEX node, Dist_Globals *dh, enum FORM type));
STATIC(void, store_partition,(AST_INDEX stmt, Dist_Globals *dh));
STATIC(void, store_decomposition,(char *str, Dist_Globals *dh));
STATIC(void, store_alignment,(char *str, Dist_Globals *dh));
STATIC(void, store_distribution,(char *str, Dist_Globals *dh));
STATIC(void, store_param,(AST_INDEX param, Dist_Globals *dh));
/*STATIC(void, store_size,());*/
STATIC(void, store_distrib_pattern,(SNODE *sp, struct dc_subs *subs, Dist_Globals *dh));

STATIC(SNODE, *check_align_array,(struct dc_id *id, Dist_Globals *dh));
STATIC(void, check_align_decomp,(struct dc_id *id, struct dc_list *arrays, 
                                 Dist_Globals *dh));
STATIC(void, map_decomps,(SNODE *sp, Dist_Globals *dh));
STATIC(void, map_distarrays,(SNODE *array_sp, Dist_Globals *dh));
STATIC(void, convert_ph,(SUBSCR_INFO *subs, struct dc_expr *expr, int maxdim));

STATIC(char, *parse_expr,(char *str, struct dc_expr *expr));
STATIC(char, *parse_subs,(char *str, struct dc_subs *subs));
STATIC(char, *parse_id,(char *str, struct dc_id *id));
STATIC(char, *parse_list,(char *str, struct dc_list *list));


/****************************************************************/
/* analyze variables and data decomposition specifications      */
/****************************************************************/
/* Note: two phase approach won't work with dynamic data decomps */

void
dc_analyze(Dist_Globals *dh)
{
  int i;

  walk_statements(dh->root, LEVEL1, (WK_STMT_CLBACK)store_all, NULL, (Generic)dh);

  if (dh->in_ped)
  {
    /* calculate processor layout for each decomposition */
    for (i = 0; i < dh->numdecomps; i++)
      map_decomps(dh->decomps[i], dh);

    /* calculate local sections for each distributed array */
    for (i = 0; i < dh->numglobals; i++)
      map_distarrays(dh->dist_arrays[i], dh);
  }
}

/**************************************************************/
/* this routine returns the type of partition given           */
/* the astindex of the array variable  and the                */
/* dimension                                                  */
/**************************************************************/

Dist_type
is_part(AST_INDEX name_id, int dim, Dist_Globals *dh)
{
  SNODE *sp;
  char *name;

  name = gen_get_text(name_id);

  if (!(sp = findadd(name, 0, 0, dh->ihash)))
    die_with_message("array %s not declared", name);

  return sp_is_part(sp, dim);
}



/*******************************************************************/
/* this routine is called by walk statements. It calls the         */
/* appropriate routines to store declarations in the symbol table  */
/* and distribution information in the symbol table                */
/*******************************************************************/
static int
store_all(AST_INDEX stmt, int level, Dist_Globals *dh)      /* called by walk statements */
{
  switch (gen_get_node_type(stmt))
  {
    case GEN_COMMENT:
      if (dh->in_ped)
        store_partition(stmt, dh);
      break;

    case GEN_TYPE_STATEMENT:
      /* 10/8/93 RvH: Want to allow symbolics */
      if (dh->in_ped)
        store_decl(stmt, dh);
      break;

    case GEN_PARAMETER:
      if (dh->in_ped)
        store_param(stmt, dh);
      break;
  }

  return (WALK_CONTINUE);
}


/*******************************************************************/
/* this routine stores the declarations                            */
/*******************************************************************/
static int
store_decl(AST_INDEX stmt, Dist_Globals *dh)
{
  AST_INDEX tlen, curr, ttype;
  enum FORM type;

  tlen = gen_TYPE_STATEMENT_get_type_len(stmt);
  ttype = gen_TYPE_LEN_get_type(tlen);

  switch (gen_get_node_type(ttype))
  {

    case GEN_REAL:
      type = REAL;
      break;

    case GEN_INTEGER:
      type = INTTYPE;
      break;

    case GEN_CHARACTER:
      type = CHARTYPE;
      break;

    case GEN_DOUBLE_PRECISION:
      type = DOUBLE_P;
      break;

    case GEN_COMPLEX:
      type = COMPLEX;
      break;

    default:
      type = UNKNOWN;
      break;

  }

  curr = gen_TYPE_STATEMENT_get_array_decl_len_LIST(stmt);
  if (is_list(curr))
  {

    curr = list_first(curr);

    while (curr != AST_NIL)
    {
      store_sym_table(curr, dh, type);
      curr = list_next(curr);
    }
  }
  else if (curr != AST_NIL)
    store_sym_table(curr, dh, type);

}

/*********************************************************************/
/* store variables found in the declaration list in the symbol table */
/*********************************************************************/
static void
store_sym_table(AST_INDEX node, Dist_Globals *dh, enum FORM type)
{
  AST_INDEX curr, name_id, dimlist, upperb, lowerb;
  char *name;
  SNODE *sp;
  int i, upperbound, lowerbound;
  int numdim = 0;

  switch (gen_get_node_type(node))
  {
    case GEN_IDENTIFIER:
      name = gen_get_text(node);
      if (sp = findadd(name, 1, 0, dh->ihash))
        init_var_snode(sp, numdim, type);
      break;

    case GEN_ARRAY_DECL_LEN:
      name_id = gen_ARRAY_DECL_LEN_get_name(node);
      name = gen_get_text(name_id);
      dimlist = gen_ARRAY_DECL_LEN_get_dim_LIST(node);

      if (dimlist == AST_NIL)  /* is actually a scalar */
      {
        if (sp = findadd(name, 1, 0, dh->ihash))
          init_var_snode(sp, numdim, type);
        break;
      }

      numdim = list_length(dimlist);

      sp = findadd(name, 1, 0, dh->ihash);
      init_var_snode(sp, numdim, type);
      sp->numdim = numdim;

      curr = list_first(dimlist);

      for (i = 0; i < numdim; i++)
      {
        /*** if the upper bound of the array is a constant ***/
        /***       store its value in the symbol table     ***/

        upperb = gen_DIM_get_upper(curr);
        lowerb = gen_DIM_get_lower(curr);

        if (is_constant(upperb))
        {
          upperbound = atoi(gen_get_text(upperb));
          sp_put_upper(sp,i, upperbound);
        }
        else
        {
          die_with_message("non-constant size for array %s", name);
        }

        if ((lowerb != AST_NIL) && is_constant(lowerb))
        {
          lowerbound = atoi(gen_get_text(lowerb));
          sp_put_lower(sp, i, lowerbound);
        }
        if(lowerb == AST_NIL)
        {
          sp_put_lower(sp, i, 1);
        }
        curr = list_next(curr);
      }
      break;
  }
}


/*******************************************************************/
/* this routine examines parameter stmts to find n$proc            */
/*******************************************************************/
static void
store_param(AST_INDEX param, Dist_Globals *dh)
{
  AST_INDEX name_id, value_id;
  char *name;
  int value;

  param = gen_PARAMETER_get_param_elt_LIST(param);
  param = list_first(param);
  name_id = gen_PARAM_ELT_get_name(param);
  name = gen_get_text(name_id);

  /* look for assignment to n$proc (or nproc) */

  if (strcmp("n$proc", name) && strcmp("nproc", name) &&
      strcmp("N$PROC", name) && strcmp("NPROC", name))
    return;

  value_id = gen_PARAM_ELT_get_rvalue(param);
  value = 0;

  if (ast_eval(value_id, &value))
    die_with_message("illegal n$proc specification");

  /* store total # of processors in Dist_Globals */
  dh->numprocs = value;
}


/*----------------------------------------------------------------

  store_partition()  stores data partition information in the symbol table

*/

static void
store_partition(AST_INDEX stmt, Dist_Globals *dh)
{
  char *str;
  char buf[MAXCOMMENT];
  int i, len;

  /* get text of comment, convert it into all lower case */

  str = gen_get_text(gen_COMMENT_get_text(stmt));
  len = strlen(str);
  for (i = 0; i < len; i++)
    buf[i] = tolower(str[i]);
  buf[i] = '\0';

  /* check whether comment is a Fortran D specification */

  if (!strncmp("decomposition", buf, DECOMP_LEN))
  {
    store_decomposition(buf+DECOMP_LEN, dh);
  }
  else if (!strncmp("align", buf, ALIGN_LEN))
  {
    store_alignment(buf+ALIGN_LEN, dh);
  }
  else if (!strncmp("distribute", buf, DIST_LEN))
  {
    store_distribution(buf+DIST_LEN, dh);
  }
}


/*----------------------------------------------------------------

  store_decomposition()   Parse DECOMPOSITION statement

*/

static void
store_decomposition(char *str, Dist_Globals *dh)
{
  char *name;
  SNODE *sp;
  int i, j, numdim;
  struct dc_list list;

  while (*str == ' ')  /* eat spaces */
    str++;

  /* parse list of decompositions */

  str = parse_list(str, &list);
  if (*str)
    printf("store_decomposition(): Illegal list of decompositions\n");

  /* get decomposition name */

  for (j = 0; j < list.num; j++)
  {
    name = list.id[j].str;

    if (list.id[j].ident)
      die_with_message("DECOMPOSITION decl missing size");

    if (!(sp = findadd(name, 1, 0, dh->ihash)))
      die_with_message("name of DECOMPOSITION is not unique");

    numdim = list.id[j].subs.num;
    init_var_snode(sp, numdim, DECOMPTYPE);
    for (i = 0; i < numdim; i++)
    {
      if (list.id[j].subs.expr[i].type == value)
      {
      expr_lower(get_expr_lo(sp,i), Expr_constant, 1);
      expr_upper(get_expr_up(sp,i), Expr_constant,list.id[j].subs.expr[i].val);
      }
      else if (list.id[j].subs.expr[i].type == variable)
      {
        expr_lower(get_expr_lo(sp,i), Expr_constant, 1);
        expr_upper(get_expr_up(sp,i), Expr_simple_sym, 100,
                   list.id[j].subs.expr[i].val);
      }
      else 
      { 
        expr_lower(get_expr_lo(sp,i), Expr_complex, 1);
        expr_upper(get_expr_up(sp,i), Expr_complex, 100);
      }
    }

    /* store decomposition in Dist_Globals */
    dh->decomps[dh->numdecomps++] = sp;
  }
}


/*----------------------------------------------------------------

  store_alignment()   Parse ALIGN statement

*/

static void
store_alignment(char *str, Dist_Globals *dh)
{
  struct dc_list arrays;
  struct dc_id wth;
  struct dc_id decomp;
  int j;                 /* number of arrays to align */

  while (*str == ' ')  /* eat spaces */
    str++;

  str = parse_list(str, &arrays);  /* parse ALIGN statement */
  str = parse_id(str, &wth);
  str = parse_id(str, &decomp);

  while (*str == ' ')  /* eat spaces */
    str++;

  if (*str)
    printf("store_alignment(): Illegal ALIGN syntax\n");

  for (j = 0; j < arrays.num; j++)
    check_align_array(arrays.id+j, dh);

  if (NOT(wth.ident) || strcmp("with", wth.str))
    printf("store_alignment(): Illegal ALIGN syntax\n");

  check_align_decomp(&decomp, &arrays, dh);
}


/*----------------------------------------------------------------

  check_align_array()   check & process array in ALIGN statement

*/

static SNODE *
check_align_array(struct dc_id *id, Dist_Globals *dh)
{
  int i, numdim;
  char *name;
  SNODE *sp;
  char abuf[2];
  Boolean perfect_align = false;

  /*-------------------------------------------------------------*/
  /* check first alignment argument, must be global array        */
  /* if it is simply an identifier then it is perfectly aligned  */
  /* with respect to a decomposition                             */

  name = id->str;
  if (id->ident)
    perfect_align = true;

  /*--------------------------------------------------------*/
  /* look up in symbol table */

  if (!(sp = findadd(name, 0, 0, dh->ihash)))
    die_with_message("array %s not declared", name);

  if ((sp->fform1 != ARRAYTYPE) || (sp->fform1 == DECOMPTYPE))
    die_with_message("trying to ALIGN non-array %s", name);

  if (sp->decomp)
    die_with_message("realignment of arrays not currently supported");

  if (perfect_align)
    sp->perfect_align = true;

  /*--------------------------------------------------------*/
  /* check placeholders for array, must be of form A(i,j,k,...) */

  else
  {
    numdim = id->subs.num;

    if (numdim != sp->numdim)
      die_with_message("wrong # of dims for array in ALIGN");

    abuf[1] = '\0';
    for (i = 0; i < numdim; i++)
    {
      abuf[0] = 'i' + i;

      if (strcmp(abuf, id->subs.expr[i].str))
        die_with_message("wrong placeholder for ALIGN");
    }
  }

  /* store the symbol table address of the global array in Dist_Globals */
  dh->dist_arrays[dh->numglobals++] = sp;

  return sp;
}


/*----------------------------------------------------------------

  check_align_decomp()   check & process decomposition in ALIGN statement

*/

static void
check_align_decomp(struct dc_id *id, struct dc_list *arrays, Dist_Globals *dh)
{
  int i, j, size, numdim;
  char *name;
  SNODE *sp, *array_sp;

  name = id->str;
    size = sizeof(SUBSCR_INFO);

  /*--------------------------------------------------------*/
  /* look up in symbol table */

  if (!(sp = findadd(name, 0, 0, dh->ihash)) || (sp->fform1 != DECOMPTYPE))
    die_with_message("Trying to ALIGN with non-decomposition");

  numdim = sp->numdim;           /* # of dims in decomp */

  for (j = 0; j < arrays->num; j++)
  {
    array_sp = findadd(arrays->id[j].str, 0, 0, dh->ihash);
    array_sp->decomp = sp;

    /*--------------------------------------------------------*/
    /* check placeholders for decompositions */

    if (NOT(id->ident))
    {
      if (numdim != id->subs.num)
        die_with_message("check_align_decomp(): illegal ALIGN syntax");

      /* find dimensional alignment of decomp with respect to array */
      for (i = 0; i < numdim; i++)
      {
        convert_ph(array_sp->align[i], id->subs.expr + i, array_sp->numdim);
      }
    }

    /*--------------------------------------------------------------*/
    /* no placeholders specified, use default alignment if possible */

    else
    {
      if (numdim != array_sp->numdim)
        die_with_message("default ALIGN illegal, different dimensional size");

      /* specify default alignment of decomp to/with array   */
      for (i = 0; i < numdim; i++)
      {
        sp_set_align_info(array_sp, i, i+1, 0, 1, ALIGN_PERFECT);
      }
    }
  }
}


/*----------------------------------------------------------------

  convert_ph()   convert placeholder into subscript info

*/

static void
convert_ph(SUBSCR_INFO *subs, struct dc_expr *expr, int maxdim)
{
  subs->coeff = 1;              /* defaults */
  subs->offset = 0;
  subs->index = 0;
  subs->stype = ALIGN_UNKNOWN;

  switch (expr->type)
  {
    case value:
      subs->offset = expr->val;
      subs->stype = ALIGN_CONST;
      return;

    case variable:
      subs->stype = ALIGN_PERFECT;
      break;

    case plus:
      subs->offset = expr->val;
      subs->stype = ALIGN_OFFSET;
      break;

    case unknown:
      die_with_message("illegal placeholder in ALIGN");
  }

  /*----------------------------------------*/
  /* finally check for depth of placeholder */

  subs->index = expr->str[0] - 'i' + 1;

  if ((subs->index > maxdim) || (expr->str[1] != '\0'))
    die_with_message("illegal ALIGN placeholder");
}


/*----------------------------------------------------------------

  store_distribution()   parse DISTRIBUTION statement

*/

static void
store_distribution(char *str, Dist_Globals *dh)
{
  struct dc_list decomp;
  int j;                 /* number of arrays to align */
  char *name;
  SNODE *sp;

  while (*str == ' ')  /* eat spaces */
    str++;

  str = parse_list(str, &decomp);  /* parse DISTRIBUTE statement */

  if (*str)
    printf("store_distribution(): Illegal DISTRIBUTE syntax\n");

  for (j = 0; j < decomp.num; j++)
  {
    name = decomp.id[j].str;
    if (!(sp = findadd(name, 0, 0, dh->ihash)))
      die_with_message("DECOMPOSITION %s not declared", name);

    if (sp->fform1 != DECOMPTYPE)
      die_with_message("Trying to DISTRIBUTE non-decomposition");

    if (decomp.id[j].ident || (decomp.id[j].subs.num != sp->numdim))
      die_with_message("store_distribution(): Illegal DISTRIBUTE syntax");

    store_distrib_pattern(sp, &decomp.id[j].subs, dh);
  }
}


/*----------------------------------------------------------------

  store_distrib_pattern()  stores the distribution pattern for decomp

*/

static void
store_distrib_pattern(SNODE *sp, struct dc_subs *subs, Dist_Globals *dh)
{
  int numdim, i;
  char *name;

  numdim = subs->num;
  if (numdim != sp->numdim)
    die_with_message("store_distrib_pattern(): Illegal # of attributes");

  for (i = 0; i < numdim; i++)
  {
    if (subs->expr[i].type != variable)
    {
      printf("store_distrib_pattern(): Illegal DISTRIBUTE attribute\n");
      return;
    }

    name = subs->expr[i].str;

    if (!strcmp(":", name))
     put_distrib_info2(FD_DIST_LOCAL, 0, sp_get_dist_info(sp, i));
    else if (!strcmp("block", name))
     put_distrib_info(FD_DIST_BLOCK, sp_get_dist_info(sp, i));
    else if (!strcmp("cyclic", name))
      put_distrib_info(FD_DIST_CYCLIC, sp_get_dist_info(sp, i));
    else if (!strcmp("block_cyclic", name))
      put_distrib_info(FD_DIST_BLOCK_CYCLIC, sp_get_dist_info(sp, i));
    else
    {
      put_distrib_info3(FD_DIST_USER, name, 0, sp_get_dist_info(sp,i));
    }
  }
}

/********************************************************************/
/* this routine calculates the processor layout for each            */
/* decomposition based on n$proc and the distribution specification */
/********************************************************************/
static void
map_decomps(SNODE *sp, Dist_Globals *dh)
{
  int distdim, nproc, i;
  TYPEDESCRIPT *typeform;

  nproc = dh->numprocs;    /* total # of processors      */
  distdim = 0;                  /* # of distributed dims      */

  /* count up # of unallocated distributed dimensions        */
  /* also check # of processors remaining after distribution */

  for (i = 0; i < sp->numdim; i++)
  {
    if (sp_is_part(sp, i) != FD_DIST_LOCAL)
    {
      if (sp_num_blocks(sp,i))
      {
        nproc /= sp_num_blocks(sp,i);
        if (!nproc)
          die_with_message("not enough processors for DISTRIBUTE");
      }
      else
        distdim++;
    }
  }

  /*-----------------------------------------------------*/
  /* decide how to allocate remaining processors         */
  /* assume just 2D processor grid for current prototype */

  if (!distdim || nproc == 1)   /* no more processors to allocate */
    return;                     /* return immediately             */

  else if (distdim == 2)
  {
    if (nproc < 4)              /* only enough for 1 dim, allocate the
                                 * remaining */
      distdim = 1;              /* processors to the 1st distributed dimension   */

    else                        /* else divide by half (not best, but use for
                                 * now) */
      nproc /= 2;
  }
  else if (distdim > 2)
  {
    die_with_message("Too many unallocated distributed dimensions");
  }

  /*-------------------------------*/
  /* allocate remaining processors */

  for (i = 0; i < sp->numdim; i++)
  {
    /* find unallocated distributed dimension */

    if ((sp_is_part(sp,i) != FD_DIST_LOCAL) &&
       !(sp_num_blocks(sp,i)))
       sp_put_num_blocks(sp, i, nproc);
  }
}


/*******************************************************************/
/* this routine calculates the processor layout for each           */
/* distributed array based on the decomposition layout             */
/* modified by JDO 04 June 1991                                    */
/*  handle array replication and alignment of arrays with transposed*/
/*  decompositions, e.g., ALIGN X(i,j) with D(j,i)                 */
/*******************************************************************/
static void
map_distarrays(SNODE *array_sp, Dist_Globals *dh)
{
  SNODE *decomp_sp;
  DIST_INFO *decomp_dist, *array_dist;
  int i, j, ddim, arraysize, size;

  decomp_sp = array_sp->decomp; /* pointer to decomposition   */

  /*--------------------------------------------*/
  /* find distribution for each array dimension */

  for (i = 0; i < array_sp->numdim; i++)
  {
    /*--------------------------------------------------*/
    /* default access is the entire local array section */
    sp_put_min_access(array_sp, i,sp_get_lower(array_sp, i));
    sp_put_max_access(array_sp, i,sp_get_upper(array_sp, i));

    /*----------------------------------------------------*/
    /* find decomp dim that the array dim is aligned with */

    for (j = 0, ddim = 0; j < decomp_sp->numdim; j++)
    {
      if (sp_get_align_index(array_sp,j) == i+1)
      {
        if (!ddim)
          ddim = j+1;
        else
          die_with_message("Diagonal alignment not yet supported");
      }
    }


    if (!ddim)  /* no decomp dimension found */
    {
      sp_put_dist_type(array_sp, i, FD_DIST_LOCAL);
    }
    else
    {
      /*---------------------------------------------------------*/
      /* copy distribution from appropriate dimension of decomp  */

      sp_put_ddim(array_sp, i, ddim);
      sp_put_dist_type(array_sp,i, sp_is_part(decomp_sp, ddim-1));
      sp_put_num_blocks(array_sp, i, sp_num_blocks(decomp_sp,ddim-1));
      sp_put_bksize(array_sp, i, sp_bksize(decomp_sp, ddim-1));
      arraysize =  sp_get_upper(array_sp,i) - 
                   sp_get_lower(array_sp, i) + 1;

      size = sp_num_blocks(array_sp, i);

      /*--------------------------------------------*/
      /* calculate local array section sizes */
      /* kernel code taken from store_size() */

      if (size)
      {
        if (!(arraysize % size))
        {
          sp_put_block_size1(array_sp, i,  arraysize / size);
          sp_put_block_size2(array_sp, i,  sp_block_size1(array_sp, i));
          sp_put_block_size1(decomp_sp, ddim-1, arraysize/size);
          sp_put_block_size2(decomp_sp, ddim-1, sp_block_size1(array_sp,i));
        }
        else
        {
          sp_put_block_size1(array_sp, i,  arraysize / size + 1);
          sp_put_block_size2(array_sp, i,  arraysize - 
                             sp_block_size1(array_sp, i) * (size - 1));
 
          sp_put_block_size1(decomp_sp, ddim-1, arraysize/size + 1);
          sp_put_block_size2(decomp_sp, i,  arraysize - 
                             sp_block_size1(array_sp, i) * (size - 1));
        }

        /* Reduce the access range for this dimension if distributed */
 
        switch (sp_is_part(array_sp,i))
        {
          case FD_DIST_BLOCK:
            decomp_local_bounds(array_sp, i, sp_max_access_ptr(array_sp, i),
            sp_min_access_ptr(array_sp, i));
            break;

          case FD_DIST_CYCLIC:
            sp_put_min_access(array_sp, i, 1);
            sp_put_max_access(array_sp, i, sp_block_size1(array_sp,i));
            break;
        }
      }
    }
  }
}


/*----------------------------------------------------------------

  parse_expr()

*/

static char *
parse_expr(char *str, struct dc_expr *expr)
{
  char *s;

  while (*str == ' ')  /* eat up spaces */
    str++;

  if (*str == ':')     /* special case ":" as variable */
  {
    expr->type = variable;
    strcpy(expr->str, ":");
    return ++str;
  }

  if (isalpha(*str))
  {
    expr->type = variable;
    s = expr->str;
    while (isalpha(*str) || isdigit(*str) || (*str == '$'))
      *s++ = *str++;
    *s = '\0';

    if (*str == '+')
    {
      str++;
      while (*str == ' ')  /* eat up spaces */
        str++;

      if (!isdigit(*str))
      {
        printf("parse_expr(): Illegal expression\n");
        expr->type = unknown;
        return str;
      }

      expr->type = plus;
      sscanf(str, "%d", &expr->val);
      while (isdigit(*str))
        str++;
    }
    else if (*str == '-')
    {
      str++;
      while (*str == ' ')  /* eat up spaces */
        str++;

      if (!isdigit(*str))
      {
        printf("parse_expr(): Illegal expression\n");
        expr->type = unknown;
        return str;
      }

      expr->type = plus;
      sscanf(str, "%d", &expr->val);
      expr->val = -expr->val;

      while (isdigit(*str))
        str++;
    }
  }

  else if (isdigit(*str))
  {
    expr->type = value;
    sscanf(str, "%d", &expr->val);
    while (isdigit(*str))
      str++;
  }
  else
  {
    expr->type = unknown;
    printf("parse_expr(): Illegal expression\n");
    return str;
  }

  while (*str == ' ')  /* eat up spaces */
    str++;

  return str;
}

/*----------------------------------------------------------------

  parse_subs()

*/

static char *
parse_subs(char *str, struct dc_subs *subs)
{
  subs->num = 0;

  str++;  /* get past "(" */

  while (true)
  {
    str = parse_expr(str, subs->expr + subs->num++);

    while (*str == ' ')  /* eat up spaces */
      str++;

    if (*str == ')')
      return ++str;

    if (*str++ != ',')
    {
      printf("parse_subs(): Illegal syntax\n");
      return str;
    }
  }
}

/*----------------------------------------------------------------

  parse_id()

*/

static char *
parse_id(char *str, struct dc_id *id)
{
  char *s;

  while (*str == ' ')  /* eat up spaces */
    str++;

  s = id->str;
  while (isalpha(*str) || isdigit(*str) || (*str == '$'))
    *s++ = *str++;
  *s = '\0';

  while (*str == ' ')  /* eat up spaces */
    str++;

  if (*str == '(')
  {
    str = parse_subs(str, &id->subs);
    id->ident = false;
  }
  else
  {
    id->ident = true;     /* not a subscripted variable */
  }

  while (*str == ' ')  /* eat up spaces */
    str++;

  return str;  
}


/*----------------------------------------------------------------

  parse_list()

*/

static char *
parse_list(char *str, struct dc_list *list)
{
  list->num = 0;

  while (true)
  {
    str = parse_id(str, list->id + list->num++);

    while (*str == ' ')  /* eat up spaces */
      str++;

    if (*str != ',')
      return str;

    str++;
  }
}


