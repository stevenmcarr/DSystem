/* $Id: FortAnnotSrc.C,v 1.3 1997/03/11 14:30:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      ned_cp/FortAnnot/FortAnnotSrc.C                                 */
/*                                                                      */
/*      FortAnnotSrc  -- annotation source mechanism for Fortran        */
/*      Last edited: June 18, 1993 at 4:00 pm                           */
/*                                                                      */
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>

#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotMgr.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotSrc.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnot.h>






/************************************************************************/
/*      Private Data Structures                                         */
/************************************************************************/




/*********************************/
/*  FortAnnotSrc Representation  */
/*********************************/




/* FortAnnotSrc objects */

typedef struct FortAnnotSrc_Repr_struct
  {
    FortAnnotMgr *      fam;
    Context             module;

  } FortAnnotSrc_Repr;


#define R(ob)           (ob->FortAnnotSrc_repr)


#define FASINHERITED    DBObject




/************************/
/*  Miscellaneous       */
/************************/


static int FortAnnotSrc_initCount = 0;






/************************/
/* Forward declarations */
/************************/


/* none */






/************************************************************************/
/*        Interface Operations                                          */
/************************************************************************/




/************************/
/*  Initialization      */
/************************/




void FortAnnotSrc_Init(void)
{
  if( FortAnnotSrc_initCount++ == 0 )
    { /* ... */
    }
}




void FortAnnotSrc_Fini(void)
{
  if( --FortAnnotSrc_initCount == 0 )
    { /* ... */
    }
}






/****************************/
/* Instance Initialization  */
/****************************/




META_IMP(FortAnnotSrc)




FortAnnotSrc::FortAnnotSrc(Context context, DB_FP * fp, FortAnnotMgr * fam,
                           Context module)
             : DBObject (context, fp)
{
  /* allocate a new instance */
    this->FortAnnotSrc_repr = (FortAnnotSrc_Repr *) 
                              get_mem(sizeof(FortAnnotSrc_Repr),
                                      "FortAnnot:FortAnnotSource");

  /* initialize the parts */
    /* set creation parameters */
      R(this)->fam    = fam;
      R(this)->module = module;
}




FortAnnotSrc::~FortAnnotSrc(void)
{
  /* free annotation source */
    free_mem((void*) this->FortAnnotSrc_repr);
}






/**************/
/*  Database  */
/**************/




void FortAnnotSrc::Open(Context context, Context mod_in_pgm_context,
                        Context pgm_context, DB_FP * session_fp)
{
  /* nothing */
}




void FortAnnotSrc::Close(void)
{
  /* close annotation source */
    this->FASINHERITED::Close();
}




void FortAnnotSrc::Save(Context context, DB_FP * fp)
{
  /* nothing */
}






/*************************/
/*  Change notification  */
/*************************/




void FortAnnotSrc::NoteChange(Object * ob, int kind, void * change)
{
  /* ... */
}






/***************/
/*  Selection  */
/***************/




void FortAnnotSrc::GetGlobal(CompoundFortAnnot * fan)
{
  /* nothing */
}




void FortAnnotSrc::GetSelection(CompoundFortAnnot * fan, int l1, int c1,
                                int l2, int c2)
{
  /* nothing */
}






/***********************/
/*  Protected Methods  */
/***********************/




Context FortAnnotSrc::getModule(void)
{
  return R(this)->module;
}






/************************************************************************/
/*          Private Operations                                          */
/************************************************************************/




/* none */


