/* $Id: Open.C,v 1.1 1997/03/11 14:29:51 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/frontEnd/fortTree/ft.h>
#include <libs/support/memMgmt/mem.h>

/*ARGSUSED*/
FortTree ft_Open(Context context, DB_FP *)
{
  FortTree ft;
  DB_FP *src_fp;
  
  // allocate a new instance
  ft = (FortTree)
    get_mem(sizeof(struct FortTree_internal_structure), "FortTree");
  if (ft == (FortTree) 0) return (FortTree) UNUSED;
  
  // try to read the source 
  ft->asttab = NULL;
  if(context != CONTEXT_NULL) {
    src_fp = annotOpen(context, ft_SourceAttribute, "r");
    if(src_fp != DB_NULLFP) {
      ft->root = ast_import2(fortran_nodeinfo, src_fp,
			     &ft->asttab);

      ft->state = ft_INITIALIZED;
      (void) annotClose(src_fp);
    } else {
	ft->asttab = NULL;
      }
  }
  
  // make sure the tree exists 
  if(ft->asttab == NULL) {
    ft->asttab = ast_create(fortran_nodeinfo, 100, 100);
    ast_select(ft->asttab);
    ft->root   = AST_NIL;
  } 
  else ast_select(ft->asttab);
  
  ft->needs = 0;
  ft->provs = 0;
  
  // open the subparts
  ft->td = fst_Open((Generic) ft);
  
  // node numbering hashtables do not exist until needed
  
  ft->NodeToNumber = 0;
  ft->NumberToNode = 0;
  
  return ft;
}
