/* $Id: symtab.h,v 1.5 1997/03/11 14:31:21 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#define SYM_MAX_RENTS 10

typedef struct sym_struct {
    int   hash_link ;
    char* text ;
    int   is_generated ;
    int   dims ;
    int   di_type ;
    char* distribution ;
    int   rent[SYM_MAX_RENTS] ;
} SYMTAB ;

extern SYMTAB  symtab[];

#define SYM_NIL -1 

#define sym_dims(s) 		symtab[s].dims
#define sym_distribution(s) 	symtab[s].distribution
#define sym_di_type(s) 		symtab[s].di_type
#define sym_is_generated(s) 	symtab[s].is_generated 
#define sym_text(s)		symtab[s].text
#define sym_rent(s,i)		symtab[s].rent[i]
#define set_sym_dims(s,x) 	   symtab[s].dims = x
#define set_sym_distribution(s,x)  symtab[s].distribution = (char *) x
#define set_sym_di_type(s,x) 	   symtab[s].di_type = x
#define set_sym_is_generated(s,x)  symtab[s].is_generated = x
#define set_sym_rent(s,i,x)	   symtab[s].rent[i] = (int) x 

/*extern sym_init();*/
EXTERN(int, sym_search,(char *text, Boolean *newp));
