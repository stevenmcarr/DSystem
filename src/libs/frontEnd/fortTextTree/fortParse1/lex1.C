/* $Id: lex1.C,v 1.1 1997/06/24 17:45:59 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
/************************************************************************/
/*									*/
/*	FortTextTree/FortParse1/lex1.c				        */
/*									*/
/*	FortParse1/lex1 -- scanner for low-level Fortran parser		*/
/*									*/
/************************************************************************/




#include <ctype.h>
#include <string.h>

#include <libs/frontEnd/fortTextTree/FortTextTree.i>
#include <libs/frontEnd/fortTextTree/fortParse1/FortParse1.i>
#include <libs/frontEnd/fortTextTree/fortParse1/lex1.h>



/************************************************************************/
/*	Private Data Structures						*/
/************************************************************************/




/************************/
/*  Static data		*/
/************************/




static TextString lx1_text;

static int lx1_next;
static Boolean lx1_inPlaceholder;



/************************/
/* Rn parser stuff	*/
/************************/




#define NAMELIST
#define VL 6			/* max identifier length */

#define ALLOC(t) ((struct t *)calloc(1,sizeof (struct t)))
#define ckalloc(n) ((char *) calloc(n, sizeof (char)))

# define BLANK	' '
# define MYQUOTE (2)
# define SEOF (-1)

/* card types */

# define STEOF 1
# define STINITIAL 2
# define STCONTINUE 3
# define STCOMMENT 4		/* SKW */
# define STEMPTY 5		/* SKW */
# define STERROR 6		/* DGB */

/* lex states */
typedef enum {NEWSTMT,FIRSTTOKEN,OTHERTOKEN,RETEOS,RETEOF} LEXSTATES;

static int       stkey;
static int       lastend = 1;
long int         lx1_StatNumber;
Boolean          lx1_PlaceholderStatLabel;
Boolean          lx1_IntOnly;
static long int  stno;
static long int  nxtstno;	/* number of statement read into buffer s */

/* Set by getcd                           */

char            lx1_Token[1500]; /* This is probably much too big ... */
int             lx1_TokenLen;
static int      lineno;
Boolean         lx1_NeedKwd;
Boolean		lx1_InIOControl;
KWD_OPT		lx1_IOKwd;

static int       parlev;
static int       expcom;
static int       expeql;
static char     *nextch;
static char     *lastch;
static char     *nextcd = NULL;
static char     *endcd;
static int       prevlin;
static int       thislin;
static int       code;
static LEXSTATES lexstate;

#define MAXLINE    1390
static char      s[MAXLINE];	/* statement buffer                               */
static char     *stend = s + 20 * 66;




/************************/
/* Scanner tables	*/
/************************/




struct Keylist
{
    char           *keyname;
    int             keyval;
};
struct Punctlist
{
    char            punchar;
    int             punval;
};
struct Fmtlist
{
    char            fmtchar;
    int             fmtval;
};
struct Dotlist
{
    char           *dotname;
    int             dotval;
};

static struct Keylist keys[] =
{
 {"allocatable", SALLOCATABLE},
 {"allocate", SALLOCATE},
 {"assign", SASSIGN},
 {"at", SAT},
 {"backspace", SBACKSPACE},
 {"barrier", SBARRIER},
 {"blockdata", SBLOCKDATA},
 {"block",SBLOCK},
 {"call", SCALL},
 {"character", SCHARACTER},
 {"clear", SCLEAR},
 {"close", SCLOSE},
 {"common", SCOMMON},
 {"complex", SCOMPLEX},
 {"continue", SCONTINUE},
 {"createtask", SCREATETASK},
 {"data", SDATA},
 {"deallocate", SDEALLOCATE},
 {"debug", SDEBUG},
 {"dimension", SDIMENSION},
 {"doubleprecision", SDOUBLE},
 {"doallwhile", SDOALLWHILE},
 {"dowhile", SDOWHILE},
 {"do", SDO},
 {"elseif", SELSEIF},
 {"elsewhere",SELSEWHERE},
 {"else", SELSE},
/* {"endall", SENDALL}, */
 {"enddo", SENDDO},
 {"endfile", SENDFILE},
 {"endfor", SENDALL},
 {"endif", SENDIF},
 {"endloop", SENDLOOP},
 {"endwhere",SENDWHERE},
 {"end", SEND},
 {"entry", SENTRY},
 {"equivalence", SEQUIV},
 {"event", SEVENT},
 {"exactprecision", SEXACT},
 {"external", SEXTERNAL},
 {"forall", SDOALL},
 {"format", SFORMAT},
 {"function", SFUNCTION},
 {"goto", SGOTO},
 {"implicitnone", SIMPLICITNONE},
 {"implicit", SIMPLICIT},
 {"inc", SINC},
 {"init", SINIT},
 {"inquire", SINQUIRE},
 {"integer", SINTEGER},
 {"intrinsic", SINTRINSIC},
 {"lock", SLOCK},
 {"logical", SLOGICAL},
 {"open", SOPEN},
 {"otherprocesses", SOTHERPROCESSES},
 {"parallelloopwhile", SPARALLELLOOPWHILE},
 {"parallelloop", SPARALLELLOOP},
 {"parallel", SPARALLEL},
 {"parameter", SPARAMETER},
 {"parbegin", SPARBEGIN},
 {"parend", SPAREND},
 {"pause", SPAUSE},
 {"posting", SPOSTING},
 {"post", SPOST},
 {"print", SPRINT},
 {"private", SPRIVATE},
 {"program", SPROGRAM},
 {"read", SREAD},
 {"real", SREAL},
 {"return", SRETURN},
 {"rewind", SREWIND},
 {"save", SSAVE},
 {"semaphore", SSEMAPHORE},
 {"set", SSET},
 {"stoploop", SSTOPLOOP},
 {"stop", SSTOP},
 {"subchk", SSUBCHK},
 {"subroutine", SSUBROUTINE},
 {"subtrace", SSUBTRACE},
 {"taskcommon", STASKCOMMON},
 {"task", STASK},
 {"then", STHEN},
 {"times", STIMES},
 {"to", STO},
 {"traceoff", STRACEOFF},
 {"traceon", STRACEON},
 {"trace", STRACE},
 {"unit", SUNIT},
 {"unlock", SUNLOCK},
 {"until", SUNTIL},
 {"val", SVAL},
 {"wait", SWAIT},
 {"where", SWHERE},
 {"while", SWHILE},
 {"write", SWRITE},
 {0, 0}
};
static struct Keylist *keystart[26],
               *keyend[26];


static struct Keylist placeholders[] =
{
  {"arg", SARG_PH},
/*{"any", SANY_PH},*/
  {"array-decl-len", SARRAY_DECL_LEN_PH},
  {"bound", SBOUND_PH},
  {"common-elt", SCOMMON_ELT_PH},
  {"common-vars", SCOMMON_VARS_PH},
  {"constant", SCONSTANT_PH},
  {"loop-control", SLOOP_CONTROL_PH},
  {"data-init", SDATA_INIT_PH},
  {"data", SDATA_PH},
  {"dim", SDIM_PH},
  {"equiv", SEQUIV_PH},
  {"formal", SFORMAL_PH},
  {"format", SFORMAT_PH},
/*{"guard", SGUARD_PH},*/
  {"implicit-def", SIMPLICIT_DEF_PH},
  {"len", SLEN_PH},
  {"letters", SLETTERS_PH},
  {"init", SINIT_PH},
  {"invocation", SINVOCATION_PH},
  {"kwd", SKWD_PH},
  {"specify-kwd", SSPECIFY_KWD_PH},
  {"query-kwd", SQUERY_KWD_PH},
  {"lbl", SLABEL_PH},
  {"letter", SLETTER_PH},
  {"var", SVAR_PH},
  {"name", SNAME_PH},
  {"func-name", SNAME_PH},
  {"subr-name", SNAME_PH},
  {"option", SOPTION_PH},
/*{"parallel-case", SPARALLEL_CASE_PH},*/
  {"param-def", SPARAM_DEF_PH},
  {"posting", SPOSTING_PH},
  {"post-expr", SPOST_EXPR_PH},
  {"expr", SEXPR_PH},
  {"arith-expr", SARITH_EXPR_PH},
  {"string-expr", SSTRING_EXPR_PH},
  {"relational-expr", SRELATIONAL_EXPR_PH},
  {"logical-expr", SLOGICAL_EXPR_PH},
  {"stmt", SSTMT_PH},
  {"subprogram", SSUBPROGRAM_PH},
  {"string-var", SSTRING_VAR_PH},
  {"text", STEXT_PH},
  {"type", STYPE_PH},
  {"type-name", STYPENAME_PH},
  {"unit", SUNIT_PH},
/*** NEW ***/
  {"specification-stmt", SSPECIFICATION_STMT_PH},
  {"control-stmt", SCONTROL_STMT_PH},
  {"io-stmt", SIO_STMT_PH},
  {"parascope-stmt", SPARASCOPE_STMT_PH},
  {"debug-stmt", SDEBUG_STMT_PH},
  {0, 0}
};
static struct Keylist *placeStart[26],
               *placeEnd[26];

struct Punctlist puncts[] =
{
 '(', SLPAR,
 ')', SRPAR,
 '=', SEQUALS,
 ',', SCOMMA,
 '+', SPLUS,
 '-', SMINUS,
 '*', SSTAR,
 '/', SSLASH,
 ':', SCOLON,
 '%', SPERCENT,
 0, 0};

struct Dotlist dots[] =
{
 "and.",	SAND,
 "eq.",		SEQ,
 "eqv.",	SEQV,
 "false.",	SFALSE,
 "ge.",		SGE,
 "gt.",		SGT,
 "le.",		SLE,
 "lt.",		SLT,
 "ne.",		SNE,
 "neqv.",	SNEQV,
 "not.",	SNOT,
 "or.",		SOR,
 "true.",	STRUE,
 0, 0};



/************************/
/* Forward declarations	*/
/************************/




STATIC (KWD_OPT, kwd_name, (char *name));
STATIC (int, getcds, (void));
STATIC (int, getcd, (char *b));
STATIC (int, crunch, (void));
STATIC (void, analyz, (void));
STATIC (int, getkwd, (struct Keylist *starts[], struct Keylist *ends[]));
STATIC (void, initkey, (struct Keylist words[], struct Keylist *starts[], 
                struct Keylist *ends[]));
STATIC (int, gettok, (void));



/************************************************************************/
/*	Interface Operations 						*/
/************************************************************************/




/************************/
/*  Initialization	*/
/************************/




void lx1_Init()
{
  initkey(keys,keystart,keyend);
  initkey(placeholders,placeStart,placeEnd);
}




void lx1_Fini()
{
  /* nothing */
}




void lx1_SetScan(TextString text)
{
  text.num_tc = min(MAXLINE, text.num_tc);

  lx1_text = text;
  lx1_next = 0;
  lx1_inPlaceholder = false;
  
  lexstate = NEWSTMT;
  nextcd = nil;
}






/************************************************************************/
/*	Private Operations	 					*/
/************************************************************************/



static
char getChar()
{
  TextChar tc;
  int num = lx1_text.num_tc;
  Boolean placeholder;

  if( lx1_next < num )
    { tc = lx1_text.tc_ptr[lx1_next];
      lx1_next += 1;

      /* mark the first char of a placeholder by turning on bit 7 */
        placeholder = BOOL(tc.style & ftt_PLACEHOLDER_STYLE);
        if( placeholder && !lx1_inPlaceholder && tc.ch != ' ' )
          tc.ch = ((unsigned) tc.ch) + 128;
        lx1_inPlaceholder = placeholder;

      return tc.ch;
    }
  else if( lx1_next == num )
    { lx1_next += 1;
      return '\n';
    }
  else return EOF;
}




static
void ungetChar()
{
  TextChar tc;

  lx1_next -= 1;

  if( lx1_next-1 >= 0 )
    { tc = lx1_text.tc_ptr[lx1_next-1];
      lx1_inPlaceholder = BOOL(tc.style & ftt_PLACEHOLDER_STYLE);
    }
  else
    lx1_inPlaceholder = false;
}








/************************************************************************/
/*	Original Rn Parser scanner 					*/
/************************************************************************/



int
hextoi (int c)
{
    register char  *p;
    static char     p0[17] = "0123456789abcdef";

    for (p = p0; *p; ++p)
	if (*p == c)
	    return (p - p0);
    return (16);
}


Boolean 
eqn (int n, char *a, char *b)
{
    while (--n >= 0)
	if (*a++ != *b++)
	    return (false);
    return (true);
}



/* throw away the rest of the current line */
void lx1_Flush ()
{
    lexstate = RETEOS;
}



char           *
lx1_GetLine (int *n)
{
    *n = (lastch - nextch) + 1;
    return (nextch);
}




/* Return next token */
int lx1_NextToken ()
{
    static int      tokno;

    switch (lexstate)
    {
	case NEWSTMT:		/* need a new statement */
	    code = getcds ();
	    if (code == STEOF)
              return (SEOF);
            else if (code == STCOMMENT)
              { lexstate = RETEOS;
                return (SCOMMENT);
              }
            else if (code == STEMPTY)
              { lexstate = RETEOS;
                return (SEMPTY);
              }
            else if (code == STERROR)
              return (SUNKNOWN);

	    lastend = stkey == SEND;
	    if (crunch () == 0)
	      return (SUNKNOWN);
	    tokno = 0;
	    lexstate = FIRSTTOKEN;
	    lx1_StatNumber = stno;
	    stno = nxtstno;
	    lx1_TokenLen = 0;
	    /* return(SLABEL); */
	    /* go on and return next token: */

    first:
	case FIRSTTOKEN:	/* first step on a statement */
	    analyz ();
	    lexstate = OTHERTOKEN;
	    tokno = 1;
	    if( stkey == SDOWHILE )
              return SDO;
	    else if( stkey == SDOALLWHILE )
              return SDOALL;
	    else if( stkey == SPARALLELLOOPWHILE )
              return SPARALLELLOOP;
            else
              return (stkey);

	case OTHERTOKEN:	/* return next token */
	    if (nextch > lastch)
		goto reteos;
	    ++tokno;
	    if ((stkey == SLOGIF || stkey == SELSEIF || stkey == SWHERE) 
                 && parlev == 0 && tokno > 3)
	        goto first;

	    if (stkey == SASSIGN && tokno == 3 && nextch < lastch &&
		nextch[0] == 't' && nextch[1] == 'o')
	    {
		nextch += 2;
		return (STO);
	    }

	    if( (stkey == SDOWHILE || stkey == SDOALLWHILE ||
	                          stkey == SPARALLELLOOPWHILE) && tokno == 2 )
	    {
		return (SWHILE);
	    }
	    return (gettok ());

    reteos:
	case RETEOS:
	    lexstate = RETEOF;
	    return (SEOS);

	case RETEOF:
	    return (SEOF);
    }
    die_with_message ("impossible lexstate %d", lexstate);
    /* NOTREACHED */
}


/* Read the next statement into the buffer s                    */
/* Set nextch to beginning of card body, lastch to last char.   */
/* The routine name presumably stands for get CARDS?? on UNIX?? */
/* In 1985??                                                    */
static int 
getcds ()
{
    if (nextcd == NULL)
    {
	code = getcd (nextcd = s);
	stno = nxtstno;
	prevlin = thislin;
    }

    if (code == STEOF || code == STEMPTY || code == STERROR)  return (code);

    nextcd = endcd;    /* SKW */

    nextch = s;
    lastch = nextcd - 1;
    if (nextcd >= stend)
	nextcd = NULL;
    lineno = prevlin;
    prevlin = thislin;
    return (code);
}

/* Read the next line into the buffer s */
static int
getcd (char *b)
{
    register int    c;
    register char  *p;
    static char     a[6];
    static char    *aend = a + 6;

    endcd = b;

    c = getChar();

    if (c == 'c' || c == 'C' || c == '*')
    {
	while ((c = getChar()) != '\n' && c != EOF)
	    *endcd++ = c;
	if (c == EOF)
	    return (STEOF);

	++thislin;
	return (STCOMMENT);
    }

    else
    if (c != EOF)
    {
	/* a tab in columns 1-6 skips to column 7 */
	ungetChar();
	for (p = a; p < aend && (c = getChar()) != '\n' && c != EOF;)
	    if (c == '\t')
	    {
		while (p < aend)
		    *p++ = BLANK;
	    }
	    else
		*p++ = c;
    }
    if (c == EOF)
	return (STEOF);
    if (c == '\n')
    {
	while (p < aend)
	    *p++ = BLANK;
    }
    else
    {				/* read body of line */
	while ((c = getChar()) != '\n' && c != EOF)
	    *endcd++ = c;
	if (c == EOF)
	    return (STEOF);
    }
    ++thislin;
/****************************************
    if (!isspace (a[5]) && a[5] != '0')
	return (STCONTINUE);
****************************************/    /* SKW */
    for (p = a; p < aend; ++p)
	if (!isspace (*p))
	    goto initline;
    for (p = b; p < endcd; ++p)
	if (!isspace (*p))
	    goto initline;

    return STEMPTY;		/* SKW */

initline:
    nxtstno = 0;
    for (p = a; p < a + 5; ++p)
	if (!isspace (*p))
        {
	    if (isdigit (*p))
		nxtstno = 10 * nxtstno + (*p - '0');
	    else
            if (!isascii(*p))
            {
		nxtstno = 0;
		lx1_PlaceholderStatLabel = true;  /* label placeholder */
                return (STINITIAL);
            }
            else
	    {
		lineno = thislin;
		nxtstno = 0;
		lx1_PlaceholderStatLabel = false;
		yy1error("Nondigit in statement number field.");
		return (STERROR);
	    }
        }
    lx1_PlaceholderStatLabel = false;
    return (STINITIAL);
}


/* Get rid of spaces */
static int 
crunch ()
{
    register char  *i,
                   *j,
                   *j0,
                   *j1,
                   *prvstr;
    int             ten,
                    nh,
                    quote;

    /*
     * i is the next input character to be looked at j is the next output
     * character 
     */
    parlev = 0;
    expcom = 0;			/* exposed ','s */
    expeql = 0;			/* exposed equal signs */
    j = s;
    prvstr = s;
    for (i = s; i <= lastch; ++i)
    {
	if( isspace(toascii(*i)) )  /* SKW: blank placeholder => blank */
	    continue;
	if (*i == '\'' || *i == '"')
	{
	    quote = *i;
	    *j = MYQUOTE;	/* special marker */
	    for (;;)
	    {
		if (++i > lastch)
		{
		    yy1error("Unbalanced quotes");
		    return 0;	/* dead */
		}
		if (*i == quote)
		    if (i < lastch && i[1] == quote)
			++i;
		    else
			break;
		else
		if (*i == '\\' && i < lastch)
		    switch (*++i)
		    {
			case 't':
			    *i = '\t';
			    break;
			case 'b':
			    *i = '\b';
			    break;
			case 'n':
			    *i = '\n';
			    break;
			case 'f':
			    *i = '\f';
			    break;
			case 'v':
			    *i = '\v';
			    break;
			case '0':
			    *i = '\0';
			    break;
			default:
			    break;
		    }
		*++j = *i;
	    }
	    j[1] = MYQUOTE;
	    j += 2;
	    prvstr = j;
	}
	else
	if ((*i == 'h' || *i == 'H') && j > prvstr)	/* test for Hollerith
							 * strings */
	{
	    if (!isdigit (j[-1]))
		goto copychar;
	    nh = j[-1] - '0';
	    ten = 10;
	    j1 = prvstr - 1;
	    if (j1 < j - 5)
		j1 = j - 5;
	    for (j0 = j - 2; j0 > j1; --j0)
	    {
		if (!isdigit (*j0))
		    break;
		nh += ten * (*j0 - '0');
		ten *= 10;
	    }
	    if (j0 <= j1)
		goto copychar;

	    /*
	     * a hollerith must be preceded by a punctuation mark. '*' is
	     * possible only as repetition factor in a data statement not, in
	     * particular, in character*2h 
	     */

	    if (!(*j0 == '*' && s[0] == 'd') && *j0 != '/' && *j0 != '(' &&
		*j0 != ',' && *j0 != '=' && *j0 != '.')
		goto copychar;
	    if (i + nh > lastch)
	    {
		yy1error("Holerith size too big.");
		nh = lastch - i;
		return 0;	/* dead */
	    }
	    j0[1] = MYQUOTE;	/* special marker */
	    j = j0 + 1;
	    while (nh-- > 0)
	    {
		if (*++i == '\\')
		    switch (*++i)
		    {
			case 't':
			    *i = '\t';
			    break;
			case 'b':
			    *i = '\b';
			    break;
			case 'n':
			    *i = '\n';
			    break;
			case 'f':
			    *i = '\f';
			    break;
			case '0':
			    *i = '\0';
			    break;
			default:
			    break;
		    }
		*++j = *i;
	    }
	    j[1] = MYQUOTE;
	    j += 2;
	    prvstr = j;
	}
	else
	{
	    if (*i == '(')
		++parlev;
	    else
	    if (*i == ')')
		--parlev;
	    else
	    if (parlev == 0)
		if (*i == '=')
		    expeql = 1;
		else
		if (*i == ',')
		    expcom = 1;
    copychar:			/* not a string or space -- copy, shifting
				 * case if necessary */
	    if( /* shiftcase && */ isupper(*i)  &&  isascii(*i) )  /*SKW*/
		*j++ = tolower (*i);
	    else
		*j++ = *i;
	}
    }
    lastch = j - 1;
    nextch = s;
    return 1;	/* ok */
}

/* Set stkey to represent statement kind.  Distinguish IF's DO's and GOTO's */
/* from assignments. Also F90 WHERE's.                                      */
static void 
analyz ()
{
    register char  *i, *p;

/******************************* SKW
    if (parlev != 0)
    {
	err ("unbalanced parentheses, statement skipped");
	stkey = SUNKNOWN;
	return;
    }
*************************************/
    parlev = 0;
/************************************/


    if (nextch + 2 <= lastch && nextch[0] == 'i' && nextch[1] == 'f' && nextch[2] == '(')
    {

	/*
	 * assignment or if statement -- look at character after balancing
	 * paren 
	 */
	parlev = 1;
	for (i = nextch + 3; i <= lastch; ++i)
	    if (*i == (MYQUOTE))
	    {
		while (*++i != MYQUOTE)
		    ;
	    }
	    else
	    if (*i == '(')
		++parlev;
	    else
	    if (*i == ')')
	    {
		if (--parlev == 0)
		    break;
	    }
	if (i >= lastch)
	    stkey = SLOGIF;
	else
	if (i[1] == '=')
	    stkey = SLET;
	else
 	if (isdigit(toascii(i[1])))
	    stkey = SARITHIF;
        else if (isascii(i[1]))
	    stkey = SLOGIF;
        else
        {/* placeholder on an if statement (arithmetics have commas) */
            p = strchr(i+1, ',');
            stkey = ( p && p < lastch ) ? SARITHIF : SLOGIF;
	}
	if (stkey != SLET)
	    nextch += 2;
    }
    else
    if (nextch + 5 <= lastch && nextch[0] == 'w' && nextch[1] == 'h' && 
        nextch[2] == 'e' && nextch[3] == 'r' && nextch[4] == 'e' && 
        nextch[5] == '(')
    {

	/*
	 * assignment or where statement -- look at character after balancing
	 * paren 
	 */
	parlev = 1;
	for (i = nextch + 6; i <= lastch; ++i)
	    if (*i == (MYQUOTE))
	    {
		while (*++i != MYQUOTE)
		    ;
	    }
	    else
	    if (*i == '(')
		++parlev;
	    else
	    if (*i == ')')
	    {
		if (--parlev == 0)
		    break;
	    }
	if (i[1] == '=')
	    stkey = SLET;
	else
        {
	    stkey = SWHERE;
	    nextch += 5;
	}
    }
    else
    if (expeql)			/* may be an assignment */
    {
	if (expcom && nextch < lastch && strncmp(nextch, "forall",6) == 0 )
	{
	    stkey = SDOALL;
	    nextch += 6;
	}
        else
	if (expcom && nextch < lastch && 
	     strncmp(nextch, "parallelloop", 12) == 0 )
	{
	    stkey = SPARALLELLOOP;
	    nextch += 12;
	}
        else
	if (expcom && nextch < lastch &&
	    nextch[0] == 'd' && nextch[1] == 'o')
	{
	    stkey = SDO;
	    nextch += 2;
	}
	else
	    stkey = SLET;
    }
    /* otherwise search for keyword */
    else
    {
	stkey = getkwd(keystart,keyend);
	if (stkey == SGOTO && lastch >= nextch)
        {
	    if (nextch[0] == '(')
		stkey = SCOMPGOTO;
	    else
	    {
		/* see if there is a "," on the rest of the line */
		p = nextch;
		while( p < lastch && *p != ',' )  p++;
		if( p < lastch )
		    stkey = SASGOTO;
	    }
        }

/***************************************************/
/*** DGB case to recognize starting placeholders ***/
/***************************************************/
        else
	if ( (stkey == SUNKNOWN)  &&  !isascii(*nextch)  &&  
                   isalpha(toascii(*nextch)) )
        {
            *nextch = toascii(*nextch);
            stkey = getkwd(placeStart,placeEnd);
        }
/***************************************************/
/*** end of DGB case                             ***/
/***************************************************/

    }
    parlev = 0;
}


/* Look for keyword starting at position nextch */
static int 
getkwd(struct Keylist *starts[], struct Keylist *ends[])
{
    register char  *i,
                   *j;
    register struct Keylist *pk,
                   *pend;
    int             k;

    if (!isalpha (nextch[0]))
	return (SUNKNOWN);
    k = nextch[0] - 'a';
    if (pk = starts[k])
	for (pend = ends[k]; pk <= pend; ++pk)
	{
	    i = pk->keyname;
	    j = nextch;
	    while (*++i == *++j && *i != '\0')
		;
	    if (*i == '\0' && j <= lastch + 1)
	    {
		nextch = j;
		return (pk->keyval);
	    }
	}
    return (SUNKNOWN);
}


/* Initialize keyword table */
static void
initkey(struct Keylist words[], struct Keylist *starts[], 
        struct Keylist *ends[])
{
    register struct Keylist *p;
    register int    i,
                    j;

    for (i = 0; i < 26; ++i)
	starts[i] = NULL;

    for (p = words; p->keyname; ++p)
    {
	j = p->keyname[0] - 'a';
	if (starts[j] == NULL)
	    starts[j] = p;
	ends[j] = p;
    }
}


/* Return token starting at nextch */
static int 
gettok ()
{
    Boolean         havdot,
                    havexp,
                    havdbl;
    int             val;
    struct Punctlist *pp;
    extern struct Fmtlist fmts[];
    extern struct Dotlist dots[];
    struct Dotlist *pd;

    char           *i,
                   *j,
                   *n1,
                   *p;

/******************************************/
/*** SKW case to recognize placeholders ***/
/******************************************/

    if( !isascii(*nextch)  &&  isalpha(toascii(*nextch)) )
    {
	lx1_NeedKwd = false;
	*nextch = toascii(*nextch);
	return getkwd(placeStart,placeEnd);
    }

/******************************************/
/*** end of SKW case                    ***/
/******************************************/

    if (*nextch == MYQUOTE)
    {/* Undo some of the work the scanner did. */
	p = lx1_Token;
	*p++ = '\'';
	nextch++;
	for (; *nextch != MYQUOTE; nextch++)
	{
	    switch (*nextch)
	    {
		case '\'':
		    *p++ = '\\';
		    *p++ = *nextch;
		    break;
		case '\n':
		    *p++ = '\\';
		    *p++ = 'n';
		    break;
		default:
		    if (*nextch < 32)
			*nextch = ' ';
		    *p++ = *nextch;
		    break;
	    }
	}
	nextch++;
	*p++ = '\'';
	*p = '\0';
	lx1_TokenLen = p - lx1_Token;
	return (SHOLLERITH);
    }
    if (stkey == SFORMAT)
    {/* Undo some of the work the scanner did. */
	for (p = lx1_Token; nextch <= lastch; nextch++)
	{
	    switch (*nextch)
	    {
		case MYQUOTE:
		    *p++ = '\'';
		    break;
		case '\'':
		    *p++ = '\'';
		    *p++ = '\'';
		    break;
		case '\n':
		    *p++ = '\\';
		    *p++ = 'n';
		    break;
		default:
		    if (*nextch < 32)
		        *nextch = ' ';
		    *p++ = *nextch;
		    break;
	    }
	}
	nextch++;
	*p = '\0';
	lx1_TokenLen = p - lx1_Token;
	return (SFORMATSPEC);
    }

    for (pp = puncts; pp->punchar; ++pp)
	if (*nextch == pp->punchar)
	{
	    if ((*nextch == '*' || *nextch == '/') &&
		nextch < lastch && nextch[1] == nextch[0])
	    {
		if (*nextch == '*')
		    val = SPOWER;
		else
		    val = SCONCAT;
		nextch += 2;
	    }
	    else
	    {
		val = pp->punval;
		if (val == SLPAR)
		    ++parlev;
		else
		if (val == SRPAR)
		    --parlev;
		++nextch;
	    }
	    return (val);
	}
    if (*nextch == '.')
	if (nextch >= lastch)
	    goto badchar;
	else
	if (isdigit (nextch[1]))
	    goto numconst;
	else
	{
	    for (pd = dots; (j = pd->dotname); ++pd)
	    {
		for (i = nextch + 1; i <= lastch; ++i)
		    if (*i != *j)
			break;
		    else
		    if (*i != '.')
			++j;
		    else
		    {
			nextch = i + 1;
			return (pd->dotval);
		    }
	    }
	    goto badchar;
	}

    if (isalpha (*nextch) 
	|| *nextch == '$'       /* VS FORTRAN */
	|| *nextch == '_')	/* Standard Extension */
    {
        if (lx1_NeedKwd)
        {
	    lx1_NeedKwd = false;
	    return (getkwd(keystart,keyend));
        }

	p = lx1_Token;
	*p++ = *nextch++;
	while (nextch <= lastch)
	    if (isalpha (*nextch) || isdigit (*nextch)
			|| *nextch == '$' /* VS FORTRAN */
			|| *nextch == '_' /* Standard Extension */
		)	
		*p++ = *nextch++;
	    else
		break;
	*p = '\0';
	lx1_TokenLen = p - lx1_Token;
	if (lx1_InIOControl && nextch <= lastch && *nextch == '=')
	{
	    lx1_IOKwd = kwd_name(lx1_Token);
	    if (lx1_IOKwd != IOERROR)
	    {
		++nextch;
	    	return (SNAMEEQ);
	    }
	}
	if (lx1_TokenLen > 8 && eqn (8, lx1_Token, "function") &&
	    isalpha (toascii(lx1_Token[8])) &&	/* cf. placeholders */
	    nextch < lastch && nextch[0] == '('
            /* && (nextch[1] == ')' || isalpha (toascii(nextch[1])) ) */
           )
	{
	    nextch -= (lx1_TokenLen - 8);
	    return (SFUNCTION);
	}
        return (SNAME);
    }
    if (!isdigit (*nextch))
	goto badchar;
numconst:
    havdot = false;
    havexp = false;
    havdbl = false;
    for (n1 = nextch; nextch <= lastch; ++nextch)
    {
	if (*nextch == '.')
	    if (havdot)
		break;
	    else
		if (nextch + 2 <= lastch && isalpha (nextch[1])
		    && isalpha (nextch[2]))
		break;
	    else
		havdot = true;
	else
	if (!lx1_IntOnly && (*nextch == 'd' || *nextch == 'e'))
	{
	    p = nextch;
	    havexp = true;
	    if (*nextch == 'd')
		havdbl = true;
	    if (nextch < lastch)
		if (nextch[1] == '+' || nextch[1] == '-')
		    ++nextch;
	    if ((nextch >= lastch) || !isdigit (*++nextch))
	    {
		nextch = p;
		havdbl = havexp = false;
		break;
	    }
	    for (++nextch;
		 nextch <= lastch && isdigit (*nextch);
		 ++nextch);
	    break;
	}
	else
	if (!isdigit (*nextch))
	    break;
    }
    p = lx1_Token;
    i = n1;
    while (i < nextch)
	*p++ = *i++;
    *p = '\0';
    lx1_TokenLen = p - lx1_Token;

    if (havdbl)
	return (SDCON);
    if (havdot || havexp)
	return (SRCON);
    return (SICON);
badchar:
    s[0] = *nextch++;
    return (SUNKNOWN);
}


/*
 * KEYWORD AND SPECIAL CHARACTER TABLES 
 */


static
KWD_OPT kwd_name(char *name)
{
  static char *kwd_map[] =
  {
    "unit", "fmt", "err", "end", "iostat",
    "rec", "recl", "file", "status", "access",
    "form", "blank", "exist", "opened", "number",
    "named", "name", "sequential", "direct", "formatted",
    "unformatted", "nextrec", "error"
  };

  int i;

  for( i = 0; i < sizeof(kwd_map) / sizeof(char *); i++ )
    if( strcmp(kwd_map[i], name) == 0 )  return (KWD_OPT) i;

  return IOERROR;
}


