
typedef union 
  {
   Instruction ival;
   char       *cval;
   Directive   dval;
   AST_INDEX   aval;
  } A2I_STYPE;
extern A2I_STYPE a2i_lval;
# define CDIR 257
# define PREFETCH 258
# define FLUSH 259
# define NAME 260
# define ICONST 261
# define LPAR 262
# define RPAR 263
# define COMMA 264
# define PLUS 265
# define MINUS 266
# define TIMES 267
# define DIVIDE 268
