/* $Id: CObject.h,v 1.3 1997/06/25 14:43:41 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*                                                                      */
/*      framework/CObject.h                                             */
/*                                                                      */
/*      Object -- Abstract superclass for all objects                   */
/*	Last edited: October 13, 1993 at 11:50 am			*/
/*                                                                      */
/************************************************************************/




#ifndef Object_h
#define Object_h


#include <libs/graphicInterface/framework/framework.h>




/************************************************************************/
/*	Object-oriented Interface					*/
/************************************************************************/


#ifdef __cplusplus




/********************/
/* Meta definitions */
/********************/




#define META_DEF(Class)							\
        virtual char * ClassName(void);

#define META_IMP(Class)							\
        char * Class::ClassName(void)					\
        {								\
           return #Class;						\
        }

#define REQUIRE_INIT(Class)						\
	Object::NoteAndPerformInit(Class::InitClass, Class::FiniClass)




/****************/
/* Object class */
/****************/




struct Object_Repr_struct;
class Trace;




class Object
{
public:

  struct Object_Repr_struct * Object_repr;


public:

/* class initialization */
  static void			NoteAndPerformInit(void init(void), void fini(void));
  static void			PerformNotedFinis(void);

  static void			InitClass(void);
  static void			FiniClass(void);

/* initialization */
  META_DEF(Object)
				Object(void);
  virtual			~Object(void);

/* creation */
  virtual void			Init(void);
  virtual void			PostInit(void);
  virtual void			Destroy(void);

/* change notification */
  // wantNotify = true: 
  //   add ob to the set of objects to be notified of changes
  // wantNotify = false: 
  //   remove ob from the set of objects to be notified  of changes
  virtual void			Notify(Object * ob, Boolean wantNotify);

  // notify all objects registered to receive change notification 
  virtual void			Changed(int kind, void * change);

  // receive change notification from ob
  virtual void			NoteChange(Object * ob, int kind, void * change);

/* tracing  */
  static void			TraceEnable(Boolean on);

};




/************/
/* Tracing  */
/************/




class Trace
{
public:

/* initialization */
  META_DEF(Trace)
				Trace(char * func,
				      Object * notifiee,
				      Object * ob,
                                      int kind,
                                      void * change);
  virtual			~Trace(void);

};




class ChangedTrace : public Trace
{
public:

/* initialization */
  META_DEF(ChangedTrace)
				ChangedTrace(Object * ob, int kind, void * change);
  virtual			~ChangedTrace(void);

};




class NoteChangeTrace : public Trace
{
public:

/* initialization */
  META_DEF(NoteChangeTrace)
				NoteChangeTrace(Object * notifiee,
				                Object * ob,
                                                int kind,
                                                void * change);
   virtual			~NoteChangeTrace(void);

};




#endif /* __cplusplus */

#endif /* not Object_h */
