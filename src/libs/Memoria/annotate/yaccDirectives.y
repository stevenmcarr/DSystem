/* $Id: yaccDirectives.y,v 1.11 2002/02/20 16:17:42 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

%{
#include <stdlib.h>
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

void a2i_error(char*);

char *a2i_DirectiveString;

Directive a2i_Directive;

Boolean a2i_IsDirective;

extern int a2i_lex();
extern void a2i__scan_string(char *);

%}

%union
  {
   Instruction ival;
   char       *cval;
   Directive   dval;
   AST_INDEX   aval;
  }

%token CDIR
%token PREFETCH FLUSH DEP CLUSTER
%token <cval> NAME ICONST
%token LPAR RPAR COMMA
%token PLUS MINUS TIMES DIVIDE

%type <aval> subvar subscript expr subscript_list constexpr id
%type <dval> command;

%left PLUS MINUS
%left TIMES DIVIDE

%%

directive : CDIR command  
             {
	      a2i_Directive = $2;
	     }
          ;

command : PREFETCH constexpr COMMA subvar 
            {
	     $$.Instr = PrefetchInstruction;
	     $$.Subscript = $4;
	     $$.DirectiveNumber = atoi(gen_get_text($2));
	    }
        | FLUSH LPAR subvar RPAR
            {
	     $$.Instr = FlushInstruction;
	     $$.Subscript = $3;
	    }
        | DEP constexpr COMMA subvar
            {
	      $$.Instr = Dependence;
	      $$.Subscript = $4;
	      $$.DirectiveNumber = atoi(gen_get_text($2));
	    }
        | CLUSTER constexpr
            {
	      $$.Instr = Cluster;
	      $$.Cluster = atoi(gen_get_text($2));
	    }
        ;

subvar : id LPAR subscript_list RPAR 
  {
   $$ = gen_SUBSCRIPT($1,$3);
  }
          ;

subscript_list: subscript_list COMMA subscript
                  {
		   $$ = list_insert_last($1,$3);
		  }
              | subscript
                  {
		   $$ = list_create($1);
		  }

              ;

subscript : subvar
              {$$ = $1;}
          | expr
              {$$ = $1;}
          ;

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
         
     | id
       { 
	$$ =$1;
       }
     | constexpr
       { 
	$$ =$1;
       }
     ;

constexpr: ICONST
       {
	$$ = gen_CONSTANT();
	gen_put_text($$,$1,STR_CONSTANT_INTEGER); 
       }
     ;

id: NAME
       { 
	$$ = gen_IDENTIFIER();
	gen_put_text($$,$1,STR_IDENTIFIER); 
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
		     NOFUNC,(Generic)ft_SymTable);
  }

Boolean a2i_string_parse (str,Dir,symtab)

  char *str;
  Directive *Dir;
  SymDescriptor symtab;

  {
   /* tell flex to scan the string rather than
      a file */

   a2i__scan_string(str); 
   a2i_IsDirective = true;
   a2i_parse();
   if (a2i_IsDirective)
     {
      Dir->Instr = a2i_Directive.Instr;
      Dir->Subscript = a2i_Directive.Subscript;
      Dir->DirectiveNumber = a2i_Directive.DirectiveNumber;
      if (Dir->Instr != Cluster)
        SetTypes(Dir->Subscript,symtab);
      else
	Dir->Cluster = a2i_Directive.Cluster;
      return true;
     }
   else
     return false;
  }
