/* $Id: gen.h,v 1.10 1997/03/11 14:29:29 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*************************************************************************
 This module defines the format of the Rn - Fortran abstract syntax tree,
 not its actual physical structure, but its logical - "this is what a node
 is" structure. This information has been condensed, extended, derived
 from many places.					      May-21-85-ACW
 *************************************************************************/

#ifndef gen_h
#define gen_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#ifndef astmeta_h
#include <include/frontEnd/astmeta.h>
#endif

#ifndef astnode_h
#include <include/frontEnd/astnode.h>
#endif

#ifndef astutil_h
#include <libs/frontEnd/ast/astutil.h>
#endif

/* these are cludges to allow us to initialize our tree descriptors 
   statically... C does not allow a zero-length initializer, so we
   insert one of these values into the list. (Each structure contains
   a short telling how many values are really in the list.)
*/
#define  NO_SONS    -1
#define  NO_EXPANSIONS  -1

/* These two values will be orded into the table values to indicate
   that that value is a LIST instead of a single element, or a META
   type instead of a NODE type.
*/

#define  META        (1 <<  9)
#define  LIST        (1 << 10)
#define  ANY_META    (1 << 11)
#define  OPTIONAL    (1 << 12)
   /* SKW */
#define  RECOMMENDED (1 << 13)

   /* SKW */
#define THE_TYPE(t)  (t & 0xff)
   /* SKW */
#define THE_MTYPE(t) (t & (0xff|META))

struct nodedef_t {
        char     *nodename;
	short    inson;
        unsigned number_of_sons:13,
                 statement:1,
                 compound:1,
                 simple:1;
	short    sons[6];
};

struct metamap_t {
	char     *metaname;
	unsigned text_editable:1,
	         count:15;
	short    exp[45];
};

extern  struct metamap_t metamap[];
extern  struct nodedef_t nodedef[];

#define is_LIST(n)        (n & LIST)
#define is_meta(n)        (n & META)
#define is_ANY_META(n)    (n & ANY_META)
#define is_OPTIONAL(n)    (n & OPTIONAL)
#define is_RECOMMENDED(n) (n & RECOMMENDED)

#define get_son(which,nt)          	 (nodedef[nt].sons[which-1])
#define get_jth(j,mt)              	 (metamap[mt].exp[j])
#define is_text_editable(metatype) 	 (metamap[metatype].text_editable)
#define gen_meta_type_get_text(metatype) (metamap[metatype].metaname)
#define gen_node_type_get_text(nodetype) (nodedef[nodetype].nodename)
#define gen_how_many_sons(nodetype) 	 (nodedef[nodetype].number_of_sons)
#define is_compound(n) 			 (nodedef[ast_get_node_type(n)].compound)
#define is_statement(n)			 (nodedef[ast_get_node_type(n)].statement)
#define is_simple(n) 			 (nodedef[ast_get_node_type(n)].simple)
#define in_son(n)    			 (nodedef[ast_get_node_type(n)].inson)

EXTERN(char *, gen_type_get_text, (Generic type));
EXTERN(char *, gen_type_get_text_short, (Generic type));
EXTERN(AST_INDEX, gen_rph, (META_TYPE phtype));
EXTERN(AST_INDEX, gen_oph, (META_TYPE phtype));

#define is_optional_ph(node)     (gen_get_status(node) == PLACE_OPTIONAL)
#define is_required_ph(node)     (gen_get_status(node) == PLACE_REQUIRED)

#define gen_get_real_type(n)		  ast_get_real_type(n)
#define gen_put_real_type(n,v)		  ast_put_real_type(n,v)
#define gen_get_converted_type(n) 	  ast_get_converted_type(n)
#define gen_put_converted_type(n,v) 	  ast_put_converted_type(n,v)

#define gen_get_parens(index)		  ast_get_parens(index)
#define gen_put_parens(index,value)	  ast_put_parens(index,value)
#define gen_get_meta_type(index)	  ast_get_meta_type(index)
#define gen_put_meta_type(index,value)    ast_put_meta_type(index,value)
#define gen_get_status(index)      	  ast_get_status(index)
#define gen_put_status(index,value)       ast_put_status(index,value)

#define gen_get_label(n)    		  ast_get_son1(n)
#define gen_put_label(n,m)  		  ast_put_son1(n,m)

#define gen_get_text(n)     		  string_table_get_text(ast_get_symbol(n))
#define gen_put_text(n,text,type)         gen_put_symbol(n,string_table_put_text(text,type))

#define gen_get_symbol(n)   		  ast_get_symbol(n)
#define gen_put_symbol(n,symbol)   	  ast_put_symbol(n,symbol)

#define gen_get_error_code(n)		  ast_get_error_code(n)
#define gen_put_error_code(n,m)		  ast_put_error_code(n,m)

#define gen_get_display(n)		  ast_get_display(n)
#define gen_put_display(n,m)		  ast_put_display(n,m)

/*************************************************************************
 Constructors for each of the nodetypes, forces checking of the number of
 parameters to the gen routine.
 *************************************************************************/
#include <include/frontEnd/astcons.h>
#include <include/frontEnd/astsel.h>

#define	gen_get_son_n(i,j)	ast_get_son_n(i,j)
#define	gen_put_son_n(i,j,s)	ast_put_son_n(i,j,s)
#define gen_which_son(p,s)	ast_which_son(p,s)
#define	gen_get_node_type(n)	ast_get_node_type(n)

EXTERN(AST_INDEX, gen_node, (NODE_TYPE type, ...));
/* generate a node from scraps      */

EXTERN(AST_INDEX, gen_new_node, (NODE_TYPE type));
/* generate a completely new node.. */

EXTERN(void, gen_coerce_node, (AST_INDEX node, NODE_TYPE type));
/* change a node's nodetype       */

EXTERN(AST_INDEX, gen_any_list, (AST_INDEX list));
EXTERN(Generic, get_proper_meta_type, (AST_INDEX node));
EXTERN(Generic, get_proper_status, (AST_INDEX node));
EXTERN(AST_INDEX, gen_any_node, (AST_INDEX node));
EXTERN(AST_INDEX, gen_meta_list, (AST_INDEX list, Generic status,
 META_TYPE type));
EXTERN(AST_INDEX, gen_meta_node, (AST_INDEX node, Generic status,
 META_TYPE type));
EXTERN(Boolean, is_scope, (AST_INDEX node));
EXTERN(AST_INDEX, find_scope, (AST_INDEX node));
EXTERN(AST_INDEX, get_name_in_entry, (AST_INDEX node));
EXTERN(AST_INDEX, get_stmts_in_scope, (AST_INDEX node));
EXTERN(AST_INDEX, get_formals_in_entry, (AST_INDEX node));
EXTERN(char *, gen_status_get_text, (Generic value));
EXTERN(int, int_type, (char *type_str));

# define PLACE_REQUIRED	1
# define PLACE_OPTIONAL	2

/* operators */
#define UN_MINUS	1
#define UN_NOT		2

#define BIN_PLUS	1
#define BIN_EXPONENT	2
#define BIN_TIMES	3
#define BIN_CONCAT	4
#define BIN_DIVIDE	5
#define BIN_MINUS	6
#define BIN_EQ		7
#define BIN_LT		8
#define BIN_LE		9
#define BIN_GE		10
#define BIN_GT		11
#define BIN_NE		12
#define BIN_OR		13
#define BIN_AND		14
#define BIN_EQV		15
#define BIN_NEQV	16
#define BIN_ASSIGNMENT  17

#define ERROR_UNDEFINED     -1
#define ERROR_SYNTAX        -2
#define ERROR_TWICE_DEFINED -3
   /* please, no violence ... ACW */
#define ERROR_NOT_AN_ERROR  -4
#define ERROR_WRONG_SCOPE   -5 

/* communication with big_picture .... */
/* MARF WAS HERE... I DON'T THINK THESE ARE USED AND THEY ARE CAUSING PROBLEMS */
/* #define INSERT  1 */
/* #define DELETE  2 */

/* quick nodetype recognizers */
#ifndef astrec_h
#include <include/frontEnd/astrec.h>
#endif

#endif
