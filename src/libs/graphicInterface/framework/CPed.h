/* $Id: CPed.h,v 1.2 1997/03/11 14:32:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	cped/CPed.h							*/
/*									*/
/*	CPed -- abstract Ped engine					*/
/*	Last edited: October 16, 1993 at 11:21 pm			*/
/*									*/
/************************************************************************/




#ifndef CPed_h
#define CPed_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/DBObject.h>
#include <libs/graphicInterface/framework/Dependence.h>

#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>


#include <libs/frontEnd/prettyPrinter/ft2text.h>
#include <libs/moduleAnalysis/dependence/interface/depType.h>




#define CPed_Attribute    "CPed"




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/***************/
/* Dependences */
/***************/


typedef enum
  {
    pedDepSortSource,
    pedDepSortSink,
    pedDepSortType,
    pedDepSortDim,
    pedDepSortBlock,

    pedDepSortSourceReverse,
    pedDepSortSinkReverse,
    pedDepSortTypeReverse,
    pedDepSortDimReverse,
    pedDepSortBlockReverse

  } PedDependenceOrder;




/*************/
/* Variables */
/*************/


typedef enum
  {
    pedVarShared,
    pedVarPrivate

  } PedVariableKind;


typedef struct
  {
    char *		name;
    int			dim;
    char *		block;
    FortTreeNode	defBefore;
    FortTreeNode	useAfter;
    PedVariableKind	kind;
    Boolean		user;

  } PedVariable;


typedef enum
  {
    pedVarSortName,
    pedVarSortDim,
    pedVarSortBlock,
    pedVarSortDefBefore,
    pedVarSortUseAfter,
    pedVarSortKind,

    pedVarSortNameReverse,
    pedVarSortDimReverse,
    pedVarSortBlockReverse,
    pedVarSortDefBeforeReverse,
    pedVarSortUseAfterReverse,
    pedVarSortKindReverse

  } PedVariableOrder;




/**************/
/* CPed class */
/**************/




struct CPed_Repr_struct;
class  CFortEditor;




class CPed: public DBObject
{
public:

  CPed_Repr_struct * CPed_repr;


public:

/* class initialization */
  static void			InitClass(void);
  static void			FiniClass(void);

/* initialization */
  META_DEF(CPed)
				CPed(Context context, DB_FP * session_fd);
  virtual			~CPed(void);
  virtual void			SetEditor(CFortEditor * editor);

/* database */
  virtual void			GetAttribute(char * &attr);
  virtual void			Open(Context context,
		                     Context mod_in_pgm_context,
		                     Context pgm_context,
		                     DB_FP * session_fd,
		                     FortTextTree ftt);
  virtual void			Save(Context context, DB_FP * session_fd);

/* exporting */
  virtual void			Export(void);

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
  virtual void			SortDependences(PedDependenceOrder order);

/* access to variables */
  virtual int			NumVariables(void);
  virtual void 			GetVariable(int k, PedVariable &var);
  virtual void			GetVariableTexts(int k,
                                                 char * &name,
                                                 char * &dim,
                                                 char * &block,
                                                 char * &defBefore,
                                                 char * &useAfter,
                                                 char * &kind,
                                                 char * &user);
  virtual void			MakeVarShared(int k);
  virtual void			MakeVarPrivate(int k);
  virtual void			SortVariables(PedVariableOrder order);

/* loops */
  virtual void			SetCurrentLoop(FortTreeNode node);
  virtual FortTreeNode		GetCurrentLoop(void);

/* change notification */
  virtual void			NoteChange(Object * ob, int kind, void * change);
  virtual void			Analyze(void);

/* access to internals */
  virtual void *		getPed(void);
  virtual Generic		getFortEditor(void);

};




#endif /* __cplusplus */

#endif /* not CPed_h */
