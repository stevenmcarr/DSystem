/* $Id: MapInfo.C,v 1.6 1997/03/11 14:29:39 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	dep/map/MapInfo.C					        */
/*									*/
/*	MapInfo.C -- provides interface for the source line mapping	*/
/*		     between the internal format and an external format	*/
/*									*/
/************************************************************************/


#include <libs/frontEnd/fortTextTree/MapPrivate.h>
#include <libs/frontEnd/fortTextTree/MapInfo.h>
#include <libs/frontEnd/fortTextTree/MapInfo_c.h>


/************************************************************************/
/*									*/
/*   Function:   ProcessMapLine                                         */
/*									*/
/*   Description:  Add the variables within a source line and the line  */
/*                 itself to the MapInfo object.                        */
/*									*/
/*   Input:      ftt - FortTextTree for the source file                 */
/*               ftt_line - line associated with input file             */
/*                          (continuation lines are concatenated)       */
/*               output_line - line number assoicated with file         */
/*                             with lines split.                        */
/*									*/
/************************************************************************/


void MapInfo::ProcessMapLine(FortTextTree ftt,
			     int          ftt_line,
			     int          output_line)

{
  MapLineEntry  *Entry;
  FortTreeNode node;
  int          bracket;

    
    if( ftt_line != -1 )		/* New line of code		*/
      {
       /* convert fttTextTree line number to an AST_INDEX		*/

       ftt_GetLineInfo( ftt, ftt_line, &node, &bracket );
       if (is_if(node))
         node = list_first(gen_IF_get_guard_LIST(node));	
       if (is_where_block(node))
         node = list_first(gen_WHERE_BLOCK_get_guard_LIST(node));	
       
       Entry = new MapLineEntry(output_line,node,bracket);
       EntryList->Append(Entry);
    
       /* Collect the IDs on this line and their AST_INDEXs into list	*/
       walkIDsInStmt(node,OpaqueConstructInfo,this,NULL); 

      }
    else
      {

        /* found continuation line, copy MapLineEntry information */

       Entry = new MapLineEntry(((MapLineEntry*)EntryList->Last())->
				GetLineNum(),
				((MapLineEntry*)EntryList->Last())->
				GetStmt(),
				((MapLineEntry*)EntryList->Last())->
				GetBracket());
       EntryList->Append(Entry);
      }
 }



/************************************************************************/
/*									*/
/*   Function:   MapInfo                                                */
/*									*/
/*   Description:  Create the MapInfo object.  First build a list of    */
/*                 map entries (source lines and variables).  Then      */
/*                 construct an array of map entries indexed by source  */
/*                 line number.                                         */
/*                                                                      */
/*   Input:      ftt - FortTextTree representation of a program         */
/*									*/
/************************************************************************/


MapInfo::MapInfo(FortTextTree ftt)

  {
   SinglyLinkedListIterator *MapIterator;
   MapLineEntry *Temp;
   int j;

     /*  Create a Singly Linked list of map entries that will be put
	 in an array in the final map */

     EntryList = new SinglyLinkedList;
     ftt_MapWalk(ftt,(MapInfoOpaque)this);

     /* create the array of map entries indexed by line number.
	the first map entry is null to allow 1-based line indexing from
	PFC  -- smc 1/93*/

     MapEntries = new MapLineEntry[EntryList->Count()+1];
     Size = EntryList->Count();
     MapIterator = new SinglyLinkedListIterator(EntryList);
     for (j = 1; j <= Size; j++)
       {
	MapEntries[j].PutEntry((MapLineEntry *)MapIterator->Current());
	Temp = (MapLineEntry *)MapIterator->Current();
	(*MapIterator)++;
	EntryList->Delete(Temp);
       }
     delete EntryList;
     delete MapIterator;
  }


/************************************************************************/
/*									*/
/*   Function:   ~MapInfo                                               */
/*									*/
/*   Description:  Free up the space held by the MapInfo object         */
/*									*/
/************************************************************************/


MapInfo::~MapInfo()	    

  {
   int		i, prev;
   MapVarEntry *Temp;
   SinglyLinkedListIterator *VarIterator;
    
     for (prev = 0, i = 0; i <= Size; prev = i, i++)
       {
        if (MapEntries[prev].GetLineNum() != MapEntries[i].GetLineNum() ||
	    (i == 0))
	  {	

              /* free the list of variables for this source line */

	   VarIterator = new SinglyLinkedListIterator(MapEntries[i].
						      GetVarList());
	   while(VarIterator->Current() != NULL)
	     {
	      Temp = (MapVarEntry *)VarIterator->Current();
	      (*VarIterator)++;
	      MapEntries[i].GetVarList()->Delete(Temp);
	     }
	   delete VarIterator;
	  }
       }
     delete[] MapEntries;
  }


/************************************************************************/
/*									*/
/*   Function:   MapLineToIndex                                         */
/*									*/
/*   Description:  Return the AST index associated with a source line   */
/*									*/
/*   Input:     LineNum - source line number                            */
/*									*/
/*   Output:    AST_INDEX corresponding to LineNum                      */
/*									*/
/************************************************************************/


AST_INDEX MapInfo::MapLineToIndex(int LineNum)

  {
   return MapEntries[LineNum].GetStmt();
  } 


/************************************************************************/
/*									*/
/*   Function:   MapLineToVarList                                       */
/*									*/
/*   Description:  Return the variable list for a source line           */
/*									*/
/*   Input:     LineNum - source line number                            */
/*									*/
/*   Output:    SinglyLinkList of variables on LineNum                  */
/*									*/
/************************************************************************/


SinglyLinkedList *MapInfo::MapLineToVarList(int LineNum)

  {
   return MapEntries[LineNum].GetVarList();
  }


/************************************************************************/
/*									*/
/*   Function:   MapVarToIndex                                          */
/*									*/
/*   Description:  Return the AST_INDEX associated with an occurance    */
/*                 of a variable on a specific source line              */
/*									*/
/*   Input:      Line - source line number                              */
/*               Var - variable name                                    */
/*               n - which occurance of Var in Line                     */
/*									*/
/*   Output:     AST_INDEX corresponding to the nth Var in Line         */
/*									*/
/************************************************************************/


AST_INDEX MapInfo::MapVarToIndex(int  Line,
				 char *Var,
				 int  n)
  {
   SinglyLinkedListIterator VarListIterator(MapLineToVarList(Line));

     if (VarListIterator.Current() == NULL || n < 1) 
       return(AST_NIL);
     while (VarListIterator.Current() != NULL)
       {
    	if (strcmp(Var,((MapVarEntry *)VarListIterator.Current())->GetName())
	    == 0) 
	  n--;
	if (n == 0) 
	  return(((MapVarEntry *)VarListIterator.Current())->GetNode());
	VarListIterator++;
       }
    return(AST_NIL);
  }


/************************************************************************/
/*									*/
/*   Function:   ConstructInfo                                          */
/*									*/
/*   Description:  add variable information to the MapInfo struct for   */
/*                 this line.                                           */
/*									*/
/*   Input:     node - AST index of a reference                         */
/*              atype - type of reference                               */
/*									*/
/************************************************************************/


void MapInfo::ConstructInfo(AST_INDEX           node,
			    ReferenceAccessType)

  {
   MapVarEntry *VarEntry;

     VarEntry = new MapVarEntry(node,gen_get_text(node));
     ((MapLineEntry *)EntryList->Last())->GetVarList()->
                                                   Append(VarEntry);
  }



