/* $Id: LocalOverlap.h,v 1.9 1997/03/11 14:28:42 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//-------------------------------------------------------------------------
// author : Seema Hiranandani
// content: the routines in this file are used to compute a coarse
//          approximation of overlap areas for arrays. 
//          It is extremely naive in that it looks at subscript expressions
//          of the form  (n -/+ c) where c is a constant expression and
//          n is an index variable. 
//          It stores the value of c, the array and the dimension
//          the subscript expression occurs in
// date   : August 1992
//-------------------------------------------------------------------------
#ifndef LocalOverlap_h
#define LocalOverlap_h

#include <assert.h>
#include <string.h>

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#include <libs/fortD/misc/fd_types.h>
#include <libs/fortD/localInfo/CommBlock.h>

#include <libs/support/lists/IOSinglyLinkedList.h>

#ifndef HashTable_h
#include <libs/support/tables/HashTable.h>
#endif

#include <libs/support/file/FormattedFile.h>
#include <libs/support/strings/rn_string.h>
#include <libs/support/lists/SinglyLinkedList.h>

#define OverlapBeginString "BeginOverlapInformation"
#define OverlapEndString "EndOverlapInformation"

//-------------------------------------------------------------------------
class overlap_ent: public SinglyLinkedListEntryIO
{
  char *param_name;
  
public:
  int numdim;
  int  upper[DCMAX], lower[DCMAX];
  common_block_ent *common_entry;
  
  //-------------------------------------------------------------------------
  // overlap entry I/O
  //-------------------------------------------------------------------------
  int ReadUpCall(FormattedFile &port);
  int WriteUpCall(FormattedFile &port);
  
  
//-------------------------------------------------------------------------
// return the array name
//-------------------------------------------------------------------------
  char* name() 
    {
      return param_name;
    };
  
//-------------------------------------------------------------------------
// store the array name
//-------------------------------------------------------------------------
  void put_name(char *nme)
    {
      param_name = ssave(nme);
    };
  
//-------------------------------------------------------------------------
//  store the number of dimensions of array param_name
//-------------------------------------------------------------------------
  void dim(int dim_num)
    {
      numdim = dim_num;
    };
  
//-------------------------------------------------------------------------
//  get the number of dimensions of array param_name
//-------------------------------------------------------------------------
  int getdim()
    {
      return numdim;
    };
  
//-------------------------------------------------------------------------
// constructor for overlap entry, assigns values to the data object.
//-------------------------------------------------------------------------
  overlap_ent(char *name = NULL)
    {
      int i;
      
      numdim = 0;
      if(name != NULL)
	param_name = ssave(name);
      for(i=0;i<DCMAX;++i)
	{
	  upper[i] = 0;
	  lower[i] = 0;
	};
      common_entry = 0;
    };
  
  overlap_ent(overlap_ent *e, char* name, common_block_ent *c = NULL)
    {
      param_name = ssave(name);
      numdim = e->getdim();
      
      for(int i=0;i<DCMAX;++i)
	{
	  upper[i] =  e->upper[i];
	  lower[i] =  e->lower[i];
	}
      common_entry = c;
    };
  
  overlap_ent(overlap_ent *e)
    {
      param_name = ssave(e->name());
      numdim = e->getdim();
      common_entry = e->common_entry;
      
      for(int i=0;i<DCMAX;++i)
	{
	  upper[i] =  e->upper[i];
	  lower[i] =  e->lower[i];
	}
    };
  
  void put_commonb_entry(common_block_ent *e)
    {
      common_entry = e;
    };
  
  common_block_ent *get_commonb_entry()
    {
      return(common_entry);
    };
  
//-------------------------------------------------------------------------
// union the lower, upper overlap areas for a particular dimension of an
// overlap entry
//-------------------------------------------------------------------------
  void union_t(int up, int lo, int dim)
    {
      if (up > upper[dim])
	upper[dim] = up;
      if (lo < lower[dim])
	lower[dim] = lo;
    };   
  
  void union_t(overlap_ent* e)
    {
      for (int i=0; i< numdim; ++i)
	{
	  if(upper[i] < e->upper[i])
	    upper[i] = e->upper[i];
	  if(lower[i] > e->lower[i])
	    lower[i] = e->lower[i];
	}
    };
  
};

//-------------------------------------------------------------------------
// a list of overlap entries for each procedure
//-------------------------------------------------------------------------
class OverlapList:  public SinglyLinkedListIO
{
  overlap_ent *current;
  
  public :
  SinglyLinkedListEntryIO *NewEntry() {  // never used
   return new overlap_ent();
  }
  
  overlap_ent*  append_entry(char *name)
    {
      overlap_ent *e = new overlap_ent(name);
      SinglyLinkedList::Append((SinglyLinkedListEntry *) e);
      return(e);
    };
  
  void append_entry(overlap_ent *a)
    {
      SinglyLinkedList::Append((SinglyLinkedListEntry *) a);
    };
  
  overlap_ent *first_entry() 
    { 
      return current = (overlap_ent *) SinglyLinkedList::First(); 
    };
  
  overlap_ent *next_entry() 
    { 
      return current = 
	(current ? (overlap_ent *) current->Next() : 0);
    };
  
  overlap_ent *get_entry(char *name)
    {
      overlap_ent *i;
      for(i = first_entry(); i != 0; i = next_entry())
	if(strcmp(i->name(), name) == 0)
	  return(i);
// if not found return 0;
      return(0);
    };
  
};

#endif
