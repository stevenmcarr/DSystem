%{
#include <DirectivesInclude.h>
#include <yaccDirectives.h>

#define yywrap() 1
#define EOS     '\0'

extern char *a2i_DirectiveString;
%}

%%

$directive                  return CDIR;

dep                         return DEP;

prefetch                     {
                              yylval.ival = PrefetchInstruction;
                              return PREFETCH;
                             }

flush                        {
                              yylval.ival = FlushInstruction;
                              return FLUSH;
                             }

[a-zA-Z][a-zA-Z$_0-9]*       {
                              yylval.cval = (char *)malloc(strlen(yytext) *
                                                           sizeof(char));
                              strcpy(yylval.cval,yytext);
                              return NAME;
                             }

[0-9]+                       {
                              yylval.cval = (char *)malloc(strlen(yytext) *
                                                           sizeof(char));
                              strcpy(yylval.cval,yytext);
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
