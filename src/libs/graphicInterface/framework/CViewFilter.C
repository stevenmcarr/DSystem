/* $Id: CViewFilter.C,v 1.8 1997/03/11 14:32:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      framework/CViewFilter.C                                         */
/*                                                                      */
/*      CViewFilter -- Determines how a CTextView displays lines        */
/*	Last edited: October 13, 1993 at 12:47 pm			*/
/*                                                                      */
/************************************************************************/




#include <libs/graphicInterface/framework/CViewFilter.h>

#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>



/* Ned routines needed here */
#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>
#include <libs/graphicInterface/cmdProcs/newEditor/TextView.h>
#include <libs/graphicInterface/cmdProcs/newEditor/ViewFilter.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* CViewFilter object */

typedef struct CViewFilter_Repr_struct
  {
    /* filter specs */
      int		elision;

    /* Ned components */
      ViewFilter	nedFilter;

  } CViewFilter_Repr;


#define R(ob)		(ob->CViewFilter_repr)

#define INHERITED	DBObject






/*************************/
/*  Forward declarations */
/*************************/




static void filterProc(CViewFilter * cvf,
                       Boolean countOnly,
                       Object * contents,
                       int line,
                       int &subline,
                       TextString &text,
                       TextData &data);






/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void CViewFilter::InitClass(void)
{
  /* initialize needed submodules */
    /* none */
}




void CViewFilter::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(CViewFilter)




CViewFilter::CViewFilter(void)
   : DBObject (CONTEXT_NULL, DB_NULLFP)
{
  /* allocate instance's private data */
    this->CViewFilter_repr = (CViewFilter_Repr *) 
                             get_mem(sizeof(CViewFilter_Repr),
                                     "CViewFilter instance");

  /* be an identity filter without further initialization */
    this->isnew(CONTEXT_NULL);
}




CViewFilter::CViewFilter(Context context, DB_FP * session_fp)
   : DBObject (context, session_fp)
{
  /* allocate instance's private data */
    this->CViewFilter_repr = (CViewFilter_Repr *)
                             get_mem(sizeof(CViewFilter_Repr),
                                     "CViewFilter instance");
}




CViewFilter::~CViewFilter()
{
  vf_Close(R(this)->nedFilter);
  free_mem((void*) this->CViewFilter_repr);
}






/***********************/
/* Change notification */
/***********************/




void CViewFilter::NoteChange(Object * ob, int kind, void * change)
{
  NoteChangeTrace trace(this, ob, kind, change);

  /* TEMPORARY */

  vf_NoteChange(R(this)->nedFilter, NOTIFY_DOC_CHANGED, false, 0, 99999, 99999);
}






/*************/
/*  Database */
/*************/




void CViewFilter::isnew(Context context)
{
  R(this)->nedFilter = vf_Open(CONTEXT_NULL, DB_NULLFP,
                               (Generic) this, (vf_FilterFunc) filterProc);
}




void CViewFilter::read(DB_FP * fp, DB_FP * session_fp)
{
  R(this)->nedFilter = vf_Open(CONTEXT_NULL, DB_NULLFP,
                               (Generic) this, (vf_FilterFunc) filterProc);
}






/**************************/
/* Access to filter specs */
/**************************/




void CViewFilter::SetElision(int elision)
{
  R(this)->elision = elision;
  vf_SetElision(R(this)->nedFilter, elision);
  /* TEMPORARY */
  /*** this->NoteChange(this, 0, nil); ***/  /* sic */
}




void CViewFilter::GetElision(int &elision)
{
  elision = R(this)->elision;
}






/*************************/
/* Coordinate conversion */
/*************************/




void CViewFilter::GetDocSize(Point &size)
{
  vf_GetDocSize(R(this)->nedFilter, &size);
}




Boolean CViewFilter::ContentsLineElided(int c_lineNum)
{
  int dummy;
  
  return NOT( vf_ContentsToView(R(this)->nedFilter, c_lineNum, &dummy) );
}




int CViewFilter::ContentsToViewLinenum(int c_lineNum)
{
 int v_lineNum;

  (void) vf_ContentsToView(R(this)->nedFilter, c_lineNum, &v_lineNum);
  return v_lineNum;
}




int CViewFilter::ViewToContentsLinenum(int v_lineNum)
{
 int c_lineNum;

  (void) vf_ViewToContents(R(this)->nedFilter, v_lineNum, &c_lineNum);
  return c_lineNum;
}






/*************/
/* Filtering */
/*************/




Boolean CViewFilter::filterLine(Boolean countOnly,
                                int line,
                                int &subline,
                                TextString &text,
                                TextData &data)
{
  /* default behavior is to be the identity filter */

  if( countOnly )
    subline = 1;
  else
    /* nothing */ ;

  return false;
}




Generic CViewFilter::getNedViewFilter(void)
{
  return R(this)->nedFilter;
}








/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
void filterProc(CViewFilter * cvf,
                Boolean countOnly,
                Object * contents,
                int line,
                int &subline,
                TextString &text,
                TextData &data)
{
  (void) cvf->filterLine(countOnly, line, subline, text, data);
}
