/* $Id: ClassName.h,v 1.1 1997/03/11 14:27:31 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//***************************************************************************
// ClassName.h
//
//   provide a character string name for a class for use as a type tag.
//   the implementation guarantees that these character strings can
//   be compared as pointers for an equality test.
//   
//
// Author: John Mellor-Crummey                                October 1993
//
// Copyright 1993, Rice University
//***************************************************************************

#ifndef ClassName_h
#define ClassName_h


#define CLASS_NAME(Class) Class ## ClassName

#define CLASS_NAME_FDEF(Class)  \
virtual char *Class::ClassName(void)

#define CLASS_NAME_EDEF(Class) \
extern char *CLASS_NAME(Class)

#define CLASS_NAME_EIMPL(Class) \
char *CLASS_NAME(Class) = # Class

#define CLASS_NAME_EIMPL_DERIVED(Class, basename) \
char *CLASS_NAME(Class) = # basename

#define CLASS_NAME_FIMPL(Class) \
char *Class::ClassName(void)   \
{ \
  return CLASS_NAME(Class); \
}

#define CLASS_NAME_IMPL(Class) \
CLASS_NAME_EIMPL(Class);  CLASS_NAME_FIMPL(Class)

#endif
