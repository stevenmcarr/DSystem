/* $Id: SaveInfo.C,v 1.10 1997/03/11 14:29:52 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/frontEnd/fortTree/ft.h>
#include <libs/frontEnd/fortTree/InitInfo.h>
#include <libs/config/local_info.h>
#include <libs/graphicInterface/oldMonitor/OBSOLETE/db/FileContext.h>
#include <libs/support/file/File.h>
#include <libs/support/file/FormattedFile.h>

/* C++ includes for mod/ref information */

#include <libs/frontEnd/fortTree/modrefnametree.h>
#include <libs/ipAnalysis/ipInfo/iptree.h>
#include <libs/ipAnalysis/ipInfo/module.h>
#include <libs/ipAnalysis/ipInfo/iptypes.h>

extern AST_INDEX get_name_in_entry();
extern void WalkInitialInformation(AST_INDEX root, LInfo* LocInfo);

void ft_Save(FortTree ft, Context context, DB_FP *fp)
{
  DB_FP *errors_fp;
  DB_FP *src_fp, *port;

  ast_select(ft->asttab);

  src_fp = annotOpen(context, ft_SourceAttribute, "w");
  ast_export2(src_fp, ft->root);
  (void) annotClose(src_fp);

  if (ft->state == ft_CORRECT)
  {/* dump out all of the registered local information for this module */
    /* provides and requires */
    port = annotOpen(context, "informals.needs", "w");
    ft->needs->Write(port);
    (void) annotClose(port);
    
    port = annotOpen(context, "informals.provs", "w");
    ft->provs->Write(port);
    (void) annotClose(port);

    // ensure Node Number Pairs up to date since they are written out 
    // in the initial interprocedural information
    ft_RecomputeNodeNumbers(ft); 

    // compute initial interprocedural information
    FileContext fileContext;
    
    int code = fileContext.Open(context);
    assert(code == 0);

    LInfo *LocInfo = new LInfo(ft, context);

    File *tsf = fileContext.NewAttribute("informals.initial");
    LocInfo->dbport = new FormattedFile(tsf);

    /* standard info + FortD */
    FortranDProblem *m = new FortranDProblem();

#ifdef WHEN_FORTD_BROKEN
    ModRefProblem *m = new ModRefProblem();
#endif

    LocInfo->p =  m;      
    WalkInitialInformation(ft->root, LocInfo);    
    LocInfo->dbport->Close();
    delete LocInfo->dbport;
    delete tsf;
  }

  /* save the subparts */
  fst_Save(ft->td, context, fp);
}

/* initialize the symbol table structure in LocInfo   */
void LocInfo_proc_sym_table(LInfo *LocInfo, AST_INDEX node)
{
 LocInfo->proc_sym_table = 
 fst_GetTable(LocInfo->ft->td, gen_get_text(get_name_in_entry(node)));
}
  
