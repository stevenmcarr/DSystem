/* $Id: FortVFilter.C,v 1.1 1997/06/25 13:43:34 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*	ned_cp/FortEditor/FortVFilter.c					*/
/*									*/
/*	FortVFilter -- determines how Fortran source is displayed	*/
/*									*/
/************************************************************************/




#include <include/bstring.h>
#include <libs/graphicInterface/cmdProcs/newEditor/ned.h>
#include <libs/graphicInterface/cmdProcs/newEditor/fortEditor/FortEditor.i>

#include <libs/support/lists/list.h>
#include <libs/support/strings/rn_string.h>
#include <string.h>
#include <ctype.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




typedef struct
  {
    /* creation parameters */
      FortEditor           ed;

    /* contents */
      char *               name;
      char *               def;
      char *               code;

      Boolean              doConcealed;
      Boolean              doErrors;

    /* status */
      UtilList *           filters;
      Generic              customOb;
      ffs_CustomFunc                  customProc;

  } ffs_Repr;

#define	RS(ob)		((ffs_Repr *) ob)




typedef struct
  {
    /* creation parameters */
      FortEditor           ed;
      FortTextTree         ftt;
      FortVFilterSpec      spec;

    /* subparts */
      ViewFilter           vf;
      UtilNode *           node;

    /* status */
      Boolean              doConcealed;
      Boolean              doErrors;
      int                  elision;

  } ff_Repr;

#define	R(ob)		((ff_Repr *) ob)



typedef FUNCTION_POINTER(void,FilterFunc,(Generic));



/************************/
/* Compiled definitions */
/************************/




/* results from executing a def */

#define HILITE_HIDE	1
#define HILITE_DIM	2
#define HILITE_NORMAL	3
#define HILITE_BOLD	4


/* result opcodes */

#define OP_HIDE             1
#define OP_DIM              2
#define OP_NORMAL           3
#define OP_BOLD             4
#define OP_IF               5


/* opcodes for expressions */

#define OP_STRING           6
#define OP_TRUE             7
#define OP_FALSE            8
#define OP_AND              9
#define OP_OR              10
#define OP_NOT             11


/* opcodes for filter predicates */

#define OP_IS_ERRONEOUS	  101
#define OP_IS_CONCEALED	  102
#define OP_IS_DECL        103
#define OP_IS_EXEC        104
#define OP_IS_HEADING     105
#define OP_IS_INSUBPROG	  106
#define OP_TEXTMATCH      107
#define OP_IS_COMMENT     108


/* globals for compilation of defs */

Boolean ff_ok;				/* true until a syntax error occurs */
char ff_errorMsg[200];			/* why not ok, if not */

char * ff_def;				/* definition being compiled */
int ff_d;				/* next char of 'ff_def' to read */

#define MAXCODE    500			/* limit on length of generated code */
char ff_code[MAXCODE];			/* buffer holding generated code */
int ff_c;				/* next char of 'ff_code' to write */






/************************/
/* Miscellaneous	*/
/************************/




/* answer from 'ff_GetName' */

char ff_name[200];






/************************/
/* Forward declarations */
/************************/




STATIC(void,		filterProc,(FortVFilter ff, Boolean countOnly, FortEditor ed,
                                    int line, int *subline, TextString *text,
				    FF_LineData *data));
STATIC(void,		noteChange,(FortVFilter ff));
STATIC(void,		setStyle,(TextString *text, unsigned char style));
STATIC(void,		forAllFilters,(FortVFilterSpec ffs, FilterFunc proc));
STATIC(void,		becomeIdentityFilter,(FortVFilter ff));

STATIC(Boolean,		compile,(FortVFilterSpec ffs, char *def, char **code));
STATIC(Boolean,         endOfDef,(void));
STATIC(void,            skipWhitespace,(void));
STATIC(Boolean,         nextToken,(char *t));
STATIC(void,            emit,(int op));
STATIC(void,            error,(char *msg));
STATIC(void,            statement,(void));
STATIC(void,            hilite,(void));
STATIC(void,            expression,(void));
STATIC(void,            term,(void));
STATIC(void,            factor,(void));
STATIC(void,            primary,(void));
STATIC(void,            predicate,(void));
STATIC(void,            stringConst,(void));

STATIC(void,		interpret,(FortVFilterSpec ffs, FortTextTree ftt, int line,
                                   int *result));
STATIC(Boolean,         isDeclaration,(FortTextTree ftt, FortTreeNode noe, int bracket));
STATIC(Boolean,         isExecutable,(FortTextTree ftt, FortTreeNode node, int bracket));
STATIC(Boolean,         isHeading,(FortTextTree ftt, FortTreeNode node, int bracket));
STATIC(Boolean,         isInSubprogram,(FortTextTree ftt, char *subprogName, 
                                        FortTreeNode node, int braacket));
STATIC(Boolean,         isComment,(FortTextTree ftt, FortTreeNode node, int bracket));







/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Initialization	*/
/************************/




void ff_Init()
{
  /* nothing */
}




void ff_Fini()
{
  /* nothing */
}




/*ARGSUSED*/

FortVFilterSpec ffs_Open(Context context, DB_FP *fp, FortEditor ed)
{
  FortVFilterSpec ffs;

  /* allocate a new instance */
    ffs = (FortVFilterSpec) get_mem(sizeof(ffs_Repr),"FortVFilterSpec");

  /* initialize the parts */
    /* set creation parameters */
      RS(ffs)->ed = ed;

    /* get our persistent information */
      if( fp != DB_NULLFP )
        { RS(ffs)->name = db_buffered_read_name(fp, "FortVFilterSpec: name");
          RS(ffs)->def  = db_buffered_read_name(fp, "FortVFilterSpec: def");
          RS(ffs)->code = db_buffered_read_name(fp, "FortVFilterSpec: code");
          (void) db_buffered_read(fp, (char *) &RS(ffs)->doConcealed, sizeof(Boolean));
          (void) db_buffered_read(fp, (char *) &RS(ffs)->doErrors,    sizeof(Boolean));
        }
      else
        { RS(ffs)->name = ssave("");
          RS(ffs)->def  = ssave("");
          RS(ffs)->code = ssave("");
          RS(ffs)->doConcealed = true;
          RS(ffs)->doErrors = false;
        }

    /* initialize status */
      RS(ffs)->filters = util_list_alloc(UNUSED,"FortVFilterSpec: filters");
      RS(ffs)->customOb = 0;
      RS(ffs)->customProc = (ffs_CustomFunc) 0;

  return ffs;
}




void ffs_Close(FortVFilterSpec ffs)
{
  /* destroy the contents */
    sfree(RS(ffs)->name);
    sfree(RS(ffs)->def);
    sfree(RS(ffs)->code);
  
  /* any filters using this spec become identity filters */
    forAllFilters(ffs,becomeIdentityFilter);
    util_free_nodes(RS(ffs)->filters);
    util_list_free(RS(ffs)->filters);

  free_mem((void*) ffs);
}




/*ARGSUSED*/

void ffs_Save(FortVFilterSpec ffs, Context context, DB_FP *fp)
{
  if( fp != DB_NULLFP )
    { (void) db_buffered_write_name(fp, RS(ffs)->name);
      (void) db_buffered_write_name(fp, RS(ffs)->def);
      (void) db_buffered_write_name(fp, RS(ffs)->code);
      (void) db_buffered_write(fp, (char *) &RS(ffs)->doConcealed, sizeof(Boolean));
      (void) db_buffered_write(fp, (char *) &RS(ffs)->doErrors,    sizeof(Boolean));
    }
}




FortVFilter ff_Open(Context context, DB_FP *fp, FortEditor ed, 
                    FortVFilterSpec ffs)
{
  FortVFilter ff;

  /* allocate a new instance */
    ff = (FortVFilter) get_mem(sizeof(ff_Repr),"FortVFilter");

  /* initialize the parts */
    /* set creation parameters */
      R(ff)->ed = ed;
      ed_GetTextTree(ed, &R(ff)->ftt);
      R(ff)->spec = ffs;

    /* create subparts */
      R(ff)->vf = vf_Open(context, fp, ff, (ffs_FilterFunc)filterProc);

      R(ff)->node = util_node_alloc((Generic) ff, "FortVFilter:filterlist elem");
      util_append(RS(ffs)->filters,R(ff)->node);

    /* initialize view modifiers */
      R(ff)->doConcealed = RS(ffs)->doConcealed;
      R(ff)->doErrors    = RS(ffs)->doErrors;
      R(ff)->elision     = ff_ELLIPSIS;

  return ff;
}




void ff_Close(FortVFilter ff)
{
  if( ff != UNUSED )
    { vf_Close(R(ff)->vf);

      /* remove self from list of filters using this spec */
        util_pluck(R(ff)->node);
        util_free_node(R(ff)->node);

      free_mem((void*) ff);
    }
}






/************************/
/*  Filter specs	*/
/************************/




void ffs_GetName(FortVFilterSpec ffs, char **name)
{
  *name = RS(ffs)->name;
}




void ffs_SetName(FortVFilterSpec ffs, char *name)
{
  sfree(RS(ffs)->name);
  RS(ffs)->name = ssave(name);
}




void ffs_GetDefinition(FortVFilterSpec ffs, char **def, Boolean *concealed, 
                       Boolean *errors)
{
  *def = RS(ffs)->def;
  *concealed = RS(ffs)->doConcealed;
  *errors = RS(ffs)->doErrors;
}




Boolean ffs_SetDefinition(FortVFilterSpec ffs, char *def, Boolean concealed, 
                          Boolean errors, char **msg)
{
  Boolean ok;
  char * code;

  ok = compile(ffs,def,&code);

  if( ok )
    { sfree(RS(ffs)->def);
      RS(ffs)->def = ssave(def);

      sfree(RS(ffs)->code);
      RS(ffs)->code = code;

      RS(ffs)->doConcealed = concealed;
      RS(ffs)->doErrors = errors;

      forAllFilters(ffs,noteChange);
    }
  else
    *msg = ff_errorMsg;

  return ok;
}




Boolean ffs_InUse(FortVFilterSpec ffs)
{
  return NOT( util_list_empty(RS(ffs)->filters) );
}




void ffs_Customize(FortVFilterSpec ffs, Generic customOb, ffs_CustomFunc customProc)
{
  RS(ffs)->customOb = customOb;
  RS(ffs)->customProc = customProc;
}






/************************/
/*  Filter settings	*/
/************************/




char * ff_GetName(FortVFilter ff, Boolean withError)
{
  FortVFilterSpec ffs = R(ff)->spec;
  char * specName;

  ffs_GetName(ffs,&specName);
  (void) strcpy(ff_name,specName);

  if( withError )
    { if( R(ff)->doErrors  &&  NOT( RS(ffs)->doErrors ) )
        (void) strcat(ff_name, " + errors");
      else if( NOT( R(ff)->doErrors )  &&  RS(ffs)->doErrors )
        (void) strcat(ff_name, " - errors");
    }

  return ff_name;
}




void ff_SetShowErrors(FortVFilter ff, Boolean show)
{
  R(ff)->doErrors = show;
  noteChange(ff);
}




void ff_GetShowErrors(FortVFilter ff, Boolean *show)
{
  *show = R(ff)->doErrors;
}




void ff_SetElision(FortVFilter ff, int elision)
{
  R(ff)->elision = elision;
  vf_SetElision(R(ff)->vf,elision);
  noteChange(ff);
}




void ff_GetElision(FortVFilter ff, int *elision)
{
  *elision = R(ff)->elision;
}






/************************/
/*  Change notification */
/************************/




/* ARGSUSED */

void ff_NoteChange(FortVFilter ff, int kind, Boolean autoScroll, FortTreeNode 
                   node, int first, int last, int delta)
{
  /* note that 'node' argument disappears here */
  
  vf_NoteChange(R(ff)->vf, kind, autoScroll, first, last, delta);
}






/************************/
/*  Filtering		*/
/************************/




ViewFilter ff_ViewFilter(FortVFilter ff)
{
  return R(ff)->vf;
}







/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
void filterProc(FortVFilter ff, Boolean countOnly, FortEditor ed, int line, 
                int *subline, TextString *text, FF_LineData *data)
{
  int hiliting;
  Boolean concealed,erroneous;
  char errorMessage[100];
  FortVFilterSpec ffs = R(ff)->spec;

  /* determine viewing parameters */
    hiliting = UNUSED;
    if( R(ff)->doConcealed )
      { ftt_GetConceal(R(ff)->ftt,line,&concealed);
        if( concealed )  hiliting = HILITE_HIDE;
      }

    if( hiliting == UNUSED )
      interpret(ffs, R(ff)->ftt, line, &hiliting);

    if( R(ff)->doErrors )
      erroneous = ftt_IsErroneous(R(ff)->ftt,line);
    else
      erroneous = false;

  if( countOnly )
    { if( hiliting == HILITE_HIDE )
        *subline = 0;
      else if( erroneous )
        *subline = 2;
      else
       *subline = 1;
    }
  else
    { if( *subline == 1 )
        { /* error message for an erroneous line */
          destroyTextString(*text);
          ftt_GetErrorMessage(R(ff)->ftt,line,errorMessage);
          *text = makeTextString(errorMessage,STYLE_BOLD,"filterProc");
          text->ephemeral = false;
        }
      else
        switch( hiliting )
          {
            case HILITE_HIDE:
              /* not reached */;
              break;

            case HILITE_DIM:
              setStyle(text, /** STYLE_BOLD | **/ ATTR_HALF);
              break;

            case HILITE_NORMAL:
              setStyle(text, STYLE_NORMAL);
              break;

            case HILITE_BOLD:
              setStyle(text, STYLE_BOLD);
              break;
          }
    }

  /* run customized filterspec to veto our decisions */
    if( RS(ffs)->customProc != (ffs_CustomFunc) 0 )
      (RS(ffs)->customProc)(RS(ffs)->customOb, countOnly, ed, line, subline, text, data);
}




static
void noteChange(FortVFilter ff)
{
  ff_NoteChange(ff, NOTIFY_DOC_CHANGED, false, UNUSED, 0, 99999, 99999);
}




static
void setStyle(TextString *text, unsigned char style)
{
  int k;

  for( k = 0;  k < text->num_tc;  k++ )
    text->tc_ptr[k].style |= style;
}




static
void forAllFilters(FortVFilterSpec ffs, FilterFunc proc)
{
  UtilNode * This;

  This = util_head(RS(ffs)->filters);
  while( This != NULLNODE )
    { proc( (FortVFilter) util_node_atom(This) );
      This = util_next(This);
    }
}




/*ARGSUSED*/

static
void becomeIdentityFilter(FortVFilter ff)
{
  /* ... */
}






/************************/
/*  Compiling defs	*/
/************************/




/*ARGSUSED*/

static
Boolean compile(FortVFilterSpec ffs, char *def, char **code)
{
  ff_def = def;  ff_d = 0;
  skipWhitespace();

  ff_c = 0;
  ff_ok = true;

  while( ff_ok && NOT(endOfDef()) )
    statement();

  if( ff_ok )
    { ff_code[ff_c] = '\0';
      *code = ssave(ff_code);
    }

  return ff_ok;
}




static
Boolean endOfDef()
{
  return BOOL( ff_def[ff_d] == '\0' );
}




static
void skipWhitespace()
{
  while( isspace(ff_def[ff_d]) )  ff_d += 1;
}




static
Boolean nextToken(char *t)
{
  int len,k;
  Boolean matched;

  /* ASSERT: 'ff_def[ff_d]' is non-whitespace or '\0' */

  /* see whether given token occurs next */
    matched = true;
    k = 0;
    len = strlen(t);
    while( matched  &&  k < len )
      if( t[k] == ff_def[ff_d+k] )
        k += 1;
      else
        matched = false;

  /* ensure that whole next token was matched */
    if( isalpha(t[0]) )
      matched = BOOL( matched  &&  ! isalpha(ff_def[ff_d+len]) );

  /* advance past matched token */
    if( matched )
      ff_d += len;

  skipWhitespace();

  return matched;
}




static
void emit(int op)
{
  if( ff_c < MAXCODE )
    ff_code[ff_c++] = (char) op;
  else
    error("Definition is too complicated");
}




static
void error(char *msg)
{
  (void) strcpy (ff_errorMsg, msg);
  (void) strcat (ff_errorMsg, " at '");
  (void) strncat(ff_errorMsg, &ff_def[ff_d], 20);
  (void) strcat (ff_errorMsg, "...'");

  ff_ok = false;
}




static
void statement()
{
  hilite();
  if( ff_ok )
    if( nextToken("if") )
      { expression();
        if( ff_ok )
          if( nextToken(";")  ||  endOfDef() )
            emit(OP_IF);
          else
            error("';' expected");
      }
    else
      error("'if' expected");
}




static
void hilite()
{
  if( nextToken("bold") )
    emit(OP_BOLD);
  else if( nextToken("normal") )
    emit(OP_NORMAL);
  else if( nextToken("dim") )
    emit(OP_DIM);
  else if( nextToken("hide") )
    emit(OP_HIDE);
  else error("Hilite command expected");
}




static
void expression()
{
  term();
  while( ff_ok && nextToken("or") )
    { term();
      emit(OP_OR);
    }
}




static
void term()
{
  factor();
  if( ff_ok )
    while( ff_ok && nextToken("and") )
      { factor();
        emit(OP_AND);
      }
}




static
void factor()
{
  if( nextToken("not") )
    { factor();
      emit(OP_NOT);
    }
  else
    primary();
}




static
void primary()
{
  if( nextToken("(") )
    { expression();
      if( ! nextToken(")") )
        error("')' expected");
    }
  else
    predicate();
}




static
void predicate()
{
  if( nextToken("true") )
    emit(OP_TRUE);

  else if( nextToken("false") )
    emit(OP_FALSE);

  else if( nextToken("erroneous") )
    emit(OP_IS_ERRONEOUS);

  else if( nextToken("concealed") )
    emit(OP_IS_CONCEALED);

  else if( nextToken("decl") )
    emit(OP_IS_DECL);

  else if( nextToken("exec") )
    emit(OP_IS_EXEC);

  else if( nextToken("comment") )
    emit(OP_IS_COMMENT);

  else if( nextToken("heading") )
    emit(OP_IS_HEADING);

  else if( nextToken("subprog") )
    { stringConst();
      emit(OP_IS_INSUBPROG);
    }

  else if( nextToken("text") )
    { stringConst();
      emit(OP_TEXTMATCH);
    }

  else
    error("Filter predicate expected");
}




static
void stringConst()
{
  int quote1,quote2,offset,len,k;

  if( nextToken("\"") )
    { /* gather up a string constant */
      quote1 = ff_d-1;
      offset = find(&ff_def[quote1+1],"\"");
      if( offset != UNUSED )
        { quote2 = offset + (quote1+1);
          len = quote2 - quote1 - 1;
          emit(OP_STRING);
          emit(len);
          for( k = quote1+1;  k < quote2;  k++ )
            emit((int) ff_def[k]);
          ff_d = quote2+1;
          skipWhitespace();
        }
      else
        error("String without closing quote");
    }
  else
    error("Argument string expected");
}






/************************/
/*  Interpreting defs	*/
/************************/




static
void interpret(FortVFilterSpec ffs, FortTextTree ftt, int line, int *result)
{
  int codeLen = strlen(RS(ffs)->code);
  int pc,op,sp;
  Boolean halt;
  Generic stack[32];
  int cond,res,a,b,pos,len;
  Boolean is,match;
  char * l;
  char * s;
  FortTreeNode node;
  int bracket;

# define fetch()        ((Generic) RS(ffs)->code[pc++])

# define push_i(x)      {stack[sp++] = x;}
# define pop_i(x)       {x = stack[--sp];}
# define pop2_i(x,y)    {pop_i(y);  pop_i(x);}

# define push_b(x)      {stack[sp++] = (Generic) x;}
# define pop_b(x)       {x = BOOL(stack[--sp]);}
# define pop2_b(x,y)    {pop_b(y);  pop_b(x);}

# define push_s(x)      {stack[sp++] = (Generic) x;}
# define pop_s(x)       {x = (char *) stack[--sp];}

  pc = sp = 0; 
  *result = HILITE_NORMAL;
  halt = false;

  while( ! halt  &&  pc < codeLen )
    { op = fetch();

      switch( op )
        {
         /* result opcodes */

            case OP_HIDE:
              push_i(HILITE_HIDE);
              break;

            case OP_DIM:
              push_i(HILITE_DIM);
              break;

            case OP_NORMAL:
              push_i(HILITE_NORMAL);
              break;

            case OP_BOLD:
              push_i(HILITE_BOLD);
              break;

            case OP_IF:
              pop2_i(res,cond);
              if( cond )  {*result = res; halt = true;}
              break;


        /* opcodes for expressions */

            case OP_STRING:
              len = fetch();
              s = (char *) get_mem(len+1,"FortVFilter:arg string");
              bcopy(&ff_code[pc],s,len);
              s[len] = '\0';
              push_s(s);
              pc += len;
              break;

            case OP_TRUE:
              push_b(true);
              break;

            case OP_FALSE:
              push_b(false);
              break;

            case OP_AND:
              pop2_i(a,b);
              push_i(a && b);
              break;

            case OP_OR:
              pop2_i(a,b);
              push_i(a || b);
              break;

            case OP_NOT:
              pop_i(a);
              push_i(!a);
              break;


        /* opcodes for filter predicates */

            case OP_IS_ERRONEOUS:
              is = ftt_IsErroneous(ftt,line);
              push_b(is);
              break;

            case OP_IS_CONCEALED:
              ftt_GetConceal(ftt,line,&is);
              push_b(is);
              break;

            case OP_IS_DECL:
              ftt_GetLineInfo(ftt,line,&node,&bracket);
              is = isDeclaration(ftt,node,bracket);
              push_b(is);
              break;

            case OP_IS_EXEC:
              ftt_GetLineInfo(ftt,line,&node,&bracket);
              is = isExecutable(ftt,node,bracket);
              push_b(is);
              break;

            case OP_IS_COMMENT:
              ftt_GetLineInfo(ftt,line,&node,&bracket);
              is = isComment(ftt,node,bracket);
              push_b(is);
              break;

            case OP_IS_HEADING:
              ftt_GetLineInfo(ftt,line,&node,&bracket);
              is = isHeading(ftt,node,bracket);
              push_b(is);
              break;

            case OP_IS_INSUBPROG:
              pop_s(s);
              ftt_GetLineInfo(ftt,line,&node,&bracket);
              is = isInSubprogram(ftt,s,node,bracket);
              push_b(is);
              sfree(s);
              break;

            case OP_TEXTMATCH:
              pop_s(s);
              l = ftt_GetTextLine(ftt,line);
              pos = find(l,s);
              match = BOOL( pos != UNUSED );
              push_b(match);
              sfree(l); sfree(s);
              break;

        }
    }
}




static
Boolean isDeclaration(FortTextTree ftt, FortTreeNode node, int bracket)
{
  Boolean is;

  ft_AstSelect(ftt_Tree(ftt));

  if( bracket == ftt_SIMPLE || bracket == ftt_OPEN )
    is = BOOL( is_scope(node)
            || is_common(node)
            || is_dimension(node)
            || is_equivalence(node)
           || is_type_statement(node)
            || is_external(node)
            || is_implicit(node)
            || is_intrinsic(node)
            || is_parameter(node)
            || is_save(node)
            || is_entry(node)
            || is_data(node)
            || is_stmt_function(node) );
  else
    is = false;

  return is;
}




static
Boolean isExecutable(FortTextTree ftt, FortTreeNode node, int bracket)
{
  return NOT( isDeclaration(ftt,node,bracket) );
}




static
Boolean isHeading(FortTextTree ftt, FortTreeNode node, int bracket)
{
  Boolean is;

  ft_AstSelect(ftt_Tree(ftt));

  if( bracket == ftt_OPEN )
    is = BOOL( is_scope(node) );
  else
    is = false;

  return is;
}




/*ARGSUSED*/

static
Boolean isInSubprogram(FortTextTree ftt, char *subprogName, FortTreeNode 
                       node, int bracket)
{
  FortTreeNode subprogNode,nameNode;
  char * name;

  ft_AstSelect(ftt_Tree(ftt));

  subprogNode = find_scope(node);
  nameNode    = get_name_in_entry(subprogNode);
  name        = gen_get_text(nameNode);

  return BOOL( strcmp(subprogName,name) == 0 );
}




/*ARGSUSED*/

static
Boolean isComment(FortTextTree ftt, FortTreeNode node, int bracket)
 {
  ft_AstSelect(ftt_Tree(ftt));

  return BOOL( is_comment(node) );
}




