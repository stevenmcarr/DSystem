/* $Id: DedDocument.h,v 1.2 1997/03/11 14:30:09 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	ded_cp/DedDocument.h						*/
/*									*/
/*	DedDocument -- contents of DedEditor				*/
/*	Last edited: November 6, 1993 at 2:00 pm			*/
/*									*/
/************************************************************************/




#ifndef DedDocument_h
#define DedDocument_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/DBObject.h>
#include <libs/graphicInterface/framework/Dependence.h>
#include <libs/graphicInterface/framework/CPed.h>

#include <libs/frontEnd/fortTree/FortTree.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/fortD/codeGen/FortDInterface.h>




/* miscellaneous constants */

#define DedDocument_Attribute    "DedDocument"

extern Color Ded_GreenColor;
extern Color Ded_YellowColor;
extern Color Ded_RedColor;
extern Color Ded_PurpleColor;

#define CHANGE_FORTD_INFO	9901




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/**********************/
/* Symbolic constants */
/**********************/


enum DedCurDepPolicy
  {
    DedDoc_CURDEP_1,
    DedDoc_CURDEP_CROSS,
    DedDoc_CURDEP_ALL,
    DedDoc_CURDEP_SEL
  };




/*********************/
/* DedDocument class */
/*********************/




struct DedDocument_Repr_struct;
class  Text;
class  CFortEditor;




class DedDocument: public DBObject
{
public:

  DedDocument_Repr_struct * DedDocument_repr;

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);
  
/* initialization */
  META_DEF(DedDocument)
				DedDocument(Context context, DB_FP * session_fd);
  virtual			~DedDocument(void);
  virtual void			SetEditor(CFortEditor * editor);

/* database */
  virtual void			GetAttribute(char * &attr);
  virtual void			Open(Context context,
		                     Context mod_in_pgm_context,
		                     Context pgm_context,
		                     DB_FP * fd);
  virtual void			Save(Context context, DB_FP * fd);

/* change notification */
  virtual void			NoteChange(Object * ob, int kind, void * change);
  virtual void			Analyze(void);

/* access to source */
  virtual void			GetSource(FortTextTree &ftt, FortTree &ft);

/* access to loops */
  virtual void			SetCurrentLoop(FortTreeNode node);
  virtual FortTreeNode		GetCurrentLoop(void);
  virtual Loop_type		GetLoopType(FortTreeNode loop);
  virtual Color			GetLoopColor(FortTreeNode loop);

/* access to dependences */
  virtual int			NumDependences(void);
  virtual void *		GetDependenceEdge(int k);
  virtual void			GetDependence(int k, Dependence &dep);
  virtual void			GetDependenceTexts(int k,
                                                   char * &type,
                                                   char * &src,
                                                   char * &sink,
                                                   char * &vector,
                                                   char * &level,
                                                   char * &block,
                                                   char * &stmt);
  virtual Boolean		IsDependenceCrossProcessor(void * edge);
  virtual Color			GetDependenceColor(void * edge);
  virtual void			SortDependences(PedDependenceOrder order);

/* access to communication */
  virtual int			NumMessages(void);
  virtual void			GetMessage(int k, FortDMesg * &msg);

/* access to statements */
  virtual Stmt_type		GetStatementType(FortTreeNode stmt);
  virtual Color			GetStatementColor(FortTreeNode stmt);

/* access to variables */
  virtual int			NumVariables(void);
  virtual void			GetVariable(int k, PedVariable &var, FortDRef * &ref);
  virtual void			GetVariableTexts(int k,
                                                 char * &name,
                                                 char * &dim,
                                                 char * &block,
                                                 char * &defBefore,
                                                 char * &useAfter,
                                                 char * &kind,
                                                 char * &user);
  virtual void			GetVariableRefs(int k, FortDAstSet * &refs);
  virtual void			SortVariables(PedVariableOrder order);

/* access to references */
  virtual int			NumReferences(void);
  virtual void			GetReference(int k, FortDRef * &ref);
  virtual Boolean		IsRefNonlocal(FortTreeNode ref);
  virtual Color			GetRefColor(FortTreeNode ref);


private:
  FortDRef *			findRefForVar(PedVariable &var);

};




#endif /* __cplusplus */

#endif /* not DedDocument_h */
