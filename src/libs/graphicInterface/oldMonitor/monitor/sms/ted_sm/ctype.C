/* $Id: ctype.C,v 1.1 1997/06/25 14:59:44 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/graphicInterface/oldMonitor/monitor/sms/ted_sm/tedprivate.h>


/* character mapping table */
unsigned char char_tab[256] = {
/*		0	1	2	3	4	5	6	7	*/

/* ^@ */	CTRL,	CTRL,	CTRL,	CTRL,	CTRL,	CTRL,	CTRL,	CTRL,		/* 000 */
/* ^H */	CTRL,	TAB,	LF,	CTRL,	CTRL,	RET,	CTRL,	CTRL,		/* 010 */
/* ^P */	CTRL,	CTRL,	CTRL,	CTRL,	CTRL,	CTRL,	CTRL,	CTRL,		/* 020 */
/* ^X */	CTRL,	CTRL,	CTRL,	CTRL,	CTRL,	CTRL,	CTRL,	CTRL,		/* 030 */
/* space */	SPACE,	PUNC,	PUNC,	PUNC,	PUNC,	PUNC,	PUNC,	PUNC,		/* 040 */
/* (  */	PUNC,	PUNC,	PUNC,	PUNC,	PUNC,	PUNC,	PUNC,	PUNC,		/* 050 */
/* 0  */	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,	DIGIT,		/* 060 */
/* 8  */	DIGIT,	DIGIT,	PUNC,	PUNC,	PUNC,	PUNC,	PUNC,	PUNC,		/* 070 */
/* @  */	PUNC,	UPPER,	UPPER,	UPPER,	UPPER,	UPPER,	UPPER,	UPPER,		/* 100 */
/* H  */	UPPER,	UPPER,	UPPER,	UPPER,	UPPER,	UPPER,	UPPER,	UPPER,		/* 110 */
/* P  */	UPPER,	UPPER,	UPPER,	UPPER,	UPPER,	UPPER,	UPPER,	UPPER,		/* 120 */
/* X  */	UPPER,	UPPER,	UPPER,	PUNC,	PUNC,	PUNC,	PUNC,	PUNC,		/* 130 */
/* `  */	PUNC,	LOWER,	LOWER,	LOWER,	LOWER,	LOWER,	LOWER,	LOWER,		/* 140 */
/* h  */	LOWER,	LOWER,	LOWER,	LOWER,	LOWER,	LOWER,	LOWER,	LOWER,		/* 150 */
/* p  */	LOWER,	LOWER,	LOWER,	LOWER,	LOWER,	LOWER,	LOWER,	LOWER,		/* 160 */
/* x  */	LOWER,	LOWER,	LOWER,	PUNC,	PUNC,	PUNC,	PUNC,	DEL,		/* 170 */

/* meta */
/* ^@ */	MCTRL,	MCTRL,	MCTRL,	MCTRL,	MCTRL,	MCTRL,	MCTRL,	MCTRL,		/* 200 */
/* ^H */	MCTRL,	MTAB,	MLF,	MCTRL,	MCTRL,	MRET,	MCTRL,	MCTRL,		/* 210 */
/* ^P */	MCTRL,	MCTRL,	MCTRL,	MCTRL,	MCTRL,	MCTRL,	MCTRL,	MCTRL,		/* 220 */
/* ^X */	MCTRL,	MCTRL,	MCTRL,	MCTRL,	MCTRL,	MCTRL,	MCTRL,	MCTRL,		/* 230 */
/* space */	MSPACE,	MPUNC,	MPUNC,	MPUNC,	MPUNC,	MPUNC,	MPUNC,	MPUNC,		/* 240 */
/* ( */		MPUNC,	MPUNC,	MPUNC,	MPUNC,	MPUNC,	MPUNC,	MPUNC,	MPUNC,		/* 250 */
/* 0 */		MDIGIT,	MDIGIT,	MDIGIT,	MDIGIT,	MDIGIT,	MDIGIT,	MDIGIT,	MDIGIT,		/* 260 */
/* 8 */		MDIGIT,	MDIGIT,	MPUNC,	MPUNC,	MPUNC,	MPUNC,	MPUNC,	MPUNC,		/* 270 */
/* @ */		MPUNC,	MUPPER,	MUPPER,	MUPPER,	MUPPER,	MUPPER,	MUPPER,	MUPPER,		/* 300 */
/* H */		MUPPER,	MUPPER,	MUPPER,	MUPPER,	MUPPER,	MUPPER,	MUPPER,	MUPPER,		/* 310 */
/* P */		MUPPER,	MUPPER,	MUPPER,	MUPPER,	MUPPER,	MUPPER,	MUPPER,	MUPPER,		/* 320 */
/* X */		MUPPER,	MUPPER,	MUPPER,	MPUNC,	MPUNC,	MPUNC,	MPUNC,	MPUNC,		/* 330 */
/* ` */		MPUNC,	MLOWER,	MLOWER,	MLOWER,	MLOWER,	MLOWER,	MLOWER,	MLOWER,		/* 340 */
/* h */		MLOWER,	MLOWER,	MLOWER,	MLOWER,	MLOWER,	MLOWER,	MLOWER,	MLOWER,		/* 350 */
/* p */		MLOWER,	MLOWER,	MLOWER,	MLOWER,	MLOWER,	MLOWER,	MLOWER,	MLOWER,		/* 360 */
/* x */		MLOWER,	MLOWER,	MLOWER,	MPUNC,	MPUNC,	MPUNC,	MPUNC,	MDEL,		/* 370 */
};



