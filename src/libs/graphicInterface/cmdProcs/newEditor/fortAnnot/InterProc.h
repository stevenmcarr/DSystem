/* $Id: InterProc.h,v 1.3 1997/03/11 14:30:45 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      ned_cp/FortAnnot/InterProc.h                                    */
/*                                                                      */
/*      InterProc  -  Inter-procedural annotation source                */
/*      Last edited: August 25, 1993 at 3:53 pm				*/
/*                                                                      */
/************************************************************************/




#ifndef InterProc_h
#define InterProc_h


#include <libs/graphicInterface/framework/framework.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotPI.h>

#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphAnnot.h>
#include <libs/support/strings/OrderedSetOfStrings.h>

#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/AnnotLink.h>
#include <assert.h>






/************************************************************************/
/*      Procedural Interface                                            */
/************************************************************************/




EXTERN (void, InterProcAnnot_Init, (void))


EXTERN (void, InterProcAnnot_Fini, (void))






/************************************************************************/
/*      Object-oriented Interface                                       */
/************************************************************************/


#ifdef __cplusplus




/*************************/
/* InterProcSrc Class    */
/*************************/




struct InterProcSrc_Repr_struct;


class InterProcSrc: public FortAnnotSrc
{
  public:

    InterProcSrc_Repr_struct * InterProcSrc_repr;


  public:

    /* initialization */

      META_DEF(InterProcSrc)

      InterProcSrc(Context context, DB_FP * fp, FortAnnotMgr * fam,
                   Context module);

      virtual
      ~InterProcSrc(void);


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






/**********************/
/* IPIRootAnnot Class */
/**********************/




/* Determines which kind of tree this is */

typedef enum __ipi_RootType_enum
  { 
    ipi_Root_Global,
    ipi_Root_Selected

  } ipi_RootType;




struct IPIRootAnnot_Repr_struct;


class IPIRootAnnot: public CompoundFortAnnot
{
  public:

    IPIRootAnnot_Repr_struct * IPIRootAnnot_repr;


  public:

    /* initialization */

      META_DEF(IPIRootAnnot)

      IPIRootAnnot(char * name, InterProcSrc * isrc, CallGraph * cg,
                   ipi_RootType rtype, FortTreeNode node);

      virtual
      ~IPIRootAnnot(void);


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
      void realizeGlobal(void);

      virtual
      void realizeSelected(void);
};






/*************************/
/* IPISubprogAnnot Class */
/*************************/




struct IPISubprogAnnot_Repr_struct;


class IPISubprogAnnot: public CompoundFortAnnot
{
  public:

    IPISubprogAnnot_Repr_struct * IPISubprogAnnot_repr;


  public:

    /* initialization */

      META_DEF(IPISubprogAnnot)

      IPISubprogAnnot(InterProcSrc * isrc, CallGraph * cg, FortTreeNode node,
                      char * name);

      virtual
      ~IPISubprogAnnot(void);


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






/**************************/
/* IPICallsiteAnnot Class */
/**************************/




struct IPICallsiteAnnot_Repr_struct;


class IPICallsiteAnnot: public CompoundFortAnnot
{
  public:

    IPICallsiteAnnot_Repr_struct * IPICallsiteAnnot_repr;


  public:

    /* initialization */

      META_DEF(IPICallsiteAnnot)

      IPICallsiteAnnot(char * name, InterProcSrc * isrc, CallGraph * cg,
                       FortTreeNode node, int cls_id, char * callername);

      virtual
      ~IPICallsiteAnnot(void);


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






/*****************************/
/* IPISubprogLeafAnnot Class */
/*****************************/




struct IPISubprogLeafAnnot_Repr_struct;


class IPISubprogLeafAnnot: public SimpleFortAnnot
{
  public:

    IPISubprogLeafAnnot_Repr_struct * IPISubprogLeafAnnot_repr;


  public:

    /* initialization */

      META_DEF(IPISubprogLeafAnnot)

      IPISubprogLeafAnnot(InterProcSrc * isrc, CallGraph * cg,
                          FortTreeNode fnode, char * funcname, char * aname);

      virtual
      ~IPISubprogLeafAnnot(void);


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






/******************************/
/* IPICallsiteLeafAnnot Class */
/******************************/




struct IPICallsiteLeafAnnot_Repr_struct;


class IPICallsiteLeafAnnot: public SimpleFortAnnot
{
  public:

    IPICallsiteLeafAnnot_Repr_struct * IPICallsiteLeafAnnot_repr;


  public:

    /* initialization */

      META_DEF(IPICallsiteLeafAnnot)

      IPICallsiteLeafAnnot(InterProcSrc * isrc, CallGraph * cg,
                           FortTreeNode csnode, int cls_id, char * aname,
                           char * callername);

      virtual
      ~IPICallsiteLeafAnnot(void);


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

#endif /* not InterProc_h */
