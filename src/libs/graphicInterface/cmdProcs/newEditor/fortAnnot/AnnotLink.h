/* $Id: AnnotLink.h,v 1.2 1997/03/11 14:30:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/*
 * External definitions for the routines in AnnotLink.C. See that
 * file for more information.
 *
 * Author: N. McIntosh
 */

#ifndef AnnotLink_h
#define AnnotLink_h

/*
 * The following routines return lists of strings containing the names of
 * the available callgraph node and edge annotation routines. The storage
 * must be freed by the caller.
 */
EXTERN(char **, ipi_GetCGnodeAnnotList, (void));
EXTERN(char **, ipi_GetCGedgeAnnotList, (void));

#endif /* AnnotLink_h */

