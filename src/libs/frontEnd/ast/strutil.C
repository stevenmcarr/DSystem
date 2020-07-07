/* $Id: strutil.C,v 1.2 2001/10/12 19:37:04 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <string.h>
#include <iostream>
using namespace std;
#include <stdlib.h>

#include <libs/support/misc/general.h>
#include <include/bstring.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/frontEnd/ast/strutil.h>
#include <libs/support/strings/rn_string.h>

static void str_grow_strings();

/* actual declarations for global variables....*/

STR_INDEX str_null_string;
/* Pointer to the current asttab */
Strtab *strtab;

Strtab *str_select(Strtab* New)
{
	Strtab *old = strtab;

	/* If New is non-zero, select the new table */

	if (New != 0) strtab = New;

	/* Either way, return the old table pointer */

	return old;
}

Strtab *str_open(Generic size)
{
	Strtab *newtab = (Strtab *) get_mem(STR_TABLE_SIZE, "Strtab");
	Strtab *othertab = strtab;

	/* STR_INDEX is an unsigned short, so we must reject large requests */

	if (size > 65536)
		{
		cout << "string table size"<< size << "too big!" << endl;
		size = 65536;
		}

	/* Initialize the string table */
	newtab->table_entry = (Str_table_entry *) get_mem(STR_ENT_SIZE * size,"strings");
	bzero((char *)newtab->table_entry, STR_ENT_SIZE*size);

	newtab->stats.size            = size;
	newtab->stats.in_use          = 0;
	newtab->stats.total_allocs    = 0;
	newtab->stats.total_frees     = 0;
	newtab->stats.bytes_used      = 0;

	/* STR_INDEX holds the NULL string.... */
	newtab->table_entry[0].text   = ssave("");
	newtab->table_entry[0].type   = STR_NULL_STRING;
	newtab->table_entry[0].in_use = 1;
	NIL_STR = 0;

	newtab->str_free_list         = 0;
	newtab->stats.high_water_mark = 1;
	newtab->stats.bytes_used      = 1;
	strtab = othertab;
	return newtab;
}


void str_close(Strtab* s)
{
	Generic i;

	/* For each string in the table... */
	for (i=0; i < s->stats.high_water_mark; i++)
		{
#define MULLIN_STOP_LEAK
#ifdef  MULLIN_STOP_LEAK
		if ((s->table_entry[i].in_use)||
			(*(s->table_entry[i].text) == '\0'))
			sfree(s->table_entry[i].text);
#else  /* MULLIN_STOP_LEAK */
		if (s->table_entry[i].in_use)
			sfree(s->table_entry[i].text);
#endif /* MULLIN_STOP_LEAK */
		}

	free_mem((void*) s->table_entry);
	free_mem((void*)s);
	if (s == strtab) strtab = (Strtab *) 0;
}

Str_stats str_statistics()
{
	return strtab->stats;
}


STR_TEXT string_table_get_text(STR_INDEX ind)
{
	return strtab->table_entry[ind].text;
}

void    string_table_set_text(STR_INDEX ind, STR_TEXT text)
{
	sfree(strtab->table_entry[ind].text);
	strtab->table_entry[ind].text = text;
}

void str_reuse_symbol(STR_INDEX ind)
{
	strtab->table_entry[ind].in_use++;
}

STR_INDEX string_table_put_text(STR_TEXT text, STR_TYPE type)
{   
	STR_INDEX ind;
        if (strtab->str_free_list != 0) 
		{
		ind = strtab->str_free_list;
		strtab->str_free_list = 
			strtab->table_entry[ind].next;
		}
	else  {
	      if (strtab->stats.high_water_mark >= strtab->stats.size) {
		/* better grow the string table now..... */
		str_grow_strings();
		}

	      ind = strtab->stats.high_water_mark++;
	}
	strtab->stats.in_use++;
	strtab->stats.total_allocs++;
	strtab->table_entry[ind].text = ssave(text);
	strtab->table_entry[ind].type = type;
	strtab->table_entry[ind].in_use = 0;
	strtab->stats.bytes_used += strlen(text) + 1;
	return ind;
}

void string_table_free_symbol(STR_INDEX ind)
{
	Generic count = strtab->table_entry[ind].in_use;

        if (count == 0) {
		cout << "oops..freeing a freed string...." << endl;
		str_plain_dump(ind);
		abort();
		}
	else if (count == 1) {
		strtab->stats.bytes_used -= 
			    (strlen(strtab->table_entry[ind].text) + 1);
		sfree(strtab->table_entry[ind].text);
		strtab->table_entry[ind].text = ssave("");
		strtab->table_entry[ind].type = STR_FREED;
		strtab->table_entry[ind].next = strtab->str_free_list;
		strtab->str_free_list = ind;
		strtab->stats.in_use--;
		strtab->stats.total_frees++;			
		}
	strtab->table_entry[ind].in_use--;
}

STR_TYPE str_get_type(STR_INDEX ind)
{
	return strtab->table_entry[ind].type;
}

void str_put_type(STR_INDEX ind, STR_TYPE type)
{
	strtab->table_entry[ind].type = type;
}

void str_plain_dump(STR_INDEX i)
{
	cout << i << ": " << str_get_type(i) << "[count = "
	     << strtab->table_entry[i].in_use << "] '" 
	     << string_table_get_text(i) << "'" << endl;
}

void str_dump()
{
	int i;
	for (i=0;i< (int)strtab->stats.high_water_mark;i++) 
		if (strtab->table_entry[i].in_use)
	cout << i << ": " << "[count = " << strtab->table_entry[i].in_use << "] '" 
	     << string_table_get_text((STR_INDEX)i) << "'" << endl;
}

static void str_grow_strings()
{
	int i;
	Str_table_entry *entries;
	Generic newsize;

	newsize = 2 * strtab->stats.size;
	if (newsize > 65336) {
		if (newsize != 65336) newsize = 65336;
		cout << "string table size " << newsize << "too big!" << endl;
		};
	entries = (Str_table_entry *) 
			get_mem(STR_ENT_SIZE * newsize,"growing");
	bzero((char *)entries, STR_ENT_SIZE*newsize);

	for (i=0;i<(int)strtab->stats.size;i++) {
		entries[i] = strtab->table_entry[i];
		}

	strtab->stats.size = newsize;
	free_mem((void*)strtab->table_entry);
	strtab->table_entry  = entries;
}
