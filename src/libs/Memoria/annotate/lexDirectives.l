%{
#include <string.h>
#include <DirectivesInclude.h>
#include <yaccDirectives.h>

#define a2i_wrap() 1
#define EOS     '\0'

extern char *a2i_DirectiveString;
%}

%%

$directive                  return CDIR;

dep                         return DEP;

prefetch                     {
                              a2i_lval.ival = PrefetchInstruction;
                              return PREFETCH;
                             }

flush                        {
                              a2i_lval.ival = FlushInstruction;
                              return FLUSH;
                             }

[a-zA-Z][a-zA-Z$_0-9]*       {
                              a2i_lval.cval = (char *)malloc(strlen((const char *)yytext) *
                                                           sizeof(char));
                              strcpy(a2i_lval.cval,(const char *)yytext);
                              return NAME;
                             }

[0-9]+                       {
                              a2i_lval.cval = (char *)malloc(strlen((const char *)yytext) *
                                                           sizeof(char));
                              strcpy(a2i_lval.cval,(const char *)yytext);
                              return ICONST;
                             }

\(                           return LPAR;

\)                           return RPAR;

\+                           return PLUS;

\-                           return MINUS;

\*                           return TIMES;

\\                           return DIVIDE;

\,                           return COMMA;

[\n]     |
.                           ;
%%
