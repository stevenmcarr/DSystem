/* $Id: Memoria.C,v 1.3 1997/04/09 19:49:15 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/************************************************************************/
/*   File:  Memoria.C                                                        */
/*                                                                      */
/*   Description:  This is the wrapper for the memory compiler          */
/*                 stand alone executable.  The memory compiler code    */
/*                 is in ped_cp/memory                                  */
/*                                                                      */
/************************************************************************/

#include <strstream.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#ifdef OSF1
#include <String.h>
#else
#include <rw/cstring.h>
#endif

#include <libs/support/misc/general.h>
#include <libs/graphicInterface/oldMonitor/include/mon/standalone.h>
#include <libs/support/msgHandlers/interact.h>
#include <ctype.h>

#include <libs/support/memMgmt/mem.h>
#include <libs/support/strings/rn_string.h>

#include <libs/support/database/context.h>

#include <sys/file.h>
#include <include/maxdefs.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <string.h>
#include <libs/Memoria/include/memory_menu.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#include <libs/frontEnd/prettyPrinter/ft2text.h>
#include <libs/moduleAnalysis/dependence/edgeList/el_header.h>
#include <libs/moduleAnalysis/dependence/edgeList/el_instance.h>

#include <execs/Memoria/MemoriaOptions.h>
#include <libs/Memoria/include/header.h>

#include <libs/moduleAnalysis/cfg/cfg.h>
#include <libs/moduleAnalysis/ssa/ssa.h>
#include <libs/moduleAnalysis/cfgValNum/cfgval.h>

#include <libs/support/msgHandlers/ErrorMsgHandler.h>

#include <libs/support/file/UnixFile.h>

#include <libs/fileAttrMgmt/composition/Composition.h>
#include <libs/fileAttrMgmt/composition/CompositionIterators.h>

#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>
#include <libs/fileAttrMgmt/fortranModule/FortTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/FortTextTreeModAttr.h>



extern int selection;                /* memory optimizations to be performed */
extern char select_char;             /* character for output file name */
extern char *mc_program;             /* composition to optimize */
extern char *mc_module;              /* Fortran file to optimize */
extern char *mc_output;              /* Output file */
extern char *mc_config;              /* Configuration file */
extern char *mc_module_list;         /* file with list of Fortran files */
extern Boolean mc_extended_cache;
extern Boolean inputs_needed;
extern int ReplaceLevel;
extern int DependenceLevel;

DB_FP   *stderr_fp;

/************************************************************************/
/*                                                                      */
/*   Function:  CompileFile                                             */
/*                                                                      */
/*   Input:     None (uses global file names)                           */
/*   Output:    None                                                    */
/*                                                                      */
/*   Description:  Run the memory compiler on file mc_module.           */
/*                                                                      */
/************************************************************************/
    
void CompileFile(FortranModule *module)
{
  PedInfo ped;
  AST_INDEX root;
  FILE *fd;
  Boolean has_errors;
  DG_Instance     *DG;
  EL_Instance     *EL;
  LI_Instance     *LI;
  SideInfo        *SI;
  DT_info         *DT;
  CfgInfo         cfgModule;
  char level_str[10];
  FortTreeModAttr*      ftAttr = ATTACH_ATTRIBUTE(module, FortTreeModAttr);
  FortTextTreeModAttr*  fttAttr = ATTACH_ATTRIBUTE(module, FortTextTreeModAttr);
  FortTree              ft = ftAttr->ft;
  FortTextTree          ftt = fttAttr->ftt;
  char            *NewFile;
  
  
  ped = new Ped;
  root = ft_Root(ft);
  
  /* Perform dependence analysis (with input dependences) and 
     intialize the PedInfo structure */
  
  if (selection != ANNOTATE)
    {
      dg_all((Context)module,CONTEXT_NULL,CONTEXT_NULL,ftt,ft,&DG,&EL,&LI,&SI,&DT,
	     &cfgModule,true);   

      PED_DT_INFO(ped) = dt_init(root, PED_INFO(ped), PED_CFG(ped));

    }

  PED_DG(ped)         = DG;
  PED_FTT(ped)        = ftt;
  PED_FT(ped)	      = (Generic)ft;
  PED_ROOT(ped)	      = root;
  PED_INFO(ped)	      = SI;
  PED_DT_INFO(ped)    = DT;
  PED_MH_CONFIG(ped)  = NULL;
  
  
  /* Run the memory compiler on the ast of the input file */
  
  ApplyMemoryCompiler(selection,ped,root,ft,(Context)module,mc_config);
  
  if (selection != LI_STATS && selection != UJ_STATS && selection != SR_STATS)
    {
      /* save the transformed ast in a new file.  If stats are
	 chosen then no changes are done to the tree. */
      
      if (mc_output != NULL)
	NewFile = mc_output;
      else
	{
#ifdef OSF1
	  String Filename(mc_output);
	  Filename = Filename(0,Filename.index(".f"));
#else
	  RWCString Filename(mc_output);
	  Filename.remove(Filename.index(".f"));
#endif
	  switch(selection) {
	  case INTERCHANGE:
	    Filename += ".perm.f";
	    break;
	  case SCALAR_REP:
	    {
	      char Level[2];

	      Filename += ".sr";
	      sprintf(Level,"%d",ReplaceLevel);
	      Filename += (const char *)Level;
	      Filename += ".f";
	      break;
	    }
	  case UNROLL_AND_JAM:
	    Filename += ".uj.f";
	    break;
	  case PREFETCH:
	    Filename += ".fetch.f";
	    break;
	  case ANNOTATE:
	    Filename += ".cache.f";
	    break;
	  case LDST:
	    Filename += ".ldst.f";
	    break;
	  case DEAD:
	    Filename += ".dead.f";
	    break;
	  case PARTITION_UNROLL:
	    Filename += ".unroll.f";
	    break;
	  default:
	    Filename += ".memoria.f";
	    break;
	  }
#ifdef OSF1
	  NewFile = (char *)Filename;
#else
	  NewFile = (char *)Filename.data();
#endif
	}


      fd = fopen(NewFile, "w");
      if (fd == 0)
	{
	  fprintf(stderr, "The output file could not be opened...\n");
	  exit(-1);
	}
      ftt_TreeChanged(ftt,root);
      ft_export((Context)module, ft, ftt, fd, None);
      fclose(fd);
    }

    // destroy the dependence graph structures

    if (selection != ANNOTATE)
      {
       if (cfgModule)
	 {
	  cfgval_Close(cfgModule);
	  ssa_Close(cfgModule);
	  cfg_Close(cfgModule);
	 }
       
       dg_destroy(PED_DG(ped));
       el_destroy_instance(EL);
       li_free(LI);
       destroy_side_info(ft,SI);
       (void)dt_finalize_info(PED_DT_INFO(ped));
      }

}

int MemoriaMain(int argc, char **argv)
{
  Boolean has_errors;
  FILE    *fd;
  FortranModule *module;

  stderr_fp = db_convert_fp_to_db_fp(stderr);

  if (mc_program) {
    
    /* Run the memory compiler on each file in a composition */

    Composition *comp = new Composition;
    if (comp->Open(mc_program) != 0 ||
	comp->IsCompleteAndConsistent() == false) 
      errorMsgHandler.HandleMsg
	("Not using program context %s, since it contains errors\n",mc_program);
    else
      for(CompModulesIterator modules(comp);
	  module = (FortranModule *)modules.Current(); 
	  ++modules) 
	if (module->Open(module->ReferenceFilePathName()) != 0)
	  CompileFile(module);
	else
	  errorMsgHandler.HandleMsg("Module %s not found.\n",
				    module->ReferenceFilePathName());
    delete comp;

  } else if (mc_module_list) {
    /* Run the memory compiler on each file in a file list */

    istrstream ModuleList(mc_module_list);
    if (!ModuleList)
      {
	cerr << "The module list file " << mc_module_list <<  "could not be opened..."
	     << endl;
	return -1;
      }
    mc_module = (char *)new char[512];
    while (ModuleList >> mc_module)
      {
	module = new FortranModule;
	if (module->Open(mc_module) != 0)
	  CompileFile(module);
	else
	  errorMsgHandler.HandleMsg("Module %s not found.\n", mc_module);
	delete module;
      }
    delete mc_module;
    
    switch (selection)
      {
       case LI_STATS:
         memory_stats_total(mc_module_list);
         break;
       case UJ_STATS:
	 memory_UnrollStatsTotal(mc_module_list);
	 break;
       case SR_STATS:
	 /* PUT CALL HERE TO DUMP PROGRAM STATS TOTALS */
         memory_SRStatsTotal(mc_module_list);
	 break;
        default:
	 break;
      }
  }
  else
    {
      module = new FortranModule;
      if (module->Open(mc_module) != 0)
	CompileFile(module);
      else
	errorMsgHandler.HandleMsg("Module %s not found.\n", mc_module);
      delete module;
    }
  return 0; // JMC 2/93
}

int main(int argc, char **argv)
{
  int ret;
  Boolean answer;
  
  ret = runRoot(argc, (char **)argv, MemoriaInitOptions, MemoriaMain);
  return ret;
}

  
  
   
