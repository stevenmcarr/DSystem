/* $Id: DedCP.C,v 1.2 1997/03/11 14:30:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/DedCP.C							*/
/*									*/
/*	DedCP -- Ded Command Processor					*/
/*	Last edited: November 10, 1993 at 11:50 pm			*/
/*									*/
/************************************************************************/


#include <unistd.h>
#include <sys/param.h>

#include <libs/support/msgHandlers/ErrorMsgHandler.h>

#include <libs/fileAttrMgmt/composition/Composition.h>
#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>

#include <libs/graphicInterface/cmdProcs/dEditor/DedCP.h>

#include <libs/graphicInterface/cmdProcs/dEditor/DedEditor.h>
#include <libs/graphicInterface/framework/SplitView.h>
#include <libs/graphicInterface/cmdProcs/dEditor/DedCP_opt.i>
#include <libs/support/file/UnixFile.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* DedCP object */

typedef struct DedCP_Repr_struct
  {
    DedEditor *	editor;

  } DedCP_Repr;


#define R(ob)		(ob->DedCP_repr)


#define INHERITED	EditorCP






/************************/
/* View filter commands */
/************************/




static Generic dcp_defineFilterCmds[] =
  {
    CMD_DEF_NAV_FILTER,
    CMD_DEF_SRC_FILTER,
    CMD_DEF_DEP_FILTER,
    CMD_DEF_COMM_FILTER,
    CMD_DEF_DIST_FILTER
  };
  
  
static Generic dcp_setFilterCmds[] =
  {
    CMD_SET_NAV_FILTER,
    CMD_SET_SRC_FILTER,
    CMD_SET_DEP_FILTER,
    CMD_SET_COMM_FILTER,
    CMD_SET_DIST_FILTER
  };






/*************************/
/*  Forward declarations */
/*************************/




static Boolean choosePane(char * popupTitle, int &pane);






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/**************************/
/*  Startup of executable */
/**************************/




#if 0
int dedcp_Edit(int argc, char **argv)
{
  Boolean accessible;
  int len;
  char buffer[200];
  char *mod_name;
  char *pgm_name;
  EditorCPStartupStruct startup;
  extern short ded_cp_index;

  mod_name = ded_cp_module_name();

  /* compute module context */
    /* verify that the source file can be read or created */
      accessible = file_access(mod_name, R_OK);
      
      if( ! accessible )
        { (void) file_touch(mod_name);
          accessible =  file_access(mod_name, R_OK);
        }
        
      if( ! accessible )
        { fprintf(stderr,
                  "Module %s is unreadable, or doesn't exist and can't be created.\n",
                  mod_name);
          return -1;
        }

    /* create the context */
      startup.mod_context = ctxAlloc(ObjectFortSrc, mod_name);
      if( startup.mod_context == CONTEXT_NULL )
        { fprintf(stderr, "Can't create module context for %s.\n", mod_name);
          return -1;
        }

  /* compute program context */
    pgm_name = ded_cp_program_name();
    
  /* missing program name => use module name with suffix ".comp" */
    if( pgm_name == NULL )
      { strcpy(buffer, mod_name);
              
        /* remove trailing ".f" if present */
          len = strlen(buffer);
          if( buffer[len-2] == '.' && buffer[len-1] == 'f' )
            buffer[len-2] = '\0';
            
        strcat(buffer, ".comp");
        
        pgm_name = buffer;
      }
      
    startup.pgm_context = ctxAlloc(ObjectFortComp, pgm_name);

  /* start the cp */
    if( cp_new((anInstance*)cp_root_cp_id(), ded_cp_index, (Generic)&startup) == UNUSED )
      { message("Can't start the Fortran D Editor.");  
        return -1;
      }
    else
      return 0;
}

#endif 


int dedcp_Edit(int argc, char **argv)
{
  extern short ded_cp_index;

  char buffer[MAXPATHLEN];

  char *mod_name = ded_cp_module_name();
  char *pgm_name = ded_cp_program_name();
  
  // missing program name => use module name with suffix ".cmp"
  if (pgm_name == 0) {
    int len = strlen(buffer);
    assert(len < (MAXPATHLEN + 5));

    strcpy(buffer, mod_name);
    
    /* remove trailing ".f" if present */
    if( buffer[len-2] == '.' && buffer[len-1] == 'f' )
      buffer[len-2] = '\0';
            
    strcat(buffer, ".cmp");
    if (file_access(buffer, R_OK)) pgm_name = buffer;
  }

  EditorCPStartupStruct startup;

  Composition *comp = 0;
  if (pgm_name) {
    comp = new Composition;
    if (comp->Open(pgm_name) != 0 || 
	comp->IsCompleteAndConsistent() == false) {
      errorMsgHandler.HandleMsg
	("Not using program context %s, since it contains errors\n",
	 pgm_name);
      delete comp;
      comp = 0;
    } 
  }
  startup.pgm_context = comp;
  
  if (comp) {
    OrderedSetOfStrings *names = comp->LookupModuleNameBySuffix(mod_name);
    int numNames = names->NumberOfEntries();
    switch(numNames) {
    case 0:
      errorMsgHandler.HandleMsg
	("Module %s not found in composition %s.\n", mod_name, pgm_name);
      delete names;
	  return -1;
    case 1: 
      {
	char *filename = (*names)[0];
	if (!file_access(filename, R_OK)) {
	  errorMsgHandler.HandleMsg
	    ("Module %s is unreadable, or does not exist.\n", filename);
	  delete names;
	  return -1;
	}
	startup.mod_context = (FortranModule *) comp->GetModule(filename);
	delete names;
	assert(startup.mod_context != 0);
	break;
      }
    default: 
      {
	errorMsgHandler.HandleMsg
	  ("Module %s does not uniquely specify a member of composition %s.\n\
Members found: ", mod_name, pgm_name);
	for (; numNames--;) {
	  errorMsgHandler.HandleMsg("%s ", (*names)[numNames]);
	}
	delete names;
	return -1;
      }
    }
  } else {

    /* force the specified file to exist, if it doesn't already */

    if (!file_access(mod_name, R_OK)) { 
      // cannot read the file, can we create it?
	(void) file_touch(mod_name);
	if (!file_access(mod_name, R_OK)) {
	  errorMsgHandler.HandleMsg
	    ("Module %s is unreadable, or does not exist and cannot be created.\n", mod_name);
	    return -1;
	}
      }

    startup.mod_context = new FortranModule;
    if (startup.mod_context->Open(mod_name) != 0) {
      errorMsgHandler.HandleMsg
	("Errors encountered in creating context for %s.\n", mod_name);
      delete startup.mod_context; 
      return -1;
    }
  }

#if 0
  //----------------------------------------------------------------
  // while editting, we want explicit control over saves so that
  // values are not saved except at the request of the user
  //----------------------------------------------------------------
  startup.mod_context->DisableAttributeCaching();
#endif
  
  if (cp_new((anInstance*)cp_root_cp_id(), ded_cp_index, (Generic) &startup) 
      == UNUSED)
  {
     errorMsgHandler.HandleMsg("Can't start the Fortran D Editor.");  
    return -1;
  }
  else
    return 0;
}






/*******************/
/*  CP declaration */
/*******************/




CP_DECLARATION(DedCP,
               "Fortran D editor",
               false,
               DedCP_Processor,
               EditorCP_startup)






/*************************/
/*  Class initialization */
/*************************/




void DedCP::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(EditorCP);
    REQUIRE_INIT(DedEditor);
}




void DedCP::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(DedCP)




DedCP::DedCP(Generic parent_id, Generic cp_id, void * startup)
     : EditorCP(parent_id, cp_id, startup)
{
  /* allocate instance's private data */
    this->DedCP_repr = (DedCP_Repr *) get_mem(sizeof(DedCP_Repr),
                                              "DedCP instance");
}




DedCP::~DedCP(void)
{
  /* destroy instance's private data */
    free_mem((void*) this->DedCP_repr);
}




void DedCP::Init(void)
{
  this->INHERITED::Init();
}




void DedCP::Fini(void)
{
  this->INHERITED::Fini();
}




Editor * DedCP::openEditor(Context context, 
                           Context mod_in_pgm_context,
                           Context pgm_context,
                           DB_FP * session_fd)
{
  R(this)->editor = new DedEditor(context, session_fd);
  R(this)->editor->Open(context, mod_in_pgm_context, pgm_context, session_fd, nil);

  return R(this)->editor;
}




void DedCP::closeEditor(void)
{
  R(this)->editor->Close();
}






/*************/
/*  Database */
/*************/




void DedCP::GetSessionAttribute(char * &session_attr, int &oldest, int &newest)
{
  session_attr = "DedSession";
  oldest       = 1;
  newest       = 1;
}






/*******************/
/*  Window layout  */
/*******************/




char * DedCP::GetWindowTitleFormat(void)
{
  return "Fortran D Editor: %s %s";
}






/*************************/
/*  Subordinate windows  */
/*************************/




void DedCP::CloseAuxWindow(Window * window)
{
  /* TEMPORARY -- disable closing source pane */

  /* nothing */
}






/***********/
/*  Menus  */
/***********/




void DedCP::InitMenuBar(MenuBar * mb)
{
  this->INHERITED::InitMenuBar(mb);

#if 0
  mb->AddMenu("dependence",   UNUSED, "");
  mb->AddMenu("distribution", UNUSED, "");
  mb->AddMenu("transform",    UNUSED, "");
#endif
}





void DedCP::InitMenu(char * name, CMenu * m)
{
  if( ! strcmp("view", name) == 0 )
    this->INHERITED::InitMenu(name, m);

  if( strcmp("edit", name) == 0 )
    {
      m->AddItem(CMD_ANALYZE,       KB_Nul, "compile", "", UNUSED, 0);
    }

  if( strcmp("view", name) == 0 )
    {
      m->AddItem(CMD_DEF_FILTER,    KB_Nul, "define filter...",    "", UNUSED, 0);
      m->AddItem(CMD_SET_FILTER,    KB_Nul, "set filter...",       "", UNUSED, 0);
      m->AddItem(CMD_ARROWS,        KB_Nul, "show arrows...",      "", UNUSED, 0);
    }
}






/********************/
/*  Input handling  */
/********************/




Boolean DedCP::MenuChoice(Generic cmd)
{
  Boolean handled = true;
  Generic pane;
  
  switch( cmd )
    {
      case CMD_DEF_FILTER:
        if( choosePane("Define Filter", pane) )
          handled = R(this)->editor->MenuChoice(dcp_defineFilterCmds[pane]);
        break;
        
      case CMD_SET_FILTER:
        if( choosePane("Set Filter", pane) )
          handled = R(this)->editor->MenuChoice(dcp_setFilterCmds[pane]);
        break;
        
      default:
        handled = R(this)->editor->MenuChoice(cmd);
        if( ! handled )
          handled = this->INHERITED::MenuChoice(cmd);
        break;
    }

  return handled;
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
Boolean choosePane(char * popupTitle, int &pane)
{
  CMenu * m;
  Boolean chosen;

  m = new CMenu(popupTitle);
  m->AddItem(0, KB_Nul, "overview",      "", UNUSED, 0);
  m->AddItem(1, KB_Nul, "source code",   "", UNUSED, 0);
  m->AddItem(2, KB_Nul, "dependences",   "", UNUSED, 0);
  m->AddItem(3, KB_Nul, "communication", "", UNUSED, 0);
  m->AddItem(4, KB_Nul, "distribution",  "", UNUSED, 0);
  chosen = m->Select(pane);
  delete m;

  return chosen;
}
