/* $Id: Checker.C,v 1.4 1997/03/11 14:27:26 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/******************************************************************************
 *
 * File
 *    checker.C
 *
 * Original Author
 *    Unknown
 *
 * Creation Date
 *    Unknown
 *
 * Description
 *    Fortran file type checker.
 *
 * History
 *    08/94 - Kevin Cureton - rewrite to use the file attribute abstraction.
 *
 ******************************************************************************/

/**************************** System Include Files ****************************/

#include <assert.h>
#include <ctype.h>
#include <iostream.h>
#include <stdlib.h>
#include <unistd.h>


/***************************** User Include Files *****************************/

#include <libs/support/misc/general.h>

#include <libs/support/msgHandlers/ErrorMsgHandler.h>
#include <libs/support/file/UnixFile.h>

#include <libs/fileAttrMgmt/composition/Composition.h>
#include <libs/fileAttrMgmt/composition/CompositionIterators.h>

#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>
#include <libs/fileAttrMgmt/fortranModule/FortTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/FortTextTreeModAttr.h>

#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/problems/fortD/FortDNumProcsAnnot.h>

#include <execs/checker/CheckerOptions.h>




/* 
#include <libs/support/msgHandlers/interact.h>

#include <libs/support/memMgmt/mem.h>
#include <libs/support/strings/rn_string.h>

#include <libs/support/database/context.h>
#include <libs/frontEnd/ast/OBSOLETE/fortObject.h>

#include <libs/moduleAnalysis/dependence/utilities/side_info.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_instance.h>
#include <libs/moduleAnalysis/dependence/dependenceGraph/dg_header.h>
#include <libs/moduleAnalysis/dependence/loopInfo/li_instance.h>
#include <libs/moduleAnalysis/dependence/edgeList/el_instance.h>
#include <libs/moduleAnalysis/dependence/edgeList/el_header.h>

#include <libs/moduleAnalysis/cfg/cfg.h>
#include <libs/moduleAnalysis/ssa/ssa.h>
#include <libs/moduleAnalysis/cfgValNum/cfgval.h>
*/

/**************************** Variable Definitions ****************************/

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

/************************* Static Function Prototypes *************************/

static void CheckFile(DB_FP *stderr_fp, FortranModule* module);


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
 *     CheckerMain (extern)
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
int CheckerMain(int argc, char **argv)
{
  FortranModule*   mod_context;
  DB_FP*           stderr_fp;
  char             buffer[MAXPATHLEN];

  stderr_fp = db_convert_fp_to_db_fp(stderr);

  char* mod_name = global_mod_loc;
  char* pgm_name = global_pgm_loc;

     // missing program name => use module name with suffix ".comp"
  if (pgm_name == 0) 
  {
     int len = strlen(buffer);
     assert(len < (MAXPATHLEN + 5));

     strcpy(buffer, mod_name);

        // remove trailing ".f" if present
     if( buffer[len-2] == '.' && buffer[len-1] == 'f' )
       buffer[len-2] = '\0';

     strcat(buffer, ".comp");
     if (file_access(buffer, R_OK)) pgm_name = buffer;
  }

  Composition* comp = 0;
  if (pgm_name) 
  {
     comp = new Composition;
     if (comp->Open(pgm_name) != 0 ||
         comp->IsCompleteAndConsistent() == false) 
     {
        errorMsgHandler.HandleMsg
           ("Not using program context %s, since it contains errors\n",
            pgm_name);
        delete comp;
        comp = 0;
     }
  }

  if (comp) 
  {
     OrderedSetOfStrings* names = comp->LookupModuleNameBySuffix(mod_name);
     int numNames = names->NumberOfEntries();
     switch(numNames) 
     {
        case 0:
        {
           errorMsgHandler.HandleMsg
              ("Module %s not found in composition %s.\n", mod_name, pgm_name);
           delete names;
           return -1;
        }
        break;

        case 1:
        {
           char* filename = (*names)[0];
           if (!file_access(filename, R_OK)) 
           {
              errorMsgHandler.HandleMsg
                 ("Module %s is unreadable, or does not exist.\n", filename);
              delete names;
              return -1;
           }
           mod_context = (FortranModule*)comp->GetModule(filename);
           delete names;
           assert(mod_context != 0);
        }
        break;
 
        default:
        {
           errorMsgHandler.HandleMsg
              ("Module %s does not uniquely specify a member of composition %s.\n",
               mod_name, pgm_name);
           errorMsgHandler.HandleMsg("Members found: ");
           for (; numNames--;) 
           {
              errorMsgHandler.HandleMsg("%s ", (*names)[numNames]);
           }
           delete names;
           return -1;
        }
        break;
     }
  } 
  else
  {
       // something needs to be done for compositions that don't exist
     return -1;
  }

  CheckFile(stderr_fp, mod_context);
}

/******************************************************************************
 *
 *  Function
 *     main (extern)
 *
 *  Parameters
 *     argc - number of arguments
 *     argv - pointer to the array of arguments
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
  int returnValue;

  returnValue = runRoot(argc, argv, CheckerOptsProcess, CheckerMain);

  return returnValue;
}

/******************************* Static Functions *****************************/

/******************************************************************************
 *
 *  Function
 *     CheckFile (static)
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
static void CheckFile(DB_FP *stderr_fp, FortranModule* module)
{
  FortTreeModAttr*      ftAttr = ATTACH_ATTRIBUTE(module, FortTreeModAttr);
  FortTextTreeModAttr*  fttAttr = ATTACH_ATTRIBUTE(module, FortTextTreeModAttr);
  FortTree              ft = ftAttr->ft;
  FortTextTree          ftt = fttAttr->ftt;


#if 0
  //DG_Instance*DG;
  //EL_Instance*EL;
  //LI_Instance*LI;
  //SideInfo*SI;
  //char        suffix[80];
  //char* depGraph_path;
  //FILE*gptr;
  //FortObject  fo;
  //Boolean has_errors;
  //CfgInfo      cfgModule;

  if (global_dep_opt)
  {
     dg_all(m_context,
            CONTEXT_NULL,
            p_context,
            ftt,
            ft,
            &DG,
            &EL,
            &LI,
            &SI,
            &cfgModule,
            false);

     if (NOT(dg_save_instance(ftt, DG, global_dep_ptr)))
     {
    cerr << "Error saving dependence graph"
             << endl;
     }

        // destroy cfg information
     if (cfgModule)
     {
        cfgval_Close(cfgModule);
        ssa_Close(cfgModule);
        cfg_Close(cfgModule);
     }

        // destroy the dependence graph structures
     dg_destroy(DG);
     el_destroy_instance(EL);
     li_free(LI);
     destroy_side_info(ft,SI);

     fclose(global_dep_ptr);
  }
#endif
}
