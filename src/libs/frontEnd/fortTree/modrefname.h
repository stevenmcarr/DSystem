/* $Id: modrefname.h,v 1.5 1997/03/11 14:29:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef modrefname_h
#define modrefname_h

#include <libs/support/tables/StringHashTable.h>
#include <libs/ipAnalysis/ipInfo/iptypes.h>

class ModRefNameInfo {
	StringHashTable modref[2];
	int muindex[2];
public:
    // add information to the mod or ref set, respectively
    void Add(char *name, ModRefType mu) {
		if (mu != MODREFTYPE_NEITHER) modref[mu].add_entry(name);
	};

    // ------------- query functions -------------

    // enumerate ref information for a leader
    unsigned int count(ModRefType mu) { 
		return (mu == MODREFTYPE_NEITHER) ? 0 : modref[mu].count(); 
	};
    char *first_entry(ModRefType mu) { 
		if (mu == MODREFTYPE_NEITHER) return 0;
		else {
			muindex[mu] = 0;
			return modref[mu].get_entry_by_index(muindex[mu]);
		}
	};
    char *next_entry(ModRefType mu) { 
		if (mu == MODREFTYPE_NEITHER) return 0;
		else return modref[mu].get_entry_by_index(++muindex[mu]);
	};
};

#endif modrefname_h
