/* $Id: Contents.h,v 1.4 1997/06/25 13:43:04 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      ned_cp/FortAnnot/Contents.h                                     */
/*                                                                      */
/*      Contents  -  Table-of-contents annotation source                */
/*      Last edited: August 25, 1993 at 3:52 pm				*/
/*                                                                      */
/************************************************************************/




#ifndef Contents_h
#define Contents_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotPI.h>






/************************************************************************/
/*      Procedural Interface                                            */
/************************************************************************/




EXTERN (void, ContentsAnnot_Init, (void));


EXTERN (void, ContentsAnnot_Fini, (void));






/************************************************************************/
/*      Object-oriented Interface                                       */
/************************************************************************/


#ifdef __cplusplus




/************************/
/* ContentsSrc Class    */
/************************/




struct ContentsSrc_Repr_struct;


class ContentsSrc: public FortAnnotSrc
{
  public:

    ContentsSrc_Repr_struct * ContentsSrc_repr;


  public:

    /* initialization */

      META_DEF(ContentsSrc)

      ContentsSrc(Context context, DB_FP * fp, FortAnnotMgr * fam,
                  Context module);

      virtual
      ~ContentsSrc(void);


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






/*************************/
/* TOCSubprogAnnot Class */
/*************************/




struct TOCSubprogAnnot_Repr_struct;


class TOCSubprogAnnot: public CompoundFortAnnot
{
  public:

    TOCSubprogAnnot_Repr_struct * TOCSubprogAnnot_repr;


  public:

    /* initialization */

      META_DEF(TOCSubprogAnnot)

      TOCSubprogAnnot(char * name, ContentsSrc * toc, Boolean sorted);

      virtual
      ~TOCSubprogAnnot(void);


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






/************************/
/* TOCCommonAnnot Class */
/************************/




struct TOCCommonAnnot_Repr_struct;


class TOCCommonAnnot: public CompoundFortAnnot
{
  public:

    TOCCommonAnnot_Repr_struct * TOCCommonAnnot_repr;


  public:

    /* initialization */

      META_DEF(TOCCommonAnnot)

      TOCCommonAnnot(char * name, ContentsSrc * toc, Boolean sorted);

      virtual
      ~TOCCommonAnnot(void);


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






/***********************/
/* TOCErrorAnnot Class */
/***********************/




struct TOCErrorAnnot_Repr_struct;


class TOCErrorAnnot: public CompoundFortAnnot
{
  public:

    TOCErrorAnnot_Repr_struct * TOCErrorAnnot_repr;


  public:

    /* initialization */

      META_DEF(TOCErrorAnnot)

      TOCErrorAnnot(char * name, ContentsSrc * toc, Boolean sorted, int first,
                    int last);

      virtual
      ~TOCErrorAnnot(void);


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

#endif /* not Contents_h */
