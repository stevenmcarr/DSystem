/* $Id: FortAnnot.C,v 1.4 1997/03/11 14:30:42 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      ned_cp/FortAnnot/FortAnnot.C                                    */
/*                                                                      */
/*      FortAnnot -- annotation mechanism for Fortran                   */
/*      Last edited: August 25, 1993 at 3:53 pm				*/
/*                                                                      */
/************************************************************************/




#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>

#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotMgr.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnotSrc.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortAnnot/FortAnnot.h>

#include <libs/support/arrays/FlexibleArray.h>






/************************************************************************/
/*      Private Data Structures                                         */
/************************************************************************/




/***********************************/
/*  FortAnnotation Representation  */
/***********************************/




/* FortAnnotation objects */

typedef struct FortAnnot_Repr_struct
  {
    /* creation parameters */
      FortEditor        ed;

      char *            name;
      FortAnnotSrc *    source;
      Boolean           compound;
      Boolean           sorted;

    /* contents */
      Flex *            elements;

    /* status */
      Boolean           lazy;
      Boolean           realizing;

  } FortAnnot_Repr;


#define R(ob)           (ob->FortAnnot_repr)


#define FAINHERITED     Object
#define SCINHERITED     FortAnnot




/* lines of text in a SimpleFortAnnotation */

typedef struct
  {
    char *              text;

    /* link */
      Context           module;
      FortTreeNode      node;
      goto_Link         kind;

  } fan_TextLine;






/************************/
/*  Miscellaneous       */
/************************/


static int FortAnnot_initCount = 0;






/************************/
/* Forward declarations */
/************************/


/* none */






/************************************************************************/
/*      Interface Operations                                            */
/************************************************************************/




/************************/
/*  Initialization      */
/************************/




void FortAnnot_Init(void)
{
  if( FortAnnot_initCount++ == 0 )
    { /* ... */
    }
}




void FortAnnot_Fini(void)
{
  if( --FortAnnot_initCount == 0 )
    { /* ... */
    }
}






/******************************/
/*  Ned procedural interface  */
/******************************/




Generic fan_InitSimple(char * name, Boolean sorted,
                         int source, Generic ob)
{
  FortAnnotSrc * source_ptr;
  SimpleFortAnnot * fan;

  /* There is no longer an ob element in FortAnnot.
     Each specific Fortran annotation has its' own
     class.  SimpleFortAnnot is an abstract class. */
     
  source_ptr = (FortAnnotSrc *) source;

  fan = new SimpleFortAnnot(name, source_ptr, sorted);
  fan->Init();

  return (Generic) fan;
}




Generic fan_InitCompound(char * name, Boolean sorted,
                           int source, Generic ob)
{
  FortAnnotSrc * source_ptr;
  CompoundFortAnnot * fan;

  /* There is no longer an ob element in FortAnnot.
     Each specific Fortran annotation has its' own
     class.  CompoundFortAnnot is an abstract class. */
     
  source_ptr = (FortAnnotSrc *) source;

  fan = new CompoundFortAnnot(name, source_ptr, sorted);
  fan->Init();

  return (Generic) fan;
}




void fan_Destroy(Generic fan)
{
  FortAnnot * fan_ptr;

  fan_ptr = (FortAnnot *) fan;
  fan_ptr->Destroy();
}




void fan_GetName(Generic fan, char * *name)
{
  FortAnnot * fan_ptr;

  fan_ptr = (FortAnnot *) fan;
  fan_ptr->GetName(name);
}




Boolean fan_IsCompound(Generic fan)
{
  FortAnnot * fan_ptr;

  fan_ptr = (FortAnnot *) fan;
  return fan_ptr->IsCompound();
}




void fan_AddTextLine(Generic fan, char * text, Context module,
                     FortTreeNode node, goto_Link kind)
{
  SimpleFortAnnot * fan_ptr;

  fan_ptr = (SimpleFortAnnot *) fan;
  fan_ptr->AddTextLine(text, module, node, kind);
}




int fan_NumTextLines(Generic fan)
{
  SimpleFortAnnot * fan_ptr;

  fan_ptr = (SimpleFortAnnot *) fan;
  return fan_ptr->NumTextLines();
}




void fan_GetTextLine(Generic fan, int k, char * *text)
{
  SimpleFortAnnot * fan_ptr;

  fan_ptr = (SimpleFortAnnot *) fan;
  fan_ptr->GetTextLine(k, text);
}




Boolean fan_HasLink(Generic fan, int k)
{
  SimpleFortAnnot * fan_ptr;

  fan_ptr = (SimpleFortAnnot *) fan;
  return fan_ptr->HasLink(k);
}




void fan_GotoLink(Generic fan, int k)
{
  SimpleFortAnnot * fan_ptr;

  fan_ptr = (SimpleFortAnnot *) fan;
  fan_ptr->GotoLink(k);
}




void fan_AddElement(Generic fan, Generic elem)
{
  CompoundFortAnnot * fan_ptr;
  FortAnnot * elem_ptr;

  fan_ptr = (CompoundFortAnnot *) fan;
  elem_ptr = (FortAnnot *) elem;
  fan_ptr->AddElement(elem_ptr);
}




int fan_NumElements(Generic fan)
{
  CompoundFortAnnot * fan_ptr;

  fan_ptr = (CompoundFortAnnot *) fan;
  return fan_ptr->NumElements();
}




Generic fan_GetElement(Generic fan, int k)
{
  CompoundFortAnnot * fan_ptr;

  fan_ptr = (CompoundFortAnnot *) fan;
  return (Generic) fan_ptr->GetElement(k);
}






/********************/
/*  Access to text  */
/********************/




char * getLine(FortEditor ed, FortTreeNode node, goto_Link kind)
{
  int line1, line2, dummy;

  /* nb -- the string returned here must be freed by the caller */

  ed_NodeToText(ed, node, &line1, &dummy, &line2, &dummy);
  return ed_GetTextLine( ed, (kind == LINK_TO_FIRST ? line1 : line2) );
}






/****************************/
/* Instance Initialization  */
/****************************/




META_IMP(FortAnnot)




FortAnnot::FortAnnot(char * name, FortAnnotSrc * source, Boolean compound,
                     Boolean sorted)
{
  /* allocate a new instance */
    this->FortAnnot_repr = (FortAnnot_Repr *) get_mem(sizeof(FortAnnot_Repr),
                                                    "FortAnnot:FortAnnotation");

  /* set creation arguments */
    R(this)->name     = ssave(name);
    R(this)->source   = source;
    R(this)->compound = compound;
    R(this)->sorted   = sorted;

  /* set status */
    R(this)->lazy = true;
    R(this)->realizing = false;
}




FortAnnot::~FortAnnot(void)
{
  /* free annotation representation */
  sfree(R(this)->name);
  free_mem((void*) this->FortAnnot_repr);
}






/*************************/
/*  Annotation Creation  */
/*************************/




void FortAnnot::Init(void)
{
  /* nothing */
}




void FortAnnot::Destroy(void)
{
  /* destroy annotation */
    this->FAINHERITED::Destroy();
}






/***************************/
/*  Access to annotations  */
/***************************/




void FortAnnot::GetName(char * *name)
{
  *name = R(this)->name;
}




Boolean FortAnnot::IsCompound(void)
{
  return R(this)->compound;
}






/****************************/
/* Instance Initialization  */
/****************************/




META_IMP(SimpleFortAnnot)




SimpleFortAnnot::SimpleFortAnnot(char * name, FortAnnotSrc * source,
                                 Boolean sorted)
                : FortAnnot (name, source, false, sorted)
{
  /* make subparts */
    R(this)->elements = flex_create(sizeof(fan_TextLine));
}




SimpleFortAnnot::~SimpleFortAnnot(void)
{
  /* free simple annotation elements */
    flex_destroy(R(this)->elements);
}






/*************************/
/*  Annotation Creation  */
/*************************/




void SimpleFortAnnot::Init(void)
{
  /* nothing */
}




void SimpleFortAnnot::Destroy(void)
{
  Flex * elements = R(this)->elements;
  int numElements = flex_length(R(this)->elements);
  int k;
  fan_TextLine line;

  /* destroy each element */
    for( k = 0;  k < numElements;  k++ )
      { flex_get_buffer(elements, k, 1, (char *) &line);
        sfree(line.text);
      }

  /* destroy annotation */
    this->SCINHERITED::Destroy();
}






/*******************/
/* Access to text  */
/*******************/




void SimpleFortAnnot::AddTextLine(char * text, Context module,
                                  FortTreeNode node, goto_Link kind)
{
  Flex * elements = R(this)->elements;
  int numElements = flex_length(elements);
  int k, pos;
  fan_TextLine line_k, line;

  /* determine where to insert new line */
    if( R(this)->sorted )
      { pos = UNUSED;
        k = 0;
        while( pos == UNUSED  &&  k < numElements )
          { flex_get_buffer(elements, k, 1, (char *) &line_k);
            if( strcmp(text, line_k.text) < 0 )
              pos = k;
            else
              k += 1;
          }
        if( pos == UNUSED )  pos = numElements;
      }
    else
      pos = numElements;


  /* insert the new line */
    line.text   = ssave(text);
    line.module = module;
    line.node   = node;
    line.kind   = kind;
    flex_insert_one(elements, pos, (char *) &line);

    R(this)->lazy = false;
}




int SimpleFortAnnot::NumTextLines(void)
{
  ensureRealize();
  return flex_length(R(this)->elements);
}




void SimpleFortAnnot::GetTextLine(int k, char * *text)
{
  Flex * elements = R(this)->elements;
  fan_TextLine line;

  ensureRealize();
  flex_get_buffer(elements, k-1, 1, (char *) &line);

  *text = line.text;
}




Boolean SimpleFortAnnot::HasLink(int k)
{
  Flex * elements = R(this)->elements;
  fan_TextLine line;

  ensureRealize();
  flex_get_buffer(elements, k-1, 1, (char *) &line);

  return BOOL( line.module != CONTEXT_NULL );
}




void SimpleFortAnnot::GotoLink(int k)
{
  Flex * elements = R(this)->elements;
  fan_TextLine line;

  ensureRealize();
  flex_get_buffer(elements, k-1, 1, (char *) &line);

  gotoLink(line.module, line.node, line.kind);
}






/****************************/
/* Instance Initialization  */
/****************************/




META_IMP(CompoundFortAnnot)




CompoundFortAnnot::CompoundFortAnnot(char * name, FortAnnotSrc * source,
                                     Boolean sorted)
                  : FortAnnot (name, source, true, sorted)
{
  /* make subparts */
    R(this)->elements = flex_create(sizeof(FortAnnot *));
}




CompoundFortAnnot::~CompoundFortAnnot(void)
{
  /* free compound annotation elements */
    flex_destroy(R(this)->elements);
}






/*************************/
/*  Annotation Creation  */
/*************************/




void CompoundFortAnnot::Init(void)
{
  /* nothing */
}




void CompoundFortAnnot::Destroy(void)
{
  Flex * elements = R(this)->elements;
  int numElements = flex_length(R(this)->elements);
  int k;
  FortAnnot * annot;

  /* destroy each element */
    for( k = 0;  k < numElements;  k++ )
      { flex_get_buffer(elements, k, 1, (char *) &annot);
        annot->Destroy();
      }

  /* destroy annotation */
    this->SCINHERITED::Destroy();
}






/***********************/
/* Access to elements  */
/***********************/




void CompoundFortAnnot::AddElement(FortAnnot * elem)
{
  Flex * elements = R(this)->elements;
  int numElements = flex_length(elements);
  int k, pos;
  FortAnnot * elem_k;

  /* determine where to insert new element */
    if( R(this)->sorted )
      { pos = UNUSED;
        k = 0;
        while( pos == UNUSED  &&  k < numElements )
          { flex_get_buffer(elements, k, 1, (char *) &elem_k);
            if( strcmp(R(elem)->name, R(elem_k)->name) < 0 )
              pos = k;
            else
              k += 1;
          }
        if( pos == UNUSED )  pos = numElements;
      }
    else
      pos = numElements;

  /* insert the element */
    flex_insert_one(elements, pos, (char *) &elem);
    R(this)->lazy = false;
}




int CompoundFortAnnot::NumElements(void)
{
  /* TRICK to permit realizing-methods to say 'fan_NumElements' */
  if( ! R(this)->realizing )  ensureRealize();

  return flex_length(R(this)->elements);
}




FortAnnot * CompoundFortAnnot::GetElement(int k)
{
  Flex * elements = R(this)->elements;
  FortAnnot * annot;

  ensureRealize();
  flex_get_buffer(elements, k-1, 1, (char *) &annot);

  return annot;
}






/************************************************************************/
/*            Internal Operations for Subparts                          */
/************************************************************************/




/***********************/
/*  Protected methods  */
/***********************/




void FortAnnot::realize(void)
{
  /* nothing */
}




void FortAnnot::ensureRealize(void)
{
  /* TRICK to permit realizing-methods to say 'fan_NumElements' */

  if( R(this)->lazy )
    { R(this)->realizing = true;
      beginComputeBound();
      this->realize();
      endComputeBound();
      R(this)->realizing = false;
    }
}




/************************/
/*  Utility operations  */
/************************/




FortAnnot * CompoundFortAnnot::findElement(char * name,
                                           Boolean addIfMissing,
                                           Boolean abortIfPresent,
                                           Boolean sorted,
                                           fan_MakeAnnotFunc makeAnnotFunc,
                                           FortAnnotSrc * source,
                                           void * data)
{
  FortAnnot * theAnnot;
  FortAnnot * annot_k;
  int numElements, k;
  char * name_k;
  Boolean present;

  /* see if the given name is already present */
    theAnnot = nil;
    numElements = this->NumElements();
    k = 1;
    while( theAnnot == nil  &&  k <= numElements )
      { annot_k = this->GetElement(k);
        annot_k->GetName(&name_k);
        if( strcmp(name, name_k) == 0 )
          theAnnot = annot_k;
        else
          k += 1;
      }

  /* respond appropriately */
    present = BOOL( theAnnot != nil );

    if( ! present  &&  addIfMissing )
      { theAnnot = makeAnnotFunc(name, source, sorted, data);
        this->AddElement(theAnnot);
      }

    if( present  &&  abortIfPresent )
      theAnnot = nil;

  return theAnnot;
}






/************************************************************************/
/*      Private Operations                                              */
/************************************************************************/




/* none */


