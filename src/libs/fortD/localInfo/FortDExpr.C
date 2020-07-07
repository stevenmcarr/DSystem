/* $Id: FortDExpr.C,v 1.9 1997/03/11 14:28:40 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <iostream>

using namespace std;

#include <libs/fortD/localInfo/FortDExpr.h>

//-----------------------------------------------------
// Expression utilities
//-----------------------------------------------------
void expr_init(Expr *e)
   {
   e->type = Expr_complex;
   e->val = 0;
   e->ast = AST_NIL;
   e->str = "";
   }

void expr_copy(Expr *copy_into, Expr *copy_from)
{
 copy_into->type = copy_from->type;
 copy_into->val = copy_from->val;
 copy_into->ast = copy_from->ast;
 if(copy_from->str != 0)
 {
 copy_into->str = ssave(copy_from->str);
 }
}

void expr_lower(Expr *e, Expr_type t, int v)
   {
   e->type = t;
   e->val  = v;
	 }

void expr_upper(Expr *e, Expr_type t, int v)
{
   e->type = t;
   e->val  = v;
}
 
void expr_upper_2(Expr *e, Expr_type t, int v , char* s)
{
   e->type = t;
   e->val  = v;
   e->str  = ssave(s);
}

//----------------------------------------------------------------
// Expr writes itself out to the database
//----------------------------------------------------------------
void expr_write(Expr *e, FormattedFile& port)
{
 port.Write(e->type);
   
 switch(e->type) {
  case Expr_simple_sym:
  case Expr_index_var:
  port.Write(e->str, NAME_LENGTH);
  break;

  case Expr_constant:
  port.Write(e->val);
  break;

  case  Expr_linear:
  case  Expr_invocation:
  case  Expr_linear_ivar_only:
  case  Expr_complex:
  cout << "Expr::Write does not handle expression type \n";
  break;
  }
}

//----------------------------------------------------------------
// return a string containing the  expression
//----------------------------------------------------------------
char* expr_string(Expr *e)
{
 char *s;

 switch(e->type) {
  case Expr_simple_sym:
  case Expr_index_var:
  s = ssave(e->str);
  return(s);
  break;

 case  Expr_constant:
 s = new char [100];
 sprintf(s, "%d", e->val);
 return(s);
 break;
 
 default:
 cout << "Expr::string  does not handle expression type \n";
 return 0;
 break;
 }
}

//----------------------------------------------------------------
// Expr reads itself from the database
//----------------------------------------------------------------
void expr_read (Expr *e, FormattedFile& port)
{
 int t_type;
 char read_name[NAME_LENGTH];
 port.Read(t_type);
 e->type = (Expr_type)t_type;
 switch(e->type) {
  case Expr_simple_sym:
  case Expr_index_var:
  port.Read(read_name,NAME_LENGTH);
  e->str = ssave(read_name);
  break;

  case Expr_constant:
  port.Read(e->val);
  break;

  case  Expr_linear:
  case  Expr_invocation:
  case  Expr_linear_ivar_only:
  case  Expr_complex:
  cout << "Expr::Write does not handle expression type \n";
  break;
  }
}
