/* $Id: MapPrivate.h,v 1.5 1997/03/11 14:29:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*   File:   MapPrivate.h                                               */
/*									*/
/*   Description:  Contains the definitions of the List objects used    */
/*                 in the MapInfo object.                               */
/*									*/
/************************************************************************/


#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/support/database/context.h>
#include <libs/frontEnd/fortTextTree/FortTextTree.h>
#include <libs/frontEnd/fortTextTree/MapInfo.h>
#include <libs/frontEnd/fortTextTree/MapInfo_c.h>
#include <libs/support/lists/SinglyLinkedList.h>
#include <string.h>

/************************************************************************/
/*									*/
/*   Class:  MapVarEntry                                                */
/*									*/
/*   Description:  Singly linked list containing variable names and the */
/*                 AST index of the names.                              */
/*									*/
/************************************************************************/


class MapVarEntry : public SinglyLinkedListEntry {

  char	    *Name;
  AST_INDEX  Node;

  public:

    MapVarEntry(AST_INDEX n,char *v)
      {
       Node = n;
       Name = new char[strlen(v)];
       strcpy(Name,v);
      };

    virtual ~MapVarEntry() { delete[] Name;};

    void PutName(char *n)
      {
       delete[] Name;
       Name = new char[strlen(n)];
       strcpy(Name,n);
      };

    void PutNode(AST_INDEX n) { Node = n;};

    AST_INDEX GetNode() {return Node;};
    char *GetName() { return Name;};
      
 };

/************************************************************************/
/*									*/
/*   Class:  MapLineEntry                                               */
/*									*/
/*   Description:  Singly linked list containing the AST index of a     */
/*                 source line and a list of variables contained within */
/*                 the source line.                                     */
/*									*/
/************************************************************************/


class MapLineEntry : public SinglyLinkedListEntry {

  int 	           LineNum;
  AST_INDEX	   StmtAstIndex;
  int		   Bracket;
  SinglyLinkedList *VarList;

  public:

    MapLineEntry(int l = -1,AST_INDEX s = AST_NIL,int b = -1)
      {
       LineNum = l;
       StmtAstIndex = s;
       Bracket = b;
       VarList = new SinglyLinkedList;
      }

    virtual ~MapLineEntry() { delete VarList;};

    void PutLineNum(int l) { LineNum = l; };
    void PutStmt(AST_INDEX s) {StmtAstIndex = s;};
    void PutBracket(int b) { Bracket = b;};
    void PutVarList(SinglyLinkedList *l) { VarList = l;};
    void PutEntry(MapLineEntry *e)
      {
       LineNum = e->GetLineNum();
       StmtAstIndex = e->GetStmt();
       Bracket = e->GetBracket();
       VarList = e->GetVarList();
       e->PutVarList(NULL);
      }

    int GetLineNum() { return LineNum;};
    AST_INDEX GetStmt() { return StmtAstIndex;};
    int GetBracket() { return Bracket;};
    SinglyLinkedList *GetVarList() {return VarList;};
 };

			   
