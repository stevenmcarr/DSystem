/* $Id: FortAnnotMgr.h,v 1.3 1997/03/11 14:30:43 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      ned_cp/FortAnnot/FortAnnotMgr.h                                 */
/*                                                                      */
/*      FortAnnotMgr - Abstract class for Fortran Annotation Manager.   */
/*      Last edited: August 25, 1993 at 12:07 pm			*/
/*                                                                      */
/************************************************************************/




#ifndef FortAnnotMgr_h
#define FortAnnotMgr_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/framework/DBObject.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotPI.h>






/************************************************************************/
/*      Procedural Interface                                            */
/************************************************************************/




EXTERN (void,   accessModule, (Context module, FortEditor * ed,
                               FortTree *ft, FortTreeNode * root))


EXTERN (void,   gotoLink,     (Context module, FortTreeNode node, 
                               goto_Link kind))






/************************************************************************/
/*      Object-oriented Interface                                       */
/************************************************************************/


#ifdef __cplusplus






/*****************************/
/* Function pointer typedef  */
/*****************************/


class FortAnnotMgr;
class FortAnnotSrc;

typedef FUNCTION_POINTER(FortAnnotSrc *,  fam_MakeSrcFunc, (Context context,
                                                            DB_FP * fp,
                                                            FortAnnotMgr * fam,
                                                            Context module));






/************************************/
/* Fortran Annotation Manager Class */
/************************************/




class FortAnnot;


struct FortAnnotMgr_Repr_struct;


class FortAnnotMgr: public DBObject
{
  public:

    FortAnnotMgr_Repr_struct * FortAnnotMgr_repr;


  public:

    /* initialization */

      META_DEF(FortAnnotMgr)

      FortAnnotMgr(Context context, DB_FP * fp, Context module);

      virtual
      ~FortAnnotMgr(void);


    /* source registration */

      static
      void RegisterSrc(fam_MakeSrcFunc makeSrcFunc);


    /* database */

      virtual
      void Open(Context context, DB_FP * fp, Context module, FortEditorCP edcp,
                fam_GotoFunc gotoFunc, FortEditor ed);

      virtual
      void Close(void);

      virtual
      void Save(Context context, DB_FP * fp);


    /* change notification */

      virtual
      void NoteChange(Object * ob, int kind, void * change);


    /* selection */

      virtual
      FortAnnot * GetGlobal(void);

      virtual
      FortAnnot * GetSelection(int l1, int c1, int l2, int c2);
};






#endif /* __cplusplus */

#endif /* not FortAnnotMgr_h */
