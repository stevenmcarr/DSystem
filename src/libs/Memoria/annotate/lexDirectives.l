%{
#include <string.h>
#include <DirectivesInclude.h>
#include <yaccDirectives.h>

#define EOS     '\0'

extern char *a2i_DirectiveString;
%}

%%

$directive                  return CDIR;

dep                         return DEP;

prefetch                    return PREFETCH;

flush                       return FLUSH;

cluster                     return CLUSTER;

[a-zA-Z][a-zA-Z$_0-9]*       {
                              a2i_lval->cval = (char *)malloc(strlen((const char *)a2i_text) *
                                                           sizeof(char));
                              strcpy(a2i_lval->cval,(const char *)a2i_text);
                              return NAME;
                             }

[0-9]+                       {
                              a2i_lval->cval = (char *)malloc(strlen((const char *)a2i_text) *
                                                           sizeof(char));
                              strcpy(a2i_lval->cval,(const char *)a2i_text);
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
