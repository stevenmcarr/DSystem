/* $Id: UserFilterDef.C,v 1.9 1997/03/11 14:32:56 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	framework/UserFilterDef.C					*/
/*									*/
/*	UserFilterDef -- User-notation definition of view filters	*/
/*	Last edited: October 13, 1993 at 11:00 pm			*/
/*									*/
/************************************************************************/




#include <libs/graphicInterface/framework/UserFilterDef.h>


#include <libs/graphicInterface/framework/UserFilter.h>
#include <libs/graphicInterface/oldMonitor/include/mon/sm.h>
#include <libs/support/lists/list.h>
#include <libs/support/strings/rn_string.h>
#include <string.h>
#include <ctype.h>
#include <include/bstring.h>






/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Representation	*/
/************************/




/* UserFilterDef object */

typedef struct UserFilterDef_Repr_struct
  {
    /* contents */
      char *               name;
      char *               def;
      char *               code;

      Boolean              doConcealed;
      Boolean              doErrors;

    /* status */
      UtilList *           filters;

  } UserFilterDef_Repr;


#define R(ob)		(ob->UserFilterDef_repr)

#define INHERITED	DBObject






/************************/
/* Compiled definitions */
/************************/




/* results from executing a def */

#define HILITE_HIDE		  1
#define HILITE_DIM		  2
#define HILITE_NORMAL		  3
#define HILITE_BOLD		  4
#define HILITE_FOREGROUND	  5
#define HILITE_BACKGROUND	  6


/* result opcodes */

#define OP_HIDE			  1
#define OP_DIM			  2
#define OP_NORMAL		  3
#define OP_BOLD			  4
#define OP_FOREGROUND		  5
#define OP_BACKGROUND		  6
#define OP_IF			  7
#define OP_SHOW			  8


/* opcodes for expressions */

#define OP_STRING		  9
#define OP_TRUE			 10
#define OP_FALSE		 11
#define OP_AND			 12
#define OP_OR			 13
#define OP_NOT			 14


/* opcodes for filter predicates */

#define OP_IS_ERRONEOUS		101
#define OP_IS_CONCEALED		102
#define OP_IS_DECL		103
#define OP_IS_EXEC		104
#define OP_IS_HEADING		105
#define OP_IS_INSUBPROG		106
#define OP_TEXTMATCH		107
#define OP_IS_COMMENT		108




/* globals for compilation of defs */

static UserFilterDef * the_ud;			/* def which is running */
static Boolean the_ok;				/* true until a syntax error occurs */
static char the_errorMsg[200];			/* why not ok, if not */

static char * the_def;				/* definition being compiled */
static int the_d;				/* next char of 'uvfp_def' to read */

#define MAXCODE    500				/* limit on length of generated code */
static char the_code[MAXCODE];			/* buffer holding generated code */
static int the_c;				/* next char of 'uvfp_code' to write */




/* primitives for interpreting compiled code */

static char * the_codePtr;
static int the_pc;
static int the_stack[32];
static int the_sp;


# define fetch()        ((int) the_codePtr[the_pc++])

# define push_i(x)      {the_stack[the_sp++] = x;}
# define pop_i(x)       {x = the_stack[--the_sp];}
# define pop2_i(x,y)    {pop_i(y);  pop_i(x);}

# define push_b(x)      {the_stack[the_sp++] = (int) x;}
# define pop_b(x)       {x = BOOL(the_stack[--the_sp]);}
# define pop2_b(x,y)    {pop_b(y);  pop_b(x);}

# define push_s(x)      {the_stack[the_sp++] = (int) x;}
# define pop_s(x)       {x = (char *) the_stack[--the_sp];}






/*************************/
/*  Forward declarations */
/*************************/




static void setStyle(TextString &text, unsigned char style);

typedef FUNCTION_POINTER(void, ForAllFunc, (UserFilter * uf));
static void forAllFilters(UserFilterDef * ud, ForAllFunc proc);

static void becomeIdentityFilter(UserFilter * uf);

static Boolean compile(UserFilterDef * ud,
                      char * def,
                      char * &code);

static Boolean  endOfDef(void);

static void skipWhitespace(void);

static Boolean nextToken(char * t);

static void emit(int op);

static void error(char * msg);

static void statement(void);

static void hilite(void);

static void expression(void);

static void term(void);

static void factor(void);

static void primary(void);

static void predicate(void);

static void stringConst(void);

static Boolean interpret(UserFilterDef * ud,
                         UserFilter * uf,
                         int linenum,
                         TextString text,
                         int &hiliting,
                         Color &foreground,
                         Color &background,
                         void * environment);

static char * findToken(void);

static char * textToString(TextString text);





/************************************************************************/
/*	Interface Operations						*/
/************************************************************************/






/*************************/
/*  Class initialization */
/*************************/




void UserFilterDef::InitClass(void)
{
  /* initialize needed submodules */
    REQUIRE_INIT(DBObject);
}




void UserFilterDef::FiniClass(void)
{
  /* nothing */
}






/****************************/
/*  Instance initialization */
/****************************/




META_IMP(UserFilterDef)




UserFilterDef::UserFilterDef(Context context, DB_FP * session_fp)
                           : DBObject (context, session_fp)
{
  /* allocate instance's private data */
    this->UserFilterDef_repr = (UserFilterDef_Repr *)
                               get_mem(sizeof(UserFilterDef_Repr),
                                       "UserFilterDef instance");

  /* initialize status */
    R(this)->filters = util_list_alloc(UNUSED,"UserFilterDef: filters");
}




UserFilterDef::~UserFilterDef()
{
  /* destroy the contents */
    sfree(R(this)->name);
    sfree(R(this)->def);
    sfree(R(this)->code);
  
  /* any filters using this def become identity filters */
    forAllFilters(this, becomeIdentityFilter);
    util_free_nodes(R(this)->filters);
    util_list_free(R(this)->filters);

  free_mem((void*) this->UserFilterDef_repr);
}






/************/
/* Database */
/************/




void UserFilterDef::isnew(Context context)
{
  R(this)->name        = ssave("");
  R(this)->def         = ssave("");
  R(this)->code        = ssave("");
  R(this)->doConcealed = true;
  R(this)->doErrors    = false;
}




void UserFilterDef::read(DB_FP * fp, DB_FP * session_fp)
{
  if( session_fp != DB_NULLFP )
    { R(this)->name = db_buffered_read_name(session_fp, "UserFilterDef: name");
      R(this)->def  = db_buffered_read_name(session_fp, "UserFilterDef: def");
      R(this)->code = db_buffered_read_name(session_fp, "UserFilterDef: code");
      (void) db_buffered_read(session_fp, (char *) &R(this)->doConcealed, sizeof(Boolean));
      (void) db_buffered_read(session_fp, (char *) &R(this)->doErrors,    sizeof(Boolean));
    }
  else
    this->isnew(CONTEXT_NULL);
}




void UserFilterDef::write(DB_FP * fp, DB_FP * session_fp)
{
  (void) db_buffered_write_name(session_fp, R(this)->name);
  (void) db_buffered_write_name(session_fp, R(this)->def);
  (void) db_buffered_write_name(session_fp, R(this)->code);
  (void) db_buffered_write(session_fp, (char *) &R(this)->doConcealed, sizeof(Boolean));
  (void) db_buffered_write(session_fp, (char *) &R(this)->doErrors,    sizeof(Boolean));
}






/************************/
/* Access to definition */
/************************/




void UserFilterDef::GetName(char * &name)
{
  name = R(this)->name;
}




void UserFilterDef::SetName(char * name)
{
  sfree(R(this)->name);
  R(this)->name = ssave(name);
}




void UserFilterDef::GetDefinition(char * &def,
                                   Boolean &concealed,
                                   Boolean &errors)
{
  def       = R(this)->def;
  concealed = R(this)->doConcealed;
  errors    = R(this)->doErrors;
}




Boolean UserFilterDef::SetDefinition(char * def,
                                      Boolean concealed,
                                      Boolean errors,
                                      char * &msg)
{
  Boolean ok;
  char * code;

  ok = compile(this, def, code);

  if( ok )
    { sfree(R(this)->def);
      R(this)->def = ssave(def);

      sfree(R(this)->code);
      R(this)->code = code;

      R(this)->doConcealed = concealed;
      R(this)->doErrors = errors;

      this->Changed(CHANGE_UNKNOWN, nil);	/* ??? */
    }
  else
    msg = the_errorMsg;

  return ok;
}




Boolean UserFilterDef::InUse(void)
{
  return NOT( util_list_empty(R(this)->filters) );
}





/********************/
/* Filter instances */
/********************/




UserFilter * UserFilterDef::MakeFilter(void * environment)
{
  UserFilter * f;

  f = new UserFilter(CONTEXT_NULL, DB_NULLFP, this, environment);
  f->Open(CONTEXT_NULL, CONTEXT_NULL, CONTEXT_NULL, DB_NULLFP);

  return f;
}






/**************************/
/* Filter def compilation */
/**************************/




Boolean UserFilterDef::function(char * name, int &numArgs, int &opcode)
{
  return false;
}






/*****************************/
/* Filter def interpretation */
/*****************************/




int UserFilterDef::operand(void)
{
  int n;

  pop_i(n);
  return n;
}




Boolean UserFilterDef::execute(int opcode,
                               int linenum,
                               char * line,
                               void * environment,
                               int &result)
{
  return false;
}




Boolean UserFilterDef::getConcealed(int linenum, void * environment)
{
  return false;
}




Boolean UserFilterDef::getErroneous(int linenum, void * environment)
{
  return false;
}




char * UserFilterDef::getErrorMessage(int linenum, void * environment)
{
  return "Error";
}






/*****************/
/* Filter census */
/*****************/




void UserFilterDef::addFilter(UserFilter * uf)
{
  UtilNode * n;

  this->Notify(uf, true);

  n = util_node_alloc((Generic) uf, "UserFilter: filter list elem");
  util_append(R(this)->filters, n);
}




void UserFilterDef::removeFilter(UserFilter * uf)
{
  UtilNode * n, * found;

  this->Notify(uf, false);

  /* remove 'uf' from filter list */
    n = util_head(R(this)->filters);
    found = nil;

    while( n != NULLNODE && found == nil )
      if( uf == (UserFilter *) util_node_atom(n) )
        found = n;
      else
        n = util_next(n);

    util_pluck(n);
}






/*************/
/* Filtering */
/*************/




Boolean UserFilterDef::filterLine(UserFilter * uf,
                                  Boolean countOnly,
                                  int line,
                                  int &subline,
                                  TextString &text,
                                  TextData &data,
                                  void * environment)
{
  int hiliting;
  Color foreground, background;
  Boolean concealed,erroneous;
  Boolean marked;
  char * errorMessage;

  /* determine viewing parameters */
    hiliting   = UNUSED;
    foreground = data.all.foreground;
    background = data.all.background;

    if( R(this)->doConcealed )    /* TEMPORARY -- should be R(uf)->doConcealed */
      { concealed = this->getConcealed(line, environment);
        if( concealed )  hiliting = HILITE_HIDE;
      }

    if( hiliting == UNUSED )
      marked = interpret(this, uf, line, text, hiliting, foreground, background, environment);

    if( R(this)->doErrors )	/* !!! */
      erroneous = this->getErroneous(line, environment);
    else
      erroneous = false;

  if( countOnly )
    { if( hiliting == HILITE_HIDE )
        subline = 0;
      else if( erroneous )
        subline = 2;
      else
        subline = 1;
    }
  else
    { if( subline == 1 )
        { /* error message for an erroneous line */
          destroyTextString(text);
          errorMessage = this->getErrorMessage(line, environment);
          text = makeTextString(errorMessage, STYLE_BOLD, "UserFilterDef");
          text.ephemeral = false;
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

  /* set the line's colors */
    data.all.foreground = foreground;
    data.all.background = background;

  return marked;
}







/************************************************************************/
/*	Private Operations 						*/
/************************************************************************/




static
void setStyle(TextString &text, unsigned char style)
{
  int k;

  for( k = 0;  k < text.num_tc;  k++ )
    text.tc_ptr[k].style |= style;
}




static
void forAllFilters(UserFilterDef * ud, ForAllFunc proc)
{
  UtilNode * n;

  n = util_head(R(ud)->filters);
  while( n != NULLNODE )
    { proc( (UserFilter *) util_node_atom(n) );
      n = util_next(n);
    }
}





static
void becomeIdentityFilter(UserFilter * uf)
{
  /* ... */
}







/*******************/
/*  Compiling defs */
/*******************/




/*ARGSUSED*/

static
Boolean compile(UserFilterDef * ud, char * def, char * &code)
{
  the_ud = ud;
  the_def = def;
  the_d = 0;
  skipWhitespace();

  the_c = 0;
  the_ok = true;

  while( the_ok && NOT(endOfDef()) )
    statement();

  if( the_ok )
    { the_code[the_c] = '\0';
      code = ssave(the_code);
    }

  return the_ok;
}




static
Boolean endOfDef(void)
{
  return BOOL( the_def[the_d] == '\0' );
}




static
void skipWhitespace(void)
{
  while( isspace(the_def[the_d]) )  the_d += 1;
}




static
Boolean nextToken(char * t)
{
  int len,k;
  Boolean matched;

  /* ASSERT: 'the_def[the_d]' is non-whitespace or '\0' */

  /* see whether given token occurs next */
    matched = true;
    k = 0;
    len = strlen(t);
    while( matched  &&  k < len )
      if( t[k] == the_def[the_d+k] )
        k += 1;
      else
        matched = false;

  /* ensure that whole next token was matched */
    if( isalpha(t[0]) )
      matched = BOOL( matched  &&  ! isalpha(the_def[the_d+len]) );

  /* advance past matched token */
    if( matched )
      the_d += len;

  skipWhitespace();

  return matched;
}




static
void emit(int op)
{
  if( the_c < MAXCODE )
    the_code[the_c++] = (char) op;
  else
    error("Definition is too complicated");
}




static
void error(char * msg)
{
  (void) strcpy (the_errorMsg, msg);
  (void) strcat (the_errorMsg, " at '");
  (void) strncat(the_errorMsg, &the_def[the_d], 20);
  (void) strcat (the_errorMsg, "...'");

  the_ok = false;
}




static
void statement(void)
{
  hilite();
  if( the_ok )
    if( nextToken("if") )
      { expression();
        if( the_ok )
          if( nextToken(";")  ||  endOfDef() )
            emit(OP_IF);
          else
            error("';' expected");
      }
    else if( endOfDef() )
      { emit(OP_TRUE);
        emit(OP_IF);
      }
    else
      error("'if' expected");
}




static
void hilite(void)
{
  if( nextToken("bold") )
    emit(OP_BOLD);
  else if( nextToken("normal") )
    emit(OP_NORMAL);
  else if( nextToken("show") )
    emit(OP_SHOW);
  else if( nextToken("dim") )
    emit(OP_DIM);
  else if( nextToken("hide") )
    emit(OP_HIDE);
  else if( nextToken("color") )
    { stringConst();
      emit(OP_FOREGROUND);
    }
  else if( nextToken("hilite") )
    { stringConst();
      emit(OP_BACKGROUND);
    }
  else error("Hilite command expected");
}




static
void expression(void)
{
  term();
  while( the_ok && nextToken("or") )
    { term();
      emit(OP_OR);
    }
}




static
void term(void)
{
  factor();
  if( the_ok )
    while( the_ok && nextToken("and") )
      { factor();
        emit(OP_AND);
      }
}




static
void factor(void)
{
  if( nextToken("not") )
    { factor();
      emit(OP_NOT);
    }
  else
    primary();
}




static
void primary(void)
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
void predicate(void)
{
  char * name;
  int numArgs, opcode;
  int k;

  name = findToken();

  if( the_ud->function(name, numArgs, opcode) )
    { (void) nextToken(name);

      for( k = 1;  k <= numArgs;  k++ )
        stringConst();

      emit(opcode);
    }

  else if( nextToken("true") )
    emit(OP_TRUE);

  else if( nextToken("false") )
    emit(OP_FALSE);

  else if( nextToken("erroneous") )
    emit(OP_IS_ERRONEOUS);

  else if( nextToken("concealed") )
    emit(OP_IS_CONCEALED);

  else if( nextToken("text") )
    { stringConst();
      emit(OP_TEXTMATCH);
    }

  else
    error("Filter predicate expected");
}




static
void stringConst(void)
{
  int quote1,quote2,offset,len,k;

  if( nextToken("\"") )
    { /* gather up a string constant */
      quote1 = the_d-1;
      offset = find(&the_def[quote1+1],"\"");
      if( offset != UNUSED )
        { quote2 = offset + (quote1+1);
          len = quote2 - quote1 - 1;
          emit(OP_STRING);
          emit(len);
          for( k = quote1+1;  k < quote2;  k++ )
            emit((int) the_def[k]);
          the_d = quote2+1;
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
Boolean interpret(UserFilterDef * ud,
                  UserFilter * uf,
                  int linenum,
                  TextString text,
                  int &hiliting,
                  Color &foreground,
                  Color &background,
                  void * environment)
{
  int codeLen, op;
  Boolean halt;
  int cond, res, a, b, pos, len;
  Boolean is, match;
  char * l;
  char * s;
  char * c;
  int result;
  Boolean marked;

  the_codePtr = R(ud)->code;
  codeLen     = strlen(the_codePtr);
  the_pc      = 0; 
  the_sp      = 0; 
  halt        = false;

  hiliting    = HILITE_NORMAL;
  /* ASSERT: 'foreground' and 'background' are initialized by caller */
  marked = false;

  while( ! halt  &&  the_pc < codeLen )
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

              case OP_SHOW:
                push_i(HILITE_NORMAL);
                hiliting = HILITE_HIDE;
                break;

              case OP_BOLD:
                push_i(HILITE_BOLD);
                break;

              case OP_FOREGROUND:
                push_i(HILITE_FOREGROUND);
                break;

              case OP_BACKGROUND:
                push_i(HILITE_BACKGROUND);
                break;

              case OP_IF:
                pop2_i(res, cond);
                if( cond )
                  { if( res == HILITE_FOREGROUND )
                      { pop_s(c);
                        foreground = color(c);
                        hiliting   = HILITE_NORMAL;
                        marked     = true;
                      }
                    else if( res == HILITE_BACKGROUND )
                      { pop_s(c);
                        background = color(c);
                        hiliting   = HILITE_NORMAL;
                        marked     = true;
                      }
                    else
                      { hiliting = res;
                        marked   = BOOL(hiliting >= HILITE_NORMAL);
                      }

                    halt = true;
                }
                break;


          /* opcodes for expressions */

              case OP_STRING:
                len = fetch();
                s = (char *) get_mem(len+1,"UserFilterDef: arg string");
                bcopy(&R(ud)->code[the_pc], s, len);
                s[len] = '\0';
                push_s(s);
                the_pc += len;
                break;

              case OP_TRUE:
                push_b(true);
                break;

              case OP_FALSE:
                push_b(false);
                break;

              case OP_AND:
                pop2_i(a, b);
                push_i(a && b);
                break;

              case OP_OR:
                pop2_i(a, b);
                push_i(a || b);
                break;

              case OP_NOT:
                pop_i(a);
                push_i(!a);
                break;


          /* opcodes for filter predicates */

              case OP_IS_ERRONEOUS:
                is = ud->getErroneous(linenum, environment);
                push_b(is);
                break;

              case OP_IS_CONCEALED:
                is = ud->getConcealed(linenum, environment);
                push_b(is);
                break;

              case OP_TEXTMATCH:
                pop_s(s);
                l = textToString(text);
                pos = find(l, s);
                match = BOOL( pos != UNUSED );
                push_b(match);
                sfree(l); sfree(s);
                break;

              default:
                if( ud->execute(op, linenum, textToString(text), environment, result) )
                  push_i(result) /* ; */
                else
                  die_with_message("UserFilterDef.interpret: unexecuted opcode");
                break;
          }
    }

  return marked;
}




/**** T E M P O R A R Y  ****/


static
char * findToken(void)
{
  int len, k;
  static char token[100];

  k = the_d;
  while( isalpha(the_def[k]) || the_def[k] == '_' )
    k += 1;
  len = k - the_d;
  strncpy(token, &the_def[the_d], len);
  token[len] = '\0';

  return token;
}




static
char * textToString(TextString text)
{
  static char s[200];
  int k;

  for( k = 0;  k < text.num_tc;  k++ )
    s[k] = text.tc_ptr[k].ch;
  s[text.num_tc] = '\0';

  return s;
}
