/* $Id: f2i.C,v 1.1 1997/03/20 19:56:39 carr Exp $ */
/************************************************************************/
/*   File:  a2i.C                                                        */
/*                                                                      */
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <general.h>
#include <mon/standalone.h>
#include <interact.h>
#include <ctype.h>

#include <misc/mem.h>
#include <misc/rn_string.h>

#include <context.h>
#include <fort/fortObject.h>
#include <fort/compositions.h>

#include <IPQuery.h>
#include <CallGraph.h>
#include <sys/file.h>
#include <maxdefs.h>
#include <fort/gi.h>
#include <fort/ast.h>
#include <fort/FortTree.h>
#include <fort/FortTextTree.h>
#include <string.h>

#include <a2i_options.h>
#include <ai.h>

extern char *a2i_program;             /* composition to optimize */
extern char *a2i_module;
extern char *a2i_module_list;

C_CallGraph ProgramCallGraph;

DB_FP   *stderr_fp;

/************************************************************************/
/*                                                                      */
/*   Function:  a2iFile                                                 */
/*                                                                      */
/************************************************************************/
    
void a2iFile(Context m_context,
	     char    *a2i_module)
{
  FortTextTree ftt;
  AST_INDEX root;
  FILE *fd;
  char fn[80];
  FortObject     fo;
  Boolean has_errors;
  
  if (m_context == CONTEXT_NULL)
    {
      fprintf(stderr, "Failed to allocate context for module %s.\n",
	      a2i_module);
      exit(-1);
    }
  
  /* create a database object */
  fo = fo_Open(m_context, false, stderr_fp, &has_errors); 
  
  if (has_errors)
    {
      fprintf(stderr, "Module %s contains errors.\n", a2i_module);
      exit(-1);
    }
  
   
  ai(fo,m_context);
  
  fo_Close(fo);
}

int a2iWrapper(int argc, char **argv)
{
  Options a2i_options("memory compiler options");
  Context p_context;
  Context m_context;
  Composition comp;
  FortObject  fo;
  Boolean has_errors;
  FILE    *fd;

  a2i_init_options(a2i_options);
  
  stderr_fp = db_convert_fp_to_db_fp(stderr);
  if (opt_parse_argv(&a2i_options,0,argc,argv)) {
    a2i_options_usage(argv[0]);
    return -1;
  }
  
  if (!a2i_module) {
    fprintf(stderr, "Specify one of -P, -L and -M\n");
    a2i_options_usage(argv[0]);
    return -1;
  }
  else {
    
    /* Run the a2i on each file in a composition */
    
    if (a2i_program) {
       p_context = ctxAlloc(ObjectFortComp, a2i_program);
       if (p_context == CONTEXT_NULL)
	 {
	  fprintf(stderr, "Failed to allocate context for composition %s.\n",
		  a2i_program);
	  return -1;
	 }
       comp = comp_Open(p_context, false,stderr_fp, &has_errors);
    
       if (has_errors)
	 {
	  fprintf(stderr, "Composition %s contains errors.\n",
		  a2i_program);
	  return -1;
	 }
       else
	 ProgramCallGraph = IPQuery_Init(p_context);
      }
    a2iFile(ctxAlloc(ObjectFortSrc, a2i_module),a2i_module);;
    if (a2i_program)
       IPQuery_Fini(ProgramCallGraph);
    return 0;    
  }
  return 0; // JA2I 2/93
}

int main(int argc, char **argv)
{
  int ret;
  Boolean answer;
  
  ret = runRoot(argc, argv, NULL, a2iWrapper);
  return ret;
}

  
  
   
