/* $Id: DedDocument.C,v 1.2 1997/03/11 14:30:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/DedDocument.C						*/
/*									*/
/*	DedDocument --  contents of DedEditor				*/
/*	Last edited: November 11, 1993 at 1:25 am			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/dEditor/DedDocument.h>

#include <libs/graphicInterface/framework/CPed.h>

#include <libs/graphicInterface/framework/Dependence.h>
#include <libs/fileAttrMgmt/composition/Composition.h>
#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/fortD/codeGen/FortDInterface.h>
#include <libs/fortD/localInfo/fd_symtable.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/pedCP/PedPrivate.h>

#include <libs/fileAttrMgmt/fortranModule/FortranModule.h>
#include <libs/fileAttrMgmt/fortranModule/FortTreeModAttr.h>
#include <libs/fileAttrMgmt/fortranModule/FortTextTreeModAttr.h>
#undef R

#include <unistd.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* DedDocument object */

typedef struct DedDocument_Repr_struct
  {
    /* creation parameters */
      FortTreeModAttr *ftAttr;
      FortTextTree	ftt;

      FortTextTreeModAttr *fttAttr;
      FortTree		ft;

    /* Fortran D compiler stuff */
      CPed *		cped;
      CallGraph *	cg;
      FortDInterface *	fortD;
      FortDProg *	fortD_prog;
      
    /* current loop details */
      FortTreeNode	curLoopNode;
      FortDLoop *	curLoopInfo;
      FortDMesgSet *	curMessages;
      FortDAstSet *	curRefs;

      Flex *		varMap;

  } DedDocument_Repr;




#define R(ob)		(ob->DedDocument_repr)
#define INHERITED	DBObject






/*************************/
/*  Miscellaneous	 */
/*************************/




/* object destroyer -- can't be a function because FortDIntf not single-rooted */

#define DELETE_IF(ob)	{if( ob != nil )  delete ob;}






/*************************/
/*  Forward declarations */
/*************************/




static Color dedColor(Color_type fd_color);

static void filterVariables(CPed * cped, Flex * map);






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




Color Ded_GreenColor, Ded_YellowColor, Ded_RedColor, Ded_PurpleColor;




void DedDocument::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(DBObject);
    REQUIRE_INIT(CPed);

  /* initialize famous colors */
    Ded_GreenColor  = color("green3");
    Ded_YellowColor = color("gold2");
    Ded_RedColor    = color("red2");
    Ded_PurpleColor = color("purple3");
}




void DedDocument::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(DedDocument)




DedDocument::DedDocument(Context context, DB_FP * session_fd)
           : DBObject (context, session_fd)
{
  /* allocate instance's private data */
    this->DedDocument_repr =
        (DedDocument_Repr *) get_mem(sizeof(DedDocument_Repr),
                                     "DedDocument instance");

  /* initialize status */
    R(this)->curLoopNode = nil;
    R(this)->curLoopInfo = nil;
    R(this)->curMessages = nil;
    R(this)->curRefs     = nil;

  /* initialize variable filtering map */
    R(this)->varMap = flex_create(sizeof(int));
}




DedDocument::~DedDocument()
{
  DELETE_IF(R(this)->curLoopInfo);

#if 0
  // the code below has been ifdefed out since purify shows that (at least 
  // some) of the current messages have already been freed by freeing 
  // curLoopInfo above
  // 6 September 1994 -- John Mellor-Crummey
  DELETE_IF(R(this)->curMessages);

  DELETE_IF(R(this)->curRefs);
#endif
  delete R(this)->fortD_prog;
  delete R(this)->fortD;

  ((Composition *) R(this)->cg->program)->DetachAttribute(R(this)->cg);

  delete R(this)->cped;

  flex_destroy(R(this)->varMap);

  free_mem((void*) this->DedDocument_repr);
}




void DedDocument::SetEditor(CFortEditor * editor)
{
  R(this)->cped->SetEditor(editor);
}






/*************/
/*  Database */
/*************/




void DedDocument::GetAttribute(char * &attr)
{
  attr = DedDocument_Attribute;
}




void DedDocument::Open(Context context,
		       Context mod_in_pgm_context,
		       Context pgm_context,
		       DB_FP * session_fd)
{
  this->INHERITED::Open(context, mod_in_pgm_context, pgm_context, session_fd);

  
  /* open the source module */
  R(this)->ftAttr = ATTACH_ATTRIBUTE(context, FortTreeModAttr);
  R(this)->ft  = R(this)->ftAttr->ft;

  R(this)->fttAttr = ATTACH_ATTRIBUTE(context, FortTextTreeModAttr);
  R(this)->ftt = R(this)->fttAttr->ftt;

    ft_AstSelect(R(this)->ft);
    if( NOT( ft_Check(R(this)->ft) ) )
      { message("Fortran D Editor: The module contains errors.\n");
	exit(-1);
      }

  /* create a Fortran D compiler instance */
    R(this)->cped = new CPed(context, session_fd);
    R(this)->cped->Open(context, mod_in_pgm_context, pgm_context, session_fd, R(this)->ftt);

    R(this)->cg = 
      (CallGraph *) pgm_context->AttachAttribute(CLASS_NAME(CallGraph));

    R(this)->fortD = new FortDInterface(R(this)->cped->getPed(),
                                        ft_Root(R(this)->ft),
                                        context,
                                        R(this)->cg, R(this)->ft);
                                        
    R(this)->fortD_prog = R(this)->fortD->GetProg(ft_Root(R(this)->ft));
    
  /* set up change notification */
    R(this)->cped->Notify(this, true);
}




void DedDocument::Save(Context context, DB_FP * fd)
{
  this->INHERITED::Save(context, fd);
}






/***********************/
/* Change notification */
/***********************/




void DedDocument::NoteChange(Object * ob, int kind, void * change)
{
  NoteChangeTrace trace(this, ob, kind, change);

  switch( kind )
    {
      default:
        this->INHERITED::NoteChange(ob, kind, change);
    }

  if( ob == R(this)->cped )
    this->Changed(kind, change);
  else
    R(this)->cped->NoteChange(ob, kind, change);
}




void DedDocument::Analyze(void)
{
  R(this)->cped->Analyze();
  R(this)->fortD->Analyze();
  this->Changed(CHANGE_FORTD_INFO, nil);
}






/********************/
/* Access to source */
/********************/




void DedDocument::GetSource(FortTextTree &ftt, FortTree &ft)
{
  ftt = R(this)->ftt;
  ft  = R(this)->ft;
}






/*******************/
/* Access to loops */
/*******************/




void DedDocument::SetCurrentLoop(FortTreeNode node)
{
  R(this)->curLoopNode = node;
  R(this)->cped->SetCurrentLoop(node);

  /* get new loop info */
    DELETE_IF(R(this)->curLoopInfo);
    R(this)->curLoopInfo = (node == AST_NIL ? nil : R(this)->fortD->GetLoop(R(this)->curLoopNode));

  /* get new message info */
    /* 'curMessages' is deleted implicitly along with 'curLoopInfo' */
    R(this)->curMessages = (node == AST_NIL ? nil : R(this)->curLoopInfo->Mesgs());

  /* get new references info */
    /* 'curRefs' is deleted implicitly along with 'curLoopInfo' */    /* ??? */
    R(this)->curRefs = (node == AST_NIL ? nil : R(this)->curLoopInfo->ArrayRefs());

  /* filter the loop's variables to include only arrays */
    filterVariables(R(this)->cped, R(this)->varMap);

  this->Changed(CHANGE_LOOP, (void *) node);
}




FortTreeNode DedDocument::GetCurrentLoop(void)
{
  return R(this)->curLoopNode;
}




Loop_type DedDocument::GetLoopType(FortTreeNode loop)
{
  FortDLoop * loopInfo;
  Loop_type loopType;

  loopInfo = R(this)->fortD->GetLoop(loop);
  loopType = loopInfo->LoopType();
  delete loopInfo;
  
  return loopType;
}




Color DedDocument::GetLoopColor(FortTreeNode loop)
{
  FortDLoop * loopInfo;
  Loop_type loopType;
  Color_type fd_color;

  /* NB -- there should be a method 'FortDLoop::GetColor' */

  loopInfo = R(this)->fortD->GetLoop(loop);
  loopType = loopInfo->LoopType();
  delete loopInfo;
  
  switch( loopType )
    {
      case FD_LOOP_REPLICATED:	fd_color = FD_RED;	break;
      case FD_LOOP_SEQUENTIAL:	fd_color = FD_RED;	break;
      case FD_LOOP_PIPELINED:	fd_color = FD_YELLOW;	break;
      case FD_LOOP_PARALLEL:	fd_color = FD_GREEN;	break;
      default:			fd_color = FD_BLACK;	break;
    }

  return dedColor(fd_color);
}






/**************************/
/*  Access to dependences */
/**************************/




int DedDocument::NumDependences(void)
{
  return R(this)->cped->NumDependences();
}




void * DedDocument::GetDependenceEdge(int k)
{
  return R(this)->cped->GetDependenceEdge(k);
}




void DedDocument::GetDependence(int k, Dependence &dep)
{
  R(this)->cped->GetDependence(k, dep);
}




void DedDocument::GetDependenceTexts(int k,
                                     char * &type,
                                     char * &src,
                                     char * &sink,
                                     char * &vector,
                                     char * &level,
                                     char * &block,
                                     char * &stmt)
{
  R(this)->cped->GetDependenceTexts(k, type, src, sink, vector, level, block, stmt);
}




Boolean DedDocument::IsDependenceCrossProcessor(void * edge)
{
  return R(this)->fortD_prog->CrossProcDep(edge);
}




Color DedDocument::GetDependenceColor(void * edge)
{
  if( this->IsDependenceCrossProcessor(edge) )
    return dedColor( R(this)->fortD_prog->CrossProcDepColor(edge) );
  else
    return NULL_COLOR;
}




void DedDocument::SortDependences(PedDependenceOrder order)
{
  R(this)->cped->SortDependences(order);
}






/****************************/
/*  Access to communication */
/****************************/




int DedDocument::NumMessages(void)
{
  if( R(this)->curMessages == nil )
    return 0;
  else
    return R(this)->curMessages->Size();
}




void DedDocument::GetMessage(int k, FortDMesg * &msg)
{
  msg = (*(R(this)->curMessages))[k];
}






/*************************/
/*  Access to statements */
/*************************/




Stmt_type DedDocument::GetStatementType(FortTreeNode stmt)
{
  FortDStmt * stmtInfo;
  Stmt_type stmtType;

  stmtInfo = R(this)->fortD->GetStmt(stmt);
  stmtType = stmtInfo->StmtType();
  delete stmtInfo;
  
  return stmtType;
}




Color DedDocument::GetStatementColor(FortTreeNode stmt)
{
  FortDStmt * stmtInfo;
  Color_type fd_color;

  stmtInfo = R(this)->fortD->GetStmt(stmt);
  fd_color = stmtInfo->GetColor();
  delete stmtInfo;
  
  return dedColor(fd_color);
}






/************************/
/*  Access to variables */
/************************/




int DedDocument::NumVariables(void)
{
  return flex_length(R(this)->varMap);
}




void DedDocument::GetVariable(int k, PedVariable &var, FortDRef * &ref)
{
  int kprime;

  flex_get_buffer(R(this)->varMap, k, 1, (char *) &kprime);
  R(this)->cped->GetVariable(kprime, var);
  ref = this->findRefForVar(var);
}




void DedDocument::GetVariableTexts(int k,
                                   char * &name,
                                   char * &dim,
                                   char * &block,
                                   char * &defBefore,
                                   char * &useAfter,
                                   char * &kind,
                                   char * &user)
{
  int kprime;

  flex_get_buffer(R(this)->varMap, k, 1, (char *) &kprime);
  R(this)->cped->GetVariableTexts(kprime, name, dim, block, defBefore, useAfter, kind, user);
}




void DedDocument::GetVariableRefs(int k, FortDAstSet * &refs)
{
  NOT_IMPLEMENTED("DedDocument::GetVariableRefs");
}




void DedDocument::SortVariables(PedVariableOrder order)
{
  R(this)->cped->SortVariables(order);
  filterVariables(R(this)->cped, R(this)->varMap);
}






/*************************/
/*  Access to references */
/*************************/




int DedDocument::NumReferences(void)
{
  if( R(this)->curRefs == nil )
    return 0;
  else
    return R(this)->curRefs->Size();
}




void DedDocument::GetReference(int k, FortDRef * &ref)
{
  ref = R(this)->fortD->GetRef( (*(R(this)->curRefs))[k] );
}




Boolean DedDocument::IsRefNonlocal(FortTreeNode ref)
{
  FortDRef * refInfo;
  Boolean nonlocal;

  refInfo = R(this)->fortD->GetRef(ref);
  nonlocal = refInfo->NonLocalAcc();
  delete refInfo;
  
  return nonlocal;
}




Color DedDocument::GetRefColor(FortTreeNode ref)
{
  FortDRef * refInfo;
  Color_type fd_color;

  refInfo = R(this)->fortD->GetRef(ref);
  fd_color = refInfo->GetColor();
  delete refInfo;
  
  return dedColor(fd_color);
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
Color dedColor(Color_type fd_color)
{
  Color color;
  
  switch( fd_color )
    {
      case FD_RED:	color = Ded_RedColor;		break;
      case FD_YELLOW:	color = Ded_YellowColor;	break;
      case FD_GREEN:	color = Ded_GreenColor;		break;
      default:		color = NULL_COLOR;		break;
    }

  return color;
}




FortDRef * DedDocument::findRefForVar(PedVariable &var)
{
  FortDRef * ref;
  FortDRef * ref_k;
  int num, k;
  char * refname;
  
  if( R(this)->curRefs == nil )
    return nil;
  else
    { ref = nil;
      num = this->NumReferences();
      for( k = 0;  k < num && ref == nil;  k++ )
        { this->GetReference(k, ref_k);
          refname = ((SNODE *) ref_k->GetSymEntry())->id;    /* sigh! */
          if( strcmp(refname, var.name) == 0 )  ref = ref_k;
        }
      return ref;
    }
}




static
void filterVariables(CPed * cped, Flex * map)
{
  int num, k;
  PedVariable var;

  flex_delete(map, 0, flex_length(map));

  num = cped->NumVariables();
  for( k = 0;  k < num;  k++ )
    { cped->GetVariable(k, var);
      if( var.dim > 0 )
        flex_insert_one(map, flex_length(map), (char *) &k);
    }
}
