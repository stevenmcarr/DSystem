/* $Id: mnemonics.C,v 1.4 1999/07/22 18:06:38 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/* 
 * 	The central repository of knowledge about the internal 
 *	representation for iloc.
 * 
 */
#include <libs/f2i/intrins.h>
#include <stdio.h>
#include <libs/support/misc/general.h>
#include <string.h>
#include <libs/support/memMgmt/mem.h>
#include <libs/f2i/sym.h>
#include <libs/support/strings/rn_string.h>
#include <libs/f2i/ai.h>

static void init_htable(void);
static void init_ftable(void);
static void init_ntable(void);
static int lookup(register char*,register int,register int);



/* initialize table mapping iloc opcodes to iloc instruction formats */

extern int aiDoubleReals;

int	*ftable;
char	**ntable;
struct htable_elt	*htable;

#define FALSE	0
#define TRUE	1

#define SIZE		511
#define INCREMENT 	13

#define INITIAL		13
#define LOOKUP		-13




/* initializes the internal data structures used to represent iloc */
void iloc_rep_init()
{
  register int i;

  ftable = (int *)   get_mem( sizeof(int) * (NUMBER_OF_OPS + 1), 
			      "ILOC Rep format table" );
  ntable = (char **) get_mem( sizeof (char *) * (NUMBER_OF_OPS + 1),
			      "ILOC Rep name table");
  htable = (struct htable_elt *)
	             get_mem( sizeof (struct htable_elt) * (SIZE + 1),
			      "ILOC Rep hash table");

  for (i=0 ; i < NUMBER_OF_OPS; i++)
  {
    ftable[i] = 0;
    ntable[i] = "";
    htable[i].name 	= NULL;
    htable[i].opcode	= -1;
  }
  for (i=NUMBER_OF_OPS; i <= SIZE; i++)
  {
    htable[i].name	= NULL;
    htable[i].opcode	= -1;
  }

  init_ftable();
  init_ntable();

  init_htable(); /* uses ntable! */
} /* iloc_rep_init */



/* return current version number of iloc */
int iloc_rep_version()
{
  return ILOC_VERSION;
} /* iloc_rep_version */



/* initialize iloc opcode hash table */
static void init_htable()
{
   register int i;

   for (i = 0; i < NUMBER_OF_OPS; i++)
       (void) lookup(ntable[i], i, INITIAL);
} /* init_htable */



/* initialize table mapping iloc opcodes to iloc instruction formats */
static void init_ftable()
{
	ftable[NAME] =		C;	/* pseudo */
	ftable[ALIAS] =		L;	/* pseudo */
	ftable[bDATA] =		CC;	/* pseudo */
	ftable[iDATA] =		CC;	/* pseudo */
	ftable[fDATA] =		CC;	/* pseudo */
	ftable[dDATA] =		CC;	/* pseudo */
	ftable[BYTES] =		C;	/* pseudo */
	ftable[FRAME] =		C2L;
	ftable[HALT] =		NA;
	ftable[NOP] =		NA;
	ftable[JSRr] =		NF;
	ftable[iJSRr] =		NF;
	ftable[fJSRr] =		NF;
	ftable[dJSRr] =		NF;
	ftable[cJSRr] =		NF;
	ftable[qJSRr] =		NF;
	ftable[JSRl] =		NF;
	ftable[iJSRl] =		NF;
	ftable[fJSRl] =		NF;
	ftable[dJSRl] =		NF;
	ftable[cJSRl] =		NF;
	ftable[qJSRl] =		NF;
	ftable[RTN] =		R;
	ftable[iRTN] =		RR;
	ftable[fRTN] =		RR;
	ftable[dRTN] =		RR;
	ftable[cRTN] =		RR;
	ftable[qRTN] =		RR;
	ftable[iLDI] =		C2R;
	ftable[fLDI] =		C2R;
	ftable[dLDI] =		C2R;
	ftable[cLDI] =		C2R;
	ftable[qLDI] =		C2R;
	ftable[bLDor] =		NF;
	ftable[iLDor] =		NF;
	ftable[fLDor] =		NF;
	ftable[dLDor] =		NF;
	ftable[cLDor] =		NF;
	ftable[qLDor] =		NF;
	ftable[bCONor] =	NF;
	ftable[iCONor] =	NF;
	ftable[fCONor] =	NF;
	ftable[dCONor] =	NF;
	ftable[cCONor] =	NF;
	ftable[qCONor] =	NF;
	ftable[bSLDor] =        NF;
	ftable[iSLDor] =        NF;
	ftable[fSLDor] =        NF;
	ftable[dSLDor] =        NF;
	ftable[cSLDor] =        NF;
	ftable[qSLDor] =        NF;
	ftable[bSSTor] =        NF;
	ftable[iSSTor] =        NF;
	ftable[fSSTor] =        NF;
	ftable[dSSTor] =        NF;
	ftable[cSSTor] =        NF;
	ftable[qSSTor] =        NF;
	ftable[bSTor] =		NF;
	ftable[iSTor] =		NF;
	ftable[fSTor] =		NF;
	ftable[dSTor] =		NF;
	ftable[cSTor] =		NF;
	ftable[qSTor] =		NF;
	ftable[iADD] =		RR2R;
	ftable[iSUB] =		RR2R;
	ftable[iMUL] =		RR2R;
	ftable[iDIV] =		RR2R;
	ftable[fADD] =		RR2R;
	ftable[fSUB] =		RR2R;
	ftable[fMUL] =		RR2R;
	ftable[fDIV] =		RR2R;
	ftable[dADD] =		RR2R;
	ftable[dSUB] =		RR2R;
	ftable[dMUL] =		RR2R;
	ftable[dDIV] =		RR2R;
	ftable[cADD] =		RR2R;
	ftable[cSUB] =		RR2R;
	ftable[cMUL] =		RR2R;
	ftable[cDIV] =		RR2R;
	ftable[qADD] =		RR2R;
	ftable[qSUB] =		RR2R;
	ftable[qMUL] =		RR2R;
	ftable[qDIV] =		RR2R;
	ftable[JMPl] =		C;
	ftable[JMPr] =		RL; 
	ftable[BR] =		LLR;
	ftable[iCMP] =		RR2R;
	ftable[fCMP] =		RR2R;
	ftable[dCMP] =		RR2R;
	ftable[cCMP] =		RR2R;
	ftable[qCMP] =		RR2R;
	ftable[EQ] =		R2R;
	ftable[NE] =		R2R;
	ftable[LE] =		R2R;
	ftable[GE] =		R2R;
	ftable[LT] =		R2R;
	ftable[GT] =		R2R;
	ftable[i2i] =		R2R;
	ftable[i2f] =		R2R;
	ftable[i2d] =		R2R;
	ftable[i2c] =		R2R;
	ftable[i2q] =		R2R;
	ftable[f2i] =		R2R;
	ftable[f2f] =		R2R;
	ftable[f2d] =		R2R;
	ftable[f2c] =		R2R;
	ftable[f2q] =		R2R;
	ftable[d2i] =		R2R;
	ftable[d2f] =		R2R;
	ftable[d2d] =		R2R;
	ftable[d2c] =		R2R;
	ftable[d2q] =		R2R;
	ftable[c2i] =		R2R;
	ftable[c2f] =		R2R;
	ftable[c2d] =		R2R;
	ftable[c2c] =		R2R;
	ftable[c2q] =		R2R;
	ftable[q2i] =		R2R;
	ftable[q2f] =		R2R;
	ftable[q2d] =		R2R;
	ftable[q2c] =		R2R;
	ftable[q2q] =		R2R;
	ftable[cCOMPLEX] =	RR2R;
	ftable[qCOMPLEX] =	RR2R;
	
	ftable[FETCHor] =       R;
	ftable[FLUSHor] =       R;
	ftable[dPFLD] =		NF;
	ftable[dPFLDI] =	NF;


	ftable[fTRUNC] =	R2R;
	ftable[dTRUNC] =	R2R;
	ftable[fROUND] =	R2R;
	ftable[dROUND] =	R2R;
	ftable[fNINT] =		R2R;
	ftable[dNINT]=		R2R;
	ftable[iABS] =		R2R;
	ftable[fABS] =		R2R;
	ftable[dABS] =		R2R;
	ftable[cABS] =		R2R;
	ftable[qABS] =		R2R;
	ftable[lSHIFT] =	RR2R;
	ftable[lAND] =		RR2R;
	ftable[lOR] =		RR2R;
	ftable[lNAND] =		RR2R;
	ftable[lNOR] =		RR2R;
	ftable[lEQV] =		RR2R;
	ftable[lXOR] =		RR2R;
	ftable[lNOT] =		R2R;
	ftable[iMOD] =		RR2R;
	ftable[fMOD] =		RR2R;
	ftable[dMOD] =		RR2R;
	ftable[iSIGN] =		RR2R;
	ftable[fSIGN] =		RR2R;
	ftable[dSIGN] =		RR2R;
	ftable[iDIM] =		RR2R;
	ftable[fDIM] =		RR2R;
	ftable[dDIM] =		RR2R;
	ftable[dPROD] =		RR2R;
	ftable[iMAX] =		RR2R;
	ftable[fMAX] =		RR2R;
	ftable[dMAX] =		RR2R;
	ftable[iMIN] =		RR2R;
	ftable[fMIN] =		RR2R;
	ftable[dMIN] =		RR2R;
	ftable[cIMAG] =		R2R;
	ftable[qIMAG] =		R2R;
	ftable[cCONJ] =		R2R;
	ftable[qCONJ] =		R2R;
	ftable[fSQRT] =		R2R;
	ftable[dSQRT] =		R2R;
	ftable[cSQRT] =		R2R;
	ftable[qSQRT] =		R2R;
	ftable[fEXP] =		R2R;
	ftable[dEXP] =		R2R;
	ftable[cEXP] =		R2R;
	ftable[qEXP] =		R2R;
	ftable[fLOG] =		R2R;
	ftable[dLOG] =		R2R;
	ftable[cLOG] =		R2R;
	ftable[qLOG] =		R2R;
	ftable[fLOG10]=		R2R;
	ftable[dLOG10]=		R2R;
	ftable[iPOW] =		RR2R;
	ftable[fPOW] =		RR2R;
	ftable[dPOW] =		RR2R;
	ftable[cPOW] =		RR2R;
	ftable[qPOW] =		RR2R;
	ftable[fPOWi] =		RR2R;
	ftable[dPOWi] =		RR2R;
	ftable[cPOWi] =		RR2R;
	ftable[qPOWi] =		RR2R;
	ftable[fSIN] =		R2R;
	ftable[dSIN] =		R2R;
	ftable[cSIN] =		R2R;
	ftable[qSIN] =		R2R;
	ftable[fCOS] =		R2R;
	ftable[dCOS] =		R2R;
	ftable[cCOS] =		R2R;
	ftable[qCOS] =		R2R;
	ftable[fTAN] =		R2R;
	ftable[dTAN] =		R2R;
	ftable[fASIN] =		R2R;
	ftable[dASIN] =		R2R;
	ftable[fACOS] =		R2R;
	ftable[dACOS] =		R2R;
	ftable[fATAN] =		R2R;
	ftable[dATAN] =		R2R;
	ftable[fATAN2] =	RR2R;
	ftable[dATAN2]=		RR2R;
	ftable[fSINH] =		R2R;
	ftable[dSINH] =		R2R;
	ftable[fCOSH] =		R2R;
	ftable[dCOSH] =		R2R;
	ftable[fTANH] =		R2R;
	ftable[dTANH] =		R2R;

	ftable[ERR] =		RR2R;	/* For use by AST->ILOC only */

	ftable[iCHAR] =		NF;	/* For use by AST->ILOC only */
	ftable[CHAR] =		NF;	/* For use by AST->ILOC only */
	ftable[LEN] =		NF;	/* For use by AST->ILOC only */
	ftable[INDEX] =		NF;	/* For use by AST->ILOC only */

	ftable[LGE] =		NF;	/* For use by AST->ILOC only */
	ftable[LGT] =		NF;	/* For use by AST->ILOC only */
	ftable[LLE] =		NF;	/* For use by AST->ILOC only */
	ftable[LLT] =		NF;	/* For use by AST->ILOC only */

	ftable[AMAX0] =		NF;	/* For use by AST->ILOC only */
	ftable[MAX1] =		NF;	/* For use by AST->ILOC only */

	ftable[AMIN0] =		NF;	/* For use by AST->ILOC only */
	ftable[MIN1] =		NF;	/* For use by AST->ILOC only */

} /* init_ftable */



/* intialize table mapping iloc opcodes to */
/* their character string representation   */
static void init_ntable()
{
	ntable[NAME] =		"NAME";
	ntable[ALIAS] =		"ALIAS";
	ntable[bDATA] =		"bDATA";
	ntable[iDATA] =		"iDATA";
	if (aiDoubleReals)
	  ntable[fDATA] =		"dDATA";
	else
	  ntable[fDATA] =		"fDATA";
	ntable[dDATA] =		"dDATA";
	ntable[BYTES] =		"BYTES";
	ntable[FRAME] =		"FRAME";
	ntable[HALT] =		"HALT";
	ntable[NOP] =		"NOP";
	ntable[JSRr] =		"JSRr";
	ntable[iJSRr] =		"iJSRr";
	if (aiDoubleReals)
	  ntable[fJSRr] =		"dJSRr";
	else
	  ntable[fJSRr] =		"fJSRr";
	ntable[dJSRr] =		"dJSRr";
	ntable[cJSRr] =		"cJSRr";
	ntable[qJSRr] =		"qJSRr";
	ntable[JSRl] =		"JSRl";
	ntable[iJSRl] =		"iJSRl";
	if (aiDoubleReals)
	  ntable[fJSRl] =		"dJSRl";
	else
	  ntable[fJSRl] =		"fJSRl";
	ntable[dJSRl] =		"dJSRl";
	ntable[cJSRl] =		"cJSRl";
	ntable[qJSRl] =		"qJSRl";
	ntable[RTN] =		"RTN";
	ntable[iRTN] =		"iRTN";
	if (aiDoubleReals)
	  ntable[fRTN] =		"dRTN";
	else
	  ntable[fRTN] =		"fRTN";
	ntable[dRTN] =		"dRTN";
	ntable[cRTN] =		"cRTN";
	ntable[qRTN] =		"qRTN";
	ntable[iLDI] =		"iLDI";
	if (aiDoubleReals)
	  ntable[fLDI] =		"dLDI";
	else
	  ntable[fLDI] =		"fLDI";
	ntable[dLDI] =		"dLDI";
	ntable[cLDI] =		"cLDI";
	ntable[qLDI] =		"qLDI";
	ntable[bLDor] =		"bLDor";
	ntable[iLDor] =		"iLDor";
	if (aiDoubleReals)
	  ntable[fLDor] =		"dLDor";
	else
	  ntable[fLDor] =		"fLDor";
	ntable[dLDor] =		"dLDor";
	ntable[cLDor] =		"cLDor";
	ntable[qLDor] =		"qLDor";
	ntable[bCONor] =	"bCONor";
	ntable[iCONor] =	"iCONor";
	if (aiDoubleReals)
	  ntable[fCONor] =	"dCONor";
	else
	  ntable[fCONor] =	"fCONor";
	ntable[dCONor] =	"dCONor";
	ntable[cCONor] =	"cCONor";
	ntable[qCONor] =	"qCONor";
	ntable[bSLDor] =        "bSLDor";
	ntable[iSLDor] =        "iSLDor";
	if (aiDoubleReals)
	  ntable[fSLDor] =        "dSLDor";
	else
	  ntable[fSLDor] =        "fSLDor";
	ntable[dSLDor] =        "dSLDor";
	ntable[cSLDor] =        "cSLDor";
	ntable[qSLDor] =        "qSLDor";
	ntable[bSSTor] =        "bSSTor";
	ntable[iSSTor] =        "iSSTor";
	if (aiDoubleReals)
	  ntable[fSSTor] =        "dSSTor";
	else
	  ntable[fSSTor] =        "fSSTor";
	ntable[dSSTor] =        "dSSTor";
	ntable[cSSTor] =        "cSSTor";
	ntable[qSSTor] =        "qSSTor";
	ntable[bSTor] =		"bSTor";
	ntable[iSTor] =		"iSTor";
	if (aiDoubleReals)
	  ntable[fSTor] =		"dSTor";
	else
	  ntable[fSTor] =		"fSTor";
	ntable[dSTor] =		"dSTor";
	ntable[cSTor] =		"cSTor";
	ntable[qSTor] =		"qSTor";
	ntable[iADD] =		"iADD";
	ntable[iSUB] =		"iSUB";
	ntable[iMUL] =		"iMUL";
	ntable[iDIV] =		"iDIV";
	if (aiDoubleReals)
	  {
	    ntable[fADD] =		"dADD";
	    ntable[fSUB] =		"dSUB";
	    ntable[fMUL] =		"dMUL";
	    ntable[fDIV] =		"dDIV";
	  }
	else
	  {
	    ntable[fADD] =		"fADD";
	    ntable[fSUB] =		"fSUB";
	    ntable[fMUL] =		"fMUL";
	    ntable[fDIV] =		"fDIV";
	  }
	ntable[dADD] =		"dADD";
	ntable[dSUB] =		"dSUB";
	ntable[dMUL] =		"dMUL";
	ntable[dDIV] =		"dDIV";
	ntable[cADD] =		"cADD";
	ntable[cSUB] =		"cSUB";
	ntable[cMUL] =		"cMUL";
	ntable[cDIV] =		"cDIV";
	ntable[qADD] =		"qADD";
	ntable[qSUB] =		"qSUB";
	ntable[qMUL] =		"qMUL";
	ntable[qDIV] =		"qDIV";
	ntable[JMPl] =		"JMPl";
	ntable[JMPr] =		"JMPr";
	ntable[BR] =		"BR";
	ntable[iCMP] =		"iCMP";
	if (aiDoubleReals)
	  ntable[fCMP] =		"dCMP";
	else
	  ntable[fCMP] =		"fCMP";
	ntable[dCMP] =		"dCMP";
	ntable[cCMP] =		"cCMP";
	ntable[qCMP] =		"qCMP";
	ntable[EQ] =		"EQ";
	ntable[NE] =		"NE";
	ntable[LE] =		"LE";
	ntable[GE] =		"GE";
	ntable[LT] =		"LT";
	ntable[GT] =		"GT";
	ntable[i2i] =		"i2i";
	if (aiDoubleReals)
	  ntable[i2f] =		"i2d";
	else
	  ntable[i2f] =		"i2f";
	ntable[i2d] =		"i2d";
	ntable[i2c] =		"i2c";
	ntable[i2q] =		"i2q";
	if (aiDoubleReals)
	  {
	    ntable[f2i] =		"d2i";
	    ntable[f2f] =		"d2d";
	    ntable[f2d] =		"d2d";
	    ntable[f2c] =		"d2c";
	    ntable[f2q] =		"d2q";
	  }
	else
	  {
	    ntable[f2i] =		"f2i";
	    ntable[f2f] =		"f2f";
	    ntable[f2d] =		"f2d";
	    ntable[f2c] =		"f2c";
	    ntable[f2q] =		"f2q";
	  }
	ntable[d2i] =		"d2i";
	if (aiDoubleReals)
	  ntable[d2f] =		"d2d";
	else
	  ntable[d2f] =		"d2f";
	ntable[d2d] =		"d2d";
	ntable[d2c] =		"d2c";
	ntable[d2q] =		"d2q";
	ntable[c2i] =		"c2i";
	if (aiDoubleReals)
	  ntable[c2f] =		"c2d";
	else
	  ntable[c2f] =		"c2f";
	ntable[c2d] =		"c2d";
	ntable[c2c] =		"c2c";
	ntable[c2q] =		"c2q";
	ntable[q2i] =		"q2i";
	if (aiDoubleReals)
	  ntable[q2f] =		"q2d";
	else
	  ntable[q2f] =		"q2f";
	ntable[q2d] =		"q2d";
	ntable[q2c] =		"q2c";
	ntable[q2q] =		"q2q";
	ntable[cCOMPLEX] =	"cCOMPLEX";
	ntable[qCOMPLEX] =	"qCOMPLEX";

	ntable[FETCHor] =       "FETCHor";
	ntable[FLUSHor] =       "FLUSHor";
	ntable[dPFLD] =		"dPFLD";
	ntable[dPFLDI] =	"dPFLDI";

	if (aiDoubleReals)
	  ntable[fTRUNC] =	"dTRUNC";
	else
	  ntable[fTRUNC] =	"fTRUNC";
	ntable[dTRUNC] =	"dTRUNC";
	if (aiDoubleReals)
	  ntable[fROUND] =	"dROUND";
	else
	  ntable[fROUND] =	"fROUND";
	ntable[dROUND] =	"dROUND";
	if (aiDoubleReals)
	  ntable[fNINT] =		"dNINT";
	else
	  ntable[fNINT] =		"fNINT";
	ntable[dNINT] =		"dNINT";
	ntable[iABS] =		"iABS";
	if (aiDoubleReals)
	  ntable[fABS] =		"dABS";
	else
	  ntable[fABS] =		"fABS";
	ntable[dABS] =		"dABS";
	ntable[cABS] =		"cABS";
	ntable[qABS] =		"qABS";
	ntable[lSHIFT] =	"lSHIFT";
	ntable[lAND] =		"lAND";
	ntable[lOR] =		"lOR";
	ntable[lNAND] =		"lNAND";
	ntable[lNOR] =		"lNOR";
	ntable[lEQV] =		"lEQV";
	ntable[lXOR] =		"lXOR";
	ntable[lNOT] =		"lNOT";
	ntable[iMOD] =		"iMOD";
	if (aiDoubleReals)
	  ntable[fMOD] =		"dMOD";
	else
	  ntable[fMOD] =		"fMOD";
	ntable[dMOD] =		"dMOD";
	ntable[iSIGN] =		"iSIGN";
	if (aiDoubleReals)
	  ntable[fSIGN] =		"dSIGN";
	else
	  ntable[fSIGN] =		"fSIGN";
	ntable[dSIGN] =		"dSIGN";
	ntable[iDIM] =		"iDIM";
	if (aiDoubleReals)
	  ntable[fDIM] =		"dDIM";
	else
	  ntable[fDIM] =		"fDIM";
	ntable[dDIM] =		"dDIM";
	ntable[dPROD] =		"dPROD";
	ntable[iMAX] =		"iMAX";
	if (aiDoubleReals)
	  ntable[fMAX] =		"dMAX";
	else
	  ntable[fMAX] =		"fMAX";
	ntable[dMAX] =		"dMAX";
	ntable[iMIN] =		"iMIN";
	if (aiDoubleReals)
	  ntable[fMIN] =		"dMIN";
	else
	  ntable[fMIN] =		"fMIN";
	ntable[dMIN] =		"dMIN";
	ntable[cIMAG] =		"cIMAG";
	ntable[qIMAG] =		"qIMAG";
	ntable[cCONJ]=		"cCONJ";
	ntable[qCONJ]=		"qCONJ";
	if (aiDoubleReals)
	  ntable[fSQRT] =		"dSQRT";
	else
	  ntable[fSQRT] =		"fSQRT";
	ntable[dSQRT] =		"dSQRT";
	ntable[cSQRT] =		"cSQRT";
	ntable[qSQRT] =		"qSQRT";
	if (aiDoubleReals)
	  ntable[fEXP] =		"dEXP";
	else
	  ntable[fEXP] =		"fEXP";
	ntable[dEXP] =		"dEXP";
	ntable[cEXP] =		"cEXP";
	ntable[qEXP] =		"qEXP";
	if (aiDoubleReals)
	  ntable[fLOG] =		"dLOG";
	else
	  ntable[fLOG] =		"fLOG";
	ntable[dLOG] =		"dLOG";
	ntable[cLOG] =		"cLOG";
	ntable[qLOG] =		"qLOG";
	if (aiDoubleReals)
	  ntable[fLOG10]=		"dLOG10";
	else
	  ntable[fLOG10]=		"fLOG10";
	ntable[dLOG10]=		"dLOG10";
	ntable[iPOW] =		"iPOW";
	if (aiDoubleReals)
	  ntable[fPOW] =		"dPOW";
	else
	  ntable[fPOW] =		"fPOW";
	ntable[dPOW] =		"dPOW";
	ntable[cPOW] =		"cPOW";
	ntable[qPOW] =		"qPOW";
	if (aiDoubleReals)
	  ntable[fPOWi] =		"dPOWi";
	else
	  ntable[fPOWi] =		"fPOWi";
	ntable[dPOWi] =		"dPOWi";
	ntable[cPOWi] =		"cPOWi";
	ntable[qPOWi] =		"qPOWi";
	if (aiDoubleReals)
	  ntable[fSIN] =		"dSIN";
	else
	  ntable[fSIN] =		"fSIN";
	ntable[dSIN] =		"dSIN";
	ntable[cSIN] =		"cSIN";
	ntable[qSIN] =		"qSIN";
	if (aiDoubleReals)
	  ntable[fCOS] =		"dCOS";
	else
	  ntable[fCOS] =		"fCOS";
	ntable[dCOS] =		"dCOS";
	ntable[cCOS] =		"cCOS";
	ntable[qCOS] =		"qCOS";
	if (aiDoubleReals)
	  ntable[fTAN] =		"dTAN";
	else
	  ntable[fTAN] =		"fTAN";
	ntable[dTAN] =		"dTAN";
	if (aiDoubleReals)
	  ntable[fASIN] =		"dASIN";
	else
	  ntable[fASIN] =		"fASIN";
	ntable[dASIN] =		"dASIN";
	if (aiDoubleReals)
	  ntable[fACOS] =		"dACOS";
	else
	  ntable[fACOS] =		"fACOS";
	ntable[dACOS] =		"dACOS";
	if (aiDoubleReals)
	  ntable[fATAN] =		"dATAN";
	else
	  ntable[fATAN] =		"fATAN";
	ntable[dATAN] =		"dATAN";
	if (aiDoubleReals)
	  ntable[fATAN2] =	"dATAN2";
	else
	  ntable[fATAN2] =	"fATAN2";
	ntable[dATAN2]=		"dATAN2";
	if (aiDoubleReals)
	  ntable[fSINH] =		"dSINH";
	else
	  ntable[fSINH] =		"fSINH";
	ntable[dSINH] =		"dSINH";
	if (aiDoubleReals)
	  ntable[fCOSH] =		"dCOSH";
	else
	  ntable[fCOSH] =		"fCOSH";
	ntable[dCOSH] =		"dCOSH";
	if (aiDoubleReals)
	  ntable[fTANH] =		"dTANH";
	else
	  ntable[fTANH] =		"fTANH";
	ntable[dTANH] =		"dTANH";

	ntable[ERR] =		"ERR";	/* For use by AST->ILOC only */
	
	ntable[iCHAR] =		"iCHAR";	/* For use by AST->ILOC only */
	ntable[CHAR] =		"CHAR";		/* For use by AST->ILOC only */
	ntable[LEN] =		"LEN";		/* For use by AST->ILOC only */
	ntable[INDEX] =		"INDEX";	/* For use by AST->ILOC only */

	ntable[LGE] =		"LGE";		/* For use by AST->ILOC only */
	ntable[LGT] =		"LGT";		/* For use by AST->ILOC only */
	ntable[LLE] =		"LLE";		/* For use by AST->ILOC only */
	ntable[LLT] =		"LLT";		/* For use by AST->ILOC only */

	ntable[AMAX0] =		"AMAX0";	/* For use by AST->ILOC only */
	ntable[MAX1] =		"MAX1";		/* For use by AST->ILOC only */
	ntable[AMIN0] =		"AMIN0";	/* For use by AST->ILOC only */
	ntable[MIN1]  =		"MIN1";		/* For use by AST->ILOC only */
} /* init_ntable */



/* searches a hash table for an iloc opcode and returns the number  */
/* associated with the opcode if the opcode is found.  This routine */ 
/* can be used for initializing the table or for finding entries in */
/* a completed table.						    */
static int lookup( char *name, int number, int function )
//   register char *name;
//   register int  number;
//   register int	function;
{
    register int found = FALSE;  /*  return value of the find function  */
    register int initial_index;  /*  initial value of index  */
    register int Index;


    /*  find the index in the symbol table  */
	Index = hash_string (name, SIZE);
	initial_index = Index;

    /*  search for name  */
	while ((htable[Index].name != NULL) & !found)

	/*  not it? - move to the next element  */
	    if (strcmp(name, htable[Index].name))
	      {
		Index = ((Index) +INCREMENT) % SIZE;
#ifdef DEBUG
		   (void) fprintf(stdout, "FindSymbol: init: %d, retry: %d, '%s'.\n",
			   initial_index, Index, name);
#endif
		/*  if this is true, the table is full  */
		    if (Index == initial_index)
		      {
			(void) fprintf (stderr, "\nIloc Opcode table is full!\n");
			return -1;
		      }
	      }	
	    else /* found the name */
	      {
		if (function == LOOKUP)
		   return htable[Index].opcode;
		else if (function == INITIAL)
		   return -1;
	      }

    /*  found an empty slot  */
	if (function == INITIAL)
	{
	  htable[Index].name    = ssave(name);
	  htable[Index].opcode  = number;
	}
        return number;
} /* lookup */



/* and some external lookup routines */

/* returns the character string representation */
/* of the mnemonic for an iloc opcode	       */
char *iloc_mnemonic( int opcode )
  //  int opcode;
{
  if (opcode < 0 || opcode > NUMBER_OF_OPS)
     return NULL;
  else 
     return ntable[opcode];
} /* iloc_mnemonic */



/* returns the format of the iloc opcode */
int iloc_format( int opcode )
  // int opcode;
{
  if (opcode < 0 || opcode > NUMBER_OF_OPS)
     return NULL;
  else 
     return ftable[opcode];

} /* iloc_format */




/* determines whether or not a string represents a valid */
/* iloc opcode returning the opcode when one exists      */
int iloc_opcode( char *name )
  // char *name;
{
  register int i=0;
  register int j;

  j = lookup(name, i, LOOKUP);
  if (j >= 0 && j <= NUMBER_OF_OPS)
     return j;
  else 
     return -1;
} /* iloc_opcode */




/* returns character string representation of iloc formats */
char *format_name(int format)
  // int format;
{
  switch(format)
  {
    case RR2R:		return "RR2R";
    case R2R:		return "R2R";
    case RR:		return "RR";
    case RL:		return "RL";	
    case R:		return "R";	
    case CR2R:		return "CR2R";
    case C2R:		return "C2R";
    case C2L:		return "C2L";
    case LLR:		return "LLR";
    case CC:		return "CC";	
    case C:		return "C";
    case L:		return "L";
    case NA:		return "NA";	
    case NF:		return "NF";
    default:		return "???";
  }
} /* format_name */




/* Maps a Fortran intrinsic to an iloc opcode using */
/* a binary search on a table defined in intrins.h.    */
int iloc_intrinsic( char *name )

  // char *name;	/* intrinsic name */
  
{
  int	low,	/* low end of search region */
  		mid,	/* midpoint of search region */
  		high,	/* high end of search region */
  		result,	/* result of a string compare */
  		found;	/* flag indicating item found */

  found = FALSE;
  low = 0;
  high = NUMBER_OF_BUILTINS - 1;

  while (!found && (high >= low))
    {
      /*  compute the mid point of the search area */
      mid = low + (high - low)/ 2;

      /* determine where "name" lies with respect to the midpoint */
      result = strcmp(a2i_builtins_map[mid].name, name);

      /* update the search area or declare success */
      if (result > 0)
	      high = mid - 1;
	  else if (result < 0)
	      low = mid + 1;
	  else
	      found = TRUE;
	}

  /* return the opcode associated with the intrinsic */
  if (found)
      return a2i_builtins_map[mid].opcode;

  /* otherwise, indicate that the intrinsic is not valid */
  else
      return A2I_INVALID_OPCODE;
} /* iloc_intrinsic */



/* function to determine which of the iloc opcodes are commutative */
int iloc_op_commutes( int op )
  // int op;
{
  register int result;

  switch( op )
  {
	case iADD:  case iMUL:   case fADD:  case fMUL:  case dADD:  case dMUL:
	case cADD:  case cMUL:   case qADD:  case qMUL:  case lAND:  case lOR:    
	case lNAND: case lNOR:   case lEQV:  case lXOR:  case dPROD: case iMAX:
	case fMAX:  case dMAX:   case iMIN:  case fMIN:  case dMIN:

		result = 1;
		break;

	default:
		result = 0;
		break;
  }
  return result;
} /* iloc_op_commutes */
