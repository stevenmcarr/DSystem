/* $Id: Cfg.C,v 1.1 1998/01/07 14:42:16 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/******************************************************************************
 *
 * File
 *    Cfg.C
 *
 * Original Author
 *    Unknown
 *
 * Creation Date
 *    Unknown
 *
 * Description
 *    Control flow grapher.
 *
 * History
 *    08/94 - Kevin Cureton - rewrite to use the file attribute abstraction.
 *
 ******************************************************************************/

/**************************** System Include Files ****************************/

#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/file.h>

/***************************** User Include Files *****************************/

#include <libs/support/database/context.h>

#include <libs/fileAttrMgmt/composition/Composition.h>
#include <libs/fileAttrMgmt/composition/CompositionIterators.h>
#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>
#include <libs/fileAttrMgmt/fortranModule/FortTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/FortTextTreeModAttr.h>

#include <libs/moduleAnalysis/cfgValNum/cfgval.i>

#include <libs/support/file/UnixFile.h>
#include <libs/support/msgHandlers/ErrorMsgHandler.h>
#include <libs/support/msgHandlers/interact.h>
#include <libs/support/optParsing/Options.h>

#include <execs/cfg/CfgOptions.h>



/**************************** Variable Definitions ****************************/

static DB_FP *stderr_fp;

/************************* Static Function Prototypes *************************/

static void CfgTest(FortranModule* fortModule);
static void CfgTestOnce(CfgInfo cfgGlobals, Context moduleContext);

/************************* Extern Function Prototypes *************************/

typedef FUNCTION_POINTER(int, OptionsProcessFunctPtr, (int argc, char **argv));
typedef FUNCTION_POINTER(int, MainFunctPtr, (int argc, char **argv));

EXTERN(int, runRoot, (int argc,
                      char **argv,
                      OptionsProcessFunctPtr opts_process,
                      MainFunctPtr func));


/****************************** External Functions ****************************/

/******************************************************************************
 *
 *  Function
 *     CfgMain (extern)
 *
 *  Parameters
 *
 *  Return Value
 *
 *  Description
 *
 *  Original Author
 *
 *  Creation Date
 *
 *  Change History
 *
 ******************************************************************************/
int CfgMain(int argc, char **argv)
{
  FortranModule*  fortModule = 0; 

  if (cfgProgram) 
  {
     Composition* comp = 0;
     comp = new Composition;
     if (comp->Open(cfgProgram) != 0 ||
         comp->IsCompleteAndConsistent() == false)
     {
        errorMsgHandler.HandleMsg
           ("Not using program context %s, since it contains errors\n", cfgProgram);
        delete comp;
        comp = 0;
     }

     if (comp)
     {
        OrderedSetOfStrings* names = comp->LookupModuleNameBySuffix(FORTRAN_FILE_SUFFIX);
        int numNames = names->NumberOfEntries();

        if (numNames > 0)
        {
           char* filename;

           for(int i = 0; i < numNames; i++)
           {
              char* filename = (*names)[i];
              if (!file_access(filename, R_OK))
              {
                 errorMsgHandler.HandleMsg
                    ("Module %s is unreadable, or does not exist.\n", filename);
                 delete names;
                 return -1;
              }
              fortModule = (FortranModule*)comp->GetModule(filename);
              assert(fortModule != 0);

              CfgTest(fortModule);
           }
        }
        else
        {
           errorMsgHandler.HandleMsg
              ("Error in constructing the composition %s.\n", cfgProgram);
           delete names;
           return -1;
        }

        delete names;
     }
  }
  else if (cfgModuleList)
  {
     char buffer[1024];
     FILE* fp;

     fp = fopen(cfgModuleList, "r");

     if (fp == 0) 
     {
        errorMsgHandler.HandleMsg("The module list file could not be opened...\n");
        exit(-1);
     }
    
     while (fscanf(fp, "%s", buffer) > 0)
     {
        fortModule = new FortranModule;

        fortModule->Open(buffer);

        CfgTest(fortModule);

        fortModule->Close();

        delete fortModule;
     }
    
     fclose(fp);
  }
  else if (cfgModule)
  {
     fortModule = new FortranModule;

     fortModule->Open(cfgModule);

     CfgTest(fortModule);

     fortModule->Close();

     delete fortModule;
  }
  else
  {
     errorMsgHandler.HandleMsg("cfg error: No composition, module, or module list\n");
     exit(-1);

  }

  return 0;
}

/******************************************************************************
 *
 *  Function
 *     main (extern)
 *
 *  Parameters
 *
 *  Return Value
 *
 *  Description
 *
 *  Original Author
 *
 *  Creation Date
 *
 *  Change History
 *
 ******************************************************************************/
int main(int argc, char **argv)
{
  int returnVal;
  
  returnVal = runRoot(argc, argv, CfgOptsProcess, CfgMain);

  return returnVal;
}


/******************************* Static Functions *****************************/

/******************************************************************************
 *
 *  Function
 *     CfgTest (static)
 *
 *  Parameters
 *
 *  Return Value
 *
 *  Description
 *     Routine to drive the tests.
 *
 *  Original Author
 *
 *  Creation Date
 *
 *  Change History
 *
 ******************************************************************************/
static void CfgTest(FortranModule* fortModule)
{
  CfgInfo cfgGlobals;

  FortTreeModAttr*      ftAttr = ATTACH_ATTRIBUTE(fortModule, FortTreeModAttr);
  FortTextTreeModAttr*  fttAttr = ATTACH_ATTRIBUTE(fortModule, FortTextTreeModAttr);
  FortTree              ft = ftAttr->ft;
  FortTextTree          ftt = fttAttr->ftt;

  cfgGlobals = cfg_Open(ft);

  CfgTestOnce(cfgGlobals, (Context)fortModule); 

  cfg_Close(cfgGlobals);

  return;
}

/******************************************************************************
 *
 *  Function
 *     CfgTestOnce (static)
 *
 *  Parameters
 *
 *  Return Value
 *
 *  Description
 *     Run value numbering with one setting of the flags.
 *
 *  Original Author
 *
 *  Creation Date
 *
 *  Change History
 *
 ******************************************************************************/
static void CfgTestOnce(CfgInfo cfgGlobals, Context moduleContext)
{
  CfgInstance       cfg;
 
  if (doDataFlow)
  {
     if (doVals)
     {
        cfgval_Open(cfgGlobals, false);
     }

     ssa_Open(cfgGlobals, (Generic) NULL, ipSmush, doArrays, doDefKill, doGated);
  }
 
  for (cfg = cfg_get_first_inst(cfgGlobals);
       cfg;
       cfg = cfg_get_next_inst(cfg))
  {
     cfg_get_predom(cfg);
     cfg_get_postdom(cfg);
     cfg_get_intervals(cfg);
     cfg_build_cfg_cds(cfg);
   
     if (doVals)
     {
        if (ipTest)
        {
           cfgval_build_ip(cfg);
        }
        else
        {
           cfgval_build_table(cfg);
        }
     }
   
     if (dumpCfg) 
     {
        cfg_dump(cfg);
        fflush(stdout);
     }
   
     if (dumpDom) 
     {
        dom_print(cfg_get_predom(cfg), cfg->start);

        dom_print(cfg_get_postdom(cfg), cfg->end);
     }

     if (dumpTarj)
     {
        tarj_print(cfg_get_intervals(cfg), cfg->start);
     }

     if (dumpSort)
     {
        cfg_sorted_dump(cfg);
     }

     if (dumpVals) 
     {
        cfgval_Dump((Generic) CFGVAL_parms(cfg), cfg);
     }

     if (ipTest) 
     {
        cfgval_Save(cfg, moduleContext);
        cfgval_Restore(cfg, moduleContext);

        if (dumpVals) cfgval_Dump((Generic) CFGVAL_parms(cfg), cfg);
     }

     if (dumpTree) 
     {
        tree_print(ft_Root((FortTree) cfgGlobals->ft));
        fflush(stdout);
     }

     if (doStats)
     {
        cfg_stats(cfg);
        if (doDataFlow)
        {
           ssa_stats(cfg);
          //if (doVals)
          //val_stats(cfg);
        }
        fflush(stdout);
     }
  }
 
  if (doDataFlow) 
  {
     if (doVals)
     {
       cfgval_Close(cfgGlobals);
     }
     ssa_Close(cfgGlobals);
  }

  return;
}
