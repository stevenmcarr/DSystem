/* $Id: Declarations.h,v 1.4 1997/06/25 13:43:04 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      ned_cp/FortAnnot/Declarations.h                                 */
/*                                                                      */
/*      Declarations  -  Table-of-contents annotation source            */
/*      Last edited: August 25, 1993 at 3:53 pm				*/
/*                                                                      */
/************************************************************************/




#ifndef Decl_h
#define Decl_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotPI.h>






/************************************************************************/
/*      Procedural Interface                                            */
/************************************************************************/




EXTERN (void, DeclAnnot_Init, (void));


EXTERN (void, DeclAnnot_Fini, (void));






/************************************************************************/
/*      Object-oriented Interface                                       */
/************************************************************************/


#ifdef __cplusplus




/********************/
/* DeclSrc Class    */
/********************/




struct DeclSrc_Repr_struct;


class DeclSrc: public FortAnnotSrc
{
  public:

    DeclSrc_Repr_struct * DeclSrc_repr;


  public:

    /* initialization */

      META_DEF(DeclSrc)

      DeclSrc(Context context, DB_FP * fp, FortAnnotMgr * fam, Context module);

      virtual
      ~DeclSrc(void);


    /* database */

      virtual
      void Open(Context module,
                Context mod_in_pgm_context,
                Context pgm_context,
                DB_FP * session_fp);

      virtual
      void Close(void);

      virtual
      void Save(Context context, DB_FP * fp);


    /* change notification */

      virtual
      void NoteChange(Object * ob, int kind, void * change);


    /* selection */

      virtual
      void GetGlobal(CompoundFortAnnot * fan);

      virtual
      void GetSelection(CompoundFortAnnot * fan, int l1, int c1,
                        int l2, int c2);
};






/*******************/
/* DeclAnnot Class */
/*******************/




struct DeclAnnot_Repr_struct;


class DeclAnnot: public CompoundFortAnnot
{
  public:

    DeclAnnot_Repr_struct * DeclAnnot_repr;


  public:

    /* initialization */

      META_DEF(DeclAnnot)

      DeclAnnot(DeclSrc * dcl, FortTreeNode node);

      virtual
      ~DeclAnnot(void);


    /* creation */

      virtual
      void Init(void);

      virtual
      void Destroy(void);


  protected:

    /* realize annotation */

      virtual
      void realize(void);

      virtual
      void realizeIdentifier(FortTreeNode node);
};






/***********************/
/* DeclLeafAnnot Class */
/***********************/




class DeclLeafAnnot: public SimpleFortAnnot
{
  public:

    DeclAnnot_Repr_struct * DeclAnnot_repr;


  public:

    /* initialization */

      META_DEF(DeclLeafAnnot)

      DeclLeafAnnot(char * name, DeclSrc * dcl, Boolean sorted,
                    FortTreeNode node);

      virtual
      ~DeclLeafAnnot(void);


    /* creation */

      virtual
      void Init(void);

      virtual
      void Destroy(void);


  protected:

    /* realize annotation */

      virtual
      void realize(void);
};






#endif /* __cplusplus */

#endif /* not Decl_h */
