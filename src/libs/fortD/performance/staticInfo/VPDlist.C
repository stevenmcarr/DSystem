/* $Id: VPDlist.C,v 1.1 1997/03/11 14:29:06 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// This may look like c code but it is really -*- C++ -*-

// Doubly linked list class.

// This class can have some suprises, since it destroys T when destroing
// an VPSlist node. If this is not what you want use one of the other classes.

/*
I really should scavenge loose nodes of VPS list and reuse them, rather than newing and deleteing them. But I want to get this code running first
*/

#include <libs/fortD/performance/staticInfo/VPDlist.h>


VPDlist_node::VPDlist_node(void * it) {
  item = it;
  prev = (VPDlist_node *) 0;
  next = (VPDlist_node *) 0;
}

VPDlist_node::~VPDlist_node(void) {
}

void VPDlist_node::Clear(void) {
  prev = (VPDlist_node *) 0;
  next = (VPDlist_node *) 0;
  item = (void *) 0;
}

Boolean VPDlist_node::ValidNext(void) {
  return Boolean((next != 0) && (next->prev == this));
}

Boolean VPDlist_node::ValidPrev(void) {
  return Boolean((prev != 0) && (prev->next == this));
}

////////////////////////////////////////////////////////////////////////////
// VPDfinger
////////////////////////////////////////////////////////////////////////////

VPDfinger::VPDfinger(VPDlist_node * n, VPDlist * l) {
  node = n;
  list = l;
}

VPDfinger::VPDfinger(void) {
  node = 0;
  list = 0;
}

VPDfinger::VPDfinger(const VPDfinger & it) {
  node = it.node;
  list = it.list;
}

VPDfinger::~VPDfinger(void) {
  node = 0;
  list = 0;
}

void VPDfinger::Invalidate(void) {
  node = 0;
  list = 0;
}

VPDfinger & const VPDfinger::operator = (const VPDfinger & r) {
  node = r.node;
  list = r.list;
  return *this;
}
  
int		VPDfinger::operator == (const VPDfinger & r) {
  if ((r.node == node) && (r.list == list)) {
    return true;
  } else {
    return false;
  }
}

Boolean 	VPDfinger::Is_Valid(void) {
  if ((node != 0) && (list !=0)) {
    return true;
  } else {
    return false;
  }
}
 
void 		VPDfinger::Delete(void) {
  if (!Is_Valid()) {
    Invalidate();
    return;
  } 
  if (list->KillContents) {
    delete node->item;
  }
  // Some day we should check this value and do something;
  list->Delete(node);
  Invalidate();
  return;
}

void		VPDfinger::Insert(void * i) {
  if (!Is_Valid()) return;
  VPDlist_node * prev = new VPDlist_node(i);
  // Link it in
  prev->prev = node->prev;
  prev->next = node;
  list->count ++;
  if (node->prev ==0 && list->head == node) {
    // I am at head of list:
    // Link it in
    node->prev = prev;
    list->head = prev;
  } else {
    node->prev->next = prev;
    node->prev = prev;
  }
}

void		VPDfinger::Append(void * i) {
  if (!Is_Valid()) return;
  VPDlist_node * next = new VPDlist_node(i);
  // Link it in
  next->next = node->next;
  next->prev = node;
  list->count ++;
  if (node->next ==0 && list->tail == node) {
    // I am at tail of list:
    // Link it in
    node->next = next;
    list->head = next;
  } else {
    node->next->prev = next;
    node->next = next;
  }
}

void *  	VPDfinger::Contents(void) {
  if (Is_Valid()) {
    return node->item;
  } else {
    return 0;
  }
}

void   		VPDfinger::Set_Contents(void * i) {
  if (Is_Valid()) {
    node->item = i;
  }
}

////////////////////////////////////////////////////////////////////////////
// VPDlist
////////////////////////////////////////////////////////////////////////////

VPDlist::VPDlist(void) {
  Zap();
  KillContents = false;
}

VPDlist::VPDlist(Boolean Kill) {
  Zap();
  KillContents = Kill;
}

VPDlist::~VPDlist(void) {
  VPDlist_node * pos = head;
  while (pos) {
    pos = pos->next;
    delete head;
    if (KillContents) {
      delete head->item;
    }
    head = pos;
  }
}

Boolean VPDlist::Delete(VPDlist_node *v) {
  /* We have four cases:
   * Delete head
   * Delete tail
   * delete normal node
   * delete invalid node
   */	

  // Check for head
  if (head == v) {
    // Delete head of list:
    // Need to check special case of single item list:
    if (tail == v) {
      if (KillContents && v!=0) {
	delete v->item;
      }
      v->Clear();
      tail =0;
      head =0;
      count = 0;
    } else if (v->ValidNext()) {
      // Just a normal deletion of first item:
      head = v->next;
      v->next->prev = 0;
      if (KillContents && v !=0) {
	delete v->item;
      }
      v->Clear();
      count --;
    } else {
      // Error of some sort:
      v->Clear();    
      return false;
    }
  } else if (tail == v) {
    // This is the last item on the list;
    if (v->ValidPrev()) {
      // Just a normal deletion of last item;
      tail = v->prev;
      v->prev->next = 0;
      if (KillContents && v!=0) {
	delete v->item;
      }
      v->Clear();
      count --;
    } else {
      // Error of some sort;
      v->Clear();
      return false;
    }
  } else {
    // Neither head nor tail:
    if (v->ValidPrev() && v->ValidNext()) {
      // node is ok in list:
      v->prev->next = v->next;
      v->next->prev = v->prev;
      count --;
      if (KillContents && v!=0) {
	delete v->item;
      }
      v->Clear();
    } else {
      // Bad node:
      v->Clear();
      return false;
    }
  }	
  // If we made it here, things went ok:
  return true;
}



void VPDlist::Destroy(void) {
  VPDlist_node * pos = head;
  while (pos) {
    pos = pos->next;	
    if (head->item !=0) {
      delete head->item;
    }
    delete head;
    head = pos;
  }
}

int VPDlist::Insert(void * it) {
  VPDlist_node * tmp = new VPDlist_node(it);
  if (head == (VPDlist_node *) 0) {
    // Got an empty list
    tail = tmp;	
    head = tmp;
    count = 1;
  } else {
    tmp->next = head;
    head->prev = tmp;
    head = tmp;
    count++;
  }
  return 1;
}

int VPDlist::Push(void * it) {
  VPDlist_node * tmp = new VPDlist_node(it);
  if (tail == (VPDlist_node *) 0) {
    tail = tmp;
    head = tmp;
    count = 1;
  } else {
    tmp->prev = tail;
    tail->next = tmp;
    tail = tmp;
    count++;
  }	  
  return 1;
}


VPDfinger VPDlist::InsertF(void * it) {
  VPDlist_node * tmp = new VPDlist_node(it);
  if (head == (VPDlist_node *) 0) {
    // Got an empty list
    tail = tmp;	
    head = tmp;
    count = 1;
  } else {
    tmp->next = head;
    head->prev = tmp;
    head = tmp;
    count++;
  }
  // Now we create a finger pointing at this element:
  VPDfinger f(tmp,this);
  return f;
}

VPDfinger VPDlist::PushF(void * it) {
  VPDlist_node * tmp = new VPDlist_node(it);
  if (tail == (VPDlist_node *) 0) {
    tail = tmp;
    head = tmp;
    count = 1;
  } else {
    tmp->prev = tail;
    tail->next = tmp;
    tail = tmp;
    count++;
  }	  
  // Now we create a finger pointing at this element:
  VPDfinger f(tmp,this);
  return f;
}

int VPDlist::Append(VPDlist & l) {
  if (Is_Valid()) {
    if (l.Is_Valid()) {
      // Two lists:
      tail->next = l.head;
      l.head->prev = tail;
      tail = l.tail;
      count += l.count;
    }
  } else {
    // this is an empty list
    if (l.Is_Valid()) {
      count = l.count;
      head  = l.head;
      tail  = l.tail;
    } else {
      // appending two empty lists....
      Zap();
    } 
  }  
  // l is no longer a valid list
  l.Zap();
  return 1;
}

void * VPDlist::PeekHead(void) {
  // What to do?
  if ((count <= 0) || (head == 0)) {
    // We have empty list
    return (void *)0;
  } else {
    return head->item;
  }
}

void * VPDlist::PeekTail(void) {
  // What to do?
  if ((count <= 0) || (head == 0)) {
    // We have empty list
    return (void *)0;
  } else {
    return tail->item;
  }
}

void * VPDlist::Shift(void) {
  void * tmp = (void *) 0;
  // What to do?
  if ((count <= 0) || (head == 0)) {
    // We have empty list
    return (void *)0;
  } else {
    tmp = head->item;
    if ((tail == head) || (count == 1)) {
      // Last item
      delete head;
      Zap();
    } else {
      VPDlist_node * last = head;
      head  = head->next;
      head->prev = 0;
      delete last;
      count --;
    }
  }
  return tmp;
}

void * VPDlist::Pop(void) {
  void * tmp = (void *) 0;
  // What to do?
  if ((count <= 0) || (tail == 0)) {
    // We have empty list
    return (void *)0;
  } else {
    tmp = tail->item;
    if ((tail == head) || (count == 1)) {
      // Last item
      delete tail;
      Zap();
    } else {
      VPDlist_node * last = tail;
      tail  = tail->prev;
      tail->next = 0;
      delete last;
      count --;
    }
  }
  return tmp;
}

int VPDlist::size(void) const { 
  if (Is_Valid()) 
    return count;
  else 
    return 0;
}

Boolean VPDlist::Is_Valid(void) const {
  return Boolean((this !=0) && (head !=0) && (tail !=0) && (count >=0));
}

void VPDlist::Zap(void) {
  if (this != 0) {
    head = (VPDlist_node *) 0;
    tail = (VPDlist_node *) 0;
    count = 0;
  }
}


////////////////////////////////////////////////////////////////////////////
// VPDlist_iter
////////////////////////////////////////////////////////////////////////////

void VPDlist_iter::Zap(void) {
  state = BAD;
  position = -1;
  node = (VPDlist_node *)0;
  list = (VPDlist *)0;
}


VPDlist_iter::VPDlist_iter(void) {
  Zap();
}


VPDlist_iter::VPDlist_iter(VPDlist & it) {
  list = & it;
  node = it.head;
  state = OK;
  position = 1;
  if (node == 0) 
    Zap();
}

VPDlist_iter::VPDlist_iter(VPDlist_iter & it) {
  list = it.list;	
  node = it.node;
  state = it.state;
  position = it.position;
}

VPDlist_iter::VPDlist_iter(VPDfinger & it) {
  if ((it.node != 0) && (it.list !=0)) { // it.Is_Valid
    list = it.list;	
    node = it.node;
    state = OK;
    position = -1;
  } else {
    Zap();
  }
}

void 	VPDlist_iter::Set_list(VPDlist & it) {
  list = & it;
  if (list != 0) {
    node = it.head;
    position = 1;
    if (node != 0) {
      state = OK;
      return;
    }
  }
  state = BAD;
}

void VPDlist_iter::Set_list(VPDlist_iter & it) {
  list = it.list;
  node = it.node;
  position = it.position;
  state = it.state;
}

Boolean	VPDlist_iter::Bol(void) {
  return Boolean((state == BOL) || (state == BAD));
}

Boolean	VPDlist_iter::Eol(void) {
  return Boolean((state == EOL) || (state == BAD));
}

void 	VPDlist_iter::Head(void) {
  if (list) {
    node = list->head;
    state = OK;
    position = 1;
  } else {
    state = BAD;
  }
}


void 	VPDlist_iter::Tail(void) {
  if (list) {
    node = list->tail;
    state = OK;
    position = list->count;
  } else {
    state = BAD;
  }
}

void 	VPDlist_iter::Next(void) {
  if (state == OK) { 
    if (node == 0) {
      state = EOL;
    } else {
      node = node->next;
      if (position != -1)
	position ++;	
      if (node == 0) {
	state = EOL;
     }
    }
  } else if (state == BOL) {
    node = list->head;
    state = OK;
    position = 1;
  } else {
    node = (VPDlist_node * ) 0;	
    position = -1;
  }
}

void 	VPDlist_iter::Prev(void) {
  if (state == OK) {
    if (node == 0) {
      state = BOL;
    } else {
      node = node->prev;
      if (position != -1)
	position --;
      if (node == 0) {
	state = BOL;
      }
    }
  } else if (state == EOL) {
    node = list->tail;
    state = OK;
    position = list->count;
  } else {
    node = (VPDlist_node * ) 0;	
  }
}


void  	VPDlist_iter::Delete(void) {
  VPDfinger f(node, list);
  Next();
  f.Delete();
  position = -1;
}

void 	VPDlist_iter::Insert(void * i) {
  VPDfinger::Insert(i);
  position++;
}

void 	VPDlist_iter::Append(void * i) {
  VPDfinger::Append(i);
}

void * VPDlist_iter::Current(void) { 
  return (state == OK) ? VPDfinger::Contents() : 0;
}
 
void   VPDlist_iter::Set_Cur(void * i) {
  if (state == OK) 
    VPDfinger::Set_Contents(i);
}

void * VPDlist_iter::Advance(void) { 
  void * tmp = Current(); 
  Next(); 
  return tmp;
}

void VPDlist_iter::split(VPDlist & a, VPDlist & b, Boolean beginning) {
  if (state == BAD) {
    // sigh. No way to provide an error return
    return;
  }
  // Quick special cases:
  if (state == BOL || node == list->head) {
    // everything goes on 'b'
    b.Append(*list);
    list->Zap();
    Zap();
    return;
  }
  if (state == EOL) {
    // everything goes on 'a'
    a.Append(*list);
    list->Zap();
    Zap();
    return;
  } 
  // Check for other easy cases:
  if (node == list->tail) {
    // last element goes on b:
    b.Push(list->Pop());
    a.Append(*list);
    list->Zap();
    Zap();
    return;    
  }
  if (position <1) {
    // Looks like someone started from a finger, or did a delete.
    // Sigh. We tried everything and nothing worked     
    // Linear time city.
    if (beginning) {
      // Compute the position by stepping through from the beginning:
      VPDlist_node * pos = list->head;
      position = 1;
      while(pos != node && pos->next != 0) {
	pos = pos->next;
	position ++;
      }
      if (pos->next == 0) {
	// This list is screwed. If this were the last node, then we
	// should have seen this before now. So node is not a node in
	// the list, or the list has a break in the chain.
	list->Zap();
	Zap();	
	return;
      }
    } else {
      // the luser thinks it is closer to the end:
      VPDlist_node * pos = list->tail;
      position = list->count;
      while(pos != node && pos->prev != 0) {
	pos = pos->prev;
	position ++;
      }
      if (pos->prev == 0) {
	// This list is screwed. If this were the first node, then we
	// should have seen this before now. So node is not a node in
	// the list, or the list has a break in the chain.
	list->Zap();
	Zap();
	return;
      }
    }
  }
  // State must be ok, and this is not the first or last element in
  // the list. We have a good position variable now;
  // We can do the constant time thing, since we do not need to
  // recompute the number of elements in each list:
  VPDlist parta;
  parta.head = list->head;
  parta.tail = node->prev;
  node->prev->next = 0;
  node->prev = 0;
  parta.count = position -1;
  a.Append(parta);
  list->count = list->count - position+1;
  list->head = node;
  b.Append(*list);
  list->Zap();
  Zap();
  return;        
}


