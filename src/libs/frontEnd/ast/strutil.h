/* $Id: strutil.h,v 1.6 1997/03/11 14:29:32 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#ifndef strutil_h
#define strutil_h

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

typedef unsigned short  STR_INDEX;
typedef Generic	        STR_TYPE;
typedef char*           STR_TEXT;

typedef struct {
	Generic 	size;
	Generic 	in_use;
	Generic 	total_allocs;
	Generic 	total_frees;
	Generic 	high_water_mark;
	Generic 	bytes_used;
	} Str_stats;

typedef struct {
	STR_TEXT  	text;
	STR_TYPE  	type;
	STR_INDEX 	next;
	short   	in_use;
	} Str_table_entry;

typedef struct {
	Str_stats       stats;
	Str_table_entry *table_entry;
	STR_INDEX       str_free_list;
	} Strtab;


extern STR_INDEX 	str_null_string;
extern Strtab		*strtab;

EXTERN(Strtab *, str_select, (Strtab *n));
EXTERN(Strtab *, str_open, (Generic size));
EXTERN(void, str_close, (Strtab *s));
EXTERN(Str_stats, str_statistics, (void));
EXTERN(STR_TEXT, string_table_get_text, (STR_INDEX ind));
EXTERN(void, string_table_set_text, (STR_INDEX ind, STR_TEXT text));
EXTERN(void, str_reuse_symbol, (STR_INDEX ind));
EXTERN(STR_INDEX, string_table_put_text, (STR_TEXT text, STR_TYPE type));
EXTERN(void, string_table_free_symbol, (STR_INDEX ind));
EXTERN(STR_TYPE, str_get_type, (STR_INDEX ind));
EXTERN(void, str_put_type, (STR_INDEX ind, STR_TYPE type));
EXTERN(void, str_plain_dump, (STR_INDEX i));
EXTERN(void, str_dump, (void));

#define STR_VERSION		9
#define STR_TABLE_SIZE		sizeof(Strtab)
#define STR_ENT_SIZE		sizeof(Str_table_entry)
#define NIL_STR 		str_null_string

/* type field in string table */
#define STR_FREED			0
#define STR_NULL_STRING 		1
#define STR_COMMENT_TEXT		2
#define STR_COMMON_NAME 		3
#define STR_CONSTANT_INTEGER		4
#define STR_CONSTANT_REAL		5
#define STR_CONSTANT_DOUBLE_PRECISION	6
#define STR_CONSTANT_COMPLEX		7
#define STR_CONSTANT_LOGICAL		8
#define STR_CONSTANT_BIT		9
#define STR_CONSTANT_CHARACTER		10
#define STR_EXPRESSION			11
#define STR_FORMAT_STRING		12
#define STR_IDENTIFIER			13
#define STR_LABEL_DEF			14
#define STR_LABEL_REF			15
#define STR_KEYWORD			16
#define STR_ERROR_MESSAGE		17
#define STR_TEXT_STRING         	18
#define STR_CONSTANT_EXACT		19
#define STR_CONSTANT_HEX		20
#define STR_CONSTANT_LETTER		21

#endif
