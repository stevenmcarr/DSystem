/* $Id: ted_sm_type.h,v 1.3 1997/03/11 14:33:25 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#ifndef ted_sm_type_h
#define ted_sm_type_h

/* The different possible flavors of the text editor cp */
typedef enum  {
	TED_DOCUMENTATION,	/* for the dbcp  */
	TED_DATAFILE,		/* for the dbcp  */
	TED_TEXT,		/* for the filer */
	TED_CLONE		/* for the tedcp */
} sm_ted_buftype;

#define BEHAV_SAVEIT	0
#define BEHAV_SAVEAS	1
#define BEHAV_SAVEACOPY	2

#endif
