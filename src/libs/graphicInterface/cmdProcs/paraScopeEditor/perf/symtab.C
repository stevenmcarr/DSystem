/* $Id: symtab.C,v 1.1 1997/06/25 14:41:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <stdio.h>
#include <ctype.h>

#include <libs/graphicInterface/oldMonitor/include/mon/dialog_def.h>

#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/asttree.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/include/gi.h>

#include <include/frontEnd/astsel.h>

#include <include/frontEnd/astmeta.h>
#include <include/frontEnd/astnode.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/symtab.h>
#include <string.h>

#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/dp.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/perf.h>
#include <libs/graphicInterface/cmdProcs/paraScopeEditor/include/PedExtern.h>

EXTERN(char, *ssave,(const char* const str));

#define MAX_SYM 100

SYMTAB symtab[MAX_SYM] ;
int next_sym = 0 ;

int htable[256] ;
int rent_in_use[10] ; 		/*  for each rent field */

void
perf_sym_init() 
{
    int i ;
    for( i= 0; i < 256 ; i++) htable[i] = SYM_NIL ;
    for( i= 0; i < 10; i++) rent_in_use[i] = 0;
}



int
sym_search(char *text, Boolean *newp) 
{
    	int hash_val,sym_index,i ;
	hash_val = hash(text) ;
	sym_index = htable[hash_val] ;
	while (sym_index != SYM_NIL) {
	    if (strcmp(text,symtab[sym_index].text) == 0) break ;
	    sym_index = symtab[sym_index].hash_link ;
	}
	if (sym_index != SYM_NIL) {
	    *newp = false ;
	}
	else {
	    *newp = true ;
	    if (next_sym <= MAX_SYM) {
	    	sym_index = next_sym++ ;
		symtab[sym_index].hash_link = htable[hash_val] ;
		htable[hash_val] = sym_index ;
		symtab[sym_index].text = ssave(text) ;
		symtab[sym_index].dims = 0 ;
		symtab[sym_index].di_type = 0 ;
		symtab[sym_index].distribution = NULL ;
		symtab[sym_index].is_generated = 0;
		for (i = 0 ; i < SYM_MAX_RENTS ; i++)
		 	symtab[sym_index].rent[i] = MINUS_INFTY;
	    }
	    else sym_index = SYM_NIL ;
	}
	return (sym_index) ;
}

/*
 *  simple hash function: sum the integer values of the elements of the string 
 */
int hash (char *string) 
{
    int count ;
    for (count = 0; *string ; count += (int) *string, string++) ;
    return (count % 256) ;
}

int
sym_alloc_rent()
{
    int  i ;
    for (i =0; i<10; i++) {
    	if (! rent_in_use[i]) break ;
    }
    if (i < 10) {
    	rent_in_use[i] = 1;
	return (i) ;
    }
    printf("\n no more symbol table rent fields avaiable") ;
    return (-1) ;
}

void
sym_init_rent(int field, int val)
{
    int sym ;
    for (sym = 0 ; sym < next_sym ; sym ++) 
		symtab[sym].rent[field] = val ;
}

void
sym_free_rent(int i)
{
    if (0 <- i && i < 10) {
    	rent_in_use[i] = 0;
    }
}
