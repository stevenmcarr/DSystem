/* $Id: DecorationSet.C,v 1.2 1997/03/11 14:32:36 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/DecorationSet.C					*/
/*									*/
/*	DecorationSet -- Decoration composed of subdecorations		*/
/*	Last edited: October 14, 1993 at 6:47 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/DecorationSet.h>

#include <libs/support/arrays/FlexibleArray.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* DecorationSet object */

typedef struct DecorationSet_Repr_struct
  {
    /* contents */
      Flex *		subdecs;
      Rectangle		bbox;

  } DecorationSet_Repr;


#define R(ob)		(ob->DecorationSet_repr)

#define INHERITED	Decoration






/*************************/
/*  Forward declarations */
/*************************/




/* none */







/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void DecorationSet::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(Decoration);
}




void DecorationSet::FiniClass(void)
{
  /* nothing */
}






/**********************/
/*  Instance creation */
/**********************/




DecorationSet * DecorationSet::Create(void)
{
  DecorationSet * d;
  
  d = new DecorationSet;
  d->DecorationSet::Init();
  
  return d;
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(DecorationSet)




DecorationSet::DecorationSet(void)
          : Decoration()
{
  /* allocate instance's private data */
    this->DecorationSet_repr = (DecorationSet_Repr *) get_mem(sizeof(DecorationSet_Repr), "DecorationSet instance");
}




void DecorationSet::Init(void)
{
  /* create subparts */
    R(this)->subdecs = flex_create(sizeof(Decoration *));
    
  /* initialize status */
    R(this)->bbox = EmptyRect;
}




void DecorationSet::Destroy(void)
{
  flex_destroy(R(this)->subdecs);
  this->INHERITED::Destroy();
}




DecorationSet::~DecorationSet()
{
  free_mem((void*) this->DecorationSet_repr);
}






/********************/
/*  View attachment */
/********************/




void DecorationSet::AttachView(CTextView * view)
{
  int num = flex_length(R(this)->subdecs);
  int k;
  Decoration * d;
  
  this->INHERITED::AttachView(view);

  for( k = 0;  k < num;  k++ )
    { flex_get_buffer(R(this)->subdecs, k, 1, (char *) &d);
      d->AttachView(view);
    }
}






/************/
/*  Drawing */
/************/




void DecorationSet::ColorizeLine(int c_linenum,
                                 int marginWidth,
                                 TextString &text,
                                 TextData &data)
{
  int num = flex_length(R(this)->subdecs);
  int k;
  Decoration * d;

  for( k = 0;  k < num;  k++ )
    { flex_get_buffer(R(this)->subdecs, k, 1, (char *) &d);
      d->ColorizeLine(c_linenum, marginWidth, text, data);
    }
}




void DecorationSet::Draw(void)
{
  int num = flex_length(R(this)->subdecs);
  int k;
  Decoration * d;
  
  for( k = 0;  k < num;  k++ )
    { flex_get_buffer(R(this)->subdecs, k, 1, (char *) &d);
      d->Draw();
    }
}




Rectangle DecorationSet::BBox(void)
{
  return R(this)->bbox;
}






/****************************/
/* Access to subdecorations */
/****************************/




void DecorationSet::Add(Decoration * d)
{
  int num = flex_length(R(this)->subdecs);
  CTextView * view = this->GetView();
  
  flex_insert_one(R(this)->subdecs, num, (char *) &d);
  if( view != nil )  d->AttachView(view);

  R(this)->bbox = unionRect(R(this)->bbox, d->BBox());
}



      
void DecorationSet::SetEmpty(void)
{
  int num = flex_length(R(this)->subdecs);
  
  flex_delete(R(this)->subdecs, 0, num);
  R(this)->bbox = EmptyRect;
}



      
Boolean DecorationSet::IsEmpty(void)
{
  int num = flex_length(R(this)->subdecs);
  
  return BOOL( num == 0 );
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */
