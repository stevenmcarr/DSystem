/* $Id: yaccDirectives.y,v 1.3 1997/11/19 14:45:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

%{
#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/astlist.h>
#include <include/frontEnd/astcons.h>
#include <include/frontEnd/astnode.h>
#include <include/frontEnd/astsel.h>
#include <libs/frontEnd/ast/asttree.h>
#include <include/frontEnd/astrec.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/ast/forttypes.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/frontEnd/fortTree/fortsym.h>

#include <libs/Memoria/annotate/DirectivesInclude.h>


char *a2i_DirectiveString;

Directive a2i_Directive;

Boolean a2i_IsDirective;
%}

%union
  {
   Instruction ival;
   char       *cval;
   Directive   dval;
   AST_INDEX   aval;
  }

%token CDIR
%token DEP
%token <ival> PREFETCH FLUSH
%token <cval> NAME ICONST
%token LPAR RPAR COMMA
%token PLUS MINUS TIMES DIVIDE

%type <aval> subvar subscript expr subscript_list subvarlist
%type <dval> command;

%left PLUS MINUS
%left TIMES DIVIDE

%%

directive : CDIR command  
             {
	      a2i_Directive = $2;
	     }

command : PREFETCH LPAR subvar RPAR 
            {
	     $$.Instr = PrefetchInstruction;
	     $$.Subscript = $3;
	    }
        | PREFETCH LPAR subvar RPAR DEP subvarlist
            {
	     $$.Instr = PrefetchInstruction;
	     $$.Subscript = $3;
             $$.ASTDependenceList = $6;
            }
        | FLUSH LPAR subvar RPAR
            {
	     $$.Instr = FlushInstruction;
	     $$.Subscript = $3;
	    }
        ;

subvarlist: subvar
             {
	       $$ = list_create($1);
	     }
        | subvarlist COMMA subvar
          {
	    $$ = list_insert_last($1,$3);
	   }
        ;

subvar : NAME LPAR subscript_list RPAR 
  {
   $$ = gen_SUBSCRIPT(pt_gen_ident($1),$3);
  }

subscript_list: subscript
                  {
		   $$ = list_create($1);
		  }

              | subscript_list COMMA subscript
                  {
		   $$ = list_insert_last($1,$3);
		  }
              ;

subscript : subvar
              {$$ = $1;}
          | expr
              {$$ = $1;}

expr : expr PLUS expr
         {
          $$ = gen_BINARY_PLUS($1,$3);
         }
         
     | expr MINUS expr
         {
          $$ = gen_BINARY_MINUS($1,$3);
         }
         
     | expr TIMES expr
         {
          $$ = gen_BINARY_TIMES($1,$3);
         }
         
     | expr DIVIDE expr
         {
          $$ = gen_BINARY_DIVIDE($1,$3);
         }
         
     | LPAR expr RPAR
         {
          $$ = $2;
         }
         
     | NAME
       { 
	$$ = gen_IDENTIFIER();
	gen_put_text($$,$1,STR_IDENTIFIER); 
       }

     | ICONST
       { 
	$$ = gen_CONSTANT();
	gen_put_text($$,$1,STR_CONSTANT_INTEGER); 
       }
     ;

%%

void a2i_error(s) 

  char *s;

  {
   a2i_IsDirective = false;
  }

static int SetIndexExprNodeType(node,ft_SymTable)

  AST_INDEX node;
  SymDescriptor ft_SymTable;

  {
   int type;

     if (is_identifier(node))
       type = fst_GetField(ft_SymTable,gen_get_text(node),SYMTAB_TYPE);
     else
       type = TYPE_INTEGER;
     gen_put_real_type(node,type);
     gen_put_converted_type(node,TYPE_INTEGER);
     return(WALK_CONTINUE);
  }

static void SetTypes(Subscript,ft_SymTable)

  AST_INDEX Subscript;
  SymDescriptor ft_SymTable;

  {
   AST_INDEX name;
   int type;

     name = gen_SUBSCRIPT_get_name(Subscript);
     type = fst_GetField(ft_SymTable,gen_get_text(name),SYMTAB_TYPE);
     gen_put_real_type(Subscript,type);
     gen_put_converted_type(Subscript,type);
     gen_put_real_type(name,type);
     gen_put_converted_type(name,type);
     walk_expression(gen_SUBSCRIPT_get_rvalue_LIST(Subscript),SetIndexExprNodeType,
		     NOFUNC,ft_SymTable);
  }

Boolean a2i_string_parse (str,Dir,symtab)

  char *str;
  Directive *Dir;
  SymDescriptor symtab;

  {
   a2i_DirectiveString = str;
   a2i_IsDirective = true;
   a2i_parse();
   if (a2i_IsDirective)
     {
      Dir->Instr = a2i_Directive.Instr;
      Dir->Subscript = a2i_Directive.Subscript;
      Dir->ASTDependenceList = a2i_Directive.ASTDependenceList;
      SetTypes(Dir->Subscript,symtab);
      return true;
     }
   else
     return false;
  }
