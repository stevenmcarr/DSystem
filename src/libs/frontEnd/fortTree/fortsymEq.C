/* $Id: fortsymEq.C,v 1.5 1997/03/11 14:29:55 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdlib.h>
#include <assert.h>
#include <search.h>

#include <libs/support/misc/general.h>
#include <libs/support/memMgmt/mem.h>
// #include <ip_info/iptypes.h>
#include <libs/frontEnd/ast/ast_include_all.h>

#include <libs/frontEnd/fortTree/fortsym.i>

STATIC(int, eqCompare, (const void* m1, const void* m2));


EXTERN(Boolean, fst_EquivSuperClassByIndex, (SymDescriptor d, fst_index_t var, 
					     fst_index_t *first, int *length));

typedef struct eqMemberS {
    fst_index_t sindex;
    int offset;
    int length;
    fst_index_t first;
} eqMember;

static int eqCompare(const void* m1, const void* m2)
{
    //  Go first by smaller offset
    //
    int temp = ((eqMember*)m1)->offset - ((eqMember*)m2)->offset;

    if (temp != 0)
	return temp;
    else
	//
	//  Then by larger length
	//
	return ((eqMember*)m2)->length - ((eqMember*)m1)->length;
}

Boolean fst_EquivSuperClassByIndex(SymDescriptor d, fst_index_t var, 
				   fst_index_t *first, int *length)
{
    fst_index_t temp;
    SymTable t = d->Table;
  
    LeaderTable_t *LTable = d->EQmap.LeaderTable;
    int *EqNext = d->EQmap.EqChainNext;
  
    if (var == SYM_INVALID_INDEX || var >= d->EQmap.tablesize) return false;

    if (SymFieldExists(t, SYMTAB_EQ_FIRST))
    {
	temp = SymGetFieldByIndex(t, var, SYMTAB_EQ_FIRST);

	if (temp != SYM_INVALID_INDEX)
	{
	    *first = temp;
	    *length = SymGetFieldByIndex(t, var, SYMTAB_EQ_EXTENT);
	    return true;
	}
    }
    else
    {
	SymInitField(t, SYMTAB_EQ_FIRST, SYM_INVALID_INDEX, NULL);
	SymInitField(t, SYMTAB_EQ_EXTENT, 0, NULL);
    }
  
    fst_index_t eqElt = LTable[var].leader;
  
    if (eqElt == SYM_INVALID_INDEX)
    {
	*first  = SYM_INVALID_INDEX;
	*length = 0;
	return false;
    }
    int eqMemberCount = d->EQmap.LeaderTable[eqElt].varcount;
  
    // not in common block
    int oc = SymGetFieldByIndex(t, eqElt, SYMTAB_OBJECT_CLASS);

    if (!(oc & OC_IS_COMMON_NAME))
    {
	// dummy argument or local variable; return head of equivalence class 
	// and eq class length (works for dummy args since each is in a 
	// singleton class)

	if (eqMemberCount == 0)		// singleton
	{
	    assert(eqElt == var);

	    *first  = var;
	    SymPutFieldByIndex(t, var, SYMTAB_EQ_FIRST, eqElt);

	    *length = SymGetFieldByIndex(t, var, SYMTAB_SIZE);
	    SymPutFieldByIndex(t, var, SYMTAB_EQ_EXTENT, *length);
	}
	else
	{
	    int lo = SymGetFieldByIndex(t, eqElt, SYMTAB_EQ_OFFSET);
	    int hi = lo + SymGetFieldByIndex(t, eqElt, SYMTAB_SIZE);
	    temp = eqElt;

	    if (SymGetFieldByIndex(t, eqElt, SYMTAB_STORAGE_CLASS) &
		(SC_ENTRY | SC_FUNCTION))
	    {
		// find function name
		//
		for (; eqElt != SYM_INVALID_INDEX; eqElt = EqNext[eqElt])
		{
		    if (SymGetFieldByIndex(t, eqElt, SYMTAB_STORAGE_CLASS) &
			SC_FUNCTION)
		    {
			temp = eqElt;
			break;
		    }
		}
	    }
	    else // not a function/entry name, find lowest-offset
	    {
		for (; eqElt != SYM_INVALID_INDEX; eqElt = EqNext[eqElt])
		{
		    int newLo = SymGetFieldByIndex(t, eqElt, SYMTAB_EQ_OFFSET);
		    if (newLo < lo)
		    {
			lo = newLo;
			temp = eqElt;
		    }
		    int newHi = newLo + SymGetFieldByIndex(t, eqElt, 
							   SYMTAB_SIZE);
		    hi = max(hi, newHi);
		}
		assert((lo <= 0) && (hi >= 0));
	    }

	    for (eqElt = LTable[var].leader; 
		 eqElt != SYM_INVALID_INDEX; 
		 eqElt = EqNext[eqElt])
	    {
		SymPutFieldByIndex(t, eqElt, SYMTAB_EQ_FIRST, temp);
		SymPutFieldByIndex(t, eqElt, SYMTAB_EQ_EXTENT, hi-lo);
	    }
	    *first  = temp;
	    *length = hi - lo;
	}
    }
    else if (var == eqElt) return false; // common block name
    else
    { 
	// variable in common block
	
	int lb = SymGetFieldByIndex(t, var, SYMTAB_EQ_OFFSET);
	int size = SymGetFieldByIndex(t, var, SYMTAB_SIZE);
    
	eqMember* members = 
	    (eqMember*)get_mem(sizeof(eqMember) * eqMemberCount, 
				 "fst_EquivSuperClassByIndex");
	
	eqElt = EqNext[eqElt]; // eqElt is common block name
	int i;
	for(i=0; eqElt != SYM_INVALID_INDEX; eqElt = EqNext[eqElt], i++) 
	{
	    members[i].sindex = eqElt;
	    members[i].offset = SymGetFieldByIndex(t, eqElt, 
						   SYMTAB_EQ_OFFSET);
	    members[i].length = SymGetFieldByIndex(t, eqElt, SYMTAB_SIZE);
	}
    
	qsort((char*)members, i, sizeof(eqMember), eqCompare);

	int j;
	int rep = 0;
	int high  = members[rep].length;
	members[rep].first = members[rep].sindex;

	for (j = 1; j < i; j++)
	{
	    if (members[j].offset < high)
	    {
		members[j].first = members[rep].sindex;
		high = max(high, members[j].offset + members[j].length);

		//  We're looking for the lowest-offset member of each
		//  overlapping set originally in common.
		//
		if ((members[j].offset == members[rep].offset) &&
		    (!is_null_node(SymGetFieldByIndex(t, members[j].sindex, 
						      SYMTAB_COMMON_STMT))))
		{
		    //  The next loop will set members[].first to
		    //  this for all vars in the overlapping set.
		    //
		    members[rep].first = members[j].sindex;
		}
	    }
	    else
	    {
		SymPutFieldByIndex(t, members[rep].sindex, SYMTAB_EQ_FIRST,
				   members[rep].first);
		SymPutFieldByIndex(t, members[rep].sindex, SYMTAB_EQ_EXTENT,
				   high - members[rep].offset);

		//  Entering a new overlapping set, disjoint from the last
		//
		rep = j;
		high = members[rep].offset + members[rep].length;
		members[rep].first = members[rep].sindex;
	    }
	}
	//  These didn't get set for the last iteration
	//
	SymPutFieldByIndex(t, members[rep].sindex, SYMTAB_EQ_FIRST,
			   members[rep].first);
	SymPutFieldByIndex(t, members[rep].sindex, SYMTAB_EQ_EXTENT,
			   high - members[rep].offset);

	rep = 0;

	for (j = 1; j < i; j++)
	{
	    if (members[j].first == members[rep].sindex)
	    {
		SymPutFieldByIndex(t, members[j].sindex, SYMTAB_EQ_FIRST,
				   members[rep].first);

		SymPutFieldByIndex(t, members[j].sindex, SYMTAB_EQ_EXTENT,
				   SymGetFieldByIndex(t, members[rep].sindex,
						      SYMTAB_EQ_EXTENT));
	    }
	    else rep = j;
	}

	free_mem(members);

	*first  = SymGetFieldByIndex(t, var, SYMTAB_EQ_FIRST);
	*length = SymGetFieldByIndex(t, var, SYMTAB_EQ_EXTENT);
    }
    assert(*first > SYM_INVALID_INDEX);
    return true;
}
