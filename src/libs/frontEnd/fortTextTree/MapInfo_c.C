/* $Id: MapInfo_c.C,v 1.3 1997/03/11 14:29:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	File:   MapInfo_c.C	      				        */
/*									*/
/*      Description:  Provides a C interface to the C++ object that     */
/*                    handles the mapping of source lines to AST.       */
/*									*/
/************************************************************************/

#include <libs/support/misc/general.h>
#include <libs/support/database/context.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/fortTextTree/MapInfo.h>
#include <libs/frontEnd/fortTextTree/MapInfo_c.h>

/************************************************************************/
/*									*/
/*   Function:   CreateMapInfo                                          */
/*									*/
/*   Description:  Create the MapInfo C++ object                        */
/*                                                                      */
/*   Input:      ftt - FortTextTree representation of a program         */
/*									*/
/*   Output:     An opaque (void *) pointer to the MapInfo object.      */
/*									*/
/************************************************************************/


MapInfoOpaque CreateMapInfo(FortTextTree ftt)
  
  {
   MapInfo *Map;

     Map = new MapInfo(ftt);
     return((MapInfoOpaque)Map);
  }


/************************************************************************/
/*									*/
/*   Function:   DestroyMapInfo                                         */
/*									*/
/*   Description:  Destroy the C++ MapInfo object.                      */
/*									*/
/*   Input:      Map - opaque pointer to the MapInfo object.            */
/*									*/
/************************************************************************/


void DestroyMapInfo(MapInfoOpaque Map)

  {
   delete (MapInfo *)Map;
  }


/************************************************************************/
/*									*/
/*   Function:   MapLineToIndex                                         */
/*									*/
/*   Description:  Return the AST index associated with a source line   */
/*									*/
/*   Input:     Map - opaque pointer to MapInfo object.                 */
/*              LineNum - source line number                            */
/*									*/
/*   Output:    AST_INDEX corresponding to LineNum                      */
/*									*/
/************************************************************************/


AST_INDEX MapLineToIndex(MapInfoOpaque Map,
			 int           LineNum)

  {
   return(((MapInfo *)Map)->MapLineToIndex(LineNum));
  }


/************************************************************************/
/*									*/
/*   Function:   MapVarToIndex                                          */
/*									*/
/*   Description:  Return the AST_INDEX associated with an occurance    */
/*                 of a variable on a specific source line              */
/*									*/
/*   Input:      Map - opaque pointer to MapInfo object                 */
/*               Line - source line number                              */
/*               Var - variable name                                    */
/*               n - which occurance of Var in Line                     */
/*									*/
/*   Output:     AST_INDEX corresponding to the nth Var in Line         */
/*									*/
/************************************************************************/


AST_INDEX MapVarToIndex(MapInfoOpaque Map,
			int           Line,
			char         *Var,
			int           n)

  {
   return(((MapInfo *)Map)->MapVarToIndex(Line,Var,n));
  }


/************************************************************************/
/*									*/
/*   Function:   ProcessMapLine                                         */
/*									*/
/*   Description:  Add the variables within a source line and the line  */
/*                 itself to the MapInfo object.                        */
/*									*/
/*   Input:      Map - opaque pointer to Map object.                    */
/*               ftt - FortTextTree for the source file                 */
/*               ftt_line - line associated with input file             */
/*                          (continuation lines are concatenated)       */
/*               output_line - line number assoicated with file         */
/*                             with lines split.                        */
/*									*/
/************************************************************************/


void ProcessMapLine(MapInfoOpaque Map,
		    FortTextTree ftt,
		    int          ftt_line,
		    int          output_line)

  {
   ((MapInfo *)Map)->ProcessMapLine(ftt,ftt_line,output_line);
  }
		    

/************************************************************************/
/*									*/
/*   Function:   OpaqueConstructInfo                                    */
/*									*/
/*   Description:  add variable information to the MapInfo struct for   */
/*                 this line.                                           */
/*									*/
/*   Input:     node - AST index of a reference                         */
/*              atype - type of reference                               */
/*              arg_list - variable length arg list containing pointer  */
/*                         to MapInfo object                            */
/*									*/
/************************************************************************/


void OpaqueConstructInfo(AST_INDEX           node,
			 ReferenceAccessType atype, 
			 va_list             arg_list)

  {
   MapInfo *Map;

     Map = va_arg(arg_list, MapInfo*);
     Map->ConstructInfo(node,atype);
  }
