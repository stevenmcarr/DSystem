/* $Id: stack.h,v 1.5 1997/03/11 14:37:27 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef _STACK_
#define _STACK_

/* Generic STACK class. Macro definitions to have generic stacks of
 * elements or stacks of pointers to elements.
 * To use a stack of elements of type WHATEVER (you choose your type):
 *		- write a function void print(WHATEVER) ( WHATEVER ) that
 *		  will print an element of type WHATEVER.
 *		- declare the stack class for WHATEVER:
 *		  declare(Stack,WHATEVER) <-- like that, no spaces.
 *		- to use the stack, declare the variable to be of
 *		  type Stack(WHATEVER). E.g.: Stack(WHATEVER) my_stack;
 *    Operations:
 *	void push( WHATEVER ); puts an element in the stack.
 * 	WHATEVER pop(); removes top element from the stack and returns it.
 *	WHATEVER top(); returns top element. Stack is unchanged.
 *	void print_Stack(); prints all elements in the stack.
 *	void map( mapfun f ); mapfun is defined as void (* mapfun) (WHATEVER)
 *		applies function f to all elements in the stack.
 *
 * To use a stack of pointer to elements of type WHATEVER:
 *		- write a function void print(WHATEVER) ( WHATEVER * ) that
 *		  will print an element of type WHATEVER using a WHATEVER *.
 *		- declare the stack class of pointers to WHATEVER:
 *		  declare(ptr_Stack,WHATEVER) <-- like that, no spaces.
 *		- to use the stack of pointers, declare the variable to be of
 *		  type ptr_Stack(WHATEVER).
 *		  E.g.: ptr_Stack(WHATEVER) my_stack;
 *    Operations:
 *	void push( WHATEVER * ); puts an element in the stack.
 * 	WHATEVER *pop(); removes top element from the stack and returns it.
 *	WHATEVER *top(); returns top element. Stack is unchanged.
 *	void print_Stack(); prints all elements in the stack.
 *	void map( mapfunptr f ); mapfunptr is defined as
 *				 void (* mapfunptr) (WHATEVER *)
 *		applies function f to all elements in the stack.
 *
 */


#include <generic.h>

#define Stack(ETYPE) name2(Stack,ETYPE)
#define Stack_elt(ETYPE) name2(Stack_elt,ETYPE)
#define Print(ETYPE) name2(Print_,ETYPE)

#define Stackdeclare(ETYPE) \
typedef void (* mapfun) (ETYPE); 	/* type of mapping function */ \
class Stack(ETYPE); \
class Stack_elt(ETYPE) {		/* Stack_elt defines the objects stored in the Stack */ \
  ETYPE el;				/* data to be stored */ \
  Stack_elt(ETYPE) *next;		/* pointer to next Stack_elt */ \
  Stack_elt(ETYPE) ( ETYPE elt, Stack_elt(ETYPE) *ptr ) /* Constructor, needs the data and */ \
    : el( elt ), next( ptr ) {}		/* a pointer to the next in the stack. */ \
  friend Stack(ETYPE);			/* class Stack is a friend */ \
}; \
class Stack(ETYPE) {			/* class Stack defines how to handle Stack_elt */ \
  Stack_elt(ETYPE) *tos;		/* tos is the top-of-stack pointer */ \
  uint16 count;				/* number of elements in stack */ \
 public: \
  Stack(ETYPE) () { count = 0; tos = 0; }		/* Stack constructor sets tos to 0 */ \
  Stack(ETYPE) ( Stack(ETYPE) &s ) {	/* Deep copy */ \
    Stack_elt(ETYPE) *s_iter = s.tos;	/* old stack iterator */ \
    Stack_elt(ETYPE) *iter = 0;		/* new stack iterator */ \
    Stack_elt(ETYPE) *aux;		/* auxiliary var to aid in deletion */ \
    tos = 0;				/* new stack is initially empty */ \
    count = 0; \
    while( s_iter ) {			/* copy old stack into dummy stack (backwards) */ \
      iter = new Stack_elt(ETYPE)( s_iter->el, iter ); \
      s_iter = s_iter->next; \
    }; \
    while( iter ) {			/* recopy in the right direction */ \
      tos = new Stack_elt(ETYPE)( iter->el, tos ); \
      count++; \
      aux = iter;			/* delete intermediate reversed stack */ \
      iter = iter->next; \
      delete aux; \
    }; \
  } \
  uint16 how_many() { return count; }	/* how many elements in stack */ \
  void push( ETYPE elt ) {		/* push( elt ) adds elt to stack, creating new Stack_elt*/ \
    tos = new Stack_elt(ETYPE)( elt, tos );	/* and making it point to previous tos */ \
    count++; \
  } \
  ETYPE pop() {				/* pop() removes current tos and returns it */ \
    ETYPE elt; \
    elt = 0; \
    if( tos ) {				/* if tos is not 0 */ \
      elt = tos->el;			/* get tos's data, keep it in elt */ \
      Stack_elt(ETYPE) *old = tos;	/* keep pointer to tos for deletion */ \
      tos = tos->next;			/* point to next in stack */ \
      count--; \
      delete old;			/* delete old tos */ \
    }; \
    return elt;				/* if tos is 0, quietly return */ \
  } \
  ETYPE top() { return tos->el; }	/* top() returns data for tos */ \
  void Print_Stack() {			/* print all data in the stack */ \
    Stack_elt(ETYPE) *iter = tos;	/* iteration variable initialized to tos */ \
    printf("STACK:\n"); \
    while( iter ) {			/* while there is data in the stack */ \
      Print(ETYPE) ( iter->el );	/* print data */ \
      iter = iter->next;		/* move to next in stack */ \
    }; \
    printf("EOS\n"); \
  } \
  void map( mapfun f ) {		/* map f to all data */ \
    Stack_elt(ETYPE) *iter = tos;	/* initialize iteration variable to tos */ \
    while( iter ) {			/* while there is data in the stack */ \
      f( iter->el );			/* apply f to data */ \
      iter = iter->next;		/* move to next in stack */ \
    }; \
  } \
};

#define ptr_Stack(ETYPE) name2(Stack,ETYPE)
#define ptr_Stack_elt(ETYPE) name2(Stack_elt,ETYPE)

#define ptr_Stackdeclare(ETYPE) \
typedef void (* mapfunptr) (ETYPE *);	/* type of mappinf function */ \
class ptr_Stack(ETYPE); \
class ptr_Stack_elt(ETYPE) {		/* Same as Stack, ptr_Stack operates over elements */ \
  ETYPE *el;				/* with the only difference that data here are _pointers_ */ \
  ptr_Stack_elt(ETYPE) *next;		/* pointer to next guy in the stack */ \
  ptr_Stack_elt(ETYPE) ( ETYPE *elt, ptr_Stack_elt(ETYPE) *ptr ) */ \
    : el( elt ), next( ptr ) {}		/* constructor for a ptr_Stack element */ \
  friend ptr_Stack(ETYPE); \
}; \
class ptr_Stack(ETYPE) { \
  ptr_Stack_elt(ETYPE) *tos;		/* pointer to ptr_Stack_elt at the top */ \
  uint16 count;				/* number of elements in stack */ \
 public: \
  ptr_Stack(ETYPE) () { count = 0; tos = 0; }	/* constructor. Set tos to 0 */ \
  ptr_Stack(ETYPE) ( ptr_Stack(ETYPE) &s ) {	/* Deep copy */ \
    ptr_Stack_elt(ETYPE) *s_iter = s.tos;	/* old stack iterator */ \
    ptr_Stack_elt(ETYPE) *iter = 0;		/* new stack iterator */ \
    ptr_Stack_elt(ETYPE) *aux;		/* auxiliary var to aid in deletion */ \
    tos = 0;				/* new stack is initially empty */ \
    count = 0; \
    while( s_iter ) {			/* copy old stack into dummy stack (backwards) */ \
      iter = new ptr_Stack_elt(ETYPE)( s_iter->el, iter ); \
      s_iter = s_iter->next; \
    }; \
    while( iter ) {			/* recopy in the right direction */ \
      tos = new ptr_Stack_elt(ETYPE)( iter->el, tos ); \
      count++; \
      aux = iter;			/* delete intermediate reversed stack */ \
      iter = iter->next; \
      delete aux; \
    }; \
  } \
  uint16 how_many() { return count; }		/* how many elements in stack */ \
  void push( ETYPE *elt ) {		/* adds a new data at the top by creating a ptr_Stack_elt */ \
    tos = new ptr_Stack_elt(ETYPE)( elt, tos ); /* and chaining it to the previous top*/ \
    count++; \
  } \
  ETYPE *pop() {			/* pop() removes tos and return it */ \
    ETYPE *elt = 0; \
    if( tos ) {				/* if tos exists */ \
      elt = tos->el;			/* save data for returning it */ \
      ptr_Stack_elt(ETYPE) *old = tos;	/* save tos for deletion */ \
      tos = tos->next;			/* move tos to point to next */ \
      count--; \
      delete old;			/* delete old tos */ \
    }; \
    return elt;				/* return data */ \
  } \
  ETYPE *top() { return tos->el; }	/* top returns data in tos */ \
  void Print_Stack() {			/* print contents of ptr_Stack */ \
    ptr_Stack_elt(ETYPE) *iter = tos;	/* initialize iteration variable */ \
    printf("STACK:\n"); \
    while( iter ) {			/* while there are stack elements */ \
      Print(ETYPE) ( iter->el );	/* print the data of the current one */ \
      iter = iter->next;		/* move to the next */ \
    }; \
    printf("EOS\n"); \
  } \
  void map( mapfunptr f ) {		/* map(f) applies f to all data in ptr_Stack */ \
    Stack_elt(ETYPE) *iter = tos;	/* initialize iteration variable */ \
    while( iter ) {			/* while there are stack elements */ \
      f( iter->el );			/* apply f to data */ \
      iter = iter->next;		/* move to next element */ \
    }; \
  } \
};

#endif _STACK_
