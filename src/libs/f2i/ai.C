/* $Id: ai.C,v 1.3 1998/04/29 13:00:23 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <stdio.h>
#include <sys/file.h>
#include <libs/support/misc/general.h>
#include <libs/graphicInterface/support/graphics/point.h>
#include <libs/support/strings/rn_string.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/support/database/context.h>


#include <libs/ipAnalysis/interface/IPQuery.h>

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/moduleAnalysis/cfg/cfg.h>
#include <libs/moduleAnalysis/ssa/ssa.h>
#include <libs/moduleAnalysis/cfgValNum/cfgval.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#include <libs/Memoria/include/memory_menu.h>
#include <libs/moduleAnalysis/dependence/edgeList/el_header.h>

/* forward declaration */
static void fixDoEnddoExact(FortTree  , FortTextTree );
static void fixItems(AST_INDEX );
static void aiInit(void);

static long glabel = 99999;

/* globals used in label.c, io.c, idfa.c, and stmts.c */

   FortTree             ft;
   FortTextTree		ftt;


/* the real main routine of the AST->iloc translator */
void ai(Context m_context,FortTree local_ft,FortTextTree local_ftt,char *FileName)
//   Context m_context;
//   FortTree local_ft;
//   FortTextTree local_ftt;
//   char *FileName;
{
  Boolean result;
  DG_Instance     *DG;
  EL_Instance     *EL;
  LI_Instance     *LI;
  SideInfo        *SI;
  DT_info         *DT;
  CfgInfo         cfgModule;
  PedInfo         ped;
  char            *IlocFile;

  /* open the fortran abstractions  */

  ped = (PedInfo)malloc(sizeof(Ped));
  ft = local_ft;
  ftt = local_ftt;

  result = (Boolean)ft_SymRecompute(ft);
  if (result == false)
        ERROR("ai", "FortTreeTypeChecker: the module has errors", SERIOUS);

  /* repair the do-enddo */
    fixDoEnddoExact(ft,ftt);

  if (aiTreeDump) 
     tree_print(ft_Root(ft));

  if (aiTreeDump >2)
     ast_dump_all();

  /* should check to see if the tree has recorded */
  /* syntactic or semantic errors in this block   */
  {
    if (FileName != NULL)
      {
	int length = strlen(FileName);
	IlocFile = (char*)malloc(sizeof(char)*(length+6));
	(void)strcpy(IlocFile,FileName);
	(void)strcat(IlocFile,".f2i.i");
	if (!freopen(IlocFile,"w",stdout))
	  {
	    fprintf(stderr,"Error opening output file %s\n",IlocFile);
	    exit(-1);
	  }
      }

    aiInit();
    if (aiCache)
      {
       dg_all(m_context,CONTEXT_NULL,CONTEXT_NULL,ftt,ft,&DG,&EL,&LI,&SI,&DT,
	      &cfgModule,true);
       PED_DG(ped)         = DG;
       PED_FTT(ped)        = ftt;
       PED_FT(ped)	      = (Generic)ft;
       PED_ROOT(ped)	      = ft_Root(ft);
       PED_INFO(ped)	      = SI;
       PED_DT_INFO(ped)    = DT;
       PED_MH_CONFIG(ped)  = NULL; 

       ApplyMemoryCompiler(CACHE_ANALYSIS,ped,PED_ROOT(ped),ft,m_context,NULL);

       cfgval_Close(cfgModule);
       ssa_Close(cfgModule);
       cfg_Close(cfgModule);
       dg_destroy(DG);
       el_destroy_instance(EL);
       li_free(LI);
       destroy_side_info(ft,SI);
       (void)dt_finalize_info(DT);

       ftt_TreeChanged(ftt,ft_Root(ft));
      }
    aiProcedures(ft_Root(ft), ft); 
    (void) fflush(stdout);
    if (FileName != NULL)
      fclose(stdout);
  }

} /* ai */




/* convert odd AST features into standard Fortran 77 */
static void fixDoEnddoExact(FortTree ai_ft,FortTextTree ai_ftt)
//   FortTree     ai_ft;
//   FortTextTree ai_ftt;
{
  FortTreeNode root;
  root = ft_Root(ai_ft);
  
  fixItems(root);
  ftt_TreeChanged(ai_ftt, root);
} /* fixDoEnddoExact */




/* called by fixDoEnddoExact to perform conversion */
/* of odd AST features in standard Fortran 77      */
static void fixItems(AST_INDEX node)
  // AST_INDEX node;
{
   int          n,i;
   AST_INDEX label_ref, cont_stmt, stmt_list, elt;
   char         buf[7];

   if( node == AST_NIL )
       return;

   switch( gen_get_node_type(node) )
    {
      case GEN_LIST_OF_NODES:
        elt = list_first(node);
        while( elt != AST_NIL )
          { fixItems(elt);
            elt = list_next(elt);
          }
        break;
      case GEN_DO:
	if( gen_DO_get_lbl_ref(node) == AST_NIL )
          {
	    (void) sprintf (buf, "%ld", glabel--);
	    label_ref = gen_LABEL_REF();
            gen_put_text(label_ref, buf, STR_LABEL_DEF);

            gen_DO_put_lbl_ref(node,label_ref);
	    stmt_list = gen_DO_get_stmt_LIST(node);
	    fixItems(stmt_list);

	    cont_stmt = gen_CONTINUE(tree_copy(label_ref));
	    list_insert_last(stmt_list,cont_stmt);
          }
        break;
      case GEN_EXACT:

	/* coerce TYPE_EXACT to TYPE_DOUBLE_PRECISION */
	if (gen_get_real_type(node)      == TYPE_EXACT)
	  gen_put_real_type(node,      TYPE_DOUBLE_PRECISION);
	if (gen_get_converted_type(node) == TYPE_EXACT)
	  gen_put_converted_type(node, TYPE_DOUBLE_PRECISION);

	/* prune out the EXACT and put in the DOUBLE PRECISION */
	tree_replace(node, gen_DOUBLE_PRECISION());
        tree_free(node);
	break;
      default:
	n = gen_how_many_sons(gen_get_node_type(node));
        for( i = 1; i <= n; i++ )
          fixItems(gen_get_son_n(node, i));
        break;
   }
} /* fixItems */




/* initialize global ai variables */
static void aiInit(void)
{
  aiStmtCount 	 	= 0;
  aiNextRegister 	= 3;	/* 3 => avoid unitialized vars		*/
  aiNextLabel	 	= 1;
  aiNextStack	 	= 0;
  aiNextCallSite	= 1;
  aiNumParameters	= 0;
  aiNumInstructions	= 0;
  aiEpilogue		= aiNextLabel++;
  aiNextReg             = 2;

  SymHaveSeenASave        = 0;
  SymHaveSeenAnEquivalence= 0;

  if (aiSparc > 0)
  {
    aiNextStatic = 0;
  }
  else if (aiRt)
  {
    aiNextStatic = 16;	/* reserve space for label, profile counters */
  }
  else
    aiNextStatic = GetDataSize(TYPE_LABEL);  /* reserve space for label */


  proc_name = "Uninitialized!";
} /* aiInit */
