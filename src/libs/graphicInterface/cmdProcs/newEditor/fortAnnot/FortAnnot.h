/* $Id: FortAnnot.h,v 1.4 1997/06/25 13:43:04 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      ned_cp/FortAnnot/FortAnnot.h                                    */
/*                                                                      */
/*      FortAnnot - Abstract class for Fortran Annotation.              */
/*      Last edited: August 25, 1993 at 3:53 pm				*/
/*                                                                      */
/************************************************************************/




#ifndef FortAnnot_h
#define FortAnnot_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotPI.h>






/************************************************************************/
/*      Procedural Interface                                            */
/************************************************************************/




EXTERN (void, FortAnnot_Init,   (void));


EXTERN (void, FortAnnot_Fini,   (void));


EXTERN (char *, getLine,        (FortEditor ed, FortTreeNode node,
                                 goto_Link kind));






/************************************************************************/
/*      Object-oriented Interface                                       */
/************************************************************************/


#ifdef __cplusplus






/******************************/
/* Function pointer typedefs  */
/******************************/


class FortAnnotSrc;
class FortAnnot;

typedef FUNCTION_POINTER(FortAnnot *, fan_MakeAnnotFunc, (char * name,
                                                          FortAnnotSrc * source,
                                                          Boolean sorted,
                                                          void * data));






/****************************/
/* Fortran Annotation Class */
/****************************/




struct FortAnnot_Repr_struct;


class FortAnnot: public Object
{
  public:

    FortAnnot_Repr_struct * FortAnnot_repr;


  public:

    /* initialization */

      META_DEF(FortAnnot)

      FortAnnot(char * name, FortAnnotSrc * source, Boolean compound,
                Boolean sorted);

      virtual
      ~FortAnnot(void);


    /* creation */

      virtual
      void Init(void);

      virtual
      void Destroy(void);


    /* access to annotations */

      virtual
      void GetName(char * *name);

      virtual
      Boolean IsCompound(void);


  protected:

    /* realize annotation */

      virtual
      void realize(void);

      virtual
      void ensureRealize(void);
};






class SimpleFortAnnot: public FortAnnot
{
  public:

    /* initialization */

      META_DEF(SimpleFortAnnot)

      SimpleFortAnnot(char * name, FortAnnotSrc * source,
                      Boolean sorted);

      virtual
      ~SimpleFortAnnot(void);


    /* creation */

      virtual
      void Init(void);

      virtual
      void Destroy(void);


    /* access to text */

      virtual
      void AddTextLine(char * text, Context module,
                       FortTreeNode node, goto_Link kind);

      virtual
      int NumTextLines(void);

      virtual
      void GetTextLine(int k, char * *text);

      virtual
      Boolean HasLink(int k);

      virtual
      void GotoLink(int k);
};






class CompoundFortAnnot: public FortAnnot
{
  public:

    /* initialization */

      META_DEF(CompoundFortAnnot)

      CompoundFortAnnot(char * name, FortAnnotSrc * source,
                        Boolean sorted);

      virtual
      ~CompoundFortAnnot(void);


    /* creation */

      virtual
      void Init(void);

      virtual
      void Destroy(void);


    /* access to elements */

      virtual
      void AddElement(FortAnnot * elem);

      virtual
      int NumElements(void);

      virtual
      FortAnnot * GetElement(int k);


  protected:

    /* utility operations */

      virtual
      FortAnnot * findElement(char * name,
                              Boolean addIfMissing,
                              Boolean abortIfPresent,
                              Boolean sorted,
                              fan_MakeAnnotFunc makeAnnotFunc,
                              FortAnnotSrc * source,
                              void * data);
};






#endif /* __cplusplus */

#endif /* not FortAnnot_h */
