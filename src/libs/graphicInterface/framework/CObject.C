/* $Id: CObject.C,v 1.2 1997/03/11 14:32:29 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      framework/CObject.C                                             */
/*                                                                      */
/*      Object -- Abstract superclass for all objects                   */
/*	Last edited: October 13, 1993 at 11:50 am			*/
/*                                                                      */
/************************************************************************/




#include <iostream>
using namespace std;
#include <string.h>


#include <libs/graphicInterface/framework/CObject.h>


#include <libs/support/arrays/FlexibleArray.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* Object object */

typedef struct Object_Repr_struct
  {
    /* change notification */
      Flex *		notifiees;

  } Object_Repr;


#define R(ob)		(ob->Object_repr)

#define INHERITED	NONE






/*************************/
/*  Miscellaneous	 */
/*************************/




/* tracing */

static Boolean	Trace_enable  = false;
static int	Trace_counter = 0;




/* class initialization control */

struct ob_InitRecord
  {
    void		(*init)(void);
    void		(*fini)(void);
    
    ob_InitRecord *	prev;

  };

static ob_InitRecord * ob_initList = nil;






/*************************/
/*  Forward declarations */
/*************************/




/* none */







/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/






/*********************************/
/*  Class initialization control */
/*********************************/




void Object::NoteAndPerformInit(void init(void), void fini(void))
{
  Boolean found;
  ob_InitRecord * current;
  
  /* see if requested init has already been done (or is being done) */
    found = false;
    current = ob_initList;
    while( ! found && current != nil )
      if( current->init == init )
        found = true;
      else
        current = current->prev;
        
  /* note and perform init if necessary */
    if( ! found )
      { current = new ob_InitRecord;
        current->init = init;
        current->fini = fini;
        current->prev = ob_initList;        
        ob_initList   = current;
        
        current->init();
      }
}




void Object::PerformNotedFinis(void)
{
  ob_InitRecord * next;
  ob_InitRecord * current;

  current = ob_initList;
  while( current != nil )
    { current->fini();
      next = current->prev;
      delete current;
      current = next;
    }
}






/*************************/
/*  Class initialization */
/*************************/




void Object::InitClass(void)
{
  /* nothing */
}




void Object::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(Object)




Object::Object(void)
{
  this->Object_repr =
      (Object_Repr *) get_mem(sizeof(Object_Repr), "Object instance");

  R(this)->notifiees = nil;
}




Object::~Object(void)
{
  if( R(this)->notifiees != nil )
    flex_destroy(R(this)->notifiees);

  free_mem((void*) this->Object_repr);
}






/*********************/
/*  Object Creation  */
/*********************/




void Object::Init(void)
{
  /* nothing */
}




void Object::PostInit(void)
{
  /* nothing */
}




void Object::Destroy(void)
{
  delete this;
}






/******************/
/*  Trace enable  */
/******************/




void Object::TraceEnable(Boolean on)
{
  Trace_enable = on;
}






/*************/
/*  Tracing  */
/*************/




META_IMP(Trace)




Trace::Trace(char * func, Object * notifiee, Object * ob,
             int kind, void * change)
{
  int i;
  char * name;
  char indent_str[] = "   ";

  if( Trace_enable )
    { name = notifiee->ClassName();

      for( i = 0; i < Trace_counter; i++ )
        cout << indent_str << flush;

      cout << name << "::" << func << "(" << ob << ", " << kind 
           << ", " << change << ")\n" << flush;

      Trace_counter++;
    }
}




Trace::~Trace(void)
{
  if( Trace_enable && (--Trace_counter == 0) )
    { cout << "----------------------------------------" 
           << "----------------------------------------\n" 
           << flush;
    }
}




META_IMP(ChangedTrace)




ChangedTrace::ChangedTrace(Object * ob, int kind, void * change)
             : Trace ("Changed", ob, ob, kind, change)
{
  /* nothing */
}




ChangedTrace::~ChangedTrace(void)
{
  /* nothing */
}




META_IMP(NoteChangeTrace)




NoteChangeTrace::NoteChangeTrace(Object * notifiee, Object * ob,
                                 int kind, void * change)
           : Trace ("NoteChange", notifiee, ob, kind, change)
{
  /* nothing */
}




NoteChangeTrace::~NoteChangeTrace(void)
{
  /* nothing */
}






/*************************/
/*  Change notification  */
/*************************/




void Object::Notify(Object * ob, Boolean wantNotify)
{
  Flex * notifiees = R(this)->notifiees;
  int len;
  Object * ob2;
  int k;


  if( wantNotify )
    { if( notifiees == nil )
        notifiees = R(this)->notifiees = flex_create(sizeof(Object *));

      flex_insert_one(notifiees, flex_length(notifiees), (char *) &ob);
    }
  else
    { if( notifiees != nil )
        { len = flex_length(notifiees);
          for( k = len-1;  k >= 0;  k-- )
            { flex_get_buffer(notifiees, k, 1, (char *) &ob2);
              if( ob2 == ob )
                flex_delete(notifiees, k, 1);
            }
        }
    }
}




void Object::Changed(int kind, void * change)
{
  ChangedTrace trace(this, kind, change);
  Flex * notifiees = R(this)->notifiees;
  int k;
  Object * ob;

  if( notifiees != nil )
    for( k = 0;  k < flex_length(notifiees);  k++ )
      { (void) flex_get_buffer(notifiees, k, 1, (char *) &ob);
        ob->NoteChange(this, kind, change);
      }
}




void Object::NoteChange(Object * ob, int kind, void * change)
{
  /* nothing */
}






/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




/* none */



