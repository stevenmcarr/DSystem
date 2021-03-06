/* $Id: iloc.h,v 1.2 1997/03/27 20:30:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/f2i/mnemonics.h>

static char *iloc_mnemonics[ ] = {
	"MEM",	"FRAME",
	"LDB",	"LD",	"LDF",	"LDD",	"LDC",
	"LDI",
	"STB",	"ST",	"STF",	"STD",	"STC",
	"MVB",	"MV",	"MVF",	"MVD",	"MVC",
	"CONV",
	"INC",	"DEC",
	"ADD",	"SUB",	"MUL",	"DIV",
	"FADD", "FSUB", "FMUL", "FDIV",
	"DADD", "DSUB", "DMUL", "DDIV",
	"CADD", "CSUB", "CMUL", "CDIV",
	"AND", 	"OR",	"COMPL","LSR",	"ASR",	"LSL",	"ASL",
	"B",	"BC",	"BR",   "FBR",  "DBR",   "CBR",
	"BS",	"BSC",
	"RTN",
	"BPUT",	"PUT",	"FPUT",	"DPUT",	"CPUT",
	"BGET",	"GET",	"FGET",	"DGET",	"CGET",
	"BRET",	"RET",	"FRET",	"DRET",	"CRET",
	"BVAL",	"VAL",	"FVAL",	"DVAL",	"CVAL",
	"IN",	"OUT",
	"HALT",	"NOP",
	"ERR",
	"DATA", "BSS", "TBL",
	"STATIC",	"PROG", "NAME",
	/* fill in the hole in the table */
	"invalid (84)",
	"invalid (85)", "invalid (86)",
	"invalid (87)", "invalid (88)",
	"invalid (89)",

	/* and the intrinsics */
	"AINT",		"DINT",
	"ANINT",	"DNINT",
	"NINT",		"IDNINT",
	"IABS",	"ABS",	"DABS",	"CABS",
	"MOD",	"AMOD",	"DMOD",	
	"ISIGN",	"SIGN",	"DSIGN",
	"IDIM",	"DIM",	"DDIM",
	"DPROD",
	"invalid (MAX - 110)",	
	"MAX0", "AMAX1","DMAX1","AMAX0","MAX1",
	"invalid (MIN - 116)",
	"MIN0", "AMIN1","DMIN1","AMIN0","MIN1",
	"LEN",
	"INDEX",
	"CONCAT",
	"AIMAG",
	"CONJG",
	"SQRT",	"DSQRT","CSQRT",
	"EXP",	"DEXP",	"CEXP",
	"invalid (LOG - 133)",	
	"ALOG",	"DLOG",	"CLOG",
	"invalid - (LOG10 - 137)",
	"ALOG10","DLOG10",
	"SIN",	"DSIN",	"CSIN",
	"COS",	"DCOS",	"CCOS",
	"TAN",	"DTAN",
	"ASIN",	"DASIN",
	"ACOS",	"DACOS",
	"ATAN", "DATAN",
	"ATAN2","DATAN2",
	"SINH",	"DSINH",
	"COSH",	"DCOSH",
	"TANH",	"DTANH"
};

static char* conv_flags[ ]  = {	"ERR", "if", "id", "ic", "fi", 
				"fd",  "fc", "di", "df", "ci", "cf" };

static char* data_flags[ ] = { "ERR", "i", "f", "d", "c", "l" };

