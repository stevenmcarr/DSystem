/* $Id: Loops.h,v 1.3 1997/03/11 14:30:46 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      ned_cp/FortAnnot/Loops.h                                        */
/*                                                                      */
/*      Loops  -  Loop-nesting Fortran annotation.                      */
/*      Last edited: June 18, 1993 at 4:00 pm                           */
/*                                                                      */
/************************************************************************/




#ifndef Loops_h
#define Loops_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotPI.h>






/************************************************************************/
/*      Procedural Interface                                            */
/************************************************************************/




EXTERN (void, LoopsAnnot_Init, (void))


EXTERN (void, LoopsAnnot_Fini, (void))






/************************************************************************/
/*      Object-oriented Interface                                       */
/************************************************************************/


#ifdef __cplusplus




/*********************/
/* LoopsSrc Class    */
/*********************/




class LoopsAnnot;


struct LoopsSrc_Repr_struct;


class LoopsSrc: public FortAnnotSrc
{
  public:

    LoopsSrc_Repr_struct * LoopsSrc_repr;


  public:

    /* initialization */

      META_DEF(LoopsSrc)

      LoopsSrc(Context context, DB_FP * fp, FortAnnotMgr * fam, Context module);

      virtual
      ~LoopsSrc(void);


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






/********************/
/* LoopsAnnot Class */
/********************/




struct LoopsAnnot_Repr_struct;


class LoopsAnnot: public SimpleFortAnnot
{
  public:

    LoopsAnnot_Repr_struct * LoopsAnnot_repr;


  public:

    /* initialization */

      META_DEF(LoopsAnnot)

      LoopsAnnot(LoopsSrc * loo, FortTreeNode node);

      virtual
      ~LoopsAnnot(void);


    /* creation */

      virtual
      void Create(void);

      virtual
      void Destroy(void);


  protected:

    /* realize annotation */

      virtual
      void realize(void);
};






#endif /* __cplusplus */

#endif /* not Loops_h */
