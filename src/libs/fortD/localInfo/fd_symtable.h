/* $Id: fd_symtable.h,v 1.10 2001/10/12 19:31:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef FD_SYMTAB
#define FD_SYMTAB

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#include <libs/frontEnd/include/expr.h>

#include <libs/fortD/misc/fd_types.h>
#include <libs/fortD/misc/FortDEnum.h>

#include <libs/support/memMgmt/mem.h>
#include <libs/support/strings/rn_string.h>

#include <libs/support/database/context.h>

#define MAX_NAME 20
#define HMASK 0X003F
#define NIHASH 64

enum FORM           /* type of variable */
{
  INTTYPE = 1, REAL = 2, ARRAYTYPE = 3, STRTYPE = 4,
  CHARTYPE = 5, CONSTYPE = 6, DOUBLE_P = 7, COMPLEX = 8,
  DECOMPTYPE = 9, UNKNOWN = 10
};

#ifdef SOLARIS
#undef OVERFLOW
#endif

enum OVERFLOW      /* alignment overflow options */
{
  ERROR = 0, TRUNC = 1, WRAP = 2, EMP = 3
};

enum ALIGNTYPE         /* alignment type */
{
  ALIGN_UNKNOWN = -1,  /* unknown alignment          */
  ALIGN_PERFECT = 1,   /* perfect alignment          */
  ALIGN_OFFSET = 2,    /* constant offset            */
  ALIGN_COEFF = 3,     /* constant coefficient, may have constant offset */
  ALIGN_CONST = 4      /* constant                   */
};

typedef struct dist_info
{
  Dist_type distr_type;      /* type of distribution               */

  /* for Dist_type == FD_DIST_USER  */

  char *irreg_id;          /* user defined function/array name   */
  int arg_position;        /* argument position of this dimension     */
  /* (zero based) to the irreg dist above  */

  /* for all other Dist_type */

  int size;        /* # of processors in dim             */
  int bksize;      /* block size for block_cyclic        */
  int blocksize1;  /* array size for internal processors */
  int blocksize2;  /* array size for boundary processors */
  int min_access;  /* minimum location accessed in dimension */
  int max_access;  /* maximum location accessed in dimension */
  int ddim;
} DIST_INFO;

/*******************************************************/
/* description of distribution, upper and lower bounds */
/*******************************************************/

typedef struct  typedescript
{
  Expr lo;
  Expr up;
  Expr step;
} TYPEDESCRIPT;


/****************************************************/
/* description of subscript with one index variable */
/****************************************************/

typedef struct subscr_info
{
  enum ALIGNTYPE stype;    /* subscript type                              */
  int index;               /* depth of index var  or the dimension in the */
                           /* case of iter sets                           */
  int offset;              /* constant term                               */
  int coeff;               /* constant coeff                              */
  AST_INDEX sub_ast;
} SUBSCR_INFO;

/*****************************************************************/
/* this structure contains the attributes of a particular        */
/* variable or decomposition entered in the symbol table.        */
/*****************************************************************/

typedef struct snode
{
  enum FORM fform;      /* the type of the variable declared          */
  enum FORM fform1;     /* if decomp then = DECOMPTYPE                */

  int value;            /* if constant then = value                   */
  int numdim;           /* if array/decomp then = # of dimensions     */
  int num_arrays;       /* if decomp then = # of aligned arrays       */
  int scopelev;
  TYPEDESCRIPT *idtype[DC_MAXDIM]; /* declaration  details */
  DIST_INFO *distinfo[DC_MAXDIM];  /* distribution details */

  struct snode *decomp; /* if array then = mapped decomposition              */
  SUBSCR_INFO *align[DC_MAXDIM]; /* if array then = subs of decomp in align  */
  char id[MAX_NAME];    /* the name of the identifier                        */
  Boolean is_referenced;
  Boolean perfect_align;       /* if perfectly aligned to a decomposition    */
  Boolean done_assignment;     /* if an assignment has been created          */
  struct snode *next_el;
  Boolean access;              /* check if access information has been added */
  int align_id_number;       /* line number information stored for the     */
  int distrib_id_number;     /* D interface                                */
  int decomp_id_number;
  Context c;                   /* context information recorded for the       */
                               /* D interface                                */
} SNODE;

EXTERN(void, init_var_snode, (SNODE *sp, int dim, enum FORM type));
EXTERN(SNODE *,  findadd, (char *id, int flag, int level, SNODE **st));
EXTERN(int, sp_numdim, (SNODE *sp));
EXTERN(Dist_type, sp_is_part, (SNODE *sp, int dim));
EXTERN(void, sp_put_dist_type, 
                              (SNODE *sp, int dim, Dist_type d));
EXTERN(void, put_distrib_info,(enum Dist_type d, DIST_INFO *dt));
EXTERN(void, put_distrib_info2,(enum Dist_type d,int i, DIST_INFO *dt));
EXTERN(void, put_distrib_info3,(enum Dist_type d, char *nme, int i,  DIST_INFO *dt));
EXTERN (int, decomp_get_upper, (SNODE *sp, int dim));
EXTERN (int, decomp_get_lower, (SNODE *sp, int dim));
EXTERN (SUBSCR_INFO *,  get_align_info, (SNODE *sp, int dim));
EXTERN (int, sp_get_align_index, (SNODE *sp, int dim));
EXTERN (DIST_INFO *, sp_get_dist_info, (SNODE *sp, int dim));
EXTERN (void, sp_put_block_size1, (SNODE *sp, int dim, int blocksize));
EXTERN (void, sp_put_block_size2, (SNODE *sp, int dim, int blocksize));
EXTERN (int,  sp_block_size1, (SNODE *sp, int dim));
EXTERN (int,  sp_bksize, (SNODE *sp, int dim));
EXTERN (void, sp_put_bksize, (SNODE *sp, int dim, int bksize));
EXTERN (int,  sp_num_blocks, (SNODE *sp, int dim));
EXTERN (void, sp_put_num_blocks, (SNODE *sp, int dim, int size));
EXTERN (int,  sp_block_size2, (SNODE *sp, int dim));
EXTERN (void, sp_put_upper, (SNODE *sp, int dim, int val));
EXTERN (void, sp_put_lower, (SNODE *sp, int dim, int val));
EXTERN (Expr *, get_expr_lo, (SNODE *sp, int dim));
EXTERN (Expr *, get_expr_up, (SNODE *sp, int dim));
EXTERN (int,  sp_get_upper, (SNODE *sp, int dim));
EXTERN (int,  sp_get_lower, (SNODE *sp, int dim));
EXTERN (int,  sp_ddim, (SNODE *sp, int dim));
EXTERN (void, sp_put_ddim, (SNODE *sp, int dim, int ddim));
EXTERN (int,  sp_min_access, (SNODE *sp, int dim));
EXTERN (int,  sp_max_access, (SNODE *sp, int dim));
EXTERN (int *, sp_max_access_ptr, (SNODE *sp, int dim));
EXTERN (int *, sp_min_access_ptr, (SNODE *sp, int dim));
EXTERN (void, sp_put_min_access, (SNODE *sp, int dim, int val));
EXTERN (void, sp_put_max_access, (SNODE *sp, int dim, int val));
EXTERN (void, sp_set_align_info, (SNODE *sp, int dim, int index, 
                 int offset, int coeff, enum ALIGNTYPE stype));
EXTERN (enum ALIGNTYPE, dc_align_type, (SNODE *sp, int dim));
EXTERN (int, subs_get_index, (SNODE *sp, int dim));
EXTERN (int, get_align_offset, (SNODE *sp, int dim));
EXTERN (enum ALIGNTYPE,  sp_align_stype, (SNODE *sp, int dim));
EXTERN (void, sp_put_align_type, (SNODE *sp, int dim, enum ALIGNTYPE type));
EXTERN (int, sp_align_index, (SNODE *sp, int dim));
EXTERN (int, sp_align_offset, (SNODE *sp, int dim));
EXTERN (int, sp_align_coeff, (SNODE *sp, int dim));
EXTERN (void, decomp_local_bounds, (SNODE *sp, int dim, int* dupper,
                                           int* dlower));
EXTERN (void, sp_put_align_index, (SNODE *sp, int dim, int index));
EXTERN (SNODE *, sp_decomp, (SNODE *sp));
EXTERN (void, sp_access, (SNODE *sp, Boolean));
EXTERN (Boolean, sp_access_done, (SNODE *sp));
EXTERN (void, sp_put_align_id, (SNODE *, int));
EXTERN (void, sp_put_distrib_id, (SNODE*, int));
EXTERN (void, sp_put_decomp_id, (SNODE*, int));
EXTERN (int, sp_get_align_id, (SNODE *sp, int l));
EXTERN (int, sp_get_distrib_id, (SNODE*));
EXTERN (int, sp_get_decomp_id, (SNODE*));

#if 0
EXTERN (void, sp_put_context, (SNODE*, Context));
EXTERN (Context, sp_get_context, (SNODE*));
#endif

#endif
