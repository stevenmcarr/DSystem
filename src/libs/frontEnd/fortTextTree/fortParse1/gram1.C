/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy1 or YY1, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YY1BISON 1

/* Bison version.  */
#define YY1BISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YY1SKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YY1PURE 0

/* Push parsers.  */
#define YY1PUSH 0

/* Pull parsers.  */
#define YY1PULL 1




/* Copy the first part of user declarations.  */
#line 17 "gram1.y" /* yacc.c:339  */


#define gram1_h			/* will already have yacc-generated decls */

#include <string.h>
#include <libs/support/misc/general.h>

#include <libs/support/strings/rn_string.h>
#include <libs/support/memMgmt/mem.h>

#include <libs/frontEnd/fortTextTree/FortTextTree.i>

#include <libs/frontEnd/fortTextTree/fortParse1/FortParse1.i>
#include <libs/frontEnd/fortTextTree/fortParse1/lex1.h>

#line 53 "gram1.y" /* yacc.c:339  */


static int		iostmt;
STATIC(int,		yy1lex, (void));
STATIC(FortTreeNode,	coerceToLabel, (FortTreeNode node));


#line 90 "gram1.tab.c" /* yacc.c:339  */

# ifndef YY1_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY1_NULLPTR nullptr
#  else
#   define YY1_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YY1ERROR_VERBOSE
# undef YY1ERROR_VERBOSE
# define YY1ERROR_VERBOSE 1
#else
# define YY1ERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "gram1.tab.h".  */
#ifndef YY1_YY1_GRAM1_TAB_H_INCLUDED
# define YY1_YY1_GRAM1_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YY1DEBUG
# define YY1DEBUG 0
#endif
#if YY1DEBUG
extern int yy1debug;
#endif

/* Token type.  */
#ifndef YY1TOKENTYPE
# define YY1TOKENTYPE
  enum yy1tokentype
  {
    SARITHIF = 258,
    SASGOTO = 259,
    SASSIGN = 260,
    SAT = 261,
    SBACKSPACE = 262,
    SBARRIER = 263,
    SBLOCK = 264,
    SBLOCKDATA = 265,
    SCALL = 266,
    SCHARACTER = 267,
    SCLEAR = 268,
    SCLOSE = 269,
    SCOMMENT = 270,
    SCOMMON = 271,
    SCOMPGOTO = 272,
    SCOMPLEX = 273,
    SCONTINUE = 274,
    SCREATETASK = 275,
    SDATA = 276,
    SDEBUG = 277,
    SDIMENSION = 278,
    SDO = 279,
    SDOALL = 280,
    SDOALLWHILE = 281,
    SDOUBLE = 282,
    SDOWHILE = 283,
    SELSE = 284,
    SELSEIF = 285,
    SEMPTY = 286,
    SEND = 287,
    SENDALL = 288,
    SENDDO = 289,
    SENDFILE = 290,
    SENDIF = 291,
    SENDLOOP = 292,
    SENTRY = 293,
    SEOS = 294,
    SEQUIV = 295,
    SEVENT = 296,
    SEXACT = 297,
    SEXTERNAL = 298,
    SFORMAT = 299,
    SFUNCTION = 300,
    SGOTO = 301,
    SIMPLICIT = 302,
    SIMPLICITNONE = 303,
    SINC = 304,
    SINIT = 305,
    SINQUIRE = 306,
    SINTEGER = 307,
    SINTRINSIC = 308,
    SLET = 309,
    SLOCK = 310,
    SLOGICAL = 311,
    SLOGIF = 312,
    SNAME = 313,
    SNAMEEQ = 314,
    SOPEN = 315,
    SOTHERPROCESSES = 316,
    SPARALLEL = 317,
    SPARALLELLOOP = 318,
    SPARALLELLOOPWHILE = 319,
    SPARAMETER = 320,
    SPARBEGIN = 321,
    SPAREND = 322,
    SPAUSE = 323,
    SPOST = 324,
    SPOSTING = 325,
    SPRINT = 326,
    SPRIVATE = 327,
    SPROGRAM = 328,
    SREAD = 329,
    SREAL = 330,
    SRETURN = 331,
    SREWIND = 332,
    SSAVE = 333,
    SSEMAPHORE = 334,
    SSET = 335,
    SSTOP = 336,
    SSTOPLOOP = 337,
    SSUBCHK = 338,
    SSUBROUTINE = 339,
    SSUBTRACE = 340,
    STASK = 341,
    STASKCOMMON = 342,
    STHEN = 343,
    STIMES = 344,
    STO = 345,
    STRACE = 346,
    STRACEOFF = 347,
    STRACEON = 348,
    SUNIT = 349,
    SUNKNOWN = 350,
    SUNLOCK = 351,
    SUNTIL = 352,
    SVAL = 353,
    SWAIT = 354,
    SWHILE = 355,
    SWRITE = 356,
    SDCON = 357,
    SFORMATSPEC = 358,
    SHEXCON = 359,
    SHOLLERITH = 360,
    SICON = 361,
    SRCON = 362,
    SCOLON = 363,
    SCOMMA = 364,
    SCONCAT = 365,
    SEQUALS = 366,
    SLPAR = 367,
    SMINUS = 368,
    SPERCENT = 369,
    SPLUS = 370,
    SPOWER = 371,
    SRPAR = 372,
    SSLASH = 373,
    SSTAR = 374,
    SAND = 375,
    SEQ = 376,
    SEQV = 377,
    SFALSE = 378,
    SGE = 379,
    SGT = 380,
    SLE = 381,
    SLT = 382,
    SNE = 383,
    SNEQV = 384,
    SNOT = 385,
    SOR = 386,
    STRUE = 387,
    SALLOCATABLE = 388,
    SALLOCATE = 389,
    SDEALLOCATE = 390,
    SWHERE = 391,
    SELSEWHERE = 392,
    SENDWHERE = 393,
    SARG_PH = 394,
    SARRAY_DECL_LEN_PH = 395,
    SBOUND_PH = 396,
    SLEN_PH = 397,
    SCOMMON_ELT_PH = 398,
    SCOMMON_VARS_PH = 399,
    SCONSTANT_PH = 400,
    SLOOP_CONTROL_PH = 401,
    SDATA_INIT_PH = 402,
    SDATA_PH = 403,
    SDIM_PH = 404,
    SEQUIV_PH = 405,
    SFORMAL_PH = 406,
    SFORMAT_PH = 407,
    SIMPLICIT_DEF_PH = 408,
    SLETTERS_PH = 409,
    SINIT_PH = 410,
    SINVOCATION_PH = 411,
    SKWD_PH = 412,
    SSPECIFY_KWD_PH = 413,
    SQUERY_KWD_PH = 414,
    SLABEL_PH = 415,
    SLETTER_PH = 416,
    SVAR_PH = 417,
    SNAME_PH = 418,
    SOPTION_PH = 419,
    SPARAM_DEF_PH = 420,
    SPOSTING_PH = 421,
    SPOST_EXPR_PH = 422,
    SEXPR_PH = 423,
    SSTMT_PH = 424,
    SSUBPROGRAM_PH = 425,
    SARITH_EXPR_PH = 426,
    SSTRING_EXPR_PH = 427,
    SRELATIONAL_EXPR_PH = 428,
    SLOGICAL_EXPR_PH = 429,
    SSTRING_VAR_PH = 430,
    STEXT_PH = 431,
    STYPE_PH = 432,
    STYPENAME_PH = 433,
    SUNIT_PH = 434,
    SSPECIFICATION_STMT_PH = 435,
    SCONTROL_STMT_PH = 436,
    SIO_STMT_PH = 437,
    SPARASCOPE_STMT_PH = 438,
    SDEBUG_STMT_PH = 439
  };
#endif

/* Value type.  */
#if ! defined YY1STYPE && ! defined YY1STYPE_IS_DECLARED

union YY1STYPE
{
#line 72 "gram1.y" /* yacc.c:355  */

    FortTreeNode astptr;
    fx_StatToken statval;
    KWD_OPT  kwdval;
    Boolean  boolean;
    

#line 323 "gram1.tab.c" /* yacc.c:355  */
};

typedef union YY1STYPE YY1STYPE;
# define YY1STYPE_IS_TRIVIAL 1
# define YY1STYPE_IS_DECLARED 1
#endif


extern YY1STYPE yy1lval;

int yy1parse (void);

#endif /* !YY1_YY1_GRAM1_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 340 "gram1.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YY1TYPE_UINT8
typedef YY1TYPE_UINT8 yy1type_uint8;
#else
typedef unsigned char yy1type_uint8;
#endif

#ifdef YY1TYPE_INT8
typedef YY1TYPE_INT8 yy1type_int8;
#else
typedef signed char yy1type_int8;
#endif

#ifdef YY1TYPE_UINT16
typedef YY1TYPE_UINT16 yy1type_uint16;
#else
typedef unsigned short int yy1type_uint16;
#endif

#ifdef YY1TYPE_INT16
typedef YY1TYPE_INT16 yy1type_int16;
#else
typedef short int yy1type_int16;
#endif

#ifndef YY1SIZE_T
# ifdef __SIZE_TYPE__
#  define YY1SIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YY1SIZE_T size_t
# elif ! defined YY1SIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YY1SIZE_T size_t
# else
#  define YY1SIZE_T unsigned int
# endif
#endif

#define YY1SIZE_MAXIMUM ((YY1SIZE_T) -1)

#ifndef YY1_
# if defined YY1ENABLE_NLS && YY1ENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY1_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY1_
#  define YY1_(Msgid) Msgid
# endif
#endif

#ifndef YY1_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY1_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY1_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY1_ATTRIBUTE_PURE
# define YY1_ATTRIBUTE_PURE   YY1_ATTRIBUTE ((__pure__))
#endif

#ifndef YY1_ATTRIBUTE_UNUSED
# define YY1_ATTRIBUTE_UNUSED YY1_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY1_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY1USE(E) ((void) (E))
#else
# define YY1USE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yy1lval being uninitialized.  */
# define YY1_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY1_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY1_INITIAL_VALUE(Value) Value
#endif
#ifndef YY1_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY1_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY1_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY1_INITIAL_VALUE
# define YY1_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yy1overflow || YY1ERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YY1STACK_USE_ALLOCA
#  if YY1STACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YY1STACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YY1STACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YY1STACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YY1STACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YY1STACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YY1STACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YY1STACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YY1STACK_ALLOC YY1MALLOC
#  define YY1STACK_FREE YY1FREE
#  ifndef YY1STACK_ALLOC_MAXIMUM
#   define YY1STACK_ALLOC_MAXIMUM YY1SIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YY1MALLOC || defined malloc) \
             && (defined YY1FREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YY1MALLOC
#   define YY1MALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YY1SIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YY1FREE
#   define YY1FREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yy1overflow || YY1ERROR_VERBOSE */


#if (! defined yy1overflow \
     && (! defined __cplusplus \
         || (defined YY1STYPE_IS_TRIVIAL && YY1STYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yy1alloc
{
  yy1type_int16 yy1ss_alloc;
  YY1STYPE yy1vs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YY1STACK_GAP_MAXIMUM (sizeof (union yy1alloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YY1STACK_BYTES(N) \
     ((N) * (sizeof (yy1type_int16) + sizeof (YY1STYPE)) \
      + YY1STACK_GAP_MAXIMUM)

# define YY1COPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YY1SIZE and YY1STACKSIZE give the old and new number of
   elements in the stack, and YY1PTR gives the new location of the
   stack.  Advance YY1PTR to a properly aligned location for the next
   stack.  */
# define YY1STACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YY1SIZE_T yy1newbytes;                                            \
        YY1COPY (&yy1ptr->Stack_alloc, Stack, yy1size);                    \
        Stack = &yy1ptr->Stack_alloc;                                    \
        yy1newbytes = yy1stacksize * sizeof (*Stack) + YY1STACK_GAP_MAXIMUM; \
        yy1ptr += yy1newbytes / sizeof (*yy1ptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YY1COPY_NEEDED && YY1COPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YY1COPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YY1COPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YY1COPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YY1SIZE_T yy1i;                         \
          for (yy1i = 0; yy1i < (Count); yy1i++)   \
            (Dst)[yy1i] = (Src)[yy1i];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YY1COPY_NEEDED */

/* YY1FINAL -- State number of the termination state.  */
#define YY1FINAL  187
/* YY1LAST -- Last index in YY1TABLE.  */
#define YY1LAST   2614

/* YY1NTOKENS -- Number of terminals.  */
#define YY1NTOKENS  185
/* YY1NNTS -- Number of nonterminals.  */
#define YY1NNTS  109
/* YY1NRULES -- Number of rules.  */
#define YY1NRULES  411
/* YY1NSTATES -- Number of states.  */
#define YY1NSTATES  850

/* YY1TRANSLATE[YY1X] -- Symbol number corresponding to YY1X as returned
   by yy1lex, with out-of-bounds checking.  */
#define YY1UNDEFTOK  2
#define YY1MAXUTOK   439

#define YY1TRANSLATE(YY1X)                                                \
  ((unsigned int) (YY1X) <= YY1MAXUTOK ? yy1translate[YY1X] : YY1UNDEFTOK)

/* YY1TRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yy1lex, without out-of-bounds checking.  */
static const yy1type_uint8 yy1translate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    60,    61,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,   104,
     105,   106,   107,   108,   109,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,   120,   121,   122,   123,   124,
     125,   126,   127,   128,   129,   130,   131,   132,   133,   134,
     135,   136,   137,   138,   139,   140,   141,   142,   143,   144,
     145,   146,   147,   148,   149,   150,   151,   152,   153,   154,
     155,   156,   157,   158,   159,   160,   161,   162,   163,   164,
     165,   166,   167,   168,   169,   170,   171,   172,   173,   174,
     175,   176,   177,   178,   179,   180,   181,   182,   183,   184
};

#if YY1DEBUG
  /* YY1RLINE[YY1N] -- Source line where rule number YY1N was defined.  */
static const yy1type_uint16 yy1rline[] =
{
       0,   216,   216,   288,   310,   325,   342,   351,   362,   367,
     374,   380,   389,   390,   405,   421,   443,   454,   458,   463,
     470,   476,   485,   486,   491,   515,   539,   555,   571,   572,
     600,   611,   617,   631,   639,   645,   652,   666,   674,   680,
     686,   701,   711,   717,   727,   733,   741,   747,   756,   757,
     758,   763,   767,   777,   783,   792,   798,   799,   807,   813,
     814,   818,   824,   828,   829,   830,   839,   849,   855,   864,
     870,   878,   884,   893,   899,   905,   910,   918,   923,   924,
     939,   949,   955,   964,   970,   985,  1002,  1018,  1035,  1042,
    1052,  1058,  1067,  1073,  1081,  1087,  1096,  1097,  1120,  1128,
    1141,  1156,  1173,  1183,  1189,  1198,  1204,  1219,  1226,  1236,
    1242,  1251,  1252,  1263,  1274,  1280,  1289,  1295,  1304,  1308,
    1312,  1321,  1325,  1331,  1337,  1345,  1350,  1355,  1360,  1365,
    1370,  1375,  1380,  1385,  1390,  1395,  1404,  1408,  1412,  1417,
    1423,  1447,  1464,  1481,  1491,  1497,  1503,  1509,  1517,  1523,
    1532,  1533,  1539,  1545,  1560,  1577,  1585,  1593,  1601,  1607,
    1613,  1619,  1634,  1641,  1648,  1656,  1674,  1675,  1685,  1691,
    1697,  1704,  1712,  1729,  1750,  1784,  1805,  1812,  1819,  1826,
    1833,  1840,  1847,  1854,  1864,  1871,  1878,  1888,  1895,  1902,
    1912,  1919,  1926,  1933,  1937,  1947,  1959,  1969,  1984,  1990,
    1999,  2004,  2089,  2094,  2099,  2107,  2114,  2115,  2120,  2125,
    2130,  2138,  2144,  2153,  2157,  2164,  2172,  2178,  2184,  2188,
    2194,  2200,  2206,  2212,  2218,  2224,  2232,  2233,  2240,  2247,
    2257,  2258,  2266,  2267,  2268,  2273,  2279,  2285,  2291,  2297,
    2303,  2307,  2313,  2319,  2324,  2339,  2356,  2373,  2399,  2414,
    2420,  2426,  2432,  2438,  2444,  2469,  2486,  2495,  2501,  2510,
    2516,  2521,  2527,  2532,  2537,  2543,  2548,  2563,  2580,  2606,
    2624,  2641,  2658,  2666,  2674,  2682,  2688,  2703,  2720,  2725,
    2730,  2737,  2743,  2748,  2763,  2773,  2779,  2788,  2792,  2798,
    2804,  2819,  2827,  2838,  2842,  2856,  2873,  2890,  2897,  2924,
    2932,  2941,  2949,  2955,  2970,  2987,  2994,  3019,  3025,  3028,
    3034,  3042,  3057,  3068,  3074,  3082,  3098,  3104,  3112,  3129,
    3139,  3145,  3152,  3170,  3176,  3186,  3190,  3195,  3199,  3206,
    3207,  3212,  3216,  3217,  3218,  3224,  3230,  3236,  3242,  3248,
    3252,  3258,  3264,  3270,  3276,  3282,  3288,  3294,  3300,  3306,
    3312,  3318,  3324,  3330,  3335,  3339,  3344,  3349,  3354,  3359,
    3367,  3373,  3382,  3392,  3401,  3411,  3424,  3429,  3437,  3446,
    3451,  3460,  3461,  3467,  3473,  3479,  3485,  3491,  3497,  3504,
    3510,  3519,  3523,  3527,  3533,  3539,  3545,  3551,  3557,  3563,
    3569,  3577,  3596,  3619,  3628,  3634,  3643,  3649,  3657,  3663,
    3673,  3677,  3681,  3687,  3696,  3699,  3716,  3723,  3730,  3737,
    3744,  3751
};
#endif

#if YY1DEBUG || YY1ERROR_VERBOSE || 0
/* YY1TNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YY1NTOKENS, nonterminals.  */
static const char *const yy1tname[] =
{
  "$end", "error", "$undefined", "SARITHIF", "SASGOTO", "SASSIGN", "SAT",
  "SBACKSPACE", "SBARRIER", "SBLOCK", "SBLOCKDATA", "SCALL", "SCHARACTER",
  "SCLEAR", "SCLOSE", "SCOMMENT", "SCOMMON", "SCOMPGOTO", "SCOMPLEX",
  "SCONTINUE", "SCREATETASK", "SDATA", "SDEBUG", "SDIMENSION", "SDO",
  "SDOALL", "SDOALLWHILE", "SDOUBLE", "SDOWHILE", "SELSE", "SELSEIF",
  "SEMPTY", "SEND", "SENDALL", "SENDDO", "SENDFILE", "SENDIF", "SENDLOOP",
  "SENTRY", "SEOS", "SEQUIV", "SEVENT", "SEXACT", "SEXTERNAL", "SFORMAT",
  "SFUNCTION", "SGOTO", "SIMPLICIT", "SIMPLICITNONE", "SINC", "SINIT",
  "SINQUIRE", "SINTEGER", "SINTRINSIC", "SLET", "SLOCK", "SLOGICAL",
  "SLOGIF", "SNAME", "SNAMEEQ", "SOPEN", "SOTHERPROCESSES", "SPARALLEL",
  "SPARALLELLOOP", "SPARALLELLOOPWHILE", "SPARAMETER", "SPARBEGIN",
  "SPAREND", "SPAUSE", "SPOST", "SPOSTING", "SPRINT", "SPRIVATE",
  "SPROGRAM", "SREAD", "SREAL", "SRETURN", "SREWIND", "SSAVE",
  "SSEMAPHORE", "SSET", "SSTOP", "SSTOPLOOP", "SSUBCHK", "SSUBROUTINE",
  "SSUBTRACE", "STASK", "STASKCOMMON", "STHEN", "STIMES", "STO", "STRACE",
  "STRACEOFF", "STRACEON", "SUNIT", "SUNKNOWN", "SUNLOCK", "SUNTIL",
  "SVAL", "SWAIT", "SWHILE", "SWRITE", "SDCON", "SFORMATSPEC", "SHEXCON",
  "SHOLLERITH", "SICON", "SRCON", "SCOLON", "SCOMMA", "SCONCAT", "SEQUALS",
  "SLPAR", "SMINUS", "SPERCENT", "SPLUS", "SPOWER", "SRPAR", "SSLASH",
  "SSTAR", "SAND", "SEQ", "SEQV", "SFALSE", "SGE", "SGT", "SLE", "SLT",
  "SNE", "SNEQV", "SNOT", "SOR", "STRUE", "SALLOCATABLE", "SALLOCATE",
  "SDEALLOCATE", "SWHERE", "SELSEWHERE", "SENDWHERE", "SARG_PH",
  "SARRAY_DECL_LEN_PH", "SBOUND_PH", "SLEN_PH", "SCOMMON_ELT_PH",
  "SCOMMON_VARS_PH", "SCONSTANT_PH", "SLOOP_CONTROL_PH", "SDATA_INIT_PH",
  "SDATA_PH", "SDIM_PH", "SEQUIV_PH", "SFORMAL_PH", "SFORMAT_PH",
  "SIMPLICIT_DEF_PH", "SLETTERS_PH", "SINIT_PH", "SINVOCATION_PH",
  "SKWD_PH", "SSPECIFY_KWD_PH", "SQUERY_KWD_PH", "SLABEL_PH", "SLETTER_PH",
  "SVAR_PH", "SNAME_PH", "SOPTION_PH", "SPARAM_DEF_PH", "SPOSTING_PH",
  "SPOST_EXPR_PH", "SEXPR_PH", "SSTMT_PH", "SSUBPROGRAM_PH",
  "SARITH_EXPR_PH", "SSTRING_EXPR_PH", "SRELATIONAL_EXPR_PH",
  "SLOGICAL_EXPR_PH", "SSTRING_VAR_PH", "STEXT_PH", "STYPE_PH",
  "STYPENAME_PH", "SUNIT_PH", "SSPECIFICATION_STMT_PH", "SCONTROL_STMT_PH",
  "SIO_STMT_PH", "SPARASCOPE_STMT_PH", "SDEBUG_STMT_PH", "$accept", "line",
  "statement", "function_args", "function_arg_list", "function_arg",
  "subroutine_arguments", "subroutine_arg_list", "subroutine_arg",
  "common_list", "common_elt", "common_block", "common_name", "common_var",
  "data_elt_list", "data_elt", "data_lval_list", "data_lval",
  "data_implied_do", "data_rval_list", "data_rval", "data_repeat",
  "data_const", "dimension_decl_list", "dimension_decl", "dimension_list",
  "dimension_elt", "dimension_ubound", "equivalence_class_list",
  "equivalence_class", "implicit_elt_list", "implicit_elt",
  "implicit_letter_groups", "implicit_letter_group", "implicit_letter",
  "parameter_list", "parameter_elt", "save_list", "save_elt",
  "type_elt_list", "type_elt", "type_init_list", "type_dims", "type",
  "type_name", "type_lengspec", "if_able_statement", "call_invocation",
  "call_rval_list", "call_rval", "do_control", "io_bs_rew_end",
  "io_inq_open_close", "io_read", "io_write", "io_print", "io_control",
  "io_control_list", "io_control_elt", "io_control_kwd",
  "io_control_value", "io_input_list", "io_input_elt", "io_output_list",
  "io_output_unparen_elt", "io_output_paren_elt", "io_rval",
  "io_unparen_rval", "debug_option_list", "debug_option", "doall_control",
  "post_posting", "post_value", "task_posting", "parallelloop_control",
  "alloc_object", "alloc_object_list", "subscripted_array",
  "subscripted_array_list", "dealloc_object_list", "rval_list",
  "optional_rval", "optional_comma_rval", "rval", "unparen_rval",
  "expr_ph", "lval_list", "lval", "substring", "subscript",
  "subscript_elt", "subscript_elt_list", "optional_const", "simple_const",
  "complex_const", "stmt_label", "stmt_no_label", "label_list", "label",
  "name_list", "optional_name", "name", "optional_comma", "intonlyon",
  "intonlyoff", "iocontrolon", "iocontroloff", "needkwdon", "needkwdoff", YY1_NULLPTR
};
#endif

# ifdef YY1PRINT
/* YY1TOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yy1type_uint16 yy1toknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     315,   316,   317,   318,   319,   320,   321,   322,   323,   324,
     325,   326,   327,   328,   329,   330,   331,   332,   333,   334,
     335,   336,   337,   338,   339,   340,   341,   342,   343,   344,
     345,   346,   347,   348,   349,   350,   351,   352,   353,   354,
     355,   356,   357,   358,   359,   360,   361,   362,   363,   364,
     365,   366,   367,   368,   369,   370,   371,   372,   373,   374,
     375,   376,   377,   378,   379,   380,   381,   382,   383,   384,
     385,   386,   387,   388,   389,   390,   391,   392,   393,   394,
     395,   396,   397,   398,   399,   400,   401,   402,   403,   404,
     405,   406,   407,   408,   409,   410,   411,   412,   413,   414,
     415,   416,   417,   418,   419,   420,   421,   422,   423,   424,
     425,   426,   427,   428,   429,   430,   431,   432,   433,   434,
     435,   436,   437,   438,   439
};
# endif

#define YY1PACT_NINF -603

#define yy1pact_value_is_default(Yystate) \
  (!!((Yystate) == (-603)))

#define YY1TABLE_NINF -410

#define yy1table_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-410)))

  /* YY1PACT[STATE-NUM] -- Index in YY1TABLE of the portion describing
     STATE-NUM.  */
static const yy1type_int16 yy1pact[] =
{
    1227,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,    67,    32,    43,   -27,
    -603,   743,    -4,  -603,  -603,  -603,    19,     5,   -53,   -53,
    -603,   -26,     5,    38,   -26,  -603,   298,    22,  -603,    38,
      60,  -603,    57,    -7,    46,  -603,    93,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,     5,   -38,     5,   -59,     5,   221,
    -603,  -603,  -603,     5,   -26,   -26,   128,  -603,   119,   146,
     106,   149,   177,  -603,   457,   -12,  -603,     5,     5,  -603,
    1930,  -603,   250,   -26,   457,   -53,     5,    38,   298,  -603,
    -603,   -26,   -26,  -603,    57,   193,   220,   229,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
     184,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,   516,
    1017,  1017,  -603,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
     239,  -603,  -603,   547,   275,  -603,  -603,   245,   516,  -603,
     743,    -4,  1017,  1930,   184,  -603,  -603,   293,  -603,  -603,
    -603,  -603,  -603,  -603,  -603,   267,  -603,  -603,  -603,    36,
    -603,  -603,   288,  -603,   115,   234,   299,   -53,   301,    14,
    -603,  -603,  -603,  -603,  -603,  -603,  -603,    -3,  -603,   -34,
    -603,  -603,  -603,  -603,   329,    31,  -603,   330,  -603,   337,
     338,  1930,  -603,  -603,   184,   342,  -603,  -603,   184,   344,
    1930,  -603,   347,   -26,  -603,   377,  -603,   386,  -603,  -603,
    -603,   385,  -603,  -603,   390,    68,  -603,   386,   403,  -603,
    1930,  -603,  -603,   421,  -603,  -603,   184,   404,   -30,   -26,
    -603,  -603,  -603,  -603,  -603,   386,  -603,  1930,  1930,  1930,
    1930,  -603,  -603,  2466,  -603,  -603,  -603,  -603,  -603,  -603,
    -603,   393,  -603,  -603,   435,  -603,  -603,   347,   301,   288,
    -603,   441,  -603,  -603,   433,   337,     5,     5,  1930,     5,
    -603,   230,   434,  -603,  1017,  -603,  -603,  -603,  -603,   241,
    -603,   667,   617,  -603,   437,   437,  1930,  -603,  -603,  1017,
    1017,  1017,  1017,  1017,  1017,  -603,  1663,   239,   644,  -603,
      88,    20,  1752,   528,  2266,   442,     5,  1508,  -603,  -603,
     440,   298,  -603,   115,  1053,   281,  -603,  -603,  -603,   446,
      60,    14,  2019,  -603,   448,   453,  -603,  -603,   466,  -603,
    -603,    57,  1053,  1930,  2286,    84,  1930,   152,  1930,  2306,
      -6,  -603,   283,  -603,   -38,     5,   222,  -603,  -603,  -603,
    -603,   479,  1930,  2326,  1930,   166,  1930,  -603,   290,  -603,
     483,   484,   469,  2346,   488,   494,   494,  1373,  1930,  1930,
    1930,  1930,  1930,  1930,  1930,  1930,  1930,  1930,  1930,  1930,
    1930,  1930,  1930,  1930,   250,  1930,  -603,  -603,  -603,    57,
    -603,   297,   245,   303,  -603,  2366,   385,  -603,   490,  -603,
     499,  1766,  -603,  -603,   394,   289,  -603,  -603,  -603,  -603,
    -603,  -603,  2466,   516,   570,   507,   454,   465,   465,   437,
     437,   437,  1841,  -603,   811,  -603,   306,  -603,  -603,    88,
    -603,   508,  -603,  -603,  -603,   515,  1752,  -603,  -603,   517,
     518,  2466,   172,   202,  -603,  1752,  -603,   -53,   -53,  -603,
    -603,  -603,   -53,  -603,   312,  -603,  2466,  -603,  -603,  -603,
    -603,  -603,  -603,  -603,   334,  -603,  -603,   863,   -53,   184,
     546,    14,  -603,  -603,   506,  1930,   259,   259,  -603,   168,
    -603,   511,  -603,  -603,  -603,  -603,   521,    31,     5,     5,
     457,  -603,  -603,   343,  2386,  -603,  -603,  2028,  -603,  2085,
     548,  -603,  -603,  -603,   355,  -603,  -603,   -26,  -603,  -603,
    -603,  -603,  -603,   356,  -603,  -603,    68,  -603,   140,  2466,
    1366,   360,  2466,  -603,  2108,   -30,  -603,  1930,  1930,    -1,
    -603,  -603,  1930,   585,   474,   474,   494,   494,   494,  1373,
    2486,  2052,  2486,  2486,  2486,  2486,  2486,  2052,   835,  -603,
    2466,   537,  -603,     5,  -603,  -603,     5,  -603,  1505,  -603,
     230,  1053,   -27,   525,  2406,  -603,  -603,  -603,  -603,   361,
     798,  1930,  1930,  1221,  1930,  1855,  -603,   540,    88,  -603,
      88,   541,   543,   544,  -603,  1944,  1944,  1944,  -603,   549,
     366,   561,  -603,  1588,  -603,  1053,  -603,  1677,  -603,  1930,
    -603,   -29,  -603,  -603,  -603,  2019,  -603,   259,  -603,   367,
     379,   550,  -603,  -603,   575,  1930,  1930,  -603,    39,  -603,
    -603,    40,  -603,  -603,  -603,  -603,  -603,   389,  -603,   557,
    -603,  -603,  1930,  -603,  1930,  -603,  2466,  2426,  1930,  1930,
    -603,  -603,   554,  -603,  -603,  -603,  -603,  -603,   391,   -52,
    -603,  -603,  -603,   289,  -603,   564,  2466,  1930,  1353,  -603,
      88,  -603,   508,  1944,  1944,  1944,  -603,   209,  -603,   225,
    -603,   226,  -603,   -53,  -603,  -603,  -603,  -603,  -603,  2466,
    2466,   -12,  1930,  -603,  -603,  -603,  -603,  -603,  -603,  -603,
    2131,  2131,  -603,  -603,  -603,   140,  -603,     6,  2466,  2131,
    -603,  2466,  2466,  -603,  1930,  -603,  2019,  -603,  -603,  -603,
    -603,   428,  -603,  2466,  1930,    16,   114,   266,   439,   284,
     573,   572,  -603,  2154,  -603,  1930,  -603,  -603,  -603,  -603,
    -603,  2466,   181,  -603,  2466,  1930,  1930,  1930,  1930,   -53,
    1930,  1930,  2466,  -603,  2177,  2200,  2223,  2246,  -603,  2446,
    2131,  1930,  1930,  1930,  1930,  -603,   576,  2131,  2131,  2131,
    2131,  -603,   577,   582,   588,   590,  -603,  -603,  -603,  -603
};

  /* YY1DEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YY1TABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yy1type_uint16 yy1defact[] =
{
       0,   392,   392,   392,   392,   392,   134,   392,   392,   392,
     127,   392,   392,    25,   392,   392,   128,   392,   392,   392,
     392,   392,   392,   392,   129,   392,   392,   393,   392,   392,
     392,   392,   392,   392,   392,   392,   133,   131,   392,   392,
     392,   392,   392,   392,   392,   125,   392,   392,   392,   130,
     392,   392,   392,   392,   392,   392,   392,   392,   392,   392,
     408,   392,   392,   408,   126,   392,   392,   392,   132,   392,
     392,   392,   392,   392,   392,   392,   392,   248,   392,   392,
     408,   392,   392,   392,   392,   392,   392,   393,   393,   124,
     135,   393,   393,   393,   393,   393,     0,     0,   392,   136,
     166,     0,     0,   392,   392,   392,     0,     0,     0,     0,
     408,     0,   400,     0,     0,   408,     0,     0,   154,     0,
       0,   410,     0,     0,     0,   170,     0,    26,     4,   274,
     157,   408,   171,   301,     0,     0,     0,     0,     0,     0,
     410,    88,   408,     0,     0,     0,     0,   408,     0,     0,
       0,     0,   278,   283,   381,     0,   192,     0,   400,   190,
     325,   408,   107,     0,   381,   306,     0,     0,     0,   267,
     268,     0,     0,   191,     0,     0,     0,     0,   321,   322,
     249,    14,   250,   251,   252,   253,   254,     1,     2,   392,
     404,   406,   123,   402,   388,   389,   385,   386,   387,     0,
       0,     0,   234,   384,   383,   390,   244,   367,   366,   403,
       0,   243,   409,     0,   230,   232,   233,   362,     0,   409,
       0,     0,     0,     0,   404,   396,   397,     0,   255,   184,
     271,     3,   401,   147,   143,   144,   269,   189,    34,     0,
      33,    40,    27,    28,     0,    30,    38,     0,   293,     0,
      45,    50,   355,   356,   357,   358,   359,   404,    42,     0,
      46,    49,    51,    48,   256,     0,    70,    66,    67,     0,
       0,     0,   161,   155,   404,     0,   276,   272,   404,     0,
       0,   186,    17,     0,    84,    80,    81,    85,   398,    86,
      87,     0,   162,   163,    89,     0,   187,   101,     0,   277,
       0,   188,   282,   279,   303,   299,   404,     0,     0,     0,
     245,   382,   286,   284,   410,   304,    15,     0,     0,     0,
       0,   353,   246,   326,   329,   354,   333,   332,   331,   185,
     112,   108,   109,   111,     0,   247,   305,    17,   293,   295,
     296,   297,   308,   309,   311,   307,     0,     0,     0,     0,
     405,     0,     0,   205,     0,   200,   202,   203,   204,     0,
     198,     0,     0,   230,   241,   240,   325,   365,   173,     0,
       0,     0,     0,     0,     0,   174,     0,   363,     0,   175,
     409,   230,   409,   409,     0,     0,     0,     0,    36,    37,
       0,     0,    31,     0,     0,     0,   394,   410,   292,     0,
       0,     0,     0,   410,   263,   260,   265,   262,     0,   266,
     411,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     5,     0,   360,     0,     0,     0,     6,   410,    93,
     411,     0,     0,     0,     0,     0,     0,   106,     0,   103,
       0,     0,   287,     0,   329,   340,   339,   352,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    16,   291,   410,     0,
     313,     0,     0,     0,   316,     0,     0,   117,   113,   114,
     121,     0,   407,   407,     0,     0,   193,   207,   210,   208,
     209,   201,   206,     0,   194,     0,   242,   236,   235,   239,
     238,   237,   378,   369,   371,   379,     0,   364,   194,     0,
     215,   409,   211,   213,   176,   178,     0,   225,   409,   218,
     217,     0,   329,   331,   180,     0,   182,     0,     0,   141,
     410,   145,     0,   153,     0,   148,   150,    35,    29,    32,
      75,    77,    79,    76,     0,    71,    73,    78,     0,   404,
       0,     0,    43,    47,   386,     0,     0,     0,    57,     0,
      53,     0,    56,    60,    63,    64,    65,     0,     0,     0,
       0,   257,    68,     0,     0,   410,   156,     0,   273,     0,
       0,    18,    23,    24,     0,    20,    22,     0,    83,    82,
     399,     8,    13,     0,    10,    12,     0,    90,     0,   142,
       0,     0,   323,   300,     0,     0,   102,     0,     0,     0,
     411,   330,     0,   341,   335,   334,   338,   337,   336,   351,
     342,   348,   345,   343,   346,   344,   347,   349,   350,   110,
     270,     0,   310,     0,   315,   312,     0,   318,   320,     7,
       0,     0,   136,     0,     0,   140,   137,   231,   199,     0,
       0,   325,     0,   377,   372,     0,   370,     0,     0,   177,
       0,     0,     0,   329,   181,     0,     0,     0,   409,     0,
       0,     0,   151,     0,   146,     0,    39,     0,   395,     0,
     411,   362,    61,    65,    62,     0,    44,     0,   411,     0,
       0,     0,    69,   159,     0,     0,     0,   169,     0,    19,
     361,     0,     9,   411,    99,    98,   100,     0,    94,    96,
     168,   167,     0,   281,     0,   104,   105,     0,     0,     0,
     290,   285,   329,   411,   314,   317,   319,   115,     0,   118,
     407,   407,   196,     0,   195,     0,   376,     0,   373,   380,
       0,   212,   179,     0,     0,     0,   224,   329,   222,   329,
     220,   329,   183,     0,   164,   411,   149,    72,    74,    78,
     165,     0,     0,    54,    55,   258,   264,   261,   259,   411,
     327,   327,    21,    11,    91,     0,    92,     0,   324,   327,
     280,   289,   288,   391,     0,   122,     0,   120,   116,   139,
     138,     0,   368,   375,     0,   362,   362,   362,   329,   362,
       0,     0,   294,     0,   158,     0,   160,   275,    95,    97,
     302,   298,     0,   197,   374,     0,     0,     0,     0,     0,
       0,     0,   328,   119,     0,     0,     0,     0,   172,     0,
     327,     0,     0,     0,     0,   152,     0,   327,   327,   327,
     327,    52,     0,     0,     0,     0,   214,   229,   228,   227
};

  /* YY1PGOTO[NTERM-NUM].  */
static const yy1type_int16 yy1pgoto[] =
{
    -603,  -603,  -603,   214,  -603,    -9,   365,  -603,    10,   545,
     321,  -158,  -603,  -218,  -603,   314,   467,  -380,  -603,   -69,
      33,  -603,  -518,  -603,  -115,  -399,    44,    45,  -603,   304,
    -603,   133,  -603,   -44,   -40,  -603,   136,  -603,   274,  -603,
     103,  -603,  -603,   745,  -603,   105,  -382,   -76,  -603,    76,
     336,  -603,  -603,  -603,  -603,  -603,   -80,  -484,   270,  -603,
    -603,  -495,  -602,   227,   240,  -506,   120,   -72,  -603,   201,
     353,    15,  -603,   447,   340,   308,  -603,   145,  -603,  -603,
    -603,  -360,  -412,    69,  -312,  -117,  -603,   595,   407,   317,
     137,  -603,   627,   -99,  -342,   995,   513,   265,  -108,  -132,
     640,   -89,  -216,  -603,  -463,   472,  -202,  -121,  -393
};

  /* YY1DEFGOTO[NTERM-NUM].  */
static const yy1type_int16 yy1defgoto[] =
{
      -1,    96,    97,   427,   593,   594,   421,   584,   585,   242,
     243,   244,   390,   245,   257,   258,   259,   260,   261,   559,
     560,   561,   562,   267,   342,   544,   545,   546,   285,   286,
     294,   430,   707,   708,   709,   438,   439,   331,   332,   478,
     479,   788,   642,   431,    99,   192,   100,   234,   534,   535,
     273,   101,   102,   103,   104,   105,   212,   359,   360,   361,
     491,   511,   512,   518,   519,   520,   213,   363,   264,   410,
     277,   313,   610,   398,   305,   343,   344,   470,   471,   473,
     601,   322,   806,   521,   324,   325,   422,   326,   367,   377,
     505,   506,   310,   327,   328,   106,   127,   395,   396,   287,
     231,   217,   351,   352,   645,   156,   368,   265,   571
};

  /* YY1TABLE[YY1PACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YY1TABLE_NINF, syntax error.  */
static const yy1type_int16 yy1table[] =
{
     227,   228,   216,   262,   330,   444,   495,   268,   385,   649,
     662,   297,   375,   573,   657,   274,   278,   379,   224,   295,
     646,   553,   219,   232,   235,   315,   392,   246,   193,   214,
     235,   292,   193,   269,   275,   279,   -41,   597,   682,   684,
     523,   400,   306,   248,   289,   282,   193,   288,   718,   291,
     293,   193,   193,   225,   288,   311,   741,   336,   415,  -409,
     565,   307,   417,   193,   704,   311,   786,   187,   288,   232,
     522,   188,   193,   333,   283,   401,     6,   337,   235,   246,
      10,   404,   762,   376,   402,   345,    16,   393,   189,   719,
     435,   338,   191,   270,   388,    24,   193,   193,   193,   225,
     216,   216,   216,   787,   193,   271,   350,   226,   218,    36,
      37,   581,   284,   582,   405,   193,   406,   290,   193,   216,
      45,   216,   407,   216,    49,   408,   249,   815,   376,  -409,
     207,   223,   262,   209,   247,   437,   208,   209,   741,   272,
     380,   382,   193,    64,   207,   583,   193,    68,   381,   210,
     208,   209,   225,   226,   312,   246,   209,   209,   582,   746,
     748,   750,   251,   210,   193,   742,   720,   706,   209,   764,
     207,   553,   249,   193,   523,   539,   208,   209,   514,   515,
     524,   526,   252,   523,   270,   253,   254,   255,   256,   210,
     583,   592,   276,   442,   233,   409,   271,   266,   704,   389,
     509,   209,   209,   209,   663,   280,   226,   250,   251,   209,
     193,  -216,   225,   522,   565,   565,   207,   721,   711,   440,
     209,   429,   208,   209,   193,   816,   376,   302,   252,   323,
     272,   253,   254,   255,   256,   210,   510,   746,   748,   750,
     300,  -226,   728,   444,   207,    89,    90,   209,  -223,   791,
     208,   209,   304,   483,   303,   216,   726,   472,   474,   241,
     476,   308,   480,   210,  -221,  -219,   226,   789,   790,   209,
     216,   216,   216,   216,   216,   216,   550,   685,   209,   193,
     193,   667,   567,   262,   262,   563,   686,   761,   193,   309,
     685,   735,   384,   350,   705,   765,   572,   529,   276,   823,
     722,   706,   246,   564,   246,   346,   330,   596,   193,   659,
     774,  -226,   304,   566,  -409,   209,   664,   193,  -223,   362,
     364,   365,   269,   523,   523,   523,   275,   225,   279,   209,
     784,   586,   347,   679,  -221,  -219,   590,   595,   378,   591,
     414,   348,   383,   565,   238,   565,   307,   631,   353,   419,
     485,   366,   239,   747,   749,   751,   193,   376,   486,   807,
     238,   194,   801,   195,   196,   197,   198,   810,   239,   433,
     477,   555,   556,   592,   557,   333,   804,   817,   376,   387,
     345,   226,   203,   386,   209,   209,   443,   445,   446,   447,
     548,   204,   587,   209,   216,   818,   376,   391,   549,   605,
     588,   523,   523,   523,   205,   355,   633,   606,   238,   671,
     397,   394,   636,   209,   634,   655,   239,   475,   836,   669,
     637,   673,   209,   656,   672,   842,   843,   844,   845,   674,
     492,   747,   749,   798,   262,   323,   689,   690,   403,   411,
     678,   240,   241,   675,   565,   504,   356,   357,   358,   412,
     413,   676,   675,   416,   694,   418,   536,   564,   564,   420,
     692,   209,   681,   547,   698,   701,   752,   683,   683,   712,
     485,   691,   699,   702,   484,   548,   425,   713,   732,   288,
     288,   547,   574,   754,   766,   577,   424,   579,   425,   496,
     497,   498,   499,   500,   501,   425,   767,   426,   775,   428,
     675,   599,   464,   602,   369,   604,   776,   370,   785,   371,
     372,   647,   373,   374,   432,   436,   440,   613,   614,   615,
     616,   617,   618,   619,   620,   621,   622,   623,   624,   625,
     626,   627,   628,   434,   630,   159,   194,   485,   195,   196,
     197,   198,   469,   465,   472,   813,   481,   725,  -219,   468,
     644,   480,   173,   372,   528,   551,   783,   203,   537,   194,
     568,   195,   196,   197,   198,   569,   204,   370,   563,   371,
     372,   653,   373,   374,   193,   353,   482,   609,   570,   205,
     203,   372,   229,   373,   374,   443,   564,   237,   564,   204,
     451,   598,   452,   453,   607,   608,   566,   612,   683,   640,
     180,   181,   205,   281,   182,   183,   184,   185,   186,   586,
     451,   641,   595,   650,   296,   651,   680,   658,   194,   301,
     195,   196,   197,   198,   660,   -58,   665,   666,   354,   200,
     687,   201,   355,   329,   723,   202,   697,   525,   369,   203,
     -59,   370,   730,   371,   372,   800,   373,   374,   204,   740,
     743,   795,   744,   745,   796,   797,   799,   369,   753,   755,
     370,   205,   371,   372,   769,   373,   374,   768,   206,   563,
     777,   783,   207,   356,   357,   358,   716,   717,   208,   209,
    -231,   792,   819,  -231,   820,  -231,  -231,   564,  -231,  -231,
     639,   210,   773,   841,   846,   211,   215,   566,   449,   847,
     450,   451,   466,   452,   453,   848,   230,   849,   772,   236,
     547,   828,   538,   339,   552,   263,   399,   812,   763,   757,
     323,   736,   758,   738,   504,   193,   493,   369,   589,   703,
     370,   808,   371,   372,   494,   373,   374,   809,   629,   298,
     299,   715,   536,   727,   547,    98,   759,   729,   760,   756,
     314,   576,   668,   493,   369,   648,   661,   370,   334,   371,
     372,   508,   373,   374,   770,   771,   340,   341,   688,   194,
     578,   195,   196,   197,   198,   603,   802,   632,   724,   317,
     318,   778,   319,   779,   507,   467,   487,   781,   782,   635,
     203,   335,   739,   670,   215,   215,   215,   320,   316,   204,
       0,   193,     0,     0,     0,     0,   793,     0,     0,     0,
       0,     0,   205,   215,     0,   215,     0,   215,   321,   488,
       0,     0,     0,   207,     0,     0,     0,   489,     0,   208,
     209,   803,     0,     0,     0,   252,     0,     0,   253,   254,
     255,   256,   210,     0,   263,   194,   490,   195,   196,   197,
     198,     0,     0,   811,     0,   199,   200,     0,   201,     0,
       0,     0,   202,   814,     0,     0,   203,     0,     0,     0,
       0,     0,     0,     0,   822,   204,     0,     0,   423,     0,
       0,     0,     0,     0,   824,   825,   826,   827,   205,   829,
     830,     0,     0,     0,     0,   206,     0,     0,     0,   207,
     837,   838,   839,   840,   441,   208,   209,   733,   369,     0,
       0,   370,     0,   371,   372,   734,   373,   374,   210,   654,
       0,   448,   211,     0,   449,     0,   450,   451,     0,   452,
     453,   454,   455,   456,     0,   457,   458,   459,   460,   461,
     462,     0,   463,     0,     0,   448,     0,     0,   449,   215,
     450,   451,     0,   452,   453,   454,   455,     0,     0,   457,
     458,   459,   460,   461,   215,   215,   215,   215,   215,   215,
       0,   677,     0,   448,     0,   513,   449,     0,   450,   451,
       0,   452,   453,   454,   455,   456,     0,   457,   458,   459,
     460,   461,   462,     0,   463,   263,   263,   107,   108,   109,
     110,     0,   111,   112,   113,     0,   114,   115,     0,   116,
     117,     0,   118,   119,   120,   121,   122,   123,   124,     0,
     125,   126,     0,   128,   129,   130,   131,   132,   133,   134,
     135,     0,     0,   136,   137,   138,   139,   140,   141,   142,
       0,   143,   144,   145,     0,   146,   147,   148,   149,   150,
     151,   152,   153,   154,   155,     0,   157,   158,     0,     0,
     160,   161,   162,     0,   163,   164,   165,   166,   167,   168,
     169,   170,     0,   171,   172,   193,   174,   175,   176,   177,
     178,   179,     0,     0,     0,     0,     0,     0,   215,     0,
       0,     0,     0,   190,     0,     0,     0,     0,   220,   221,
     222,     0,     0,     0,   513,     0,     0,     0,     0,     0,
       0,   193,     0,     0,     0,     0,     0,     0,     0,   194,
       0,   195,   196,   197,   198,     0,     0,     0,     0,   354,
     200,     0,   201,     0,     0,     0,   202,     0,     0,     0,
     203,     0,     0,     0,     0,     0,   263,     0,     0,   204,
       0,     0,     0,     0,     0,   194,     0,   195,   196,   197,
     198,   540,   205,     0,     0,   317,   318,     0,   319,   206,
       0,     0,   541,   207,     0,     0,   203,     0,     0,   208,
     209,     0,   700,   320,   349,   204,     0,     0,     0,     0,
       0,     0,   210,     0,   542,     0,   211,     0,   205,     0,
       0,     0,   543,     0,   321,     0,     0,     0,     0,   207,
       0,     0,     0,     0,     0,   208,   209,     0,     0,     0,
       0,   252,     0,     0,   253,   254,   255,   256,   210,     0,
       1,     2,     3,     4,     5,     6,     7,     8,     9,    10,
      11,    12,    13,    14,    15,    16,    17,    18,    19,    20,
      21,    22,    23,   513,    24,   513,    25,    26,    27,    28,
      29,    30,    31,    32,    33,    34,     0,    35,    36,    37,
      38,    39,    40,    41,    42,    43,     0,     0,    44,    45,
      46,    47,    48,    49,    50,     0,     0,    51,    52,    53,
      54,     0,    55,    56,    57,    58,    59,     0,    60,    61,
      62,    63,    64,    65,    66,    67,    68,    69,    70,    71,
       0,    72,     0,    73,    74,     0,     0,     0,     0,    75,
      76,     0,    77,    78,     0,     0,    79,     0,    80,   737,
       0,   448,     0,     0,   449,   513,   450,   451,     0,   452,
     453,   454,   455,   456,     0,   457,   458,   459,   460,   461,
     462,     0,   463,     0,     0,     0,   314,     0,     0,     0,
      81,    82,    83,    84,    85,    86,     0,     0,     0,     1,
       2,     3,     0,     5,     0,     7,     0,     9,     0,    11,
      12,     0,     0,    15,     0,    17,    18,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    87,    88,     0,     0,
       0,    31,     0,     0,    89,    90,     0,    91,    92,    93,
      94,    95,    41,     0,     0,     0,     0,    44,     0,     0,
      47,    48,     0,     0,     0,     0,    51,     0,     0,     0,
       0,     0,     0,     0,    58,    59,     0,    60,     0,     0,
      63,     0,    65,    66,     0,     0,    69,    70,     0,     0,
       0,     0,    73,     0,   710,     0,     0,     0,    75,    76,
       0,   794,    78,   448,     0,    79,   449,    80,   450,   451,
       0,   452,   453,   454,   455,   456,     0,   457,   458,   459,
     460,   461,   462,   448,   463,     0,   449,     0,   450,   451,
       0,   452,   453,     0,   455,     0,     0,   457,   458,   459,
     460,   461,    84,    85,    86,     0,     0,     0,     1,     2,
       3,     0,     5,     0,     7,     0,     9,     0,    11,    12,
       0,     0,    15,     0,    17,    18,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    87,     0,     0,     0,     0,
      31,     0,     0,     0,     0,     0,    91,    92,    93,    94,
      95,    41,     0,     0,     0,     0,    44,     0,     0,    47,
      48,     0,     0,     0,     0,    51,   193,     0,     0,     0,
       0,     0,     0,    58,    59,     0,    60,     0,     0,    63,
       0,    65,    66,     0,     0,    69,    70,     0,     0,     0,
       0,    73,     0,     0,     0,     0,     0,    75,    76,     0,
       0,    78,     0,     0,    79,     0,    80,     0,     0,     0,
     194,     0,   195,   196,   197,   198,     0,     0,     0,     0,
     317,   318,   530,   319,     0,   531,     0,   532,     0,     0,
       0,   203,     0,     0,     0,     0,     0,     0,   320,     0,
     204,    84,    85,    86,     0,     0,   193,   533,     0,     0,
       0,     0,     0,   205,     0,     0,     0,     0,     0,   321,
       0,     0,     0,     0,   207,     0,     0,     0,     0,     0,
     208,   209,     0,     0,    87,     0,   252,     0,     0,   253,
     254,   255,   256,   210,     0,    91,    92,    93,    94,    95,
     194,     0,   195,   196,   197,   198,     0,     0,     0,     0,
     317,   318,   530,   319,     0,     0,     0,   532,     0,     0,
       0,   203,     0,     0,     0,     0,     0,     0,   320,     0,
     204,   193,     0,     0,     0,     0,     0,   533,     0,     0,
       0,     0,     0,   205,     0,   193,     0,     0,     0,   321,
       0,     0,     0,     0,   207,     0,     0,     0,     0,     0,
     208,   209,     0,     0,     0,     0,   252,     0,     0,   253,
     254,   255,   256,   210,     0,   194,     0,   195,   196,   197,
     198,   502,     0,     0,     0,   317,   318,     0,   319,   194,
     503,   195,   196,   197,   198,     0,   203,     0,     0,   317,
     318,     0,   319,   320,     0,   204,   541,     0,     0,     0,
     203,     0,     0,     0,     0,     0,     0,   320,   205,   204,
     193,     0,     0,     0,   321,     0,     0,     0,   542,   207,
       0,     0,   205,     0,   193,   208,   209,     0,   321,     0,
       0,   252,     0,   207,   253,   254,   255,   256,   210,   208,
     209,     0,     0,     0,     0,   252,     0,     0,   253,   254,
     255,   256,   210,     0,   194,     0,   195,   196,   197,   198,
       0,     0,     0,     0,   516,   318,     0,   319,   194,     0,
     195,   196,   197,   198,     0,   203,     0,     0,   317,   318,
       0,   319,   320,     0,   204,   643,     0,     0,     0,   203,
       0,     0,     0,     0,     0,     0,   320,   205,   204,   193,
     517,     0,     0,   321,     0,     0,     0,     0,   207,     0,
       0,   205,     0,   193,   208,   209,     0,   321,     0,     0,
     252,     0,   207,   253,   254,   255,   256,   210,   208,   209,
       0,     0,     0,     0,   252,     0,     0,   253,   254,   255,
     256,   210,     0,   194,     0,   195,   196,   197,   198,   652,
       0,     0,     0,   317,   318,     0,   319,   194,     0,   195,
     196,   197,   198,   502,   203,     0,     0,   317,   318,     0,
     319,   320,     0,   204,     0,     0,     0,     0,   203,     0,
       0,     0,     0,     0,     0,   320,   205,   204,   193,     0,
       0,     0,   321,     0,     0,     0,     0,   207,     0,     0,
     205,     0,   193,   208,   209,     0,   321,     0,     0,   252,
       0,   207,   253,   254,   255,   256,   210,   208,   209,     0,
       0,     0,     0,   252,     0,     0,   253,   254,   255,   256,
     210,     0,   194,     0,   195,   196,   197,   198,     0,     0,
       0,     0,   317,   318,     0,   319,   194,     0,   195,   196,
     197,   198,     0,   203,     0,     0,   516,   318,     0,   319,
     320,     0,   204,     0,     0,     0,     0,   203,     0,     0,
       0,     0,     0,     0,   320,   205,   204,   193,     0,     0,
       0,   321,     0,     0,     0,     0,   207,     0,     0,   205,
       0,     0,   208,   209,     0,   321,     0,     0,   252,     0,
     207,   253,   254,   255,   256,   210,   208,   209,     0,     0,
       0,     0,   252,     0,     0,   253,   254,   255,   256,   210,
       0,   194,     0,   195,   196,   554,   198,     0,     0,     0,
       0,   555,   556,     0,   557,     0,     0,   695,   448,     0,
       0,   449,   203,   450,   451,     0,   452,   453,   454,   455,
     456,   204,   457,   458,   459,   460,   461,   462,     0,   463,
       0,     0,   448,     0,   205,   449,     0,   450,   451,     0,
     452,   453,   454,   455,   558,     0,   457,   458,   459,   460,
     461,     0,   209,   463,     0,     0,     0,   252,     0,     0,
     253,   254,   255,   256,   696,   448,     0,     0,   449,     0,
     450,   451,     0,   452,   453,   454,   455,   456,     0,   457,
     458,   459,   460,   461,   462,     0,   463,   714,   448,     0,
       0,   449,     0,   450,   451,     0,   452,   453,   454,   455,
     456,     0,   457,   458,   459,   460,   461,   462,     0,   463,
     805,   448,     0,     0,   449,     0,   450,   451,     0,   452,
     453,   454,   455,   456,     0,   457,   458,   459,   460,   461,
     462,     0,   463,   821,   448,     0,     0,   449,     0,   450,
     451,     0,   452,   453,   454,   455,   456,     0,   457,   458,
     459,   460,   461,   462,     0,   463,   831,   448,     0,     0,
     449,     0,   450,   451,     0,   452,   453,   454,   455,   456,
       0,   457,   458,   459,   460,   461,   462,     0,   463,   832,
     448,     0,     0,   449,     0,   450,   451,     0,   452,   453,
     454,   455,   456,     0,   457,   458,   459,   460,   461,   462,
       0,   463,   833,   448,     0,     0,   449,     0,   450,   451,
       0,   452,   453,   454,   455,   456,     0,   457,   458,   459,
     460,   461,   462,     0,   463,   834,   448,     0,     0,   449,
       0,   450,   451,     0,   452,   453,   454,   455,   456,     0,
     457,   458,   459,   460,   461,   462,   448,   463,     0,   449,
       0,   450,   451,   527,   452,   453,   454,   455,   456,     0,
     457,   458,   459,   460,   461,   462,   448,   463,     0,   449,
       0,   450,   451,   575,   452,   453,   454,   455,   456,     0,
     457,   458,   459,   460,   461,   462,   448,   463,     0,   449,
       0,   450,   451,   580,   452,   453,   454,   455,   456,     0,
     457,   458,   459,   460,   461,   462,   448,   463,     0,   449,
       0,   450,   451,   600,   452,   453,   454,   455,   456,     0,
     457,   458,   459,   460,   461,   462,   448,   463,     0,   449,
       0,   450,   451,   611,   452,   453,   454,   455,   456,     0,
     457,   458,   459,   460,   461,   462,   448,   463,     0,   449,
       0,   450,   451,   638,   452,   453,   454,   455,   456,     0,
     457,   458,   459,   460,   461,   462,   448,   463,     0,   449,
       0,   450,   451,   693,   452,   453,   454,   455,   456,     0,
     457,   458,   459,   460,   461,   462,   448,   463,     0,   449,
       0,   450,   451,   731,   452,   453,   454,   455,   456,     0,
     457,   458,   459,   460,   461,   462,   448,   463,     0,   449,
       0,   450,   451,   780,   452,   453,   454,   455,   456,     0,
     457,   458,   459,   460,   461,   462,   448,   463,     0,   449,
       0,   450,   451,   835,   452,   453,   454,   455,   456,     0,
     457,   458,   459,   460,   461,   462,   448,   463,     0,   449,
       0,   450,   451,     0,   452,   453,   454,   455,   456,     0,
     457,   458,   459,   460,   461,   462,   448,   463,     0,   449,
       0,   450,   451,     0,   452,   453,     0,  -410,     0,     0,
    -410,  -410,  -410,  -410,  -410
};

static const yy1type_int16 yy1check[] =
{
     108,   109,   101,   120,   162,   317,   366,   122,   224,   493,
     516,   143,   214,   412,   509,   123,   124,   219,   107,   140,
     483,   401,   102,   112,   113,   157,   244,   116,    58,   101,
     119,   139,    58,   122,   123,   124,    39,   430,   556,   557,
     382,   257,   150,   119,   103,   134,    58,   136,    49,   138,
     139,    58,    58,   106,   143,   154,   658,   165,   274,    39,
     402,   150,   278,    58,    58,   164,   118,     0,   157,   158,
     382,    39,    58,   162,   112,   109,     8,   166,   167,   168,
      12,    50,   111,   112,   118,   174,    18,   245,    45,    90,
     306,   167,   119,   100,    58,    27,    58,    58,    58,   106,
     199,   200,   201,   155,    58,   112,   109,   160,   112,    41,
      42,   117,   150,   119,    83,    58,    85,   176,    58,   218,
      52,   220,    91,   222,    56,    94,   112,   111,   112,   109,
     156,   112,   249,   163,   112,   165,   162,   163,   740,   146,
     220,   221,    58,    75,   156,   151,    58,    79,   220,   175,
     162,   163,   106,   160,   166,   244,   163,   163,   119,   665,
     666,   667,   148,   175,    58,   660,   167,   161,   163,   687,
     156,   551,   112,    58,   516,   393,   162,   163,   380,   381,
     382,   383,   168,   525,   100,   171,   172,   173,   174,   175,
     151,   151,   146,   314,   156,   164,   112,   140,    58,   163,
     112,   163,   163,   163,   516,   112,   160,   147,   148,   163,
      58,    39,   106,   525,   556,   557,   156,   610,   600,   308,
     163,   153,   162,   163,    58,   111,   112,   108,   168,   160,
     146,   171,   172,   173,   174,   175,   148,   743,   744,   745,
     112,    39,   641,   555,   156,   177,   178,   163,    39,   733,
     162,   163,   146,   352,   108,   354,   638,   346,   347,   144,
     349,   112,   351,   175,    39,    39,   160,   730,   731,   163,
     369,   370,   371,   372,   373,   374,   397,   109,   163,    58,
      58,   109,   403,   400,   401,   402,   118,   680,    58,   112,
     109,   651,   223,   109,   154,   688,   411,   386,   146,   118,
     612,   161,   391,   402,   393,   112,   464,   428,    58,   511,
     703,   109,   146,   402,    39,   163,   518,    58,   109,   199,
     200,   201,   411,   665,   666,   667,   415,   106,   417,   163,
     723,   420,   112,   549,   109,   109,   425,   426,   218,   117,
     271,   112,   222,   685,   110,   687,   435,   468,    59,   280,
     109,   112,   118,   665,   666,   667,    58,   112,   117,   771,
     110,   102,   755,   104,   105,   106,   107,   779,   118,   300,
     140,   112,   113,   151,   115,   464,   769,   111,   112,   112,
     469,   160,   123,    90,   163,   163,   317,   318,   319,   320,
     109,   132,   109,   163,   493,   111,   112,   109,   117,   109,
     117,   743,   744,   745,   145,   116,   109,   117,   110,   530,
     109,   112,   109,   163,   117,   109,   118,   348,   830,   527,
     117,   109,   163,   117,   532,   837,   838,   839,   840,   117,
     361,   743,   744,   745,   551,   366,   568,   569,   109,   109,
     548,   143,   144,   109,   786,   376,   157,   158,   159,   112,
     112,   117,   109,   111,   575,   111,   387,   556,   557,   112,
     117,   163,   551,   394,   109,   109,   668,   556,   557,   109,
     109,   570,   117,   117,   354,   109,   109,   117,   117,   568,
     569,   412,   413,   117,   117,   416,   109,   418,   109,   369,
     370,   371,   372,   373,   374,   109,   117,   112,   109,   109,
     109,   432,   109,   434,   110,   436,   117,   113,   117,   115,
     116,   117,   118,   119,   111,   111,   605,   448,   449,   450,
     451,   452,   453,   454,   455,   456,   457,   458,   459,   460,
     461,   462,   463,   112,   465,    63,   102,   109,   104,   105,
     106,   107,   109,   108,   633,   117,   112,   636,   109,   108,
     481,   640,    80,   116,   112,   109,   117,   123,   118,   102,
     112,   104,   105,   106,   107,   112,   132,   113,   685,   115,
     116,   502,   118,   119,    58,    59,   142,   108,   112,   145,
     123,   116,   110,   118,   119,   516,   685,   115,   687,   132,
     116,   112,   118,   119,   111,   111,   685,   109,   687,   109,
      87,    88,   145,   131,    91,    92,    93,    94,    95,   698,
     116,   112,   701,   493,   142,   108,    70,   109,   102,   147,
     104,   105,   106,   107,   109,   119,   109,   109,   112,   113,
     119,   115,   116,   161,    97,   119,    88,   109,   110,   123,
     119,   113,   117,   115,   116,   753,   118,   119,   132,   109,
     109,   740,   109,   109,   743,   744,   745,   110,   109,    98,
     113,   145,   115,   116,    89,   118,   119,   117,   152,   786,
     113,   117,   156,   157,   158,   159,   607,   608,   162,   163,
     110,   117,   109,   113,   112,   115,   116,   786,   118,   119,
     476,   175,   701,   117,   117,   179,   101,   786,   113,   117,
     115,   116,   337,   118,   119,   117,   111,   117,   698,   114,
     641,   819,   391,   168,   400,   120,   249,   786,   685,   675,
     651,   652,   677,   654,   655,    58,   109,   110,   424,   596,
     113,   775,   115,   116,   117,   118,   119,   777,   464,   144,
     145,   605,   673,   640,   675,     0,   677,   642,   679,   673,
     155,   415,   525,   109,   110,   485,   516,   113,   163,   115,
     116,   117,   118,   119,   695,   696,   171,   172,   567,   102,
     417,   104,   105,   106,   107,   435,   761,   469,   633,   112,
     113,   712,   115,   714,   377,   338,   119,   718,   719,   472,
     123,   164,   655,   528,   199,   200,   201,   130,   158,   132,
      -1,    58,    -1,    -1,    -1,    -1,   737,    -1,    -1,    -1,
      -1,    -1,   145,   218,    -1,   220,    -1,   222,   151,   152,
      -1,    -1,    -1,   156,    -1,    -1,    -1,   160,    -1,   162,
     163,   762,    -1,    -1,    -1,   168,    -1,    -1,   171,   172,
     173,   174,   175,    -1,   249,   102,   179,   104,   105,   106,
     107,    -1,    -1,   784,    -1,   112,   113,    -1,   115,    -1,
      -1,    -1,   119,   794,    -1,    -1,   123,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,   805,   132,    -1,    -1,   283,    -1,
      -1,    -1,    -1,    -1,   815,   816,   817,   818,   145,   820,
     821,    -1,    -1,    -1,    -1,   152,    -1,    -1,    -1,   156,
     831,   832,   833,   834,   309,   162,   163,   109,   110,    -1,
      -1,   113,    -1,   115,   116,   117,   118,   119,   175,   108,
      -1,   110,   179,    -1,   113,    -1,   115,   116,    -1,   118,
     119,   120,   121,   122,    -1,   124,   125,   126,   127,   128,
     129,    -1,   131,    -1,    -1,   110,    -1,    -1,   113,   354,
     115,   116,    -1,   118,   119,   120,   121,    -1,    -1,   124,
     125,   126,   127,   128,   369,   370,   371,   372,   373,   374,
      -1,   108,    -1,   110,    -1,   380,   113,    -1,   115,   116,
      -1,   118,   119,   120,   121,   122,    -1,   124,   125,   126,
     127,   128,   129,    -1,   131,   400,   401,     2,     3,     4,
       5,    -1,     7,     8,     9,    -1,    11,    12,    -1,    14,
      15,    -1,    17,    18,    19,    20,    21,    22,    23,    -1,
      25,    26,    -1,    28,    29,    30,    31,    32,    33,    34,
      35,    -1,    -1,    38,    39,    40,    41,    42,    43,    44,
      -1,    46,    47,    48,    -1,    50,    51,    52,    53,    54,
      55,    56,    57,    58,    59,    -1,    61,    62,    -1,    -1,
      65,    66,    67,    -1,    69,    70,    71,    72,    73,    74,
      75,    76,    -1,    78,    79,    58,    81,    82,    83,    84,
      85,    86,    -1,    -1,    -1,    -1,    -1,    -1,   493,    -1,
      -1,    -1,    -1,    98,    -1,    -1,    -1,    -1,   103,   104,
     105,    -1,    -1,    -1,   509,    -1,    -1,    -1,    -1,    -1,
      -1,    58,    -1,    -1,    -1,    -1,    -1,    -1,    -1,   102,
      -1,   104,   105,   106,   107,    -1,    -1,    -1,    -1,   112,
     113,    -1,   115,    -1,    -1,    -1,   119,    -1,    -1,    -1,
     123,    -1,    -1,    -1,    -1,    -1,   551,    -1,    -1,   132,
      -1,    -1,    -1,    -1,    -1,   102,    -1,   104,   105,   106,
     107,   108,   145,    -1,    -1,   112,   113,    -1,   115,   152,
      -1,    -1,   119,   156,    -1,    -1,   123,    -1,    -1,   162,
     163,    -1,   587,   130,   189,   132,    -1,    -1,    -1,    -1,
      -1,    -1,   175,    -1,   141,    -1,   179,    -1,   145,    -1,
      -1,    -1,   149,    -1,   151,    -1,    -1,    -1,    -1,   156,
      -1,    -1,    -1,    -1,    -1,   162,   163,    -1,    -1,    -1,
      -1,   168,    -1,    -1,   171,   172,   173,   174,   175,    -1,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,    18,    19,    20,    21,    22,
      23,    24,    25,   658,    27,   660,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    -1,    40,    41,    42,
      43,    44,    45,    46,    47,    48,    -1,    -1,    51,    52,
      53,    54,    55,    56,    57,    -1,    -1,    60,    61,    62,
      63,    -1,    65,    66,    67,    68,    69,    -1,    71,    72,
      73,    74,    75,    76,    77,    78,    79,    80,    81,    82,
      -1,    84,    -1,    86,    87,    -1,    -1,    -1,    -1,    92,
      93,    -1,    95,    96,    -1,    -1,    99,    -1,   101,   108,
      -1,   110,    -1,    -1,   113,   740,   115,   116,    -1,   118,
     119,   120,   121,   122,    -1,   124,   125,   126,   127,   128,
     129,    -1,   131,    -1,    -1,    -1,   761,    -1,    -1,    -1,
     133,   134,   135,   136,   137,   138,    -1,    -1,    -1,     3,
       4,     5,    -1,     7,    -1,     9,    -1,    11,    -1,    13,
      14,    -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,   169,   170,    -1,    -1,
      -1,    35,    -1,    -1,   177,   178,    -1,   180,   181,   182,
     183,   184,    46,    -1,    -1,    -1,    -1,    51,    -1,    -1,
      54,    55,    -1,    -1,    -1,    -1,    60,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    68,    69,    -1,    71,    -1,    -1,
      74,    -1,    76,    77,    -1,    -1,    80,    81,    -1,    -1,
      -1,    -1,    86,    -1,    88,    -1,    -1,    -1,    92,    93,
      -1,   108,    96,   110,    -1,    99,   113,   101,   115,   116,
      -1,   118,   119,   120,   121,   122,    -1,   124,   125,   126,
     127,   128,   129,   110,   131,    -1,   113,    -1,   115,   116,
      -1,   118,   119,    -1,   121,    -1,    -1,   124,   125,   126,
     127,   128,   136,   137,   138,    -1,    -1,    -1,     3,     4,
       5,    -1,     7,    -1,     9,    -1,    11,    -1,    13,    14,
      -1,    -1,    17,    -1,    19,    20,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   169,    -1,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,   180,   181,   182,   183,
     184,    46,    -1,    -1,    -1,    -1,    51,    -1,    -1,    54,
      55,    -1,    -1,    -1,    -1,    60,    58,    -1,    -1,    -1,
      -1,    -1,    -1,    68,    69,    -1,    71,    -1,    -1,    74,
      -1,    76,    77,    -1,    -1,    80,    81,    -1,    -1,    -1,
      -1,    86,    -1,    -1,    -1,    -1,    -1,    92,    93,    -1,
      -1,    96,    -1,    -1,    99,    -1,   101,    -1,    -1,    -1,
     102,    -1,   104,   105,   106,   107,    -1,    -1,    -1,    -1,
     112,   113,   114,   115,    -1,   117,    -1,   119,    -1,    -1,
      -1,   123,    -1,    -1,    -1,    -1,    -1,    -1,   130,    -1,
     132,   136,   137,   138,    -1,    -1,    58,   139,    -1,    -1,
      -1,    -1,    -1,   145,    -1,    -1,    -1,    -1,    -1,   151,
      -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,    -1,
     162,   163,    -1,    -1,   169,    -1,   168,    -1,    -1,   171,
     172,   173,   174,   175,    -1,   180,   181,   182,   183,   184,
     102,    -1,   104,   105,   106,   107,    -1,    -1,    -1,    -1,
     112,   113,   114,   115,    -1,    -1,    -1,   119,    -1,    -1,
      -1,   123,    -1,    -1,    -1,    -1,    -1,    -1,   130,    -1,
     132,    58,    -1,    -1,    -1,    -1,    -1,   139,    -1,    -1,
      -1,    -1,    -1,   145,    -1,    58,    -1,    -1,    -1,   151,
      -1,    -1,    -1,    -1,   156,    -1,    -1,    -1,    -1,    -1,
     162,   163,    -1,    -1,    -1,    -1,   168,    -1,    -1,   171,
     172,   173,   174,   175,    -1,   102,    -1,   104,   105,   106,
     107,   108,    -1,    -1,    -1,   112,   113,    -1,   115,   102,
     117,   104,   105,   106,   107,    -1,   123,    -1,    -1,   112,
     113,    -1,   115,   130,    -1,   132,   119,    -1,    -1,    -1,
     123,    -1,    -1,    -1,    -1,    -1,    -1,   130,   145,   132,
      58,    -1,    -1,    -1,   151,    -1,    -1,    -1,   141,   156,
      -1,    -1,   145,    -1,    58,   162,   163,    -1,   151,    -1,
      -1,   168,    -1,   156,   171,   172,   173,   174,   175,   162,
     163,    -1,    -1,    -1,    -1,   168,    -1,    -1,   171,   172,
     173,   174,   175,    -1,   102,    -1,   104,   105,   106,   107,
      -1,    -1,    -1,    -1,   112,   113,    -1,   115,   102,    -1,
     104,   105,   106,   107,    -1,   123,    -1,    -1,   112,   113,
      -1,   115,   130,    -1,   132,   119,    -1,    -1,    -1,   123,
      -1,    -1,    -1,    -1,    -1,    -1,   130,   145,   132,    58,
     148,    -1,    -1,   151,    -1,    -1,    -1,    -1,   156,    -1,
      -1,   145,    -1,    58,   162,   163,    -1,   151,    -1,    -1,
     168,    -1,   156,   171,   172,   173,   174,   175,   162,   163,
      -1,    -1,    -1,    -1,   168,    -1,    -1,   171,   172,   173,
     174,   175,    -1,   102,    -1,   104,   105,   106,   107,   108,
      -1,    -1,    -1,   112,   113,    -1,   115,   102,    -1,   104,
     105,   106,   107,   108,   123,    -1,    -1,   112,   113,    -1,
     115,   130,    -1,   132,    -1,    -1,    -1,    -1,   123,    -1,
      -1,    -1,    -1,    -1,    -1,   130,   145,   132,    58,    -1,
      -1,    -1,   151,    -1,    -1,    -1,    -1,   156,    -1,    -1,
     145,    -1,    58,   162,   163,    -1,   151,    -1,    -1,   168,
      -1,   156,   171,   172,   173,   174,   175,   162,   163,    -1,
      -1,    -1,    -1,   168,    -1,    -1,   171,   172,   173,   174,
     175,    -1,   102,    -1,   104,   105,   106,   107,    -1,    -1,
      -1,    -1,   112,   113,    -1,   115,   102,    -1,   104,   105,
     106,   107,    -1,   123,    -1,    -1,   112,   113,    -1,   115,
     130,    -1,   132,    -1,    -1,    -1,    -1,   123,    -1,    -1,
      -1,    -1,    -1,    -1,   130,   145,   132,    58,    -1,    -1,
      -1,   151,    -1,    -1,    -1,    -1,   156,    -1,    -1,   145,
      -1,    -1,   162,   163,    -1,   151,    -1,    -1,   168,    -1,
     156,   171,   172,   173,   174,   175,   162,   163,    -1,    -1,
      -1,    -1,   168,    -1,    -1,   171,   172,   173,   174,   175,
      -1,   102,    -1,   104,   105,   106,   107,    -1,    -1,    -1,
      -1,   112,   113,    -1,   115,    -1,    -1,   109,   110,    -1,
      -1,   113,   123,   115,   116,    -1,   118,   119,   120,   121,
     122,   132,   124,   125,   126,   127,   128,   129,    -1,   131,
      -1,    -1,   110,    -1,   145,   113,    -1,   115,   116,    -1,
     118,   119,   120,   121,   155,    -1,   124,   125,   126,   127,
     128,    -1,   163,   131,    -1,    -1,    -1,   168,    -1,    -1,
     171,   172,   173,   174,   109,   110,    -1,    -1,   113,    -1,
     115,   116,    -1,   118,   119,   120,   121,   122,    -1,   124,
     125,   126,   127,   128,   129,    -1,   131,   109,   110,    -1,
      -1,   113,    -1,   115,   116,    -1,   118,   119,   120,   121,
     122,    -1,   124,   125,   126,   127,   128,   129,    -1,   131,
     109,   110,    -1,    -1,   113,    -1,   115,   116,    -1,   118,
     119,   120,   121,   122,    -1,   124,   125,   126,   127,   128,
     129,    -1,   131,   109,   110,    -1,    -1,   113,    -1,   115,
     116,    -1,   118,   119,   120,   121,   122,    -1,   124,   125,
     126,   127,   128,   129,    -1,   131,   109,   110,    -1,    -1,
     113,    -1,   115,   116,    -1,   118,   119,   120,   121,   122,
      -1,   124,   125,   126,   127,   128,   129,    -1,   131,   109,
     110,    -1,    -1,   113,    -1,   115,   116,    -1,   118,   119,
     120,   121,   122,    -1,   124,   125,   126,   127,   128,   129,
      -1,   131,   109,   110,    -1,    -1,   113,    -1,   115,   116,
      -1,   118,   119,   120,   121,   122,    -1,   124,   125,   126,
     127,   128,   129,    -1,   131,   109,   110,    -1,    -1,   113,
      -1,   115,   116,    -1,   118,   119,   120,   121,   122,    -1,
     124,   125,   126,   127,   128,   129,   110,   131,    -1,   113,
      -1,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
     124,   125,   126,   127,   128,   129,   110,   131,    -1,   113,
      -1,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
     124,   125,   126,   127,   128,   129,   110,   131,    -1,   113,
      -1,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
     124,   125,   126,   127,   128,   129,   110,   131,    -1,   113,
      -1,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
     124,   125,   126,   127,   128,   129,   110,   131,    -1,   113,
      -1,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
     124,   125,   126,   127,   128,   129,   110,   131,    -1,   113,
      -1,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
     124,   125,   126,   127,   128,   129,   110,   131,    -1,   113,
      -1,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
     124,   125,   126,   127,   128,   129,   110,   131,    -1,   113,
      -1,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
     124,   125,   126,   127,   128,   129,   110,   131,    -1,   113,
      -1,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
     124,   125,   126,   127,   128,   129,   110,   131,    -1,   113,
      -1,   115,   116,   117,   118,   119,   120,   121,   122,    -1,
     124,   125,   126,   127,   128,   129,   110,   131,    -1,   113,
      -1,   115,   116,    -1,   118,   119,   120,   121,   122,    -1,
     124,   125,   126,   127,   128,   129,   110,   131,    -1,   113,
      -1,   115,   116,    -1,   118,   119,    -1,   121,    -1,    -1,
     124,   125,   126,   127,   128
};

  /* YY1STOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yy1type_uint16 yy1stos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
      12,    13,    14,    15,    16,    17,    18,    19,    20,    21,
      22,    23,    24,    25,    27,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    51,    52,    53,    54,    55,    56,
      57,    60,    61,    62,    63,    65,    66,    67,    68,    69,
      71,    72,    73,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    84,    86,    87,    92,    93,    95,    96,    99,
     101,   133,   134,   135,   136,   137,   138,   169,   170,   177,
     178,   180,   181,   182,   183,   184,   186,   187,   228,   229,
     231,   236,   237,   238,   239,   240,   280,   280,   280,   280,
     280,   280,   280,   280,   280,   280,   280,   280,   280,   280,
     280,   280,   280,   280,   280,   280,   280,   281,   280,   280,
     280,   280,   280,   280,   280,   280,   280,   280,   280,   280,
     280,   280,   280,   280,   280,   280,   280,   280,   280,   280,
     280,   280,   280,   280,   280,   280,   290,   280,   280,   290,
     280,   280,   280,   280,   280,   280,   280,   280,   280,   280,
     280,   280,   280,   290,   280,   280,   280,   280,   280,   280,
     281,   281,   281,   281,   281,   281,   281,     0,    39,    45,
     280,   119,   230,    58,   102,   104,   105,   106,   107,   112,
     113,   115,   119,   123,   132,   145,   152,   156,   162,   163,
     175,   179,   241,   251,   252,   272,   278,   286,   112,   241,
     280,   280,   280,   112,   286,   106,   160,   283,   283,   290,
     272,   285,   286,   156,   232,   286,   272,   290,   110,   118,
     143,   144,   194,   195,   196,   198,   286,   112,   232,   112,
     147,   148,   168,   171,   172,   173,   174,   199,   200,   201,
     202,   203,   270,   272,   253,   292,   140,   208,   209,   286,
     100,   112,   146,   235,   283,   286,   146,   255,   283,   286,
     112,   290,   286,   112,   150,   213,   214,   284,   286,   103,
     176,   286,   283,   286,   215,   292,   290,   284,   272,   272,
     112,   290,   108,   108,   146,   259,   283,   286,   112,   112,
     277,   278,   166,   256,   272,   284,   285,   112,   113,   115,
     130,   151,   266,   268,   269,   270,   272,   278,   279,   290,
     196,   222,   223,   286,   272,   277,   283,   286,   232,   194,
     272,   272,   209,   260,   261,   286,   112,   112,   112,   280,
     109,   287,   288,    59,   112,   116,   157,   158,   159,   242,
     243,   244,   251,   252,   251,   251,   112,   273,   291,   110,
     113,   115,   116,   118,   119,   291,   112,   274,   251,   291,
     241,   252,   241,   251,   268,   287,    90,   112,    58,   163,
     197,   109,   198,   196,   112,   282,   283,   109,   258,   201,
     287,   109,   118,   109,    50,    83,    85,    91,    94,   164,
     254,   109,   112,   112,   268,   287,   111,   287,   111,   268,
     112,   191,   271,   272,   109,   109,   112,   188,   109,   153,
     216,   228,   111,   268,   112,   287,   111,   165,   220,   221,
     286,   272,   292,   268,   269,   268,   268,   268,   110,   113,
     115,   116,   118,   119,   120,   121,   122,   124,   125,   126,
     127,   128,   129,   131,   109,   108,   191,   258,   108,   109,
     262,   263,   286,   264,   286,   268,   286,   140,   224,   225,
     286,   112,   142,   278,   251,   109,   117,   119,   152,   160,
     179,   245,   268,   109,   117,   266,   251,   251,   251,   251,
     251,   251,   108,   117,   268,   275,   276,   273,   117,   112,
     148,   246,   247,   272,   291,   291,   112,   148,   248,   249,
     250,   268,   269,   279,   291,   109,   291,   117,   112,   286,
     114,   117,   119,   139,   233,   234,   268,   118,   195,   198,
     108,   119,   141,   149,   210,   211,   212,   268,   109,   117,
     292,   109,   200,   202,   106,   112,   113,   115,   155,   204,
     205,   206,   207,   270,   278,   279,   286,   292,   112,   112,
     112,   293,   209,   210,   268,   117,   235,   268,   255,   268,
     117,   117,   119,   151,   192,   193,   286,   109,   117,   214,
     286,   117,   151,   189,   190,   286,   292,   293,   112,   268,
     117,   265,   268,   259,   268,   109,   117,   111,   111,   108,
     257,   117,   109,   268,   268,   268,   268,   268,   268,   268,
     268,   268,   268,   268,   268,   268,   268,   268,   268,   223,
     268,   292,   260,   109,   117,   274,   109,   117,   117,   188,
     109,   112,   227,   119,   268,   289,   289,   117,   243,   242,
     251,   108,   108,   268,   108,   109,   117,   246,   109,   291,
     109,   249,   250,   269,   291,   109,   109,   109,   248,   283,
     282,   292,   283,   109,   117,   109,   117,   108,   283,   287,
      70,   286,   207,   286,   207,   109,   118,   119,   254,   284,
     284,   278,   117,   117,   292,   109,   109,    88,   109,   117,
     272,   109,   117,   216,    58,   154,   161,   217,   218,   219,
      88,   231,   109,   117,   109,   221,   268,   268,    49,    90,
     167,   293,   269,    97,   262,   286,   231,   225,   210,   230,
     117,   117,   117,   109,   117,   266,   268,   108,   268,   275,
     109,   247,   246,   109,   109,   109,   250,   269,   250,   269,
     250,   269,   291,   109,   117,    98,   234,   211,   212,   268,
     268,   293,   111,   205,   207,   293,   117,   117,   117,    89,
     268,   268,   193,   190,   293,   109,   117,   113,   268,   268,
     117,   268,   268,   117,   293,   117,   118,   155,   226,   289,
     289,   242,   117,   268,   108,   286,   286,   286,   269,   286,
     283,   293,   256,   268,   293,   109,   267,   267,   218,   219,
     267,   268,   204,   117,   268,   111,   111,   111,   111,   109,
     112,   109,   268,   118,   268,   268,   268,   268,   283,   268,
     268,   109,   109,   109,   109,   117,   267,   268,   268,   268,
     268,   117,   267,   267,   267,   267,   117,   117,   117,   117
};

  /* YY1R1[YY1N] -- Symbol number of symbol that rule YY1N derives.  */
static const yy1type_uint16 yy1r1[] =
{
       0,   185,   186,   187,   187,   187,   187,   187,   188,   188,
     189,   189,   190,   190,   187,   187,   187,   191,   191,   191,
     192,   192,   193,   193,   193,   187,   187,   187,   194,   194,
     195,   195,   195,   195,   196,   196,   197,   197,   198,   198,
     198,   187,   199,   199,   200,   200,   201,   201,   202,   202,
     202,   202,   203,   204,   204,   205,   205,   205,   206,   206,
     206,   207,   207,   207,   207,   207,   187,   208,   208,   209,
     209,   210,   210,   211,   211,   211,   211,   212,   212,   212,
     187,   213,   213,   214,   214,   187,   187,   187,   187,   187,
     215,   215,   216,   216,   217,   217,   218,   218,   218,   219,
     219,   187,   187,   220,   220,   221,   221,   187,   187,   222,
     222,   223,   223,   187,   224,   224,   225,   225,   226,   226,
     226,   227,   227,   228,   228,   229,   229,   229,   229,   229,
     229,   229,   229,   229,   229,   229,   230,   230,   230,   230,
     230,   231,   231,   231,   232,   232,   232,   232,   233,   233,
     234,   234,   234,   234,   231,   187,   187,   187,   235,   235,
     235,   235,   231,   231,   231,   231,   187,   187,   187,   187,
     187,   187,   231,   231,   231,   231,   231,   231,   231,   231,
     231,   231,   231,   231,   236,   236,   236,   237,   237,   237,
     238,   239,   240,   241,   241,   241,   241,   241,   242,   242,
     243,   243,   243,   243,   243,   244,   245,   245,   245,   245,
     245,   246,   246,   247,   247,   247,   248,   248,   248,   249,
     249,   249,   249,   249,   249,   249,   250,   250,   250,   250,
     251,   251,   252,   252,   252,   252,   252,   252,   252,   252,
     252,   252,   252,   252,   252,   231,   231,   231,   187,   231,
     231,   231,   231,   231,   231,   187,   187,   253,   253,   254,
     254,   254,   254,   254,   254,   254,   254,   231,   231,   231,
     231,   231,   187,   187,   187,   255,   255,   231,   187,   187,
     187,   187,   187,   187,   231,   256,   256,   257,   257,   257,
     257,   231,   231,   258,   258,   187,   231,   231,   231,   187,
     187,   187,   259,   259,   187,   187,   187,   260,   260,   261,
     261,   187,   262,   263,   263,   187,   264,   264,   187,   231,
     231,   231,   231,   265,   265,   266,   266,   267,   267,   268,
     268,   268,   269,   269,   269,   269,   269,   269,   269,   269,
     269,   269,   269,   269,   269,   269,   269,   269,   269,   269,
     269,   269,   269,   269,   269,   270,   270,   270,   270,   270,
     271,   271,   272,   272,   272,   272,   272,   272,   273,   274,
     274,   275,   275,   275,   275,   275,   275,   275,   275,   276,
     276,   277,   277,   278,   278,   278,   278,   278,   278,   278,
     278,   279,   280,   281,   282,   282,   283,   283,   284,   284,
     285,   285,   286,   286,   287,   287,   288,   289,   290,   291,
     292,   293
};

  /* YY1R2[YY1N] -- Number of symbols on the right hand side of rule YY1N.  */
static const yy1type_uint8 yy1r2[] =
{
       0,     2,     2,     3,     2,     4,     4,     5,     2,     3,
       1,     3,     1,     1,     2,     3,     4,     0,     2,     3,
       1,     3,     1,     1,     1,     1,     2,     3,     1,     3,
       1,     2,     3,     1,     1,     3,     1,     1,     1,     4,
       1,     3,     1,     3,     4,     1,     1,     3,     1,     1,
       1,     1,    10,     1,     3,     3,     1,     1,     1,     1,
       1,     2,     2,     1,     1,     1,     3,     1,     3,     4,
       1,     1,     3,     1,     3,     1,     1,     1,     1,     1,
       3,     1,     3,     3,     1,     3,     3,     3,     2,     3,
       3,     5,     4,     1,     1,     3,     1,     3,     1,     1,
       1,     3,     5,     1,     3,     3,     1,     2,     3,     1,
       3,     1,     1,     4,     1,     3,     4,     1,     0,     3,
       1,     0,     3,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     0,     4,     6,     6,
       4,     5,     5,     3,     1,     3,     4,     1,     1,     3,
       1,     2,     7,     1,     2,     3,     5,     2,     6,     4,
       6,     1,     3,     3,     7,     7,     1,     6,     6,     6,
       2,     2,    10,     3,     3,     3,     4,     5,     4,     6,
       4,     5,     4,     6,     3,     3,     3,     3,     3,     3,
       2,     2,     2,     3,     3,     5,     5,     7,     1,     3,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     3,     1,    10,     1,     1,     1,     1,     3,
       3,     3,     3,     3,     3,     1,     1,    10,    10,    10,
       1,     3,     1,     1,     1,     3,     3,     3,     3,     3,
       2,     2,     3,     1,     1,     3,     3,     3,     1,     2,
       2,     2,     2,     2,     2,     3,     3,     3,     5,     4,
       1,     4,     1,     1,     4,     1,     1,     2,     2,     3,
       5,     3,     3,     5,     2,     6,     1,     3,     2,     3,
       7,     6,     3,     2,     3,     4,     1,     0,     3,     3,
       2,     4,     4,     0,     5,     3,     3,     3,     8,     3,
       5,     2,     6,     1,     3,     3,     2,     1,     1,     1,
       3,     3,     2,     1,     3,     5,     1,     3,     5,     6,
       5,     2,     2,     1,     3,     0,     1,     0,     2,     1,
       3,     1,     1,     1,     3,     3,     3,     3,     3,     2,
       2,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     2,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     1,     2,     3,     2,     1,     1,     5,     2,
       3,     1,     2,     3,     5,     4,     3,     2,     1,     1,
       3,     0,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     5,     0,     0,     1,     3,     1,     1,     1,     3,
       0,     1,     1,     1,     0,     1,     0,     0,     0,     0,
       0,     0
};


#define yy1errok         (yy1errstatus = 0)
#define yy1clearin       (yy1char = YY1EMPTY)
#define YY1EMPTY         (-2)
#define YY1EOF           0

#define YY1ACCEPT        goto yy1acceptlab
#define YY1ABORT         goto yy1abortlab
#define YY1ERROR         goto yy1errorlab


#define YY1RECOVERING()  (!!yy1errstatus)

#define YY1BACKUP(Token, Value)                                  \
do                                                              \
  if (yy1char == YY1EMPTY)                                        \
    {                                                           \
      yy1char = (Token);                                         \
      yy1lval = (Value);                                         \
      YY1POPSTACK (yy1len);                                       \
      yy1state = *yy1ssp;                                         \
      goto yy1backup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yy1error (YY1_("syntax error: cannot back up")); \
      YY1ERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YY1TERROR        1
#define YY1ERRCODE       256



/* Enable debugging if requested.  */
#if YY1DEBUG

# ifndef YY1FPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YY1FPRINTF fprintf
# endif

# define YY1DPRINTF(Args)                        \
do {                                            \
  if (yy1debug)                                  \
    YY1FPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY1_LOCATION_PRINT
# define YY1_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY1_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yy1debug)                                                            \
    {                                                                     \
      YY1FPRINTF (stderr, "%s ", Title);                                   \
      yy1_symbol_print (stderr,                                            \
                  Type, Value); \
      YY1FPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YY1OUTPUT.  |
`----------------------------------------*/

static void
yy1_symbol_value_print (FILE *yy1output, int yy1type, YY1STYPE const * const yy1valuep)
{
  FILE *yy1o = yy1output;
  YY1USE (yy1o);
  if (!yy1valuep)
    return;
# ifdef YY1PRINT
  if (yy1type < YY1NTOKENS)
    YY1PRINT (yy1output, yy1toknum[yy1type], *yy1valuep);
# endif
  YY1USE (yy1type);
}


/*--------------------------------.
| Print this symbol on YY1OUTPUT.  |
`--------------------------------*/

static void
yy1_symbol_print (FILE *yy1output, int yy1type, YY1STYPE const * const yy1valuep)
{
  YY1FPRINTF (yy1output, "%s %s (",
             yy1type < YY1NTOKENS ? "token" : "nterm", yy1tname[yy1type]);

  yy1_symbol_value_print (yy1output, yy1type, yy1valuep);
  YY1FPRINTF (yy1output, ")");
}

/*------------------------------------------------------------------.
| yy1_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy1_stack_print (yy1type_int16 *yy1bottom, yy1type_int16 *yy1top)
{
  YY1FPRINTF (stderr, "Stack now");
  for (; yy1bottom <= yy1top; yy1bottom++)
    {
      int yy1bot = *yy1bottom;
      YY1FPRINTF (stderr, " %d", yy1bot);
    }
  YY1FPRINTF (stderr, "\n");
}

# define YY1_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yy1debug)                                                  \
    yy1_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YY1RULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy1_reduce_print (yy1type_int16 *yy1ssp, YY1STYPE *yy1vsp, int yy1rule)
{
  unsigned long int yy1lno = yy1rline[yy1rule];
  int yy1nrhs = yy1r2[yy1rule];
  int yy1i;
  YY1FPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yy1rule - 1, yy1lno);
  /* The symbols being reduced.  */
  for (yy1i = 0; yy1i < yy1nrhs; yy1i++)
    {
      YY1FPRINTF (stderr, "   $%d = ", yy1i + 1);
      yy1_symbol_print (stderr,
                       yy1stos[yy1ssp[yy1i + 1 - yy1nrhs]],
                       &(yy1vsp[(yy1i + 1) - (yy1nrhs)])
                                              );
      YY1FPRINTF (stderr, "\n");
    }
}

# define YY1_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yy1debug)                          \
    yy1_reduce_print (yy1ssp, yy1vsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yy1debug;
#else /* !YY1DEBUG */
# define YY1DPRINTF(Args)
# define YY1_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY1_STACK_PRINT(Bottom, Top)
# define YY1_REDUCE_PRINT(Rule)
#endif /* !YY1DEBUG */


/* YY1INITDEPTH -- initial size of the parser's stacks.  */
#ifndef YY1INITDEPTH
# define YY1INITDEPTH 200
#endif

/* YY1MAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YY1STACK_ALLOC_MAXIMUM < YY1STACK_BYTES (YY1MAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YY1MAXDEPTH
# define YY1MAXDEPTH 10000
#endif


#if YY1ERROR_VERBOSE

# ifndef yy1strlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yy1strlen strlen
#  else
/* Return the length of YY1STR.  */
static YY1SIZE_T
yy1strlen (const char *yy1str)
{
  YY1SIZE_T yy1len;
  for (yy1len = 0; yy1str[yy1len]; yy1len++)
    continue;
  return yy1len;
}
#  endif
# endif

# ifndef yy1stpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yy1stpcpy stpcpy
#  else
/* Copy YY1SRC to YY1DEST, returning the address of the terminating '\0' in
   YY1DEST.  */
static char *
yy1stpcpy (char *yy1dest, const char *yy1src)
{
  char *yy1d = yy1dest;
  const char *yy1s = yy1src;

  while ((*yy1d++ = *yy1s++) != '\0')
    continue;

  return yy1d - 1;
}
#  endif
# endif

# ifndef yy1tnamerr
/* Copy to YY1RES the contents of YY1STR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yy1error.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YY1STR is taken from yy1tname.  If YY1RES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YY1SIZE_T
yy1tnamerr (char *yy1res, const char *yy1str)
{
  if (*yy1str == '"')
    {
      YY1SIZE_T yy1n = 0;
      char const *yy1p = yy1str;

      for (;;)
        switch (*++yy1p)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yy1p != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yy1res)
              yy1res[yy1n] = *yy1p;
            yy1n++;
            break;

          case '"':
            if (yy1res)
              yy1res[yy1n] = '\0';
            return yy1n;
          }
    do_not_strip_quotes: ;
    }

  if (! yy1res)
    return yy1strlen (yy1str);

  return yy1stpcpy (yy1res, yy1str) - yy1res;
}
# endif

/* Copy into *YY1MSG, which is of size *YY1MSG_ALLOC, an error message
   about the unexpected token YY1TOKEN for the state stack whose top is
   YY1SSP.

   Return 0 if *YY1MSG was successfully written.  Return 1 if *YY1MSG is
   not large enough to hold the message.  In that case, also set
   *YY1MSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yy1syntax_error (YY1SIZE_T *yy1msg_alloc, char **yy1msg,
                yy1type_int16 *yy1ssp, int yy1token)
{
  YY1SIZE_T yy1size0 = yy1tnamerr (YY1_NULLPTR, yy1tname[yy1token]);
  YY1SIZE_T yy1size = yy1size0;
  enum { YY1ERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yy1format = YY1_NULLPTR;
  /* Arguments of yy1format. */
  char const *yy1arg[YY1ERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yy1count = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yy1char) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yy1char.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yy1token != YY1EMPTY)
    {
      int yy1n = yy1pact[*yy1ssp];
      yy1arg[yy1count++] = yy1tname[yy1token];
      if (!yy1pact_value_is_default (yy1n))
        {
          /* Start YY1X at -YY1N if negative to avoid negative indexes in
             YY1CHECK.  In other words, skip the first -YY1N actions for
             this state because they are default actions.  */
          int yy1xbegin = yy1n < 0 ? -yy1n : 0;
          /* Stay within bounds of both yy1check and yy1tname.  */
          int yy1checklim = YY1LAST - yy1n + 1;
          int yy1xend = yy1checklim < YY1NTOKENS ? yy1checklim : YY1NTOKENS;
          int yy1x;

          for (yy1x = yy1xbegin; yy1x < yy1xend; ++yy1x)
            if (yy1check[yy1x + yy1n] == yy1x && yy1x != YY1TERROR
                && !yy1table_value_is_error (yy1table[yy1x + yy1n]))
              {
                if (yy1count == YY1ERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yy1count = 1;
                    yy1size = yy1size0;
                    break;
                  }
                yy1arg[yy1count++] = yy1tname[yy1x];
                {
                  YY1SIZE_T yy1size1 = yy1size + yy1tnamerr (YY1_NULLPTR, yy1tname[yy1x]);
                  if (! (yy1size <= yy1size1
                         && yy1size1 <= YY1STACK_ALLOC_MAXIMUM))
                    return 2;
                  yy1size = yy1size1;
                }
              }
        }
    }

  switch (yy1count)
    {
# define YY1CASE_(N, S)                      \
      case N:                               \
        yy1format = S;                       \
      break
      YY1CASE_(0, YY1_("syntax error"));
      YY1CASE_(1, YY1_("syntax error, unexpected %s"));
      YY1CASE_(2, YY1_("syntax error, unexpected %s, expecting %s"));
      YY1CASE_(3, YY1_("syntax error, unexpected %s, expecting %s or %s"));
      YY1CASE_(4, YY1_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YY1CASE_(5, YY1_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YY1CASE_
    }

  {
    YY1SIZE_T yy1size1 = yy1size + yy1strlen (yy1format);
    if (! (yy1size <= yy1size1 && yy1size1 <= YY1STACK_ALLOC_MAXIMUM))
      return 2;
    yy1size = yy1size1;
  }

  if (*yy1msg_alloc < yy1size)
    {
      *yy1msg_alloc = 2 * yy1size;
      if (! (yy1size <= *yy1msg_alloc
             && *yy1msg_alloc <= YY1STACK_ALLOC_MAXIMUM))
        *yy1msg_alloc = YY1STACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yy1p = *yy1msg;
    int yy1i = 0;
    while ((*yy1p = *yy1format) != '\0')
      if (*yy1p == '%' && yy1format[1] == 's' && yy1i < yy1count)
        {
          yy1p += yy1tnamerr (yy1p, yy1arg[yy1i++]);
          yy1format += 2;
        }
      else
        {
          yy1p++;
          yy1format++;
        }
  }
  return 0;
}
#endif /* YY1ERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yy1destruct (const char *yy1msg, int yy1type, YY1STYPE *yy1valuep)
{
  YY1USE (yy1valuep);
  if (!yy1msg)
    yy1msg = "Deleting";
  YY1_SYMBOL_PRINT (yy1msg, yy1type, yy1valuep, yy1locationp);

  YY1_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY1USE (yy1type);
  YY1_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yy1char;

/* The semantic value of the lookahead symbol.  */
YY1STYPE yy1lval;
/* Number of syntax errors so far.  */
int yy1nerrs;


/*----------.
| yy1parse.  |
`----------*/

int
yy1parse (void)
{
    int yy1state;
    /* Number of tokens to shift before error messages enabled.  */
    int yy1errstatus;

    /* The stacks and their tools:
       'yy1ss': related to states.
       'yy1vs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yy1overflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yy1type_int16 yy1ssa[YY1INITDEPTH];
    yy1type_int16 *yy1ss;
    yy1type_int16 *yy1ssp;

    /* The semantic value stack.  */
    YY1STYPE yy1vsa[YY1INITDEPTH];
    YY1STYPE *yy1vs;
    YY1STYPE *yy1vsp;

    YY1SIZE_T yy1stacksize;

  int yy1n;
  int yy1result;
  /* Lookahead token as an internal (translated) token number.  */
  int yy1token = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YY1STYPE yy1val;

#if YY1ERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yy1msgbuf[128];
  char *yy1msg = yy1msgbuf;
  YY1SIZE_T yy1msg_alloc = sizeof yy1msgbuf;
#endif

#define YY1POPSTACK(N)   (yy1vsp -= (N), yy1ssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yy1len = 0;

  yy1ssp = yy1ss = yy1ssa;
  yy1vsp = yy1vs = yy1vsa;
  yy1stacksize = YY1INITDEPTH;

  YY1DPRINTF ((stderr, "Starting parse\n"));

  yy1state = 0;
  yy1errstatus = 0;
  yy1nerrs = 0;
  yy1char = YY1EMPTY; /* Cause a token to be read.  */
  goto yy1setstate;

/*------------------------------------------------------------.
| yy1newstate -- Push a new state, which is found in yy1state.  |
`------------------------------------------------------------*/
 yy1newstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yy1ssp++;

 yy1setstate:
  *yy1ssp = yy1state;

  if (yy1ss + yy1stacksize - 1 <= yy1ssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YY1SIZE_T yy1size = yy1ssp - yy1ss + 1;

#ifdef yy1overflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YY1STYPE *yy1vs1 = yy1vs;
        yy1type_int16 *yy1ss1 = yy1ss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yy1overflow is a macro.  */
        yy1overflow (YY1_("memory exhausted"),
                    &yy1ss1, yy1size * sizeof (*yy1ssp),
                    &yy1vs1, yy1size * sizeof (*yy1vsp),
                    &yy1stacksize);

        yy1ss = yy1ss1;
        yy1vs = yy1vs1;
      }
#else /* no yy1overflow */
# ifndef YY1STACK_RELOCATE
      goto yy1exhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YY1MAXDEPTH <= yy1stacksize)
        goto yy1exhaustedlab;
      yy1stacksize *= 2;
      if (YY1MAXDEPTH < yy1stacksize)
        yy1stacksize = YY1MAXDEPTH;

      {
        yy1type_int16 *yy1ss1 = yy1ss;
        union yy1alloc *yy1ptr =
          (union yy1alloc *) YY1STACK_ALLOC (YY1STACK_BYTES (yy1stacksize));
        if (! yy1ptr)
          goto yy1exhaustedlab;
        YY1STACK_RELOCATE (yy1ss_alloc, yy1ss);
        YY1STACK_RELOCATE (yy1vs_alloc, yy1vs);
#  undef YY1STACK_RELOCATE
        if (yy1ss1 != yy1ssa)
          YY1STACK_FREE (yy1ss1);
      }
# endif
#endif /* no yy1overflow */

      yy1ssp = yy1ss + yy1size - 1;
      yy1vsp = yy1vs + yy1size - 1;

      YY1DPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yy1stacksize));

      if (yy1ss + yy1stacksize - 1 <= yy1ssp)
        YY1ABORT;
    }

  YY1DPRINTF ((stderr, "Entering state %d\n", yy1state));

  if (yy1state == YY1FINAL)
    YY1ACCEPT;

  goto yy1backup;

/*-----------.
| yy1backup.  |
`-----------*/
yy1backup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yy1n = yy1pact[yy1state];
  if (yy1pact_value_is_default (yy1n))
    goto yy1default;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YY1CHAR is either YY1EMPTY or YY1EOF or a valid lookahead symbol.  */
  if (yy1char == YY1EMPTY)
    {
      YY1DPRINTF ((stderr, "Reading a token: "));
      yy1char = yy1lex ();
    }

  if (yy1char <= YY1EOF)
    {
      yy1char = yy1token = YY1EOF;
      YY1DPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yy1token = YY1TRANSLATE (yy1char);
      YY1_SYMBOL_PRINT ("Next token is", yy1token, &yy1lval, &yy1lloc);
    }

  /* If the proper action on seeing token YY1TOKEN is to reduce or to
     detect an error, take that action.  */
  yy1n += yy1token;
  if (yy1n < 0 || YY1LAST < yy1n || yy1check[yy1n] != yy1token)
    goto yy1default;
  yy1n = yy1table[yy1n];
  if (yy1n <= 0)
    {
      if (yy1table_value_is_error (yy1n))
        goto yy1errlab;
      yy1n = -yy1n;
      goto yy1reduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yy1errstatus)
    yy1errstatus--;

  /* Shift the lookahead token.  */
  YY1_SYMBOL_PRINT ("Shifting", yy1token, &yy1lval, &yy1lloc);

  /* Discard the shifted token.  */
  yy1char = YY1EMPTY;

  yy1state = yy1n;
  YY1_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yy1vsp = yy1lval;
  YY1_IGNORE_MAYBE_UNINITIALIZED_END

  goto yy1newstate;


/*-----------------------------------------------------------.
| yy1default -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yy1default:
  yy1n = yy1defact[yy1state];
  if (yy1n == 0)
    goto yy1errlab;
  goto yy1reduce;


/*-----------------------------.
| yy1reduce -- Do a reduction.  |
`-----------------------------*/
yy1reduce:
  /* yy1n is the number of a rule to reduce with.  */
  yy1len = yy1r2[yy1n];

  /* If YY1LEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YY1VAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YY1VAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YY1VAL may be used uninitialized.  */
  yy1val = yy1vsp[1-yy1len];


  YY1_REDUCE_PRINT (yy1n);
  switch (yy1n)
    {
        case 2:
#line 217 "gram1.y" /* yacc.c:1646  */
    {
        fp1_token = (yy1vsp[-1].statval);
	switch ((yy1vsp[-1].statval).token)
	{
	  case SFUNCTION_STAT:
	    treeStackDrop(4);
            break;

	  case SSUBROUTINE_STAT:
	  case SDO_STAT:
          case SDO_LABEL_STAT:
	  case SDOALL_STAT:
          case SDOALL_LABEL_STAT:
	  case SPARBEGIN_PID_STAT:
          case SPARALLELLOOP_STAT:
          case SPARALLELLOOP_LABEL_STAT:
	    treeStackDrop(3);
            break;

	  case SBLOCK_DATA_STAT:
	  case SPROGRAM_STAT:
	  case SIF_STAT:
          case SELSE_IF_STAT:
          case SELSE_STAT:
          case SDEBUG_STAT:
	  case SPARALLEL_PID_STAT:
          case SWHERE_BLOCK_STAT:
          case SELSE_WHERE_STAT:
	    treeStackDrop(2);
	    break;

	  default:
	  case SPRSCOPE_PH_STAT:
	  case SSTMT_PH_STAT:
	  case SEND_STAT:
          case SEND_DO_STAT:
          case SEND_IF_STAT:
	  case SPARBEGIN_STAT:
	  case SPARALLEL_STAT:
	  case SOTHER_PROCESSES_STAT:
	  case SPAREND_STAT:
          case SEND_LOOP_STAT:
          case SEND_WHERE_STAT:
	    treeStackDrop(1);
	    break;

	  case SERROR_STAT:
            break;
	}
      }
#line 2465 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 289 "gram1.y" /* yacc.c:1646  */
    {
        if( (yy1vsp[0].astptr) == AST_NIL )
          { (yy1vsp[0].astptr) = gen_IDENTIFIER();
            gen_put_text((yy1vsp[0].astptr), "DATA", STR_IDENTIFIER);
            ft_SetShow((yy1vsp[0].astptr), false);		/* shorthand */
          }

        (yy1val.statval).token   = SBLOCK_DATA_STAT;
        (yy1val.statval).part[0] = (yy1vsp[-1].astptr);
        (yy1val.statval).part[1] = (yy1vsp[0].astptr);
      }
#line 2481 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 311 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token   = SEND_STAT;
        (yy1val.statval).part[0] = (yy1vsp[0].astptr);
      }
#line 2490 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 326 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SENTRY_STAT;
        (yy1val.statval).tree  = gen_ENTRY((yy1vsp[-2].astptr), (yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(3);
	treeStackPush((yy1val.statval).tree);
      }
#line 2501 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 343 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token   = SFUNCTION_STAT;
	(yy1val.statval).part[0] = (yy1vsp[-2].astptr);
        (yy1val.statval).part[1] = AST_NIL;
        (yy1val.statval).part[2] = (yy1vsp[-1].astptr);
        (yy1val.statval).part[3] = (yy1vsp[0].astptr);
	treeStackPush((yy1val.statval).part[1]);
      }
#line 2514 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 352 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token   = SFUNCTION_STAT;
	(yy1val.statval).part[0] = (yy1vsp[-2].astptr);
        (yy1val.statval).part[1] = (yy1vsp[-4].astptr);
        (yy1val.statval).part[2] = (yy1vsp[-1].astptr);
        (yy1val.statval).part[3] = (yy1vsp[0].astptr);
      }
#line 2526 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 363 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create(AST_NIL);
	treeStackPush((yy1val.astptr));
      }
#line 2535 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 368 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = (yy1vsp[-1].astptr);
      }
#line 2543 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 375 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 2553 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 381 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 2563 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 391 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_formal);
	treeStackPush((yy1val.astptr));
      }
#line 2572 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 406 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token   = SPRSCOPE_PH_STAT;
        (yy1val.statval).part[0] = ph_from_mtype(GEN_subprogram);
	treeStackPush((yy1val.statval).part[0]);
      }
#line 2582 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 422 "gram1.y" /* yacc.c:1646  */
    {
        if( (yy1vsp[0].astptr) == AST_NIL )
          { (yy1vsp[0].astptr) = gen_IDENTIFIER();
            gen_put_text((yy1vsp[0].astptr), "MAIN", STR_IDENTIFIER);
            ft_SetShow((yy1vsp[0].astptr), false);		/* shorthand */
          }

        (yy1val.statval).token   = SPROGRAM_STAT;
        (yy1val.statval).part[0] = (yy1vsp[-1].astptr);
        (yy1val.statval).part[1] = (yy1vsp[0].astptr);
      }
#line 2598 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 444 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token   = SSUBROUTINE_STAT;
        (yy1val.statval).part[0] = (yy1vsp[-2].astptr);
        (yy1val.statval).part[1] = (yy1vsp[-1].astptr);
        (yy1val.statval).part[2] = (yy1vsp[0].astptr);
      }
#line 2609 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 454 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = AST_NIL;
	treeStackPush((yy1val.astptr));
      }
#line 2618 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 459 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create(AST_NIL);
	treeStackPush((yy1val.astptr));
      }
#line 2627 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 464 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = (yy1vsp[-1].astptr);
      }
#line 2635 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 471 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 2645 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 477 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 2655 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 487 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_STAR();
	treeStackPush((yy1val.astptr));
      }
#line 2664 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 492 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_formal);
	treeStackPush((yy1val.astptr));
      }
#line 2673 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 516 "gram1.y" /* yacc.c:1646  */
    {
        FortTreeNode comment_text_node;
        char * line;
        int len;

        line = lx1_GetLine(&len);
        while ((*line == ' ') && (len > 0))
        {
	  line++;
          len--;
        }
        while ((line[len - 1] == ' ') && (len > 0))
        {
          len--;
        }
        line[len] = '\0';

        comment_text_node = gen_TEXT();
        gen_put_text(comment_text_node, line, STR_COMMENT_TEXT);
        (yy1val.statval).token = SCOMMENT_STAT;
        (yy1val.statval).tree  = gen_COMMENT(comment_text_node);
	treeStackPush((yy1val.statval).tree);
      }
#line 2701 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 540 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SCOMMENT_STAT;
        (yy1val.statval).tree  = gen_COMMENT(AST_NIL);
	treeStackPush((yy1val.statval).tree);
      }
#line 2711 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 556 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SCOMMON_STAT;
        (yy1val.statval).tree  = gen_COMMON((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 2722 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 573 "gram1.y" /* yacc.c:1646  */
    {
        FortTreeNode dead, new_vars, list;
        Boolean comma;

	if( NOT(ft_GetShow(gen_COMMON_ELT_get_name(list_first((yy1vsp[0].astptr))))) )
          {
            /* The first COMMON_ELT of $3 contains a partial variable list. */
	    /* Remove the COMMON_ELT (dead) from $3 and delete it except    */
	    /* for its variable list (var_list).  Append var_list to the    */
            /* variable list of the last COMMON_ELT of $1 (list).	    */
              dead = list_remove_first((yy1vsp[0].astptr));
	      comma = ft_GetComma(dead);
	      new_vars = gen_COMMON_ELT_get_common_vars_LIST(dead);
	      tree_replace(new_vars, AST_NIL);
	      tree_free(dead);
              list = gen_COMMON_ELT_get_common_vars_LIST(list_last((yy1vsp[-2].astptr)));
	      list = list_append(list, new_vars);
              gen_COMMON_ELT_put_common_vars_LIST(list_last((yy1vsp[-2].astptr)), list);
	      ft_SetComma(list_last((yy1vsp[-2].astptr)), comma);
          }
        (yy1val.astptr) = list_append((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 2751 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 601 "gram1.y" /* yacc.c:1646  */
    {
        FortTreeNode block;

        block = gen_IDENTIFIER();
	gen_put_text(block, "//", STR_COMMON_NAME);
        ft_SetShow(block, false);
	(yy1val.astptr) = list_create(gen_COMMON_ELT(block, list_create((yy1vsp[0].astptr))));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 2766 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 612 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.astptr) = list_create(gen_COMMON_ELT((yy1vsp[-1].astptr), list_create((yy1vsp[0].astptr))));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 2776 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 618 "gram1.y" /* yacc.c:1646  */
    {
        FortTreeNode block, first, list;

        block = gen_IDENTIFIER();
	gen_put_text(block, "//", STR_COMMON_NAME);
        ft_SetShow(block, false);
        first = gen_COMMON_ELT(block, list_create((yy1vsp[-2].astptr)));
	ft_SetComma(first, false);	/* no comma */
	list = list_create(gen_COMMON_ELT((yy1vsp[-1].astptr), list_create((yy1vsp[0].astptr))));
	(yy1val.astptr) = list_insert_first(list, first);
	treeStackDrop(3);
	treeStackPush((yy1val.astptr));
      }
#line 2794 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 632 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.astptr) = list_create(ph_from_mtype(GEN_common_elt));
	treeStackPush((yy1val.astptr));
      }
#line 2803 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 640 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_IDENTIFIER();
	gen_put_text((yy1val.astptr), "//", STR_COMMON_NAME);
	treeStackPush((yy1val.astptr));
      }
#line 2813 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 646 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = (yy1vsp[-1].astptr);
      }
#line 2821 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 653 "gram1.y" /* yacc.c:1646  */
    {
        char *name;

	name = (char *) get_mem(lx1_TokenLen + 3, "gram1.y (common name)");
        (void) strcpy(name + 1, lx1_Token);
	name[0] = name[lx1_TokenLen + 1] = '/';
	name[lx1_TokenLen + 2] = '\0';

        (yy1val.astptr) = gen_IDENTIFIER();
	gen_put_text((yy1val.astptr), name, STR_COMMON_NAME);
	free_mem((void *) name);
	treeStackPush((yy1val.astptr));
      }
#line 2839 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 667 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.astptr) = ph_from_mtype(GEN_name);
	treeStackPush((yy1val.astptr));
      }
#line 2848 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 675 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_ARRAY_DECL_LEN((yy1vsp[0].astptr), AST_NIL, AST_NIL, AST_NIL);
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 2858 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 681 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_ARRAY_DECL_LEN((yy1vsp[-3].astptr), AST_NIL, (yy1vsp[-1].astptr), AST_NIL);
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 2868 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 687 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_common_vars);
	treeStackPush((yy1val.astptr));
      }
#line 2877 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 702 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SDATA_STAT;
        (yy1val.statval).tree  = gen_DATA((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 2888 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 712 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 2898 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 718 "gram1.y" /* yacc.c:1646  */
    {
        ft_SetComma(list_last((yy1vsp[-2].astptr)), (yy1vsp[-1].boolean));
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 2909 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 728 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_DATA_ELT((yy1vsp[-3].astptr), (yy1vsp[-1].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 2919 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 734 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_data_init);
	treeStackPush((yy1val.astptr));
      }
#line 2928 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 742 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 2938 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 748 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 2948 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 759 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_data);
	treeStackPush((yy1val.astptr));
      }
#line 2957 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 769 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_IMPLIED_DO((yy1vsp[-8].astptr), (yy1vsp[-6].astptr), (yy1vsp[-4].astptr), (yy1vsp[-2].astptr), (yy1vsp[-1].astptr));
	treeStackDrop(5);
	treeStackPush((yy1val.astptr));
      }
#line 2967 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 778 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 2977 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 784 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 2987 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 793 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_REPEAT((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 2997 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 800 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_init);
	treeStackPush((yy1val.astptr));
      }
#line 3006 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 808 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.astptr) = gen_CONSTANT();
	gen_put_text((yy1val.astptr), lx1_Token, STR_CONSTANT_INTEGER);
	treeStackPush((yy1val.astptr));
      }
#line 3016 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 819 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_UNARY_MINUS((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 3026 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 825 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = (yy1vsp[0].astptr);
      }
#line 3034 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 840 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SDIMENSION_STAT;
        (yy1val.statval).tree  = gen_DIMENSION((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 3045 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 850 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 3055 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 856 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 3065 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 865 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_ARRAY_DECL_LEN((yy1vsp[-3].astptr), AST_NIL, (yy1vsp[-1].astptr), AST_NIL);
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 3075 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 871 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_array_decl_len);
	treeStackPush((yy1val.astptr));
      }
#line 3084 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 879 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 3094 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 885 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 3104 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 894 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_DIM(AST_NIL, (yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 3114 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 900 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_DIM((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 3124 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 906 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_DIM(AST_NIL, AST_NIL);
	treeStackPush((yy1val.astptr));
      }
#line 3133 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 911 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_dim);
	treeStackPush((yy1val.astptr));
      }
#line 3142 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 919 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_STAR();
	treeStackPush((yy1val.astptr));
      }
#line 3151 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 925 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_bound);
	treeStackPush((yy1val.astptr));
      }
#line 3160 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 940 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SEQUIVALENCE_STAT;
        (yy1val.statval).tree  = gen_EQUIVALENCE((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 3171 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 950 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 3181 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 956 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 3191 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 965 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_EQUIV_ELT((yy1vsp[-1].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 3201 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 971 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_equiv);
	treeStackPush((yy1val.astptr));
      }
#line 3210 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 986 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SEXTERNAL_STAT;
        (yy1val.statval).tree  = gen_EXTERNAL((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 3221 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 1003 "gram1.y" /* yacc.c:1646  */
    {
        FortTreeNode text;

        if ((yy1vsp[-1].astptr) == AST_NIL)
          { yy1error("Format statments must have statment labels.");
            YY1ERROR;
          }

	text = gen_TEXT();
	gen_put_text(text, lx1_Token, STR_FORMAT_STRING);
        (yy1val.statval).token = SFORMAT_STAT;
        (yy1val.statval).tree  = gen_FORMAT((yy1vsp[-1].astptr), text);
	treeStackDrop(1);
	treeStackPush((yy1val.statval).tree);
      }
#line 3241 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 1019 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SFORMAT_STAT;
        (yy1val.statval).tree  = gen_FORMAT((yy1vsp[-1].astptr), ph_from_mtype(GEN_text));
	treeStackDrop(1);
	treeStackPush((yy1val.statval).tree);
      }
#line 3252 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 1036 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SIMPLICIT_STAT;
        (yy1val.statval).tree  = gen_IMPLICIT((yy1vsp[0].astptr), list_create(gen_NONE()));
	treeStackDrop(1);
	treeStackPush((yy1val.statval).tree);
      }
#line 3263 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 1043 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SIMPLICIT_STAT;
        (yy1val.statval).tree  = gen_IMPLICIT((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 3274 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 1053 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[-1].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 3284 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 1059 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-4].astptr), (yy1vsp[-1].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 3294 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 1068 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_IMPLICIT_ELT((yy1vsp[-3].astptr), (yy1vsp[-1].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 3304 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 1074 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_implicit_def);
	treeStackPush((yy1val.astptr));
      }
#line 3313 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 1082 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 3323 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 95:
#line 1088 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 3333 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 1098 "gram1.y" /* yacc.c:1646  */
    {
        char *start_range, *end_range;

        if( ! is_place_holder((yy1vsp[-2].astptr))  &&  ! is_place_holder((yy1vsp[0].astptr)) )
          { start_range = gen_get_text((yy1vsp[-2].astptr));
            end_range   = gen_get_text((yy1vsp[0].astptr));
            if( *start_range == '$'  &&  *end_range != '$' )
              { yy1error("Backwards implicit letter range.");
	        YY1ERROR;
                /* NOTREACHED */
              }
            else if( *end_range != '$'  &&  *start_range > *end_range )
              { yy1error("Backwards implicit letter range.");
	        YY1ERROR;
                /* NOTREACHED */
	      }
          }

	(yy1val.astptr) = gen_IMPLICIT_PAIR((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
        treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 3360 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 1121 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_letters);
	treeStackPush((yy1val.astptr));
      }
#line 3369 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 1129 "gram1.y" /* yacc.c:1646  */
    {
	char c = lx1_Token[0];

        if ( lx1_TokenLen != 1 || ((c < 'a' || c > 'z') && c != '$') )
        { yy1error("Implicits must be single letters or ranges of letters.");
	  YY1ERROR;
        }

        (yy1val.astptr) = gen_LETTER();
	gen_put_text((yy1val.astptr), lx1_Token, STR_CONSTANT_LETTER);
	treeStackPush((yy1val.astptr));
      }
#line 3386 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 1142 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_letter);
	treeStackPush((yy1val.astptr));
      }
#line 3395 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 1157 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SINTRINSIC_STAT;
        (yy1val.statval).tree  = gen_INTRINSIC((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 3406 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 1174 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SPARAMETER_STAT;
        (yy1val.statval).tree  = gen_PARAMETER((yy1vsp[-3].astptr), (yy1vsp[-1].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 3417 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 1184 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 3427 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 1190 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 3437 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 1199 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_PARAM_ELT((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 3447 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 1205 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_param_def);
	treeStackPush((yy1val.astptr));
      }
#line 3456 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 1220 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SSAVE_STAT;
        (yy1val.statval).tree  = gen_SAVE((yy1vsp[0].astptr), AST_NIL);
	treeStackDrop(1);
	treeStackPush((yy1val.statval).tree);
      }
#line 3467 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 1227 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SSAVE_STAT;
        (yy1val.statval).tree  = gen_SAVE((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 3478 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 1237 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 3488 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 1243 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 3498 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 1264 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = STYPE_STATEMENT_STAT;
        (yy1val.statval).tree  = gen_TYPE_STATEMENT((yy1vsp[-2].astptr), (yy1vsp[-3].astptr), (yy1vsp[0].astptr));
	ft_SetComma((yy1val.statval).tree, (yy1vsp[-1].boolean));
	treeStackDrop(3);
	treeStackPush((yy1val.statval).tree);
      }
#line 3510 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 1275 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 3520 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 1281 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 3530 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 1290 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_ARRAY_DECL_LEN((yy1vsp[-3].astptr), (yy1vsp[-1].astptr), (yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(4);
	treeStackPush((yy1val.astptr));
      }
#line 3540 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 1296 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.astptr) = ph_from_mtype(GEN_array_decl_len);
	treeStackPush((yy1val.astptr));
      }
#line 3549 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 1304 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.astptr) = AST_NIL;
	treeStackPush((yy1val.astptr));
      }
#line 3558 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 1309 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.astptr) = (yy1vsp[-1].astptr);
      }
#line 3566 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 1313 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create(ph_from_mtype(GEN_init));
	treeStackPush((yy1val.astptr));
      }
#line 3575 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 1321 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = AST_NIL;
	treeStackPush((yy1val.astptr));
      }
#line 3584 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 1326 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = (yy1vsp[-1].astptr);
      }
#line 3592 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 1332 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_TYPE_LEN((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 3602 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 1338 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_type);
	treeStackPush((yy1val.astptr));
      }
#line 3611 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 1346 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_INTEGER();
	treeStackPush((yy1val.astptr));
      }
#line 3620 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 1351 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_REAL();
	treeStackPush((yy1val.astptr));
      }
#line 3629 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 1356 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_CHARACTER();
	treeStackPush((yy1val.astptr));
      }
#line 3638 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 1361 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_COMPLEX();
	treeStackPush((yy1val.astptr));
      }
#line 3647 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 129:
#line 1366 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_DOUBLE_PRECISION();
	treeStackPush((yy1val.astptr));
      }
#line 3656 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 130:
#line 1371 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_LOGICAL();
	treeStackPush((yy1val.astptr));
      }
#line 3665 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 131:
#line 1376 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_EXACT();
	treeStackPush((yy1val.astptr));
      }
#line 3674 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 132:
#line 1381 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_SEMAPHORE();
	treeStackPush((yy1val.astptr));
      }
#line 3683 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 133:
#line 1386 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_EVENT();
	treeStackPush((yy1val.astptr));
      }
#line 3692 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 134:
#line 1391 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BARRIER();
	treeStackPush((yy1val.astptr));
      }
#line 3701 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 135:
#line 1396 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_type_name);
	treeStackPush((yy1val.astptr));
      }
#line 3710 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 1404 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = AST_NIL;
	treeStackPush((yy1val.astptr));
      }
#line 3719 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 1409 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = (yy1vsp[-1].astptr);
      }
#line 3727 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 1413 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = (yy1vsp[-2].astptr);
        gen_put_parens((yy1val.astptr), 1);
      }
#line 3736 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 1418 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_STAR();
        gen_put_parens((yy1val.astptr), 1);
	treeStackPush((yy1val.astptr));
      }
#line 3746 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 1424 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_len);
	treeStackPush((yy1val.astptr));
      }
#line 3755 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 1448 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SASSIGN_STAT;
        (yy1val.statval).tree  = gen_ASSIGN((yy1vsp[-3].astptr), (yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(3);
	treeStackPush((yy1val.statval).tree);
      }
#line 3766 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1465 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SASSIGNMENT_STAT;
        (yy1val.statval).tree  = gen_ASSIGNMENT((yy1vsp[-3].astptr), (yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(3);
	treeStackPush((yy1val.statval).tree);
      }
#line 3777 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 1482 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SCALL_STAT;
        (yy1val.statval).tree  = gen_CALL((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 3788 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1492 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_INVOCATION((yy1vsp[0].astptr), AST_NIL);
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 3798 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1498 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_INVOCATION((yy1vsp[-2].astptr), list_create(AST_NIL));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 3808 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1504 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_INVOCATION((yy1vsp[-3].astptr), (yy1vsp[-1].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 3818 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1510 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_invocation);
	treeStackPush((yy1val.astptr));
      }
#line 3827 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1518 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 3837 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1524 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 3847 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1534 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_RETURN_LABEL((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 3857 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1540 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_VALUE_PARAMETER((yy1vsp[-1].astptr));
        treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 3867 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1546 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_arg);
	treeStackPush((yy1val.astptr));
      }
#line 3876 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1561 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SCONTINUE_STAT;
        (yy1val.statval).tree  = gen_CONTINUE((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.statval).tree);
      }
#line 3887 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1578 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SDO_STAT;
	(yy1val.statval).part[0] = (yy1vsp[-1].astptr);
	(yy1val.statval).part[1] = AST_NIL;
        (yy1val.statval).part[2] = (yy1vsp[0].astptr);
	treeStackPush((yy1val.statval).part[1]);
      }
#line 3899 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 156:
#line 1586 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SDO_LABEL_STAT;
	(yy1val.statval).part[0] = (yy1vsp[-3].astptr);
	(yy1val.statval).part[1] = (yy1vsp[-2].astptr);
        (yy1val.statval).part[2] = (yy1vsp[0].astptr);
        ft_SetComma((yy1val.statval).part[1], (yy1vsp[-1].boolean));
      }
#line 3911 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 157:
#line 1594 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SEND_DO_STAT;
	(yy1val.statval).part[0] = (yy1vsp[0].astptr);
      }
#line 3920 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 158:
#line 1602 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_REPETITIVE((yy1vsp[-4].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 3930 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 159:
#line 1608 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_CONDITIONAL((yy1vsp[-1].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 3940 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 160:
#line 1614 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_INDUCTIVE((yy1vsp[-5].astptr), (yy1vsp[-3].astptr), (yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(4);
	treeStackPush((yy1val.astptr));
      }
#line 3950 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 161:
#line 1620 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_loop_control);
	treeStackPush((yy1val.astptr));
      }
#line 3959 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 162:
#line 1635 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SGOTO_STAT;
        (yy1val.statval).tree  = gen_GOTO((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 3970 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 163:
#line 1642 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SASSIGNED_GOTO_STAT;
        (yy1val.statval).tree  = gen_ASSIGNED_GOTO((yy1vsp[-1].astptr), (yy1vsp[0].astptr), AST_NIL);
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 3981 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 164:
#line 1649 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SASSIGNED_GOTO_STAT;
        (yy1val.statval).tree  = gen_ASSIGNED_GOTO((yy1vsp[-5].astptr), (yy1vsp[-4].astptr), (yy1vsp[-1].astptr));
        ft_SetComma((yy1val.statval).tree, (yy1vsp[-3].boolean));
	treeStackDrop(3);
	treeStackPush((yy1val.statval).tree);
      }
#line 3993 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 165:
#line 1657 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SCOMPUTED_GOTO_STAT;
        (yy1val.statval).tree  = gen_COMPUTED_GOTO((yy1vsp[-5].astptr), (yy1vsp[-3].astptr), (yy1vsp[0].astptr));
        ft_SetComma((yy1val.statval).tree, (yy1vsp[-1].boolean));
	treeStackDrop(3);
	treeStackPush((yy1val.statval).tree);
      }
#line 4005 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 167:
#line 1676 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token   = SLOGICAL_IF_STAT;
        if( (yy1vsp[0].statval).token == SSTMT_PH_STAT )
          (yy1val.statval).tree    = gen_LOGICAL_IF((yy1vsp[-4].astptr), (yy1vsp[-2].astptr), list_create((yy1vsp[0].statval).part[0]));
        else
          (yy1val.statval).tree    = gen_LOGICAL_IF((yy1vsp[-4].astptr), (yy1vsp[-2].astptr), list_create((yy1vsp[0].statval).tree));
	treeStackDrop(3);
	treeStackPush((yy1val.statval).tree);
      }
#line 4019 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 168:
#line 1686 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token   = SIF_STAT;
        (yy1val.statval).part[0] = (yy1vsp[-4].astptr);
        (yy1val.statval).part[1] = (yy1vsp[-2].astptr);
      }
#line 4029 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 169:
#line 1692 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token   = SELSE_IF_STAT;
	(yy1val.statval).part[0] = (yy1vsp[-4].astptr);
        (yy1val.statval).part[1] = (yy1vsp[-2].astptr);
      }
#line 4039 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 170:
#line 1698 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token   = SELSE_STAT;
	(yy1val.statval).part[0] = (yy1vsp[0].astptr);
	(yy1val.statval).part[1] = AST_NIL;
	treeStackPush((yy1val.statval).part[1]);
      }
#line 4050 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 171:
#line 1705 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token   = SEND_IF_STAT;
	(yy1val.statval).part[0] = (yy1vsp[0].astptr);
      }
#line 4059 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 172:
#line 1713 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SARITHMETIC_IF_STAT;
        (yy1val.statval).tree  = gen_ARITHMETIC_IF((yy1vsp[-8].astptr), (yy1vsp[-6].astptr), (yy1vsp[-4].astptr), (yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(5);
	treeStackPush((yy1val.statval).tree);
      }
#line 4070 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 173:
#line 1730 "gram1.y" /* yacc.c:1646  */
    {
        /* ASSERT kwd list of $1 is AST_NIL */
          switch(iostmt)
          {
            case SBACKSPACE_LONG_STAT:
              gen_BACKSPACE_LONG_put_kwd_LIST((yy1vsp[-2].astptr), (yy1vsp[-1].astptr));
	      break;
            case SREWIND_LONG_STAT:
              gen_REWIND_LONG_put_kwd_LIST((yy1vsp[-2].astptr), (yy1vsp[-1].astptr));
	      break;
            case SENDFILE_LONG_STAT:
              gen_ENDFILE_LONG_put_kwd_LIST((yy1vsp[-2].astptr), (yy1vsp[-1].astptr));
	      break;
          }

	(yy1val.statval).token = iostmt;
        (yy1val.statval).tree  = (yy1vsp[-2].astptr);
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 4095 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 174:
#line 1751 "gram1.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

	switch(iostmt)
        {
          case SBACKSPACE_LONG_STAT:
            label = gen_BACKSPACE_LONG_get_lbl_def((yy1vsp[-2].astptr));
	    if( label != AST_NIL )
              tree_replace(label, AST_NIL);
	    tree_free((yy1vsp[-2].astptr));
	    (yy1val.statval).token = SBACKSPACE_SHORT_STAT;
            (yy1val.statval).tree  = gen_BACKSPACE_SHORT(label, (yy1vsp[-1].astptr));
            break;
	  case SREWIND_LONG_STAT:
            label = gen_REWIND_LONG_get_lbl_def((yy1vsp[-2].astptr));
	    if( label != AST_NIL )
              tree_replace(label, AST_NIL);
	    tree_free((yy1vsp[-2].astptr));
	    (yy1val.statval).token = SREWIND_SHORT_STAT;
            (yy1val.statval).tree  = gen_REWIND_SHORT(label, (yy1vsp[-1].astptr));
            break;
	  case SENDFILE_LONG_STAT:
            label = gen_ENDFILE_LONG_get_lbl_def((yy1vsp[-2].astptr));
	    if( label != AST_NIL )
              tree_replace(label, AST_NIL);
	    tree_free((yy1vsp[-2].astptr));
	    (yy1val.statval).token = SENDFILE_SHORT_STAT;
            (yy1val.statval).tree  = gen_ENDFILE_SHORT(label, (yy1vsp[-1].astptr));
            break;
        }
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 4133 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 175:
#line 1785 "gram1.y" /* yacc.c:1646  */
    {
        /* ASSERT kwd list of $1 is AST_NIL */
          switch(iostmt)
          {
            case SINQUIRE_STAT:
              gen_INQUIRE_put_kwd_LIST((yy1vsp[-2].astptr), (yy1vsp[-1].astptr));
	      break;
            case SOPEN_STAT:
              gen_OPEN_put_kwd_LIST((yy1vsp[-2].astptr), (yy1vsp[-1].astptr));
	      break;
            case SCLOSE_STAT:
              gen_CLOSE_put_kwd_LIST((yy1vsp[-2].astptr), (yy1vsp[-1].astptr));
	      break;
          }

	(yy1val.statval).token = iostmt;
        (yy1val.statval).tree  = (yy1vsp[-2].astptr);
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 4158 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 176:
#line 1806 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.statval).token = SREAD_LONG_STAT;
        (yy1val.statval).tree  = gen_READ_LONG((yy1vsp[-2].astptr), (yy1vsp[-1].astptr), AST_NIL);
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 4169 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 177:
#line 1813 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.statval).token = SREAD_LONG_STAT;
        (yy1val.statval).tree  = gen_READ_LONG((yy1vsp[-3].astptr), (yy1vsp[-2].astptr), (yy1vsp[-1].astptr));
	treeStackDrop(3);
	treeStackPush((yy1val.statval).tree);
      }
#line 4180 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 178:
#line 1820 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.statval).token = SREAD_SHORT_STAT;
        (yy1val.statval).tree  = gen_READ_SHORT((yy1vsp[-2].astptr), coerceToLabel((yy1vsp[-1].astptr)), AST_NIL);
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 4191 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 179:
#line 1827 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.statval).token = SREAD_SHORT_STAT;
	(yy1val.statval).tree  = gen_READ_SHORT((yy1vsp[-4].astptr), coerceToLabel((yy1vsp[-3].astptr)), (yy1vsp[0].astptr));
	treeStackDrop(3);
	treeStackPush((yy1val.statval).tree);
      }
#line 4202 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 180:
#line 1834 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.statval).token = SWRITE_STAT;
        (yy1val.statval).tree  = gen_WRITE((yy1vsp[-2].astptr), (yy1vsp[-1].astptr), AST_NIL);
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 4213 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 181:
#line 1841 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.statval).token = SWRITE_STAT;
        (yy1val.statval).tree  = gen_WRITE((yy1vsp[-3].astptr), (yy1vsp[-2].astptr), (yy1vsp[-1].astptr));
	treeStackDrop(3);
	treeStackPush((yy1val.statval).tree);
      }
#line 4224 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 182:
#line 1848 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.statval).token = SPRINT_STAT;
        (yy1val.statval).tree  = gen_PRINT((yy1vsp[-2].astptr), coerceToLabel((yy1vsp[-1].astptr)), AST_NIL);
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 4235 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 183:
#line 1855 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.statval).token = SPRINT_STAT;
        (yy1val.statval).tree  = gen_PRINT((yy1vsp[-4].astptr), coerceToLabel((yy1vsp[-3].astptr)), (yy1vsp[-1].astptr));
	treeStackDrop(3);
	treeStackPush((yy1val.statval).tree);
      }
#line 4246 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 184:
#line 1865 "gram1.y" /* yacc.c:1646  */
    {
        iostmt = SBACKSPACE_LONG_STAT;
        (yy1val.astptr) = gen_BACKSPACE_LONG((yy1vsp[-1].astptr), AST_NIL);
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 4257 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 185:
#line 1872 "gram1.y" /* yacc.c:1646  */
    {
        iostmt = SREWIND_LONG_STAT;
        (yy1val.astptr) = gen_REWIND_LONG((yy1vsp[-1].astptr), AST_NIL);
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 4268 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 186:
#line 1879 "gram1.y" /* yacc.c:1646  */
    {
        iostmt = SENDFILE_LONG_STAT;
        (yy1val.astptr) = gen_ENDFILE_LONG((yy1vsp[-1].astptr), AST_NIL);
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 4279 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 187:
#line 1889 "gram1.y" /* yacc.c:1646  */
    {
        iostmt = SINQUIRE_STAT;
        (yy1val.astptr) = gen_INQUIRE((yy1vsp[-1].astptr), AST_NIL);
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 4290 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 188:
#line 1896 "gram1.y" /* yacc.c:1646  */
    {
        iostmt = SOPEN_STAT;
        (yy1val.astptr) = gen_OPEN((yy1vsp[-1].astptr), AST_NIL);
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 4301 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 189:
#line 1903 "gram1.y" /* yacc.c:1646  */
    {
        iostmt = SCLOSE_STAT;
        (yy1val.astptr) = gen_CLOSE((yy1vsp[-1].astptr), AST_NIL);
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 4312 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 190:
#line 1913 "gram1.y" /* yacc.c:1646  */
    {
        iostmt = SREAD_LONG_STAT;
      }
#line 4320 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 191:
#line 1920 "gram1.y" /* yacc.c:1646  */
    {
        iostmt = SWRITE_STAT;
      }
#line 4328 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 192:
#line 1927 "gram1.y" /* yacc.c:1646  */
    {
        iostmt = SPRINT_STAT;
      }
#line 4336 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 193:
#line 1934 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = (yy1vsp[-1].astptr);
      }
#line 4344 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 194:
#line 1938 "gram1.y" /* yacc.c:1646  */
    {
        FortTreeNode unit;

	unit = gen_UNIT_SPECIFY((yy1vsp[-1].astptr));
	ft_SetShow(unit, false);		/* shorthand */
        (yy1val.astptr) = list_create(unit);
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 4358 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 195:
#line 1948 "gram1.y" /* yacc.c:1646  */
    {
        FortTreeNode unit, fmt;

	unit = gen_UNIT_SPECIFY((yy1vsp[-3].astptr));
	ft_SetShow(unit, false);		/* shorthand */
	fmt  = gen_FMT_SPECIFY(coerceToLabel((yy1vsp[-1].astptr)));
	ft_SetShow(fmt, false);			/* shorthand */
        (yy1val.astptr) = list_insert_last(list_create(unit), fmt);
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 4374 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 196:
#line 1960 "gram1.y" /* yacc.c:1646  */
    {
        FortTreeNode unit;

	unit = gen_UNIT_SPECIFY((yy1vsp[-3].astptr));
	ft_SetShow(unit, false);		/* shorthand */
        (yy1val.astptr) = list_insert_first((yy1vsp[-1].astptr), unit);
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 4388 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 197:
#line 1970 "gram1.y" /* yacc.c:1646  */
    {
        FortTreeNode unit, fmt;

	unit = gen_UNIT_SPECIFY((yy1vsp[-5].astptr));
	ft_SetShow(unit, false);		/* shorthand */
	fmt  = gen_FMT_SPECIFY(coerceToLabel((yy1vsp[-3].astptr)));
	ft_SetShow(fmt, false);			/* shorthand */
        (yy1val.astptr) = list_insert_first(list_insert_first((yy1vsp[-1].astptr), fmt), unit);
	treeStackDrop(3);
	treeStackPush((yy1val.astptr));
      }
#line 4404 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 198:
#line 1985 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 4414 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 199:
#line 1991 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 4424 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 200:
#line 2000 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_STAR();
	treeStackPush((yy1val.astptr));
      }
#line 4433 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 201:
#line 2005 "gram1.y" /* yacc.c:1646  */
    {
        switch ((yy1vsp[-1].kwdval))
        { case IOUNIT:
            (yy1val.astptr) = gen_UNIT_SPECIFY((yy1vsp[0].astptr));
            break;
          case IOFMT:
            (yy1val.astptr) = gen_FMT_SPECIFY(coerceToLabel((yy1vsp[0].astptr)));
            break;
          case IOERR:
            (yy1val.astptr) = gen_ERR_SPECIFY(coerceToLabel((yy1vsp[0].astptr)));
            break;
          case IOEND:
            (yy1val.astptr) = gen_END_SPECIFY(coerceToLabel((yy1vsp[0].astptr)));
            break;
          case IOREC:
            (yy1val.astptr) = gen_REC_SPECIFY((yy1vsp[0].astptr));
            break;
          case IORECL:
            if (iostmt == SINQUIRE_STAT)
              (yy1val.astptr) = gen_RECL_QUERY((yy1vsp[0].astptr));
            else
              (yy1val.astptr) = gen_RECL_SPECIFY((yy1vsp[0].astptr));
            break;
          case IOFILE:
            (yy1val.astptr) = gen_FILE_SPECIFY((yy1vsp[0].astptr));
            break;
          case IOSTATUS:
            (yy1val.astptr) = gen_STATUS_SPECIFY((yy1vsp[0].astptr));
            break;
          case IOACCESS:
            if (iostmt == SINQUIRE_STAT)
              (yy1val.astptr) = gen_ACCESS_QUERY((yy1vsp[0].astptr));
            else
              (yy1val.astptr) = gen_ACCESS_SPECIFY((yy1vsp[0].astptr));
            break;
          case IOFORM:
            if (iostmt == SINQUIRE_STAT)
    	      (yy1val.astptr) = gen_FORM_QUERY((yy1vsp[0].astptr));
            else
              (yy1val.astptr) = gen_FORM_SPECIFY((yy1vsp[0].astptr));
            break;
          case IOBLANK:
            if (iostmt == SINQUIRE_STAT)
              (yy1val.astptr) = gen_BLANK_QUERY((yy1vsp[0].astptr));
            else
              (yy1val.astptr) = gen_BLANK_SPECIFY((yy1vsp[0].astptr));
            break;
          case IOEXIST:
            (yy1val.astptr) = gen_EXIST_QUERY((yy1vsp[0].astptr));
            break;
          case IOOPENED:
            (yy1val.astptr) = gen_OPENED_QUERY((yy1vsp[0].astptr));
            break;
          case IONUMBER:
            (yy1val.astptr) = gen_NUMBER_QUERY((yy1vsp[0].astptr));
            break;
          case IONAMED:
            (yy1val.astptr) = gen_NAMED_QUERY((yy1vsp[0].astptr));
            break;
          case IONAME:
            (yy1val.astptr) = gen_NAME_QUERY((yy1vsp[0].astptr));
            break;
          case IOSEQUENTIAL:
            (yy1val.astptr) = gen_SEQUENTIAL_QUERY((yy1vsp[0].astptr));
            break;
          case IODIRECT:
            (yy1val.astptr) = gen_DIRECT_QUERY((yy1vsp[0].astptr));
            break;
          case IOFORMATTED:
            (yy1val.astptr) = gen_FORMATTED_QUERY((yy1vsp[0].astptr));
            break;
          case IOUNFORMATTED:
            (yy1val.astptr) = gen_UNFORMATTED_QUERY((yy1vsp[0].astptr));
            break;
          case IONEXTREC:
            (yy1val.astptr) = gen_NEXTREC_QUERY((yy1vsp[0].astptr));
            break;
          case IOIOSTAT: 
            (yy1val.astptr) = gen_IOSTAT_QUERY((yy1vsp[0].astptr));
            break;
        }
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 4522 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 202:
#line 2090 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_kwd);
	treeStackPush((yy1val.astptr));
      }
#line 4531 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 203:
#line 2095 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_specify_kwd);
	treeStackPush((yy1val.astptr));
      }
#line 4540 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 204:
#line 2100 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_query_kwd);
	treeStackPush((yy1val.astptr));
      }
#line 4549 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 205:
#line 2108 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.kwdval) = lx1_IOKwd;
      }
#line 4557 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 207:
#line 2116 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_STAR();
	treeStackPush((yy1val.astptr));
      }
#line 4566 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 208:
#line 2121 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_lbl_ref);
	treeStackPush((yy1val.astptr));
      }
#line 4575 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 209:
#line 2126 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_unit);
	treeStackPush((yy1val.astptr));
      }
#line 4584 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 210:
#line 2131 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_format);
	treeStackPush((yy1val.astptr));
      }
#line 4593 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 211:
#line 2139 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 4603 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 212:
#line 2145 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 4613 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 213:
#line 2154 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = (yy1vsp[0].astptr);
      }
#line 4621 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 214:
#line 2159 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_IMPLIED_DO((yy1vsp[-8].astptr), (yy1vsp[-6].astptr), (yy1vsp[-4].astptr), (yy1vsp[-2].astptr), (yy1vsp[-1].astptr));
	treeStackDrop(5);
	treeStackPush((yy1val.astptr));
      }
#line 4631 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 215:
#line 2165 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_data);
	treeStackPush((yy1val.astptr));
      }
#line 4640 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 216:
#line 2173 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 4650 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 217:
#line 2179 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 4660 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 219:
#line 2189 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last(list_create((yy1vsp[-2].astptr)), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 4670 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 220:
#line 2195 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last(list_create((yy1vsp[-2].astptr)), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 4680 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 221:
#line 2201 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last(list_create((yy1vsp[-2].astptr)), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 4690 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 222:
#line 2207 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last(list_create((yy1vsp[-2].astptr)), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 4700 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 223:
#line 2213 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 4710 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 224:
#line 2219 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 4720 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 225:
#line 2225 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_data);
	treeStackPush((yy1val.astptr));
      }
#line 4729 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 227:
#line 2235 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_IMPLIED_DO(list_create((yy1vsp[-8].astptr)), (yy1vsp[-6].astptr), (yy1vsp[-4].astptr), (yy1vsp[-2].astptr), (yy1vsp[-1].astptr));
	treeStackDrop(5);
	treeStackPush((yy1val.astptr));
      }
#line 4739 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 228:
#line 2242 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_IMPLIED_DO(list_create((yy1vsp[-8].astptr)), (yy1vsp[-6].astptr), (yy1vsp[-4].astptr), (yy1vsp[-2].astptr), (yy1vsp[-1].astptr));
	treeStackDrop(5);
	treeStackPush((yy1val.astptr));
      }
#line 4749 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 229:
#line 2249 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_IMPLIED_DO((yy1vsp[-8].astptr), (yy1vsp[-6].astptr), (yy1vsp[-4].astptr), (yy1vsp[-2].astptr), (yy1vsp[-1].astptr));
	treeStackDrop(5);
	treeStackPush((yy1val.astptr));
      }
#line 4759 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 231:
#line 2259 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = (yy1vsp[-1].astptr);
        gen_put_parens((yy1val.astptr),1);
      }
#line 4768 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 234:
#line 2269 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_STAR();
	treeStackPush((yy1val.astptr));
      }
#line 4777 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 235:
#line 2274 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_PLUS((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 4787 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 236:
#line 2280 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_PLUS((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 4797 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 237:
#line 2286 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_TIMES((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 4807 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 238:
#line 2292 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_DIVIDE((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 4817 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 239:
#line 2298 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_EXPONENT((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 4827 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 240:
#line 2304 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = (yy1vsp[0].astptr);
      }
#line 4835 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 241:
#line 2308 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_UNARY_MINUS((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 4845 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 242:
#line 2314 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_CONCAT((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 4855 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 243:
#line 2320 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_unit);
	treeStackPush((yy1val.astptr));
      }
#line 4864 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 244:
#line 2325 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_format);
	treeStackPush((yy1val.astptr));
      }
#line 4873 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 245:
#line 2340 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SPAUSE_STAT;
        (yy1val.statval).tree  = gen_PAUSE((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 4884 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 246:
#line 2357 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SRETURN_STAT;
        (yy1val.statval).tree  = gen_RETURN((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 4895 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 247:
#line 2374 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SSTOP_STAT;
        (yy1val.statval).tree  = gen_STOP((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 4906 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 248:
#line 2400 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SERROR_STAT;
        lx1_Flush();
      }
#line 4915 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 249:
#line 2415 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token   = SSTMT_PH_STAT;
        (yy1val.statval).part[0] = ph_from_mtype(GEN_stmt);
	treeStackPush((yy1val.statval).part[0]);
      }
#line 4925 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 250:
#line 2421 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token   = SSPECIFICATION_STMT_PH_STAT;
        (yy1val.statval).part[0] = ph_from_mtype(GEN_specification_stmt);
	treeStackPush((yy1val.statval).part[0]);
      }
#line 4935 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 251:
#line 2427 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token   = SCONTROL_STMT_PH_STAT;
        (yy1val.statval).part[0] = ph_from_mtype(GEN_control_stmt);
	treeStackPush((yy1val.statval).part[0]);
      }
#line 4945 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 252:
#line 2433 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token   = SIO_STMT_PH_STAT;
        (yy1val.statval).part[0] = ph_from_mtype(GEN_io_stmt);
	treeStackPush((yy1val.statval).part[0]);
      }
#line 4955 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 253:
#line 2439 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token   = SPARASCOPE_STMT_PH_STAT;
        (yy1val.statval).part[0] = ph_from_mtype(GEN_parascope_stmt);
	treeStackPush((yy1val.statval).part[0]);
      }
#line 4965 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 254:
#line 2445 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token   = SDEBUG_STMT_PH_STAT;
        (yy1val.statval).part[0] = ph_from_mtype(GEN_debug_stmt);
	treeStackPush((yy1val.statval).part[0]);
      }
#line 4975 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 255:
#line 2470 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SAT_STAT;
        (yy1val.statval).tree  = gen_AT((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 4986 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 256:
#line 2487 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token   = SDEBUG_STAT;
        (yy1val.statval).part[0] = (yy1vsp[-1].astptr);
        (yy1val.statval).part[1] = (yy1vsp[0].astptr);
      }
#line 4996 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 257:
#line 2496 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[-1].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 5006 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 258:
#line 2502 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-4].astptr), (yy1vsp[-1].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5016 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 259:
#line 2511 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_UNIT((yy1vsp[-1].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 5026 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 260:
#line 2517 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.astptr) = gen_SUBCHK(AST_NIL);
	treeStackPush((yy1val.astptr));
      }
#line 5035 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 261:
#line 2522 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.astptr) = gen_SUBCHK((yy1vsp[-1].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 5045 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 262:
#line 2528 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_TRACE();
	treeStackPush((yy1val.astptr));
      }
#line 5054 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 263:
#line 2533 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_INIT(AST_NIL);
	treeStackPush((yy1val.astptr));
      }
#line 5063 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 264:
#line 2538 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_INIT((yy1vsp[-1].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 5073 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 265:
#line 2544 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_SUBTRACE();
	treeStackPush((yy1val.astptr));
      }
#line 5082 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 266:
#line 2549 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_option);
	treeStackPush((yy1val.astptr));
      }
#line 5091 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 267:
#line 2564 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.statval).token = STRACEOFF_STAT;
	(yy1val.statval).tree  = gen_TRACEOFF((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.statval).tree);
      }
#line 5102 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 268:
#line 2581 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.statval).token = STRACEON_STAT;
	(yy1val.statval).tree  = gen_TRACEON((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.statval).tree);
      }
#line 5113 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 269:
#line 2607 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.statval).token = SCLEAR_STAT;
	(yy1val.statval).tree  = gen_CLEAR((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 5124 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 270:
#line 2625 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SSET_BARRIER_STAT;
        (yy1val.statval).tree  = gen_SET_BARRIER((yy1vsp[-3].astptr), (yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(3);
	treeStackPush((yy1val.statval).tree);
      }
#line 5135 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 271:
#line 2642 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SBLOCK_STAT;
        (yy1val.statval).tree  = gen_BLOCK((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 5146 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 272:
#line 2659 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SDOALL_STAT;
	(yy1val.statval).part[0] = (yy1vsp[-1].astptr);
	(yy1val.statval).part[1] = AST_NIL;
        (yy1val.statval).part[2] = (yy1vsp[0].astptr);
	treeStackPush((yy1val.statval).part[1]);
      }
#line 5158 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 273:
#line 2667 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SDOALL_LABEL_STAT;
	(yy1val.statval).part[0] = (yy1vsp[-3].astptr);
	(yy1val.statval).part[1] = (yy1vsp[-2].astptr);
        (yy1val.statval).part[2] = (yy1vsp[0].astptr);
        ft_SetComma((yy1val.statval).part[1], (yy1vsp[-1].boolean));
      }
#line 5170 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 274:
#line 2675 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SEND_ALL_STAT;
	(yy1val.statval).part[0] = (yy1vsp[0].astptr);
      }
#line 5179 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 275:
#line 2683 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_INDUCTIVE((yy1vsp[-5].astptr), (yy1vsp[-3].astptr), (yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(4);
	treeStackPush((yy1val.astptr));
      }
#line 5189 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 276:
#line 2689 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_loop_control);
	treeStackPush((yy1val.astptr));
      }
#line 5198 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 277:
#line 2704 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.statval).token = SLOCK_STAT;
	(yy1val.statval).tree  = gen_LOCK((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 5209 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 278:
#line 2721 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SPARBEGIN_STAT;
        (yy1val.statval).part[0] = (yy1vsp[0].astptr);
      }
#line 5218 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 279:
#line 2726 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SPARALLEL_STAT;
        (yy1val.statval).part[0] = (yy1vsp[-1].astptr);
      }
#line 5227 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 280:
#line 2731 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SPARBEGIN_PID_STAT;
        (yy1val.statval).part[0] = (yy1vsp[-5].astptr);
        (yy1val.statval).part[1] = (yy1vsp[-3].astptr);
        (yy1val.statval).part[2] = (yy1vsp[-1].astptr);
      }
#line 5238 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 281:
#line 2738 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SPARALLEL_PID_STAT;
        (yy1val.statval).part[0] = (yy1vsp[-4].astptr);
        (yy1val.statval).part[1] = (yy1vsp[-1].astptr);
      }
#line 5248 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 282:
#line 2744 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SOTHER_PROCESSES_STAT;
        (yy1val.statval).part[0] = (yy1vsp[-1].astptr);
      }
#line 5257 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 283:
#line 2749 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SPAREND_STAT;
        (yy1val.statval).part[0] = (yy1vsp[0].astptr);
      }
#line 5266 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 284:
#line 2764 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.statval).token = SPOST_STAT;
	(yy1val.statval).tree  = gen_POST((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 5277 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 285:
#line 2774 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_POSTING((yy1vsp[-3].astptr), (yy1vsp[-1].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5287 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 286:
#line 2780 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_posting);
        treeStackPush((yy1val.astptr));
      }
#line 5296 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 287:
#line 2788 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = AST_NIL;
	treeStackPush((yy1val.astptr));
      }
#line 5305 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 288:
#line 2793 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_POST_TO((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 5315 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 289:
#line 2799 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_POST_INC((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 5325 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 290:
#line 2805 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_post_expr);
        treeStackPush((yy1val.astptr));
      }
#line 5334 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 291:
#line 2820 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = STASK_STAT;
        (yy1val.statval).tree  = gen_TASK((yy1vsp[-2].astptr), (yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	ft_SetShow((yy1val.statval).tree, false);		/* shorthand */
	treeStackDrop(3);
	treeStackPush((yy1val.statval).tree);
      }
#line 5346 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 292:
#line 2828 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = STASK_STAT;
        (yy1val.statval).tree  = gen_TASK((yy1vsp[-2].astptr), (yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(3);
	treeStackPush((yy1val.statval).tree);
      }
#line 5357 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 293:
#line 2838 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = AST_NIL;
        treeStackPush((yy1val.astptr));
      }
#line 5366 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 294:
#line 2843 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = (yy1vsp[0].astptr);
      }
#line 5374 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 295:
#line 2857 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.statval).token = STASK_COMMON_STAT;
	(yy1val.statval).tree  = gen_TASK_COMMON((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 5385 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 296:
#line 2874 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.statval).token = SUNLOCK_STAT;
	(yy1val.statval).tree  = gen_UNLOCK((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 5396 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 297:
#line 2891 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.statval).token = SWAIT_STAT;
	(yy1val.statval).tree  = gen_WAIT((yy1vsp[-1].astptr), (yy1vsp[0].astptr), AST_NIL);
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 5407 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 298:
#line 2899 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.statval).token = SWAIT_STAT;
	(yy1val.statval).tree  = gen_WAIT((yy1vsp[-6].astptr), (yy1vsp[-5].astptr), (yy1vsp[0].astptr));
	treeStackDrop(3);
	treeStackPush((yy1val.statval).tree);
      }
#line 5418 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 299:
#line 2925 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SPARALLELLOOP_STAT;
	(yy1val.statval).part[0] = (yy1vsp[-1].astptr);
	(yy1val.statval).part[1] = AST_NIL;
        (yy1val.statval).part[2] = (yy1vsp[0].astptr);
	treeStackPush((yy1val.statval).part[1]);
      }
#line 5430 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 300:
#line 2934 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SPARALLELLOOP_LABEL_STAT;
	(yy1val.statval).part[0] = (yy1vsp[-3].astptr);
	(yy1val.statval).part[1] = (yy1vsp[-2].astptr);
        (yy1val.statval).part[2] = (yy1vsp[0].astptr);
        ft_SetComma((yy1val.statval).part[1], (yy1vsp[-1].boolean));
      }
#line 5442 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 301:
#line 2942 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SEND_LOOP_STAT;
	(yy1val.statval).part[0] = (yy1vsp[0].astptr);
      }
#line 5451 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 302:
#line 2950 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_INDUCTIVE((yy1vsp[-5].astptr), (yy1vsp[-3].astptr), (yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(4);
	treeStackPush((yy1val.astptr));
      }
#line 5461 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 303:
#line 2956 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_loop_control);
	treeStackPush((yy1val.astptr));
      }
#line 5470 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 304:
#line 2971 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SPRIVATE_STAT;
        (yy1val.statval).tree  = gen_PRIVATE((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 5481 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 305:
#line 2988 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SSTOP_LOOP_STAT;
        (yy1val.statval).tree  = gen_STOPLOOP((yy1vsp[-1].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 5492 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 306:
#line 2995 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token = SSTOP_LOOP_STAT;
        (yy1val.statval).tree  = gen_STOPLOOP((yy1vsp[0].astptr), AST_NIL);
	treeStackDrop(1);
	treeStackPush((yy1val.statval).tree);
      }
#line 5503 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 307:
#line 3020 "gram1.y" /* yacc.c:1646  */
    {
       (yy1val.astptr) = gen_ARRAY_DECL_LEN((yy1vsp[0].astptr), AST_NIL, AST_NIL, AST_NIL);
       treeStackDrop(1);
       treeStackPush((yy1val.astptr));
     }
#line 5513 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 309:
#line 3029 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 5523 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 310:
#line 3035 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5533 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 311:
#line 3043 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.statval).token = SALLOCATABLE_STAT;
	(yy1val.statval).tree = gen_ALLOCATABLE((yy1vsp[-1].astptr),(yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 5544 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 312:
#line 3058 "gram1.y" /* yacc.c:1646  */
    {
	/* ASSERT name of $2 is AST_NIL */
          gen_SUBSCRIPT_put_name((yy1vsp[0].astptr), (yy1vsp[-1].astptr));

        (yy1val.astptr) = (yy1vsp[0].astptr);
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5557 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 313:
#line 3069 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 5567 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 314:
#line 3075 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));	
      }
#line 5577 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 315:
#line 3083 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.statval).token = SALLOCATE_STAT;
	(yy1val.statval).tree = gen_ALLOCATE((yy1vsp[-3].astptr),(yy1vsp[-1].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 5588 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 316:
#line 3099 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 5598 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 317:
#line 3105 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5608 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 318:
#line 3113 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.statval).token = SDEALLOCATE_STAT;
	(yy1val.statval).tree = gen_DEALLOCATE((yy1vsp[-3].astptr),(yy1vsp[-1].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.statval).tree);
      }
#line 5619 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 319:
#line 3130 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token   = SWHERE_STAT;
        if( (yy1vsp[0].statval).token == SSTMT_PH_STAT )
          (yy1val.statval).tree    = gen_WHERE((yy1vsp[-4].astptr), (yy1vsp[-2].astptr), list_create((yy1vsp[0].statval).part[0]));
        else
          (yy1val.statval).tree    = gen_WHERE((yy1vsp[-4].astptr), (yy1vsp[-2].astptr), list_create((yy1vsp[0].statval).tree));
	treeStackDrop(3);
	treeStackPush((yy1val.statval).tree);
      }
#line 5633 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 320:
#line 3140 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token   = SWHERE_BLOCK_STAT;
        (yy1val.statval).part[0] = (yy1vsp[-3].astptr);
        (yy1val.statval).part[1] = (yy1vsp[-1].astptr);
      }
#line 5643 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 321:
#line 3146 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token   = SELSE_WHERE_STAT;
	(yy1val.statval).part[0] = (yy1vsp[0].astptr);
	(yy1val.statval).part[1] = AST_NIL;
	treeStackPush((yy1val.statval).part[1]);
      }
#line 5654 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 322:
#line 3153 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.statval).token   = SEND_WHERE_STAT;
	(yy1val.statval).part[0] = (yy1vsp[0].astptr);
      }
#line 5663 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 323:
#line 3171 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 5673 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 324:
#line 3177 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5683 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 325:
#line 3186 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = AST_NIL;
	treeStackPush((yy1val.astptr));
      }
#line 5692 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 327:
#line 3195 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = AST_NIL;
        treeStackPush((yy1val.astptr));
      }
#line 5701 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 328:
#line 3200 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = (yy1vsp[0].astptr);
      }
#line 5709 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 330:
#line 3208 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = (yy1vsp[-1].astptr);
        gen_put_parens((yy1val.astptr), 1);
      }
#line 5718 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 334:
#line 3219 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_PLUS((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5728 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 335:
#line 3225 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_MINUS((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5738 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 336:
#line 3231 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_TIMES((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5748 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 337:
#line 3237 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_DIVIDE((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5758 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 338:
#line 3243 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_EXPONENT((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5768 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 339:
#line 3249 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = (yy1vsp[0].astptr);
      }
#line 5776 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 340:
#line 3253 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_UNARY_MINUS((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 5786 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 341:
#line 3259 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_CONCAT((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5796 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 342:
#line 3265 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_EQ((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5806 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 343:
#line 3271 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_GT((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5816 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 344:
#line 3277 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_LT((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5826 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 345:
#line 3283 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_GE((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5836 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 346:
#line 3289 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_LE((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5846 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 347:
#line 3295 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_NE((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5856 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 348:
#line 3301 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_EQV((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5866 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 349:
#line 3307 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_NEQV((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5876 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 350:
#line 3313 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_OR((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5886 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 351:
#line 3319 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_BINARY_AND((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5896 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 352:
#line 3325 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_UNARY_NOT((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 5906 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 353:
#line 3331 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_formal);
	treeStackPush((yy1val.astptr));
      }
#line 5915 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 355:
#line 3340 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_expr);
	treeStackPush((yy1val.astptr));
      }
#line 5924 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 356:
#line 3345 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_arith_expr);
	treeStackPush((yy1val.astptr));
      }
#line 5933 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 357:
#line 3350 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_string_expr);
	treeStackPush((yy1val.astptr));
      }
#line 5942 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 358:
#line 3355 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_relational_expr);
	treeStackPush((yy1val.astptr));
      }
#line 5951 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 359:
#line 3360 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_logical_expr);
	treeStackPush((yy1val.astptr));
      }
#line 5960 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 360:
#line 3368 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 5970 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 361:
#line 3374 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5980 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 363:
#line 3393 "gram1.y" /* yacc.c:1646  */
    {
	/* ASSERT name of $2 is AST_NIL */
          gen_SUBSCRIPT_put_name((yy1vsp[0].astptr), (yy1vsp[-1].astptr));

        (yy1val.astptr) = (yy1vsp[0].astptr);
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 5993 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 364:
#line 3402 "gram1.y" /* yacc.c:1646  */
    {
	/* ASSERT name of $2 and $3 are AST_NIL */
          gen_SUBSCRIPT_put_name((yy1vsp[-1].astptr), (yy1vsp[-2].astptr));
          gen_SUBSTRING_put_substring_name((yy1vsp[0].astptr), (yy1vsp[-1].astptr));

        (yy1val.astptr) = (yy1vsp[0].astptr);
	treeStackDrop(3);
	treeStackPush((yy1val.astptr));
      }
#line 6007 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 365:
#line 3412 "gram1.y" /* yacc.c:1646  */
    {
        FortTreeNode name;

	name = ph_from_mtype(GEN_string_var);

	/* ASSERT name of $2 is AST_NIL */
          gen_SUBSTRING_put_substring_name((yy1vsp[0].astptr), name);

	(yy1val.astptr) = (yy1vsp[0].astptr);
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 6024 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 366:
#line 3425 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_var);
	treeStackPush((yy1val.astptr));
      }
#line 6033 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 367:
#line 3430 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_invocation);
	treeStackPush((yy1val.astptr));
      }
#line 6042 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 368:
#line 3438 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_SUBSTRING(AST_NIL, (yy1vsp[-3].astptr), (yy1vsp[-1].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 6052 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 369:
#line 3447 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_SUBSCRIPT(AST_NIL, list_create(AST_NIL));
	treeStackPush((yy1val.astptr));
      }
#line 6061 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 370:
#line 3452 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_SUBSCRIPT(AST_NIL, (yy1vsp[-1].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 6071 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 372:
#line 3462 "gram1.y" /* yacc.c:1646  */
    {
       (yy1val.astptr) = gen_TRIPLET(AST_NIL, (yy1vsp[-1].astptr), AST_NIL);
       treeStackDrop(1);
       treeStackPush((yy1val.astptr));       
     }
#line 6081 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 373:
#line 3468 "gram1.y" /* yacc.c:1646  */
    {
       (yy1val.astptr) = gen_TRIPLET((yy1vsp[-2].astptr), (yy1vsp[0].astptr), AST_NIL);
       treeStackDrop(2);
       treeStackPush((yy1val.astptr));       
     }
#line 6091 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 374:
#line 3474 "gram1.y" /* yacc.c:1646  */
    {
       (yy1val.astptr) = gen_TRIPLET((yy1vsp[-4].astptr), (yy1vsp[-2].astptr), (yy1vsp[0].astptr));
       treeStackDrop(3);
       treeStackPush((yy1val.astptr));       
     }
#line 6101 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 375:
#line 3480 "gram1.y" /* yacc.c:1646  */
    {
       (yy1val.astptr) = gen_TRIPLET(AST_NIL, (yy1vsp[-2].astptr), (yy1vsp[0].astptr));
       treeStackDrop(2);
       treeStackPush((yy1val.astptr));       
     }
#line 6111 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 376:
#line 3486 "gram1.y" /* yacc.c:1646  */
    {
       (yy1val.astptr) = gen_TRIPLET(AST_NIL, AST_NIL, (yy1vsp[0].astptr));
       treeStackDrop(1);
       treeStackPush((yy1val.astptr));       
     }
#line 6121 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 377:
#line 3492 "gram1.y" /* yacc.c:1646  */
    {
       (yy1val.astptr) = gen_TRIPLET(AST_NIL, (yy1vsp[0].astptr), AST_NIL);
       treeStackDrop(1);
       treeStackPush((yy1val.astptr));       
     }
#line 6131 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 378:
#line 3498 "gram1.y" /* yacc.c:1646  */
    {
       (yy1val.astptr) = gen_TRIPLET(AST_NIL, AST_NIL, AST_NIL);
       treeStackPush((yy1val.astptr));       
     }
#line 6140 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 379:
#line 3505 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 6150 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 380:
#line 3511 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 6160 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 381:
#line 3519 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = AST_NIL;
	treeStackPush((yy1val.astptr));
      }
#line 6169 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 383:
#line 3528 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.astptr) = gen_CONSTANT();
	gen_put_text((yy1val.astptr), ".true.", STR_CONSTANT_LOGICAL);
	treeStackPush((yy1val.astptr));
      }
#line 6179 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 384:
#line 3534 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.astptr) = gen_CONSTANT();
	gen_put_text((yy1val.astptr), ".false.", STR_CONSTANT_LOGICAL);
	treeStackPush((yy1val.astptr));
      }
#line 6189 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 385:
#line 3540 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.astptr) = gen_CONSTANT();
	gen_put_text((yy1val.astptr), lx1_Token, STR_CONSTANT_CHARACTER);
	treeStackPush((yy1val.astptr));
      }
#line 6199 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 386:
#line 3546 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.astptr) = gen_CONSTANT();
	gen_put_text((yy1val.astptr), lx1_Token, STR_CONSTANT_INTEGER);
	treeStackPush((yy1val.astptr));
      }
#line 6209 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 387:
#line 3552 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.astptr) = gen_CONSTANT();
	gen_put_text((yy1val.astptr), lx1_Token, STR_CONSTANT_REAL);
	treeStackPush((yy1val.astptr));
      }
#line 6219 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 388:
#line 3558 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.astptr) = gen_CONSTANT();
	gen_put_text((yy1val.astptr), lx1_Token, STR_CONSTANT_DOUBLE_PRECISION);
	treeStackPush((yy1val.astptr));
      }
#line 6229 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 389:
#line 3564 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.astptr) = gen_CONSTANT();
	gen_put_text((yy1val.astptr), lx1_Token, STR_CONSTANT_HEX);
	treeStackPush((yy1val.astptr));
      }
#line 6239 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 390:
#line 3570 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_constant);
	treeStackPush((yy1val.astptr));
      }
#line 6248 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 391:
#line 3578 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_COMPLEX_CONSTANT((yy1vsp[-3].astptr), (yy1vsp[-1].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 6258 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 392:
#line 3596 "gram1.y" /* yacc.c:1646  */
    {
        /* Dummy nonterminal, packages statement number into label node */
        char buf[7];

	if( lx1_PlaceholderStatLabel )
          { (yy1val.astptr) = ph_from_mtype(GEN_lbl_def);
            lx1_PlaceholderStatLabel = false;
          }
        else if( lx1_StatNumber == 0 )
          { (yy1val.astptr) = AST_NIL;
          }
	else
          { (yy1val.astptr) = gen_LABEL_DEF();
	    (void) sprintf (buf, "%ld", lx1_StatNumber);
            gen_put_text((yy1val.astptr), buf, STR_LABEL_DEF);
          }
        lx1_StatNumber = 0;
        treeStackPush((yy1val.astptr));
      }
#line 6282 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 393:
#line 3619 "gram1.y" /* yacc.c:1646  */
    {
        if( lx1_StatNumber != 0 )
          { yy1error("Statement number not allowed");
	    YY1ERROR;
          }
      }
#line 6293 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 394:
#line 3629 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 6303 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 395:
#line 3635 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 6313 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 396:
#line 3644 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.astptr) = gen_LABEL_REF();
        gen_put_text((yy1val.astptr), lx1_Token, STR_LABEL_REF);
	treeStackPush((yy1val.astptr));
      }
#line 6323 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 397:
#line 3650 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = ph_from_mtype(GEN_lbl_ref);
	treeStackPush((yy1val.astptr));
      }
#line 6332 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 398:
#line 3658 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_create((yy1vsp[0].astptr));
	treeStackDrop(1);
	treeStackPush((yy1val.astptr));
      }
#line 6342 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 399:
#line 3664 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = list_insert_last((yy1vsp[-2].astptr), (yy1vsp[0].astptr));
	treeStackDrop(2);
	treeStackPush((yy1val.astptr));
      }
#line 6352 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 400:
#line 3673 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = AST_NIL;
	treeStackPush((yy1val.astptr));
      }
#line 6361 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 402:
#line 3682 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.astptr) = gen_IDENTIFIER();
        gen_put_text((yy1val.astptr), lx1_Token, STR_IDENTIFIER);
	treeStackPush((yy1val.astptr));
      }
#line 6371 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 403:
#line 3688 "gram1.y" /* yacc.c:1646  */
    {
	(yy1val.astptr) = ph_from_mtype(GEN_name);
	treeStackPush((yy1val.astptr));
      }
#line 6380 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 404:
#line 3696 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.boolean) = false;
      }
#line 6388 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 405:
#line 3700 "gram1.y" /* yacc.c:1646  */
    {
        (yy1val.boolean) = true;
      }
#line 6396 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 406:
#line 3716 "gram1.y" /* yacc.c:1646  */
    {
        lx1_IntOnly = true;
      }
#line 6404 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 407:
#line 3723 "gram1.y" /* yacc.c:1646  */
    {
        lx1_IntOnly = false;
      }
#line 6412 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 408:
#line 3730 "gram1.y" /* yacc.c:1646  */
    {
        lx1_InIOControl = true;
      }
#line 6420 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 409:
#line 3737 "gram1.y" /* yacc.c:1646  */
    {
        lx1_InIOControl = false;
      }
#line 6428 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 410:
#line 3744 "gram1.y" /* yacc.c:1646  */
    {
        lx1_NeedKwd = true;
      }
#line 6436 "gram1.tab.c" /* yacc.c:1646  */
    break;

  case 411:
#line 3751 "gram1.y" /* yacc.c:1646  */
    {
        lx1_NeedKwd = false;
      }
#line 6444 "gram1.tab.c" /* yacc.c:1646  */
    break;


#line 6448 "gram1.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yy1char, and that requires
     that yy1token be updated with the new translation.  We take the
     approach of translating immediately before every use of yy1token.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YY1ABORT, YY1ACCEPT, or YY1ERROR immediately after altering yy1char or
     if it invokes YY1BACKUP.  In the case of YY1ABORT or YY1ACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YY1ERROR or YY1BACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY1_SYMBOL_PRINT ("-> $$ =", yy1r1[yy1n], &yy1val, &yy1loc);

  YY1POPSTACK (yy1len);
  yy1len = 0;
  YY1_STACK_PRINT (yy1ss, yy1ssp);

  *++yy1vsp = yy1val;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yy1n = yy1r1[yy1n];

  yy1state = yy1pgoto[yy1n - YY1NTOKENS] + *yy1ssp;
  if (0 <= yy1state && yy1state <= YY1LAST && yy1check[yy1state] == *yy1ssp)
    yy1state = yy1table[yy1state];
  else
    yy1state = yy1defgoto[yy1n - YY1NTOKENS];

  goto yy1newstate;


/*--------------------------------------.
| yy1errlab -- here on detecting error.  |
`--------------------------------------*/
yy1errlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yy1token = yy1char == YY1EMPTY ? YY1EMPTY : YY1TRANSLATE (yy1char);

  /* If not already recovering from an error, report this error.  */
  if (!yy1errstatus)
    {
      ++yy1nerrs;
#if ! YY1ERROR_VERBOSE
      yy1error (YY1_("syntax error"));
#else
# define YY1SYNTAX_ERROR yy1syntax_error (&yy1msg_alloc, &yy1msg, \
                                        yy1ssp, yy1token)
      {
        char const *yy1msgp = YY1_("syntax error");
        int yy1syntax_error_status;
        yy1syntax_error_status = YY1SYNTAX_ERROR;
        if (yy1syntax_error_status == 0)
          yy1msgp = yy1msg;
        else if (yy1syntax_error_status == 1)
          {
            if (yy1msg != yy1msgbuf)
              YY1STACK_FREE (yy1msg);
            yy1msg = (char *) YY1STACK_ALLOC (yy1msg_alloc);
            if (!yy1msg)
              {
                yy1msg = yy1msgbuf;
                yy1msg_alloc = sizeof yy1msgbuf;
                yy1syntax_error_status = 2;
              }
            else
              {
                yy1syntax_error_status = YY1SYNTAX_ERROR;
                yy1msgp = yy1msg;
              }
          }
        yy1error (yy1msgp);
        if (yy1syntax_error_status == 2)
          goto yy1exhaustedlab;
      }
# undef YY1SYNTAX_ERROR
#endif
    }



  if (yy1errstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yy1char <= YY1EOF)
        {
          /* Return failure if at end of input.  */
          if (yy1char == YY1EOF)
            YY1ABORT;
        }
      else
        {
          yy1destruct ("Error: discarding",
                      yy1token, &yy1lval);
          yy1char = YY1EMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yy1errlab1;


/*---------------------------------------------------.
| yy1errorlab -- error raised explicitly by YY1ERROR.  |
`---------------------------------------------------*/
yy1errorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YY1ERROR and the label yy1errorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yy1errorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YY1ERROR.  */
  YY1POPSTACK (yy1len);
  yy1len = 0;
  YY1_STACK_PRINT (yy1ss, yy1ssp);
  yy1state = *yy1ssp;
  goto yy1errlab1;


/*-------------------------------------------------------------.
| yy1errlab1 -- common code for both syntax error and YY1ERROR.  |
`-------------------------------------------------------------*/
yy1errlab1:
  yy1errstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yy1n = yy1pact[yy1state];
      if (!yy1pact_value_is_default (yy1n))
        {
          yy1n += YY1TERROR;
          if (0 <= yy1n && yy1n <= YY1LAST && yy1check[yy1n] == YY1TERROR)
            {
              yy1n = yy1table[yy1n];
              if (0 < yy1n)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yy1ssp == yy1ss)
        YY1ABORT;


      yy1destruct ("Error: popping",
                  yy1stos[yy1state], yy1vsp);
      YY1POPSTACK (1);
      yy1state = *yy1ssp;
      YY1_STACK_PRINT (yy1ss, yy1ssp);
    }

  YY1_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yy1vsp = yy1lval;
  YY1_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY1_SYMBOL_PRINT ("Shifting", yy1stos[yy1n], yy1vsp, yy1lsp);

  yy1state = yy1n;
  goto yy1newstate;


/*-------------------------------------.
| yy1acceptlab -- YY1ACCEPT comes here.  |
`-------------------------------------*/
yy1acceptlab:
  yy1result = 0;
  goto yy1return;

/*-----------------------------------.
| yy1abortlab -- YY1ABORT comes here.  |
`-----------------------------------*/
yy1abortlab:
  yy1result = 1;
  goto yy1return;

#if !defined yy1overflow || YY1ERROR_VERBOSE
/*-------------------------------------------------.
| yy1exhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yy1exhaustedlab:
  yy1error (YY1_("memory exhausted"));
  yy1result = 2;
  /* Fall through.  */
#endif

yy1return:
  if (yy1char != YY1EMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yy1token = YY1TRANSLATE (yy1char);
      yy1destruct ("Cleanup: discarding lookahead",
                  yy1token, &yy1lval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YY1ABORT or YY1ACCEPT.  */
  YY1POPSTACK (yy1len);
  YY1_STACK_PRINT (yy1ss, yy1ssp);
  while (yy1ssp != yy1ss)
    {
      yy1destruct ("Cleanup: popping",
                  yy1stos[*yy1ssp], yy1vsp);
      YY1POPSTACK (1);
    }
#ifndef yy1overflow
  if (yy1ss != yy1ssa)
    YY1STACK_FREE (yy1ss);
#endif
#if YY1ERROR_VERBOSE
  if (yy1msg != yy1msgbuf)
    YY1STACK_FREE (yy1msg);
#endif
  return yy1result;
}
#line 3760 "gram1.y" /* yacc.c:1906  */





/*ARGSUSED*/
void parse1(FortTextTree ftt, TextString text)
{
  lx1_SetScan(text);
  lx1_NeedKwd = false;
  lx1_IntOnly = false;
  lx1_InIOControl = false;
  lx1_StatNumber = 0;
  (void) yy1parse();
  treeStackCheck();
}




static
int yy1lex()
{
  int token;    /* simplifies debugging */

  token = lx1_NextToken();
  return token;
}




void yy1error(const char *s)
{
  /* save just the first error message to occur */
  if( fp1_error == nil )  fp1_error = ssave(s);
}




static
FortTreeNode coerceToLabel(FortTreeNode node)
{
  FortTreeNode New;

  if( is_constant(node)  &&
              str_get_type(gen_get_symbol(node)) == STR_CONSTANT_INTEGER )
    { /* this integer constant should be a label--coerce it */
        New = gen_LABEL_REF();
        gen_put_text(New, gen_get_text(node), STR_LABEL_REF);
        tree_free(node);
        return New;
    }
  else
    return node;
}
