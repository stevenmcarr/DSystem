/* $Id: mnemonics.h,v 1.1 1997/03/20 14:27:37 carr Exp $ */
#ifndef mnemonics_h
#define mnemonics_h 

/* integer representation for iloc opcodes */

#define	NUMBER_OF_OPS	239
#define	ILOC_VERSION	8

/* must be set to an opcode value guaranteed to be invalid */	
#define A2I_INVALID_OPCODE -1

/* must be set to the opcode of the first executable opcode */
#define ILOC_EXECUTABLE     FRAME


/* non-executable opcodes */

#define NAME	  0	/* specifies external label */
#define ALIAS	  1	/* interprocedural alias information */

#define bDATA	  2	/* byte static data */
#define iDATA	  3	/* integer static data */
#define fDATA	  4	/* real static data */
#define dDATA	  5	/* double precision static data */

#define BYTES	  6	/* bytes of uninitialized storage */

/* executable opcodes */

#define FRAME	  7	/* routine header information */
#define HALT	  8	/* stop the program */
#define NOP	  9	/* no operation */

/* procedure calls and returns */

#define JSRr	 10	/* call subroutine from register */
#define iJSRr	 11	/* call function from register, return an integer */
#define fJSRr	 12	/* call function from register, return a real */
#define dJSRr	 13	/* call function from register, return a double */
#define cJSRr	 14	/* call function from register, return a complex */
#define qJSRr	 15	/* call function from register, return a double complex */

#define JSRl	 16	/* call subroutine from label */
#define iJSRl	 17	/* call function from label, return an integer */
#define fJSRl	 18	/* call function from label, return a real */
#define dJSRl	 19	/* call function from label, return a double */
#define cJSRl	 20	/* call function from label, return a complex */
#define qJSRl	 21	/* call function from label, return a double complex */
#define RTN	 22	/* return from subroutine */
#define iRTN	 23	/* return from integer function with result */
#define fRTN	 24	/* return from real function with result */
#define dRTN	 25	/* return from double function with result */
#define cRTN	 26	/* return from complex function with result */
#define qRTN	 27	/* return from double complex function with result */

/* load immediate instructions */

#define iLDI	 28	/* load integer constant */
#define fLDI	 29	/* load integer constant and convert to real */
#define dLDI	 30	/* load integer constant and convert to double */
#define	cLDI	 31	/* load integer constant and convert to complex */
#define	qLDI	 32	/* load integer constant and convert to double complex */

/* memory references */

#define	bLDor	 33	/* load byte from memory (constant offset) */
#define	iLDor	 34	/* load integer from memory (constant offset) */
#define fLDor	 35	/* load real from memory (constant offset) */
#define dLDor	 36	/* load double from memory (constant offset) */
#define cLDor	 37	/* load complex from memory (constant offset) */
#define qLDor	 38	/* load double complex from memory (constant offset) */

#define	bCONor	 39	/* load byte constant from memory (constant offset) */
#define	iCONor	 40	/* load integer constant from memory (constant offset) */
#define fCONor	 41	/* load real constant from memory (constant offset) */
#define dCONor	 42	/* load double constant from memory (constant offset) */
#define cCONor	 43	/* load complex constant from memory (constant offset) */
#define qCONor	 44	/* load double complex constant from memory (constant offset) */

#define bSLDor   45     /* load scalar byte from memory (constant offset) */
#define iSLDor   46     /* load scalar integer from memory (constant offset) */
#define fSLDor   47     /* load scalar real from memory (constant offset) */
#define dSLDor   48     /* load scalar double from memory (constant offset) */
#define cSLDor   49     /* load scalar complex from memory (constant offset) */
#define qSLDor   50     /* load scalar double complex from memory (constant offset) */

#define bSSTor   51     /* store scalar byte to memory (constant offset) */
#define iSSTor   52     /* store scalar integer to memory (constant offset) */
#define fSSTor   53     /* store scalar real to memory (constant offset) */
#define dSSTor   54     /* store scalar double to memory (constant offset) */
#define cSSTor   55     /* store scalar complex to memory (constant offset) */
#define qSSTor   56     /* store scalar double complex to memory (constant offset) */

#define	bSTor	 57	/* store byte to memory (constant offset) */
#define	iSTor	 58	/* store integer to memory (constant offset) */
#define fSTor	 59	/* store real to memory (constant offset) */
#define dSTor	 60	/* store double to memory (constant offset) */
#define cSTor	 61	/* store complex to memory (constant offset) */
#define qSTor	 62	/* store double complex to memory (constant offset) */

/* arithmetic operations */

#define	iADD	 63	/* integer add */
#define	iSUB	 64	/* integer subtract */
#define	iMUL	 65	/* integer multiply */
#define	iDIV	 66	/* integer division */

#define	fADD	 67	/* real add */
#define	fSUB	 68	/* real subtract */
#define	fMUL	 69	/* real multiply */
#define	fDIV	 70	/* real divide */

#define	dADD	 71	/* double precision add */
#define	dSUB	 72	/* double precision subtract */
#define	dMUL	 73	/* double precision multiply */
#define	dDIV	 74	/* double precision divide */

#define	cADD	 75	/* complex add */
#define	cSUB	 75	/* complex subtract */
#define	cMUL	 77	/* complex multiply */
#define	cDIV	 78	/* complex divide */

#define	qADD	 79	/* double-precision complex add */
#define	qSUB	 80	/* double-precision complex subtract */
#define	qMUL	 81	/* double-precision complex multiply */
#define	qDIV	 82	/* double-precision complex divide */

/* control flow */

#define	JMPl	 83	/* jump to location of label */
#define	JMPr	 84	/* jump to location in register */
#define	BR	 85	/* conditional branch */

/* comparisons */

#define	iCMP	 86	/* compare integer values */
#define	fCMP	 87	/* compare real values */
#define	dCMP	 88	/* compare double precision values */
#define	cCMP	 89	/* compare complex values */
#define	qCMP	 90	/* compare double-precision complex values */

/* logical operations */

#define	EQ	 91	/* equal (based on condition code) */
#define	NE	 92	/* not equal (based on condition code) */
#define	LE	 93	/* less than or equal (based on condition code) */
#define	GE	 94	/* greater than or equal (based on condition code) */
#define	LT	 95	/* less than (based on condition code) */
#define	GT	 96	/* greater than (based on condition code) */

/* copies and conversions */

#define	i2i	 97	/* integer to integer copy */
#define	i2f	 98	/* integer to real conversion */
#define	i2d	 99	/* integer to double conversion */
#define	i2c	100	/* integer to complex conversion */
#define	i2q	101	/* integer to double-precision complex conversion */

#define	f2i	102	/* real to integer conversion */
#define	f2f	103	/* real to real copy */
#define	f2d	104	/* real to double precision conversion */
#define f2c	105	/* real to complex conversion */
#define f2q	106	/* real to double-precision complex conversion */

#define d2i	107	/* double precision to integer conversion */
#define d2f	108	/* double precision to real conversion */
#define d2d	109	/* double precision to double precision copy */
#define d2c	110	/* double precision to complex conversion */
#define d2q	111	/* double precision to double-precision complex conversion */

#define c2i	112	/* complex to integer conversion */
#define c2f	113	/* complex to real conversion */
#define c2d	114	/* complex to double precision conversion */
#define c2c	115	/* complex to complex copy */
#define	c2q	116	/* complex to double precision complex conversion */

#define q2i	117	/* double-precision complex to integer conversion */
#define q2f	118	/* double-precision complex to real conversion */
#define q2d	119	/* double-precision complex to double precision conversion */
#define q2c	120	/* double-precision complex to complex conversion */
#define q2q	121	/* double-precision complex to double-precision complex copy */

#define cCOMPLEX	122	/* construct a complex number */
#define qCOMPLEX	123	/* construct a double-precision complex number */

/* cache instructions */

#define FETCHor   124
#define FLUSHor   125

/* leave numbers for additional opcodes */

/* intrinsic functions */

#define fTRUNC	140	/* truncate real (AINT) */
#define dTRUNC	141	/* truncate double precision (DINT) */

#define	fROUND	142	/* round real number to real whole number (ANINT) */
#define dROUND	143	/* round double precision to double precision whole number (DNINT) */
#define fNINT	144	/* round real to nearest integer (NINT) */
#define dNINT	145	/* round double precision to nearest integer (IDNINT) */

#define iABS	146	/* integer absolute value (IABS) */
#define fABS	147	/* real absolute value (ABS) */
#define dABS	148	/* double precision absolute value (DABS) */
#define cABS	149	/* complex absolute value (CABS) */
#define qABS	150	/* double precision complex absolute value */

#define lSHIFT	151	/* bitwise shift (ISHFT) */

#define	lAND	152	/* boolean and */
#define	lOR	153	/* boolean or */
#define	lNAND	154	/* boolean not and */
#define	lNOR	155	/* boolean not or */
#define	lEQV	156	/* boolean equal */
#define	lXOR	157	/* boolean exclusive or */
#define	lNOT	158	/* boolean not */

#define iMOD	159	/* integer mod function (MOD) */
#define fMOD	160	/* real mod function (AMOD) */
#define dMOD	161	/* double precision mod function (DMOD) */

#define iSIGN	162	/* integer sign transfer (ISIGN) */
#define fSIGN	163	/* real sign transfer (SIGN) */
#define dSIGN	164	/* double precision sign transfer (DSIGN) */

#define iDIM	165	/* integer positive difference (IDIM) */
#define fDIM	166	/* real positive difference (DIM) */
#define dDIM	167	/* double precision positive difference (DDIM) */

#define dPROD	168	/* double precision product (DPROD) */

#define iMAX	169	/* maximum of two or more integers (MAX0) */
#define fMAX	170	/* maximum of two or more reals (AMAX1) */
#define dMAX	171	/* maximum of two or more doubles (DMAX1) */

#define iMIN	172	/* minimum of two or more integers (MIN0) */
#define fMIN	173	/* minimum of two or more reals (AMIN1) */
#define dMIN	174	/* minimum of two of more doubles (DMIN1) */
#define cIMAG	175	/* imaginary part of a complex number (AIMAG) */
#define qIMAG	176	/* imaginary part of a double-precision complex number */

#define cCONJ	177	/* conjugate of a complex number (CONJG) */
#define qCONJ	178	/* conjugate of a double-precision complex number */

#define fSQRT	179	/* square root of a real number (SQRT) */
#define dSQRT	180	/* square root of a double precision number (DSQRT) */
#define cSQRT	181	/* square root of a complex number (CSQRT) */
#define qSQRT	182	/* square root of a double-precision complex number */

#define fEXP	183	/* exponential of a real number (EXP) */
#define dEXP	184	/* exponential of a double precision number (DEXP) */
#define cEXP	185	/* exponential of a complex number (CEXP) */
#define qEXP	186	/* exponential of a double-precision complex number */

#define fLOG	187	/* natural log of a real number (ALOG) */
#define dLOG	188	/* natural log of a double precision number (DLOG) */
#define cLOG	189	/* natural log of a complex number (CLOG) */
#define qLOG	190	/* natural log of a double-precision complex number */

#define fLOG10	191	/* common log of a real number (ALOG10) */
#define dLOG10	192	/* common log of a double precision number (DLOG10) */

#define	iPOW	193	/* exponentiation with integer arguments */
#define	fPOW	194	/* exponentiation with real arguments */
#define	dPOW	195	/* exponentiation with double precision arguments */
#define	cPOW	196	/* exponentiation with complex arguments */
#define	qPOW	197	/* exponentiation with double-precision complex arguments */

#define	fPOWi	198	/* exponentiation (real ** integer) */
#define	dPOWi	199	/* exponentiation (double ** integer) */
#define	cPOWi	200	/* exponentiation (complex ** integer) */
#define	qPOWi	201	/* exponentiation (double-precision complex ** integer) */

#define fSIN	202	/* sine of a real number (SIN) */
#define dSIN	203	/* sine of a double precision number (DSIN) */
#define cSIN	204	/* sine of a complex number (CSIN) */
#define qSIN	205	/* sine of a double-precision complex number */

#define fCOS	206	/* cosine of a real number (COS) */
#define dCOS	207	/* cosine of a double precision number (DCOS) */
#define cCOS	208	/* cosine of a complex number (CCOS) */
#define qCOS	209	/* cosine of a double-precision complex number */

#define fTAN	210	/* tangent of a real number (TAN) */
#define dTAN	211	/* tangent of a double precision number (DTAN) */

#define fASIN	212	/* arcsine of a real number (ASIN) */
#define dASIN	213	/* arcsine of a double precision number (DASIN) */

#define fACOS	214	/* arccosine of a real number (ACOS) */
#define dACOS	215	/* arccosine of a double precision number (DACOS) */

#define fATAN	216	/* arctangent of a real number (ATAN) */
#define dATAN	217	/* arctangent of a double precision number (DATAN) */

#define fATAN2	218	/* arctangent of a real quotient (ATAN2) */
#define dATAN2	219	/* arctangent of a double precision quotient (DATAN2) */

#define fSINH	220	/* hyperbolic sine of a real number (SINH) */
#define dSINH	221	/* hyperbolic sine of a double precision number (DSINH) */

#define fCOSH	222	/* hyperbolic cosine of a real number (COSH) */
#define dCOSH	223	/* hyperbolic cosine of a double precision number (DCOSH) */

#define fTANH	224	/* hyperbolic tangent of a real number (TANH) */
#define dTANH	225	/* hyperbolic tangent of a double precision number (DTANH) */

/* An error opcode is needed.  This code is intended to be used by a2i only */

#define ERR		226	/* error opcode */

/* Additional opcodes are needed to represent Fortran intrinsic */
/* functions that do not have a corresponding representation in */
/* iloc.  These codes are intended to be used only by a2i.  The */
/* numbers should be unique with respect to the iloc opcodes.   */

#define iCHAR	227	/* character to integer conversion */
#define CHAR	228	/* integer to character conversion */
#define LEN	229	/* length of character entity */
#define INDEX	230	/* index of a substring */

#define LGE	231	/* character comparison (greater than or equal ) */
#define LGT	232	/* character comparison (greater than) */
#define LLE	233	/* character comparison (less than or equal) */
#define LLT	234	/* character comparison (less than) */

#define AMAX0	235	/* integer comparison, returns real */
#define MAX1	236	/* real comparison, returns integer */
#define AMIN0	237	/* integer comparison, returns real */
#define MIN1	238	/* real comparison, returns integer */



/* format indicators */

#define	RR2R	 0
#define	R2R	 1
#define	RR	 3
#define	RL	 4
#define	R	 5
#define	CR2R	 6
#define	C2R	 7
#define C2L	 8
#define	LLR	 9
#define	C	10
#define CC	11
#define L	12
#define	NA	13
#define	NF	14

/* tables for the iloc representation */

struct htable_elt
{
	char	*name;
	int	opcode;
};

int	*ftable;
char	**ntable;
struct htable_elt	*htable;

#endif

