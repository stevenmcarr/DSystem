#ifndef mh_walk_h
#define mh_walk_h

#include <fortsym.h>
#include <Arena.h>

typedef struct {
  int     selection;
  PedInfo ped;
  FortTree ft;
  SymDescriptor symtab;
  arena_type    *ar;
 } walk_info_type;

typedef struct {
  AST_INDEX dbl_prec_list,
            real_list,
            cmplx_list;
 } decl_list_type;

#define REFD   "mh: refd"

EXTERN_FUNCTION(void mh_walk_ast,(int selection,PedInfo ped,AST_INDEX root,
				  FortTree ft,arena_type *ar));

#endif
