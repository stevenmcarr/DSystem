/* $Id: f2i.C,v 1.7 2003/02/28 22:20:49 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/************************************************************************/
/*   File:  f2i.C                                                        */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <strstream.h>

#include <libs/support/misc/general.h>
#include <libs/graphicInterface/oldMonitor/include/mon/standalone.h>
#include <libs/support/msgHandlers/interact.h>
#include <ctype.h>

#include <libs/support/memMgmt/mem.h>
#include <libs/support/strings/rn_string.h>

#include <libs/support/database/context.h>

#include <libs/ipAnalysis/interface/IPQuery.h>
#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <sys/file.h>
#include <include/maxdefs.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <string.h>

#include <execs/f2i/f2i_options.h>
#include <libs/f2i/ai.h>

#include <libs/support/msgHandlers/ErrorMsgHandler.h>
#include <libs/support/file/UnixFile.h>

#include <libs/fileAttrMgmt/composition/Composition.h>
#include <libs/fileAttrMgmt/composition/CompositionIterators.h>

#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>
#include <libs/fileAttrMgmt/fortranModule/FortTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/FortTextTreeModAttr.h>

#include <libs/ipAnalysis/callGraph/CallGraph.h>

#include <iostream.h>

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

extern char *f2i_program;             /* composition to optimize */
extern char *f2i_module;
extern char *f2i_module_list;

C_CallGraph ProgramCallGraph;

/************************************************************************/
/*                                                                      */
/*   Function:  f2iFile                                                 */
/*                                                                      */
/************************************************************************/
    
void f2iFile(FortranModule *module,
	     char          *FileName)
{
  FortTreeModAttr*      ftAttr = ATTACH_ATTRIBUTE(module, FortTreeModAttr);
  FortTextTreeModAttr*  fttAttr = ATTACH_ATTRIBUTE(module, FortTextTreeModAttr);
  FortTree              ft = ftAttr->ft;
  FortTextTree          ftt = fttAttr->ftt;
  
  module->DisableAttributeCaching();
  ai((Context)module,ft,ftt,FileName);
  module->Close();
  
}

int f2iMain(int argc, char **argv)
{
  Boolean has_errors;
  FILE    *fd;
  DB_FP*           stderr_fp;
  char             buffer[MAXPATHLEN];
  FortranModule *module;

  stderr_fp = db_convert_fp_to_db_fp(stderr);
  if (f2i_module)
    {
      module = new FortranModule;
      if (module->Open(f2i_module) == 0)
	f2iFile(module,(char*)NULL);
      else
	errorMsgHandler.HandleMsg("Module %s not found.\n", f2i_module);
    }
  else if (f2i_program) 
    {
      Composition *comp = new Composition;
      if (comp->Open(f2i_program) != 0 ||
	  comp->IsCompleteAndConsistent() == false) 
	{
	  errorMsgHandler.HandleMsg
	    ("Not using program context %s, since it contains errors\n",
	     f2i_program);
	}
      else
	{ 
	  ProgramCallGraph = IPQuery_Init((Context)comp);
	  for(CompModulesIterator modules(comp);
	      module = (FortranModule *)modules.Current(); 
	      ++modules) 
	    {
	      if (module->Open(module->ReferenceFilePathName()) != 0)
		f2iFile(module,(char *)module->ReferenceFilePathName());
	      else
		errorMsgHandler.HandleMsg("Module %s not found.\n",
					  module->ReferenceFilePathName());
	    }
	  IPQuery_Fini(ProgramCallGraph);
	}
      delete comp;
    } 
  else /* f2i_module_list */
    {


      istrstream ModuleList(f2i_module_list);
      if (!ModuleList)
	{
	  cerr << "The module list file " << f2i_module_list <<  "could not be opened..."
	       << endl;
	  return -1;
	}
      f2i_module = (char *)new char[512];
      while (ModuleList >> f2i_module)
	{
	  module = new FortranModule;
	  if (module->Open(f2i_module) != 0)
	    f2iFile(module,f2i_module);
	  else
	    errorMsgHandler.HandleMsg("Module %s not found.\n", f2i_module);
	  delete module;
	}
      delete f2i_module;
    }
  
  return 0; // JA2I 2/93
}

int main(int argc, char **argv)
{
  int ret;
  Boolean answer;
  
  ret = runRoot(argc, (char **)argv,f2i_init_options , f2iMain);
  return ret;
}

  
  
   
