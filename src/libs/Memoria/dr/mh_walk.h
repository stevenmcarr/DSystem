/* $Id: mh_walk.h,v 1.3 1992/10/03 15:48:36 rn Exp $ */
#ifndef mh_walk_h
#define mh_walk_h

#include <fort/fortsym.h>
#include <Arena.h>

typedef struct {
  int     selection;
  PedInfo ped;
  FortTree ft;
  SymDescriptor symtab;
  arena_type    *ar;
  int nests,
      total_loops,
      imperfect;
 } walk_info_type;

typedef struct {
  AST_INDEX dbl_prec_list,
            real_list,
            cmplx_list;
 } decl_list_type;

#define REFD   "mh: refd"

EXTERN(void, mh_walk_ast,(int selection,PedInfo ped,AST_INDEX root,
				  FortTree ft,arena_type *ar));

#endif
