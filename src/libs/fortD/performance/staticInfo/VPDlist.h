/* $Id: VPDlist.h,v 1.1 1997/03/11 14:29:06 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// This may look like c code but it is really -*- C++ -*-

// Doubly linked list class used several places here.


/* This list is used everywhere
   Just an ordinary doubly linked list class with primitives for
   stacks and queues .
*/


// All destructors are virtual , so as to make inheritance easier...


#ifndef _VPDlist_h
#define _VPDlist_h 1

//This is rn's include file
#include <libs/support/misc/general.h>

class VPDlist_node {
public:
 VPDlist_node * prev;     
 VPDlist_node * next;     
 void *        item;
 VPDlist_node(void * it);
virtual ~VPDlist_node(void);
 void Clear(void);
 Boolean ValidNext(void);
 Boolean ValidPrev(void);
};

class VPDlist; 

class VPDfinger {
protected:
  friend class VPDlist;
  friend class VPDlist_iter;
  friend class VPDlist_const_iter;
  VPDlist_node * node;
  VPDlist *	 list;
  VPDfinger(VPDlist_node * n, VPDlist * l);
public:	
  VPDfinger(void);
  VPDfinger(const VPDfinger & it);
  virtual ~VPDfinger(void);
  
  VPDfinger & const operator = (const VPDfinger & r);
  int 		operator == (const VPDfinger & r);

  Boolean 	Is_Valid(void);
  void Invalidate(void);

  // Remove this finger from the parent list;
  void	 	Delete(void);
  void 		Insert(void * i);
  void		Append(void * i);
  void *  	Contents(void);
  void 		Set_Contents(void * i);

};

class VPDlist {
private:
  friend class VPDlist_iter;
  friend class VPDfinger;
  VPDlist_node * head, * tail;
  int count;
  Boolean KillContents;
  // Returns false on fail
  Boolean Delete(VPDlist_node * v);
  // Unsafely nukes list
  void Zap(void);
public:
  VPDlist(void);
  // If Kill is true, then contents deleted on destroy of list
  // Default is false
  VPDlist(Boolean Kill);
virtual ~VPDlist(void);
  // Destroys contents of list as well. Assumes objects created with new.
  void Destroy(void);
 
  // if fail returns 0.
  int Insert(void * it); // Put on beginning of list
  int Push(void * it);   // Put on end of list
  
  // If fail returns invalid finger...
  VPDfinger InsertF(void * it);
  VPDfinger PushF(void * it);

  int Append(VPDlist & l); // Put l on end of current list. Destroys l
  
  // returns null (0) on empty list.
  void * PeekHead(void);     // Peek at beginning
  void * PeekTail(void);     // Peek at end
      
  void * Shift(void);        // Pull off beginning
  void * Pop(void);          // Pull off end

  int size(void) const;
  Boolean Is_Valid(void) const;
} ;

// If you make any changes to list while this list iterator is in
// use at the least split will very likely break, at the worst you may
// spew utter garbage. You have been warned.
class VPDlist_iter : private VPDfinger {
private:
  enum iter_state { OK, BOL, EOL, BAD };
  iter_state state;
  int position;
  void Zap(void);
public:
  VPDlist_iter(void);
  VPDlist_iter(VPDlist & it);
  VPDlist_iter(VPDlist_iter & it);
  VPDlist_iter(VPDfinger & it);
  void Set_list(VPDlist & it);
  void Set_list(VPDlist_iter & it);
  void Head(void);
  void Tail(void);
  void Next(void);
  void Prev(void);
  
  /* 
    Takes everything before current element and appends onto 'a'
    Current element and everything after goes on 'b' splitf assumes that
    point is near beginning of list if beginning is true, near end of
    list if false. These are
    linear in size of list due to member count updates. However, if only
    Next & Prev operators were used on this list, this is constant
    time...
    Special note: kills list (and thus iterator)!
    */

  void split(VPDlist & a, VPDlist & b,Boolean beginning = true);

  Boolean	Bol(void);
  Boolean	Eol(void); 
  void 	Delete(void);
  // inserts an item in before this position. If Eol(void) true, ends up
  // as an append. 
  void	Insert(void * i);
  void	Append(void * i);
  
  void * Current(void);
  void   Set_Cur(void * i);
  // combines two commonly used routines into one.
  void * Advance(void);
};
#endif
