/* $Id: intrins.h,v 1.2 1997/03/27 20:30:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#define	NUMBER_OF_BUILTINS	94
#include <libs/f2i/mnemonics.h>



/* The following structure maps Fortran intrinsic functions to iloc opcodes */

struct a2i_builtins
 {
   char	*name;
   int	opcode;
 };

struct a2i_builtins a2i_builtins_map[] = {
"abs",		fABS,
"acos",		fACOS,
"aimag",	cIMAG,
"aint",		fTRUNC,
"alog",		fLOG,
"alog10",	fLOG10,
"amax0",	AMAX0,
"amax1",	fMAX,
"amin0",	AMIN0,
"amin1",	fMIN,
"amod",		fMOD,
"anint",	fROUND,
"asin",		fASIN,
"atan",		fATAN,
"atan2",	fATAN2,
"cabs",		cABS,
"ccos",		cCOS,
"cexp",		cEXP,
"char",		CHAR,
"clog",		cLOG,
"cmplx",	i2c,
"conjg",	cCONJ,
"cos",		fCOS,
"cosh",		fCOSH,
"csin",		cSIN,
"csqrt",	cSQRT,
"dabs",		dABS,
"dacos",	dACOS,
"dasin",	dASIN,
"datan",	dATAN,
"datan2",	dATAN2,
"dble",		f2d,
"dcmplx",	qCOMPLEX,
"dconjg",       qCONJ,
"dcos",		dCOS,
"dcosh",	dCOSH,
"ddim",		dDIM,
"dexp",		dEXP,
"dfloat",       i2d,
"dim",		fDIM,
"dimag",        qIMAG,
"dint",		dTRUNC,
"dlog",		dLOG,
"dlog10",	dLOG10,
"dmax1",	dMAX,
"dmin1",	dMIN,
"dmod",		dMOD,
"dnint",	dROUND,
"dprod",	dPROD,
"dsign",	dSIGN,
"dsin",		dSIN,
"dsinh",	dSINH,
"dsqrt",	dSQRT,
"dtan",		dTAN,
"dtanh",	dTANH,
"exp",		fEXP,
"float",	i2f,
"iabs",		iABS,
"iand",		lAND,		/* IBM extension */
"ichar",	iCHAR,
"idim",		iDIM,
"idint",	d2i,
"idnint",	dNINT,
"ieor",		lXOR,		/* IBM extension */
"ifix",		f2i,
"index",	INDEX,
"inot",		lNOT,		/* IBM extension */
"int",		f2i,
"ior",		lOR,		/* IBM extension */
"ishft",	lSHIFT,		/* IBM extension */
"isign",	iSIGN,
"len",		LEN,
"lge",		LGE,
"lgt",		LGT,
"lle",		LLE,
"llt",		LLT,
"log",		fLOG,
"log10",	fLOG10,
"max",		iMAX,
"max0",		iMAX,
"max1",		MAX1,
"min",		iMIN,
"min0",		iMIN,
"min1",		MIN1,
"mod",		iMOD,
"nint",		fNINT,
"real",		i2f,
"sign",		fSIGN,
"sin",		fSIN,
"sinh",		fSINH,
"sngl",		d2f,
"sqrt",		fSQRT,
"tan",		fTAN,
"tanh",		fTANH
};
