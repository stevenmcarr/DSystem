/* $Id: classes.h,v 1.2 1997/03/27 20:30:00 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

/* the classification for names */

/* use the TypeChecker's Symbol Tables classification */

/* plus    - cij 6/22/92*/

#define SYMTAB_alias "alias"
#define SYMTAB_addressReg "addressReg"
#define SYMTAB_scratch "scratch"
#define SYMTAB_offset "offset"
#define SYMTAB_high "high"
#define SYMTAB_low "low"
#define SYMTAB_saved "saved"
#define SYMTAB_REG "reg"
#define SYMTAB_temp "temp"
#define SYMTAB_used "used"
#define SYMTAB_fortran_parameter "fortran_parameter"
#define SYMTAB_common_name "common_name"

#define OC_IS_SPECIAL       0x0008
#define TYPE_DOUBLE_COMPLEX 15

