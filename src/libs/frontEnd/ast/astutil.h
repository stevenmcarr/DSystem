/* $Id: astutil.h,v 1.12 1997/03/11 14:29:24 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*

  astutil.h -- for users of AST who need to know some
  aspect of the internal structure of ASTs.

  The new implementation was designed by Keith Cooper,
  Dave Schwartz, and Scott Warren.  7/8/91

*/

#ifndef astutil_h
#define astutil_h

#include <libs/frontEnd/ast/ast.h>
#include <libs/support/database/newdatabase.h>
#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/strutil.h>


#define AST_VERSION     10  
/* I chose this number since the previous value was 9 -- dcs */


typedef unsigned short    NODE_TYPE;
typedef unsigned short    META_TYPE;
typedef unsigned char     STATUS;
typedef unsigned char     TYPE;      /* bottom 4 bits - real type
					top 4 bits    - converted type */

#define REAL_BITS       0x0f
#define CONVERTED_BITS  0xf0
#define ERROR_BIT       0x8000
#define NODE_TYPE_BITS  0x7fff
#define PAREN_BIT       0x8000

typedef unsigned char  AST_DATA;
typedef unsigned int   SCRATCH;


typedef union {
  struct LeafNodeStruct    *Leafnode;
  struct ListNodeStruct    *Listnode;
  struct Int_1_NodeStruct  *Int1node;
  struct Int_2_NodeStruct  *Int2node;
  struct Int_3_NodeStruct  *Int3node;
  struct Int_4_NodeStruct  *Int4node;
  struct Int_5_NodeStruct  *Int5node;
  struct Int_6_NodeStruct  *Int6node;
} NODE;


typedef struct LeafNodeStruct 
{
  NODE_TYPE      node_type;
  Generic       *side_array_ptr;
  TYPE           type;
  STATUS         status;
  SCRATCH        scratch;
  AST_INDEX      father;
  short          display;
  META_TYPE      meta_type;
  STR_INDEX      symbol;
} LeafNode;


typedef struct Int_1_NodeStruct 
{
  NODE_TYPE      node_type;
  Generic       *side_array_ptr;
  TYPE           type;
  STATUS         status;
  SCRATCH        scratch;
  AST_INDEX      father;
  short          display;
  META_TYPE      meta_type;
  AST_INDEX      son1;
} Int_1_Node;

typedef struct Int_2_NodeStruct 
{
  NODE_TYPE      node_type;
  Generic       *side_array_ptr;
  TYPE           type;
  STATUS         status;
  SCRATCH        scratch;
  AST_INDEX      father;
  short          display;
  META_TYPE      meta_type;
  AST_INDEX      son1;
  AST_INDEX      son2;
} Int_2_Node;

typedef struct Int_3_NodeStruct 
{
  NODE_TYPE      node_type;
  Generic       *side_array_ptr;
  TYPE           type;
  STATUS         status;
  SCRATCH        scratch;
  AST_INDEX      father;
  short          display;
  META_TYPE      meta_type;
  AST_INDEX      son1;
  AST_INDEX      son2;
  AST_INDEX      son3;
} Int_3_Node;

typedef struct Int_4_NodeStruct 
{
  NODE_TYPE      node_type;
  Generic       *side_array_ptr;
  TYPE           type;
  STATUS         status;
  SCRATCH        scratch;
  AST_INDEX      father;
  short          display;
  META_TYPE      meta_type;
  AST_INDEX      son1;
  AST_INDEX      son2;
  AST_INDEX      son3;
  AST_INDEX      son4;
} Int_4_Node;

typedef struct Int_5_NodeStruct 
{
  NODE_TYPE      node_type;
  Generic       *side_array_ptr;
  TYPE           type;
  STATUS         status;
  SCRATCH        scratch;
  AST_INDEX      father;
  short          display;
  META_TYPE      meta_type;
  AST_INDEX      son1;
  AST_INDEX      son2;
  AST_INDEX      son3;
  AST_INDEX      son4;
  AST_INDEX      son5;
} Int_5_Node;

typedef struct Int_6_NodeStruct 
{
  NODE_TYPE      node_type;
  Generic       *side_array_ptr;
  TYPE           type;
  STATUS         status;
  SCRATCH        scratch;
  AST_INDEX      father;
  short          display;
  META_TYPE      meta_type;
  AST_INDEX      son1;
  AST_INDEX      son2;
  AST_INDEX      son3;
  AST_INDEX      son4;
  AST_INDEX      son5;
  AST_INDEX      son6;
} Int_6_Node;

typedef struct ListNodeStruct
{
  NODE_TYPE      node_type;
  Generic       *side_array_ptr;
  AST_INDEX      next;
  AST_INDEX      prev;
  AST_INDEX      father;
  AST_INDEX      son;
} ListNode;




/* typedef struct SideArray
{
  int                 width;
  Generic            *array;
  Generic            *initial_values;
  struct SideArray   *next;
} Ast_side_array; */

typedef struct 
{
  int              total_allocs;
  int              total_frees;
  Generic         *side_array_in_use;
  Generic         *side_array_initial_values;
  int              side_array_width;
  Generic        **SideArrayTable;
  int              SideArrayTableSize;
  int              SideArrayTableHWM;
} Ast_stats;

typedef struct {
  char      *node_name;
  int        son_count;
} Nodeinfo;

typedef struct AsttabStruct
{
  Strtab      *strtab;
  Ast_stats    stats;
  Nodeinfo    *nodeinfo;
  AST_INDEX    root;
} Asttab;



/* This is used to convert an AST_INDEX to a NODE *       */
/* so we can still have functions take AST_INDEX as an    */
/* argument.                                              */

#define N(ob)  ((NODE *)ob)

#define INIT_SIDE_ARRAY_TABLE_SIZE 128

/****************************************************************************/
# define AST_FREED_NODE		0
# define AST_NULL_NODE		1
# define AST_LIST_OF_NODES	2

# define AST_TABLE_SIZE 	sizeof(struct AsttabStruct)

# define AST_MAX_SON 		6 

/****************************************************************************/

/* In the following macros, ptr is a NODE *.  */


#define ast_put_node_type(ptr, value)                      \
                      {                                    \
		      ((NODE *)ptr)->Leafnode->node_type &= ~NODE_TYPE_BITS;  \
                      ((NODE *)ptr)->Leafnode->node_type |= value;   \
		      }
#define ast_get_node_type_name(nt)     \
                      asttab->nodeinfo[nt].node_name

#define ast_get_node_type_son_count(nt) \
                      asttab->nodeinfo[nt].son_count
#define ast_get_son_count(ptr) \
                      ast_get_node_type_son_count(ast_get_node_type(ptr))
#define ast_get_common_info(ptr) ast_get_var_info(ptr)
#define ast_put_common_info(ptr, value) ast_put_var_info(ptr, value)

#define ast_get_usage(ptr) ast_get_other(ptr)
#define ast_put_usage(ptr, value) ast_put_other(ptr, value)

#define ast_get_son1(ptr)   \
            ast_get_son_n(ptr, 1)
#define ast_get_son2(ptr)   \
            ast_get_son_n(ptr, 2)
#define ast_get_son3(ptr)   \
            ast_get_son_n(ptr, 3)
#define ast_get_son4(ptr)   \
            ast_get_son_n(ptr, 4)
#define ast_get_son5(ptr)   \
            ast_get_son_n(ptr, 5)
#define ast_get_son6(ptr)   \
            ast_get_son_n(ptr, 6)
#define ast_put_son1(ptr, s) \
            ast_put_son_n(ptr, 1, s)
#define ast_put_son2(ptr, s) \
            ast_put_son_n(ptr, 2, s)
#define ast_put_son3(ptr, s) \
            ast_put_son_n(ptr, 3, s)
#define ast_put_son4(ptr, s) \
            ast_put_son_n(ptr, 4, s)
#define ast_put_son5(ptr, s) \
            ast_put_son_n(ptr, 5, s)
#define ast_put_son6(ptr, s) \
            ast_put_son_n(ptr, 6, s)

#define ast_get_in_use() (asttab->stats.total_allocs - asttab->stats.total_frees)


/*
The (minimal) header for each ast file is laid out as follows:
system_version	- the contents of the current system version structure
type		- the file type
flags		- flags depending upon type
time		- the system time at the time the file was written
uid		- the login unix user id of the person writing the file
gid		- the login unix group id of the person writing the file
ast_size	- the number of ast nodes allocated when the file was written
ast_used        - the number of ast nodes actually in use
str_size	- the number of string table entries allocated when the 
		  file was written
str_used	- actual number of strings in use
str_bytes	- the number of bytes taken up by the actual strings from
		  the string table
*/

typedef struct
	{
	unsigned char hdr_version;
	unsigned char sym_version;
	unsigned char ast_version;
	unsigned char system;
	} System_version;

# define HDR_VERSION 7
# define SYSTEM_VAX 1
# define SYSTEM_SUN 2
# define SYSTEM_RT  3

typedef struct AstHeaderStruct
	{
	int        type;
	int        flags;
	int        time;
	int        uid, gid;
	int        str_size, str_used, str_bytes;
	/* Asttab    *asttab;  -- dcs 5/18/92 */
	AST_INDEX  tree;
	DB_FP     *buff_fp;
	int        checksum;
	} Header;

typedef struct
	{
	int     reads;
	int     rbytes;
	int     writes;
	int     wbytes;
	} Dba_stats;

# define DBTYPE_UNKNOWN		0
# define DBTYPE_FORTRAN_CODE	1
# define DBTYPE_LAST		2

# define HDR_HEADER_SIZE sizeof(struct AstHeaderStruct)




/* The following EXTERN declarations used to be */
/* macros.  They were turned into functions because error */
/* checking was needed. */

EXTERN(NODE_TYPE, ast_get_node_type, (AST_INDEX  node));
EXTERN(STR_INDEX, ast_get_symbol, (AST_INDEX  node));
EXTERN(void, ast_put_symbol, (AST_INDEX  node, STR_INDEX value));
EXTERN(AST_INDEX,   ast_get_father, (AST_INDEX  node));
EXTERN(META_TYPE, ast_get_meta_type, (AST_INDEX  node));
EXTERN(void, ast_put_meta_type, (AST_INDEX  node, META_TYPE meta_type));
EXTERN(STATUS, ast_get_status, (AST_INDEX  node));
EXTERN(void, ast_put_status, (AST_INDEX  node, STATUS status));
EXTERN(int, ast_get_real_type, (AST_INDEX  node));
EXTERN(int, ast_get_converted_type, (AST_INDEX  node));
EXTERN(Generic, ast_get_error_code, (AST_INDEX  node));
EXTERN(void, ast_put_error_code, (AST_INDEX  node, Generic error_code));
EXTERN(short, ast_get_var_info, (AST_INDEX  node));
EXTERN(void, ast_put_var_info, (AST_INDEX  node, short var_info));
EXTERN(short, ast_get_entry_info, (AST_INDEX  node));
EXTERN(void, ast_put_entry_info, (AST_INDEX  node, short entry_info));
EXTERN(AST_INDEX,   ast_get_link_prev, (AST_INDEX  node));
EXTERN(void, ast_put_link_prev, (AST_INDEX  node, AST_INDEX  link_prev));
EXTERN(AST_INDEX,   ast_get_link_next, (AST_INDEX  node));
EXTERN(void, ast_put_link_next, (AST_INDEX  node, AST_INDEX  link_next));
EXTERN(AST_INDEX,   ast_get_decl, (AST_INDEX  node));
EXTERN(void, ast_put_decl, (AST_INDEX  node, AST_INDEX  decl));
EXTERN(AST_INDEX,   ast_get_declaration, (AST_INDEX  node));
EXTERN(void, ast_put_declaration, (AST_INDEX  node, AST_INDEX  declaration));
EXTERN(SCRATCH, ast_get_scratch, (AST_INDEX  node));
EXTERN(void, ast_put_scratch, (AST_INDEX  node, SCRATCH scratch));
EXTERN(short, ast_get_display, (AST_INDEX  node));
EXTERN(void, ast_put_display, (AST_INDEX  node, short display));
EXTERN(short, ast_get_is_param, (AST_INDEX  node));
EXTERN(void, ast_put_is_param, (AST_INDEX  node, short is_param));
EXTERN(short, ast_get_other, (AST_INDEX  node));
EXTERN(void, ast_put_other, (AST_INDEX  node, short other));
EXTERN(short, ast_get_parens, (AST_INDEX  node));
EXTERN(void, ast_put_parens, (AST_INDEX  node, short value));
EXTERN(AST_INDEX,   ast_get_head, (AST_INDEX  node));
EXTERN(void, ast_put_head, (AST_INDEX  node, AST_INDEX  head));
EXTERN(AST_INDEX,   ast_get_next, (AST_INDEX  node));
EXTERN(void, ast_put_next, (AST_INDEX  node, AST_INDEX  next));
EXTERN(AST_INDEX,   ast_get_prev, (AST_INDEX  node));
EXTERN(void, ast_put_prev, (AST_INDEX  node, AST_INDEX  prev));
EXTERN(short, ast_get_length, (AST_INDEX  node));
EXTERN(void, ast_put_length, (AST_INDEX  node, short length));
EXTERN(AST_INDEX,   ast_get_last, (AST_INDEX  node));
EXTERN(void, ast_put_last, (AST_INDEX  node, AST_INDEX  last));
EXTERN(AST_INDEX,   ast_get_first, (AST_INDEX  node));
EXTERN(void, ast_put_first, (AST_INDEX  node, AST_INDEX  first));

/* The following EXTERN declarations have always been */
/* functions.  */

EXTERN(Boolean, is_leaf_node, (AST_INDEX  node));
EXTERN(Boolean, is_list_node, (AST_INDEX  node));
EXTERN(Boolean, is_error_code, (AST_INDEX  node));
EXTERN(void, ast_put_father, (AST_INDEX  son, AST_INDEX  father));
EXTERN(AST_INDEX,   ast_get_son, (AST_INDEX  node));
EXTERN(Generic, ast_which_son, (AST_INDEX   ind, AST_INDEX   son));
EXTERN(AST_INDEX,   ast_get_son_n, (AST_INDEX   ind, Generic n));
EXTERN(void, ast_put_son_n, (AST_INDEX   ind, Generic n, AST_INDEX   son));
EXTERN(void, ast_dump_all, (void));
EXTERN(void, ast_dump, (AST_INDEX   ind));
EXTERN(Ast_stats, ast_statistics, (void));
EXTERN(Asttab *, ast_open, (Nodeinfo *nodeinfo, Generic nodes));
EXTERN(void, ast_close, (Asttab *a));
EXTERN(void, ast_setroot, (Asttab *a, AST_INDEX root));
EXTERN(Asttab *, ast_create, (Nodeinfo *nodeinfo, Generic ast_size,
 Generic str_size));
EXTERN(void, ast_destroy, (Asttab *a));
EXTERN(Asttab *, ast_select, (Asttab *new_ast));
EXTERN(void, ast_zero_display_top, (void));
EXTERN(void, ast_zero_scratch, (AST_INDEX ind));
EXTERN(AST_INDEX,   ast_alloc, (NODE_TYPE node_type));
EXTERN(void, ast_free, (AST_INDEX   ind));
EXTERN(AST_INDEX,   ast_copy, (AST_INDEX   ind));
EXTERN(AST_INDEX,   ast_copy_with_type, (AST_INDEX   ind));
EXTERN(void, ast_put_real_type, (AST_INDEX   ind, Generic value));
EXTERN(void, ast_put_converted_type, (AST_INDEX   ind, Generic value));
EXTERN(AST_INDEX,   ast_gc, (AST_INDEX   root));

EXTERN(void, ast_export2, (DB_FP *fp, AST_INDEX   tree));
EXTERN(AST_INDEX,   ast_import2, (Nodeinfo *nodeinfo, DB_FP *fp,
 Asttab **tab));
EXTERN(AST_INDEX,   ast_read_nodes, (DB_FP *buff_fp));
EXTERN(void, ast_merge, (Nodeinfo nodeinfo, DB_FP *fp1, DB_FP *fp2,
 Asttab **tab, AST_INDEX  *tree1, AST_INDEX  *tree2));


EXTERN(Generic, ast_get_side_array, (Generic which, AST_INDEX   ind,
 Generic slot));
EXTERN(void, ast_put_side_array, (Generic which, AST_INDEX   ind, Generic slot,
 Generic value));
EXTERN(Generic, ast_attach_side_array, (Asttab *tab, Generic width,
 Generic *initial_values));
EXTERN(void, ast_detach_side_array, (Asttab *tab, Generic which));

/**********************************************************/
/**********************************************************/
/**********************************************************/
/**********************************************************/
/***********************old astutil.h *********************/
/**********************************************************/
/**********************************************************/

#define AST_VERSION_OLD    9


struct edges 
	{
	AST_INDEX    	son1;
	AST_INDEX    	son2;
	AST_INDEX    	son3;
	AST_INDEX    	son4;
	AST_INDEX	son5;
	AST_INDEX	son6;
	};

struct list 
	{
	short		length;
	AST_INDEX       first;
	AST_INDEX       last;
	AST_INDEX       head;
	AST_INDEX       prev;
	AST_INDEX       next;
	};

struct other { 
	short		entry_info;		/* now unused -- JMC 5/1/92 */
	short		var_info;		/* now unused -- JMC 5/1/92 */ 
	AST_INDEX 	link_prev;		/* now unused -- JMC 5/1/92 */
	AST_INDEX 	link_next;		/* now unused -- JMC 5/1/92 */
	union {
		AST_INDEX decl;			/* now unused -- JMC 5/1/92 */
		AST_INDEX declaration;		/* now unused -- JMC 5/1/92 */
	      } dcl_info;			/* now unused -- JMC 5/1/92 */
	Generic         display;
	short	   	is_param;		/* now unused -- JMC 5/1/92 */
	short	   	other;
	Generic		error_code;
	META_TYPE	meta_type;
	STATUS		status;
	TYPE		type;
	};

typedef struct AstnodeStruct
	{
	NODE_TYPE  	node_type; 
	STR_INDEX	symbol;
	SCRATCH		scratch;
	struct list     list;	/* list info */
	AST_INDEX	father;		
	struct edges	edges;  /* son  info */
	struct other	other;
	} Astnode;

typedef struct SideArray
	{
	int		width;
	Generic		*array;
        Generic		*initial_values;
	struct SideArray *next;
	} Ast_side_array;

typedef struct
	{
	int		table_size;
	int		node_size;
	int		in_use;
	int		total_allocs;
	int		total_frees;
	int		high_water_mark;
	Ast_side_array  *side_array_list;
	} Ast_statsOld;


typedef struct AsttabStructOld
	{
	Strtab		*strtab;
	Astnode		*astnodes;
	AST_INDEX	ast_free_list;
	Ast_statsOld	stats;
	Nodeinfo        *nodeinfo;
	} AsttabOld;



typedef struct AstHeaderOldStruct
        {
        int     type;
        int     flags;
        int     time;
        int     uid, gid;
        int     ast_size, ast_used;
        int     str_size, str_used, str_bytes;
        Asttab *asttab;
        int     tree;
        int     buff_port;
        int     checksum;
        } HeaderOld;

# define HDR_HEADER_SIZE_OLD sizeof(struct AstHeaderOldStruct)
# define AST_NODE_SIZE_OLD sizeof(struct AstnodeStruct)
# define AST_TABLE_SIZE_OLD sizeof(struct AsttabStructOld)
#define ast_get_next_old(index)	\
			(asttabOld->astnodes[(index)].list.next)

# define ast_get_node_type_old(index)       (asttabOld->astnodes[(index)].node_type)

#define ast_get_first_old(index)    \
                        (asttabOld->astnodes[(index)].list.first)

EXTERN(AsttabOld *, ast_open_old, (Nodeinfo *nodeinfo, Generic nodes));
EXTERN(AsttabOld *, ast_create_old, (Nodeinfo *nodeinfo, Generic ast_size, Generic str_size));
EXTERN(AST_INDEX, ast_alloc_old, (NODE_TYPE node_type));
EXTERN(AST_INDEX, ast_import2_old, (Nodeinfo *nodeinfo, DB_FP *buff_fp, AsttabOld **tab));
EXTERN(Boolean, is_list_old, (AST_INDEX node));
EXTERN(AST_INDEX, list_first_old, (AST_INDEX list));
EXTERN(AST_INDEX, ast_get_son_n_old, (AST_INDEX ind, Generic n));
EXTERN(AST_INDEX, ast_convert, (AST_INDEX node));
EXTERN(AST_INDEX, ast_copy_old_to_new, (AST_INDEX node));

#if 0
/**********************************************************/
/**********************************************************/
/*     AST I/O interface using C++ File abstraction       */
/**********************************************************/
/**********************************************************/

#ifdef __cplusplus
class File;
EXTERN(void, ast_export3, (File *fp, AST_INDEX tree));
EXTERN(AST_INDEX, ast_import3, (Nodeinfo *nodeinfo, File *fp, Asttab **tab));
#endif
#endif /* 0 */

#endif
