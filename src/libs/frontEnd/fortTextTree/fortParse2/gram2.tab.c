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

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 17 "gram2.y" /* yacc.c:339  */


#define gram2_h			/* already have yacc-generated decls */

#ifndef general_h
#include <libs/support/misc/general.h>
#endif

#include <libs/frontEnd/fortTextTree/FortTextTree.i>

#include <libs/frontEnd/fortTextTree/fortParse2/FortParse2.i>

#line 50 "gram2.y" /* yacc.c:339  */



/* Special Values */

#define MISSING     ((FortTreeNode) -1)
#define IMPLIED     ((FortTreeNode) -2)


/* Forward declarations */

STATIC(FortTreeNode, misplaced,(fx_StatToken st, char *what, char *prev));
STATIC(fx_EndingStat, missing,(char *what));
STATIC(void, setLineTags1, (FortTreeNode node, fx_StatToken st));
STATIC(void, setLineTags2, (FortTreeNode node, fx_StatToken st, fx_EndingStat endval));


#line 97 "gram2.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "gram2.tab.h".  */
#ifndef YY_YY_GRAM2_TAB_H_INCLUDED
# define YY_YY_GRAM2_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    SGOAL_MODULE = 258,
    SGOAL_UNIT = 259,
    SGOAL_DO = 260,
    SGOAL_DOALL = 261,
    SGOAL_IF = 262,
    SGOAL_PAR = 263,
    SGOAL_PARALLELLOOP = 264,
    SGOAL_STAT = 265,
    SGOAL_WHERE = 266,
    SARITHMETIC_IF_STAT = 267,
    SASSIGNED_GOTO_STAT = 268,
    SASSIGNMENT_STAT = 269,
    SASSIGN_STAT = 270,
    SAT_STAT = 271,
    SBACKSPACE_LONG_STAT = 272,
    SBACKSPACE_SHORT_STAT = 273,
    SBLOCK_DATA_STAT = 274,
    SBLOCK_STAT = 275,
    SCALL_STAT = 276,
    SCLEAR_STAT = 277,
    SCLOSE_STAT = 278,
    SCOMMENT_STAT = 279,
    SCOMMON_STAT = 280,
    SCOMPUTED_GOTO_STAT = 281,
    SCONTINUE_STAT = 282,
    SDATA_STAT = 283,
    SDEBUG_STAT = 284,
    SDIMENSION_STAT = 285,
    SDOALL_LABEL_STAT = 286,
    SDOALL_STAT = 287,
    SDO_LABEL_STAT = 288,
    SDO_STAT = 289,
    SELSE_IF_STAT = 290,
    SELSE_STAT = 291,
    SENDFILE_LONG_STAT = 292,
    SENDFILE_SHORT_STAT = 293,
    SEND_ALL_STAT = 294,
    SEND_DO_STAT = 295,
    SEND_DO_IMPLIED = 296,
    SEND_IF_STAT = 297,
    SEND_LOOP_STAT = 298,
    SEND_STAT = 299,
    SENTRY_STAT = 300,
    SEQUIVALENCE_STAT = 301,
    SERROR_STAT = 302,
    SEXTERNAL_STAT = 303,
    SFORMAT_STAT = 304,
    SFUNCTION_STAT = 305,
    SGOTO_STAT = 306,
    SIF_STAT = 307,
    SIMPLICIT_STAT = 308,
    SINQUIRE_STAT = 309,
    SINTRINSIC_STAT = 310,
    SLOCK_STAT = 311,
    SLOGICAL_IF_STAT = 312,
    SOPEN_STAT = 313,
    SOTHER_PROCESSES_STAT = 314,
    SPARALLEL_PID_STAT = 315,
    SPARALLEL_STAT = 316,
    SPARALLELLOOP_STAT = 317,
    SPARALLELLOOP_LABEL_STAT = 318,
    SPARAMETER_STAT = 319,
    SPARBEGIN_PID_STAT = 320,
    SPARBEGIN_STAT = 321,
    SPAREND_STAT = 322,
    SPAUSE_STAT = 323,
    SPOST_STAT = 324,
    SPRINT_STAT = 325,
    SPRIVATE_STAT = 326,
    SPROGRAM_STAT = 327,
    SPRSCOPE_PH_STAT = 328,
    SREAD_LONG_STAT = 329,
    SREAD_SHORT_STAT = 330,
    SRETURN_STAT = 331,
    SREWIND_LONG_STAT = 332,
    SREWIND_SHORT_STAT = 333,
    SSAVE_STAT = 334,
    SSET_BARRIER_STAT = 335,
    SSTMT_FUNCTION_STAT = 336,
    SSTOP_STAT = 337,
    SSTOP_LOOP_STAT = 338,
    SSUBROUTINE_STAT = 339,
    STASK_COMMON_STAT = 340,
    STASK_STAT = 341,
    STRACEOFF_STAT = 342,
    STRACEON_STAT = 343,
    STYPE_STATEMENT_STAT = 344,
    SUNLOCK_STAT = 345,
    SWAIT_STAT = 346,
    SWRITE_STAT = 347,
    SSTMT_PH_STAT = 348,
    SSPECIFICATION_STMT_PH_STAT = 349,
    SCONTROL_STMT_PH_STAT = 350,
    SIO_STMT_PH_STAT = 351,
    SPARASCOPE_STMT_PH_STAT = 352,
    SDEBUG_STMT_PH_STAT = 353,
    SALLOCATABLE_STAT = 354,
    SALLOCATE_STAT = 355,
    SDEALLOCATE_STAT = 356,
    SWHERE_STAT = 357,
    SWHERE_BLOCK_STAT = 358,
    SELSE_WHERE_STAT = 359,
    SEND_WHERE_STAT = 360,
    SENDMARKER = 361
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 79 "gram2.y" /* yacc.c:355  */

    FortTreeNode astptr;
    fx_StatToken statval;
    fx_EndingStat endval;
    

#line 251 "gram2.tab.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_GRAM2_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 268 "gram2.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
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

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  120
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1071

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  107
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  35
/* YYNRULES -- Number of rules.  */
#define YYNRULES  155
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  210

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   361

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
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
     105,   106
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   231,   231,   238,   242,   246,   250,   254,   258,   262,
     266,   270,   274,   281,   289,   292,   299,   311,   325,   337,
     349,   362,   374,   379,   387,   397,   400,   417,   422,   427,
     432,   437,   442,   447,   452,   457,   462,   467,   472,   477,
     482,   487,   492,   497,   502,   507,   512,   517,   522,   527,
     532,   537,   542,   547,   552,   557,   562,   567,   572,   577,
     582,   587,   592,   597,   602,   607,   612,   617,   622,   627,
     632,   637,   642,   647,   652,   657,   662,   667,   672,   677,
     682,   687,   692,   697,   702,   707,   712,   717,   722,   727,
     751,   755,   770,   788,   794,   810,   814,   826,   844,   850,
     866,   870,   887,   915,   919,   925,   933,   949,   953,   969,
     973,   980,   988,  1004,  1008,  1020,  1041,  1045,  1052,  1067,
    1071,  1084,  1104,  1110,  1126,  1130,  1147,  1175,  1193,  1198,
    1199,  1200,  1201,  1202,  1203,  1204,  1208,  1212,  1219,  1226,
    1234,  1241,  1245,  1249,  1256,  1263,  1267,  1274,  1278,  1286,
    1293,  1301,  1308,  1314,  1318,  1325
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "SGOAL_MODULE", "SGOAL_UNIT", "SGOAL_DO",
  "SGOAL_DOALL", "SGOAL_IF", "SGOAL_PAR", "SGOAL_PARALLELLOOP",
  "SGOAL_STAT", "SGOAL_WHERE", "SARITHMETIC_IF_STAT",
  "SASSIGNED_GOTO_STAT", "SASSIGNMENT_STAT", "SASSIGN_STAT", "SAT_STAT",
  "SBACKSPACE_LONG_STAT", "SBACKSPACE_SHORT_STAT", "SBLOCK_DATA_STAT",
  "SBLOCK_STAT", "SCALL_STAT", "SCLEAR_STAT", "SCLOSE_STAT",
  "SCOMMENT_STAT", "SCOMMON_STAT", "SCOMPUTED_GOTO_STAT", "SCONTINUE_STAT",
  "SDATA_STAT", "SDEBUG_STAT", "SDIMENSION_STAT", "SDOALL_LABEL_STAT",
  "SDOALL_STAT", "SDO_LABEL_STAT", "SDO_STAT", "SELSE_IF_STAT",
  "SELSE_STAT", "SENDFILE_LONG_STAT", "SENDFILE_SHORT_STAT",
  "SEND_ALL_STAT", "SEND_DO_STAT", "SEND_DO_IMPLIED", "SEND_IF_STAT",
  "SEND_LOOP_STAT", "SEND_STAT", "SENTRY_STAT", "SEQUIVALENCE_STAT",
  "SERROR_STAT", "SEXTERNAL_STAT", "SFORMAT_STAT", "SFUNCTION_STAT",
  "SGOTO_STAT", "SIF_STAT", "SIMPLICIT_STAT", "SINQUIRE_STAT",
  "SINTRINSIC_STAT", "SLOCK_STAT", "SLOGICAL_IF_STAT", "SOPEN_STAT",
  "SOTHER_PROCESSES_STAT", "SPARALLEL_PID_STAT", "SPARALLEL_STAT",
  "SPARALLELLOOP_STAT", "SPARALLELLOOP_LABEL_STAT", "SPARAMETER_STAT",
  "SPARBEGIN_PID_STAT", "SPARBEGIN_STAT", "SPAREND_STAT", "SPAUSE_STAT",
  "SPOST_STAT", "SPRINT_STAT", "SPRIVATE_STAT", "SPROGRAM_STAT",
  "SPRSCOPE_PH_STAT", "SREAD_LONG_STAT", "SREAD_SHORT_STAT",
  "SRETURN_STAT", "SREWIND_LONG_STAT", "SREWIND_SHORT_STAT", "SSAVE_STAT",
  "SSET_BARRIER_STAT", "SSTMT_FUNCTION_STAT", "SSTOP_STAT",
  "SSTOP_LOOP_STAT", "SSUBROUTINE_STAT", "STASK_COMMON_STAT", "STASK_STAT",
  "STRACEOFF_STAT", "STRACEON_STAT", "STYPE_STATEMENT_STAT",
  "SUNLOCK_STAT", "SWAIT_STAT", "SWRITE_STAT", "SSTMT_PH_STAT",
  "SSPECIFICATION_STMT_PH_STAT", "SCONTROL_STMT_PH_STAT",
  "SIO_STMT_PH_STAT", "SPARASCOPE_STMT_PH_STAT", "SDEBUG_STMT_PH_STAT",
  "SALLOCATABLE_STAT", "SALLOCATE_STAT", "SDEALLOCATE_STAT", "SWHERE_STAT",
  "SWHERE_BLOCK_STAT", "SELSE_WHERE_STAT", "SEND_WHERE_STAT", "SENDMARKER",
  "$accept", "startSymbol", "goal", "module", "program_unit_list",
  "program_unit", "program_unit_end_stat", "stat_list", "stat",
  "do_compound_stat", "do_end_stat", "do_all_compound_stat",
  "do_all_end_stat", "if_compound_stat", "if_guard_list", "if_guard",
  "if_end_stat", "par_compound_stat", "par_case_list", "par_case",
  "par_end_stat", "par_pid_compound_stat", "par_pid_case_list",
  "par_pid_case", "parallelloop_compound_stat", "parallel_loop_end_stat",
  "where_block_stat", "where_end_stat", "do_misplaced", "do_all_misplaced",
  "if_misplaced", "par_misplaced", "par_pid_misplaced",
  "parallel_loop_misplaced", "where_misplaced", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
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
     355,   356,   357,   358,   359,   360,   361
};
# endif

#define YYPACT_NINF -172

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-172)))

#define YYTABLE_NINF -14

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     142,  -172,   967,   -26,   -12,   -47,   -41,   -32,   788,   -92,
      14,   -90,  -172,   965,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,   479,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,   101,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,   479,   479,   479,   479,   479,   101,  -172,
    -172,  -172,   170,   170,   273,   273,   880,   880,   880,   376,
     376,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,   880,  -172,  -172,  -172,  -172,  -172,
     101,  -172,  -172,  -172,  -172,   101,  -172,  -172,  -172,  -172,
      93,   153,    65,   101,  -172,  -172,  -172,  -172,    26,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,
    -172,  -172,  -172,  -172,  -172,  -172,  -172,  -172,   880,   582,
     685,   880,   880,    22,   101,  -172,   101,  -172,   101,  -172
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    14,    25,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     3,    25,    25,    22,    25,    25,    25,    23,
      25,     4,     0,    25,    25,     5,    25,    25,     6,    25,
       7,    25,    25,     8,     9,    25,    25,    10,     0,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    41,    42,    43,    44,    46,    47,    48,    49,    50,
     128,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    83,    84,    85,    86,    87,    88,    89,    27,
      28,    45,    82,    25,    11,    90,    95,   100,   107,   113,
     119,   124,   129,   130,   131,   132,   133,   134,   135,    12,
       1,     2,    15,     0,     0,     0,     0,     0,   152,    24,
      17,    26,     0,     0,     0,     0,   103,   116,   109,     0,
       0,   141,   142,   139,   136,   137,   143,   150,   148,   147,
     145,   146,   153,   154,   103,    18,    21,    20,    16,    19,
     138,    93,    94,    92,    91,   140,    98,    99,    97,    96,
       0,     0,     0,   151,   123,   122,   120,   121,     0,   144,
      25,    25,   106,   104,   101,   149,    25,    25,   112,   114,
     117,    25,   110,   108,   155,    25,   127,   125,   105,     0,
       0,   118,   111,     0,   144,   102,   149,   115,   155,   126
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -172,  -172,  -172,  -172,  -172,    13,    32,   -14,    20,    29,
    -100,    37,   -84,    52,   -96,  -172,  -137,    57,  -172,  -172,
    -171,    58,  -172,  -172,    74,   -75,    63,  -121,  -172,  -172,
    -172,  -172,  -172,  -172,  -172
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    10,    11,    12,    13,    21,   130,    22,   131,   105,
     163,   106,   168,   107,   170,   183,   184,   108,   172,   192,
     189,   109,   171,   190,   110,   176,   111,   197,   112,   113,
     114,   115,   116,   117,   118
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     123,   193,   124,   125,   126,    29,   127,    23,    24,   132,
     133,   103,   134,   135,   120,   136,   121,   137,   138,    26,
      27,   139,   140,   208,    31,    32,   122,   194,   104,   207,
      35,    36,    25,   164,    39,    40,    41,    42,    43,    44,
      45,    28,    46,    47,    48,    49,    50,    51,    52,    53,
      54,   169,    55,    26,    27,    23,    24,    30,   178,    56,
      57,   180,   205,    33,    34,   177,   185,    58,    59,    60,
      61,    62,   119,    63,    29,    64,    65,    66,    67,    68,
      69,    37,   209,     0,    35,    36,    70,    31,    32,   154,
      71,    72,    73,    74,   179,     0,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,     0,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   191,   196,   180,   181,
     195,   196,   188,     0,     0,   182,   141,   142,     0,     0,
     143,   144,   145,   146,   147,     1,     2,     3,     4,     5,
       6,     7,     8,     9,   185,   155,   156,   157,   158,   159,
     148,   149,   150,     0,     0,     0,   198,   199,   151,     0,
       0,   160,   200,   201,     0,     0,     0,   202,     0,     0,
       0,   203,    39,    40,    41,    42,    43,    44,    45,     0,
      46,    47,    48,    49,    50,    51,    52,    53,    54,     0,
      55,    26,    27,    23,    24,   152,   153,    56,    57,     0,
     161,   162,   186,   187,     0,    58,    59,    60,    61,    62,
     188,    63,    29,    64,    65,    66,    67,    68,    69,     0,
       0,     0,    35,    36,    70,    31,    32,     0,    71,    72,
      73,    74,     0,     0,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,     0,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   165,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    39,    40,    41,    42,    43,
      44,    45,     0,    46,    47,    48,    49,    50,    51,    52,
      53,    54,     0,    55,    26,    27,    23,    24,     0,     0,
      56,    57,   166,     0,   167,     0,     0,     0,    58,    59,
      60,    61,    62,     0,    63,    29,    64,    65,    66,    67,
      68,    69,     0,     0,     0,    35,    36,    70,    31,    32,
       0,    71,    72,    73,    74,     0,     0,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    84,     0,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,   173,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,    39,    40,
      41,    42,    43,    44,    45,     0,    46,    47,    48,    49,
      50,    51,    52,    53,    54,     0,    55,    26,    27,    23,
      24,     0,     0,    56,    57,     0,     0,   174,     0,   175,
       0,    58,    59,    60,    61,    62,     0,    63,    29,    64,
      65,    66,    67,    68,    69,     0,     0,     0,    35,    36,
      70,    31,    32,     0,    71,    72,    73,    74,     0,     0,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    84,
       0,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
     128,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    39,    40,    41,    42,    43,    44,    45,     0,    46,
      47,    48,    49,    50,    51,    52,    53,    54,     0,    55,
      26,    27,    23,    24,     0,     0,    56,    57,     0,     0,
       0,     0,     0,   129,    58,    59,    60,    61,    62,     0,
      63,    29,    64,    65,    66,    67,    68,    69,     0,     0,
       0,    35,    36,    70,    31,    32,     0,    71,    72,    73,
      74,     0,     0,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    84,     0,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,   204,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    39,    40,    41,    42,    43,    44,
      45,     0,    46,    47,    48,    49,    50,    51,    52,    53,
      54,     0,    55,    26,    27,    23,    24,     0,     0,    56,
      57,     0,     0,     0,   182,     0,     0,    58,    59,    60,
      61,    62,     0,    63,    29,    64,    65,    66,    67,    68,
      69,     0,     0,     0,    35,    36,    70,    31,    32,     0,
      71,    72,    73,    74,     0,     0,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    84,     0,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,   206,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    39,    40,    41,
      42,    43,    44,    45,     0,    46,    47,    48,    49,    50,
      51,    52,    53,    54,     0,    55,    26,    27,    23,    24,
       0,     0,    56,    57,     0,     0,     0,     0,     0,     0,
      58,    59,    60,    61,    62,     0,    63,    29,    64,    65,
      66,    67,    68,    69,     0,     0,     0,    35,    36,    70,
      31,    32,   188,    71,    72,    73,    74,     0,     0,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,     0,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,    38,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
      39,    40,    41,    42,    43,    44,    45,     0,    46,    47,
      48,    49,    50,    51,    52,    53,    54,     0,    55,    26,
      27,    23,    24,     0,     0,    56,    57,     0,     0,     0,
       0,     0,     0,    58,    59,    60,    61,    62,     0,    63,
      29,    64,    65,    66,    67,    68,    69,     0,     0,     0,
      35,    36,    70,    31,    32,     0,    71,    72,    73,    74,
       0,     0,    75,    76,    77,    78,    79,    80,    81,    82,
      83,    84,     0,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,    39,    40,    41,    42,    43,    44,    45,     0,
      46,    47,    48,    49,    50,    51,    52,    53,    54,     0,
      55,    26,    27,    23,    24,     0,     0,    56,    57,     0,
       0,     0,     0,     0,     0,    58,    59,    60,    61,    62,
       0,    63,    29,    64,    65,    66,    67,    68,    69,     0,
       0,     0,    35,    36,    70,    31,    32,     0,    71,    72,
      73,    74,     0,     0,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    84,     0,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,    14,     0,    14,     0,     0,    15,
       0,    15,     0,     0,    16,     0,    16,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    17,     0,    17,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,    18,    19,    18,
      19,     0,     0,     0,     0,     0,     0,     0,     0,    20,
       0,    20,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   -13
};

static const yytype_int16 yycheck[] =
{
      14,   172,    16,    17,    18,    52,    20,    33,    34,    23,
      24,   103,    26,    27,     0,    29,   106,    31,    32,    31,
      32,    35,    36,     1,    65,    66,    13,     1,     8,   200,
      62,    63,     3,   133,    12,    13,    14,    15,    16,    17,
      18,     4,    20,    21,    22,    23,    24,    25,    26,    27,
      28,   135,    30,    31,    32,    33,    34,     5,   154,    37,
      38,    35,   199,     6,     6,   140,     1,    45,    46,    47,
      48,    49,     9,    51,    52,    53,    54,    55,    56,    57,
      58,     7,   203,    -1,    62,    63,    64,    65,    66,   103,
      68,    69,    70,    71,     1,    -1,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    -1,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,    61,   105,    35,    36,
     104,   105,    67,    -1,    -1,    42,    35,    36,    -1,    -1,
      39,    40,    41,    42,    43,     3,     4,     5,     6,     7,
       8,     9,    10,    11,     1,   123,   124,   125,   126,   127,
      59,    60,    61,    -1,    -1,    -1,   180,   181,    67,    -1,
      -1,     1,   186,   187,    -1,    -1,    -1,   191,    -1,    -1,
      -1,   195,    12,    13,    14,    15,    16,    17,    18,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    31,    32,    33,    34,   104,   105,    37,    38,    -1,
      40,    41,    59,    60,    -1,    45,    46,    47,    48,    49,
      67,    51,    52,    53,    54,    55,    56,    57,    58,    -1,
      -1,    -1,    62,    63,    64,    65,    66,    -1,    68,    69,
      70,    71,    -1,    -1,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    -1,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,     1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    12,    13,    14,    15,    16,
      17,    18,    -1,    20,    21,    22,    23,    24,    25,    26,
      27,    28,    -1,    30,    31,    32,    33,    34,    -1,    -1,
      37,    38,    39,    -1,    41,    -1,    -1,    -1,    45,    46,
      47,    48,    49,    -1,    51,    52,    53,    54,    55,    56,
      57,    58,    -1,    -1,    -1,    62,    63,    64,    65,    66,
      -1,    68,    69,    70,    71,    -1,    -1,    74,    75,    76,
      77,    78,    79,    80,    81,    82,    83,    -1,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,    95,    96,
      97,    98,    99,   100,   101,   102,   103,     1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    12,    13,
      14,    15,    16,    17,    18,    -1,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    -1,    30,    31,    32,    33,
      34,    -1,    -1,    37,    38,    -1,    -1,    41,    -1,    43,
      -1,    45,    46,    47,    48,    49,    -1,    51,    52,    53,
      54,    55,    56,    57,    58,    -1,    -1,    -1,    62,    63,
      64,    65,    66,    -1,    68,    69,    70,    71,    -1,    -1,
      74,    75,    76,    77,    78,    79,    80,    81,    82,    83,
      -1,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,    95,    96,    97,    98,    99,   100,   101,   102,   103,
       1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    12,    13,    14,    15,    16,    17,    18,    -1,    20,
      21,    22,    23,    24,    25,    26,    27,    28,    -1,    30,
      31,    32,    33,    34,    -1,    -1,    37,    38,    -1,    -1,
      -1,    -1,    -1,    44,    45,    46,    47,    48,    49,    -1,
      51,    52,    53,    54,    55,    56,    57,    58,    -1,    -1,
      -1,    62,    63,    64,    65,    66,    -1,    68,    69,    70,
      71,    -1,    -1,    74,    75,    76,    77,    78,    79,    80,
      81,    82,    83,    -1,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,    95,    96,    97,    98,    99,   100,
     101,   102,   103,     1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    12,    13,    14,    15,    16,    17,
      18,    -1,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    -1,    30,    31,    32,    33,    34,    -1,    -1,    37,
      38,    -1,    -1,    -1,    42,    -1,    -1,    45,    46,    47,
      48,    49,    -1,    51,    52,    53,    54,    55,    56,    57,
      58,    -1,    -1,    -1,    62,    63,    64,    65,    66,    -1,
      68,    69,    70,    71,    -1,    -1,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    83,    -1,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,    95,    96,    97,
      98,    99,   100,   101,   102,   103,     1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    12,    13,    14,
      15,    16,    17,    18,    -1,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    -1,    30,    31,    32,    33,    34,
      -1,    -1,    37,    38,    -1,    -1,    -1,    -1,    -1,    -1,
      45,    46,    47,    48,    49,    -1,    51,    52,    53,    54,
      55,    56,    57,    58,    -1,    -1,    -1,    62,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    -1,    -1,    74,
      75,    76,    77,    78,    79,    80,    81,    82,    83,    -1,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
      95,    96,    97,    98,    99,   100,   101,   102,   103,     1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      12,    13,    14,    15,    16,    17,    18,    -1,    20,    21,
      22,    23,    24,    25,    26,    27,    28,    -1,    30,    31,
      32,    33,    34,    -1,    -1,    37,    38,    -1,    -1,    -1,
      -1,    -1,    -1,    45,    46,    47,    48,    49,    -1,    51,
      52,    53,    54,    55,    56,    57,    58,    -1,    -1,    -1,
      62,    63,    64,    65,    66,    -1,    68,    69,    70,    71,
      -1,    -1,    74,    75,    76,    77,    78,    79,    80,    81,
      82,    83,    -1,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,    95,    96,    97,    98,    99,   100,   101,
     102,   103,    12,    13,    14,    15,    16,    17,    18,    -1,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    -1,
      30,    31,    32,    33,    34,    -1,    -1,    37,    38,    -1,
      -1,    -1,    -1,    -1,    -1,    45,    46,    47,    48,    49,
      -1,    51,    52,    53,    54,    55,    56,    57,    58,    -1,
      -1,    -1,    62,    63,    64,    65,    66,    -1,    68,    69,
      70,    71,    -1,    -1,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    83,    -1,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,    19,    -1,    19,    -1,    -1,    24,
      -1,    24,    -1,    -1,    29,    -1,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    50,    -1,    50,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    72,    73,    72,
      73,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    84,
      -1,    84,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,   106
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     5,     6,     7,     8,     9,    10,    11,
     108,   109,   110,   111,    19,    24,    29,    50,    72,    73,
      84,   112,   114,    33,    34,   116,    31,    32,   118,    52,
     120,    65,    66,   124,   128,    62,    63,   131,     1,    12,
      13,    14,    15,    16,    17,    18,    20,    21,    22,    23,
      24,    25,    26,    27,    28,    30,    37,    38,    45,    46,
      47,    48,    49,    51,    53,    54,    55,    56,    57,    58,
      64,    68,    69,    70,    71,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    83,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    96,    97,    98,    99,
     100,   101,   102,   103,   115,   116,   118,   120,   124,   128,
     131,   133,   135,   136,   137,   138,   139,   140,   141,   133,
       0,   106,   112,   114,   114,   114,   114,   114,     1,    44,
     113,   115,   114,   114,   114,   114,   114,   114,   114,   114,
     114,    35,    36,    39,    40,    41,    42,    43,    59,    60,
      61,    67,   104,   105,   114,   113,   113,   113,   113,   113,
       1,    40,    41,   117,   117,     1,    39,    41,   119,   119,
     121,   129,   125,     1,    41,    43,   132,   132,   121,     1,
      35,    36,    42,   122,   123,     1,    59,    60,    67,   127,
     130,    61,   126,   127,     1,   104,   105,   134,   114,   114,
     114,   114,   114,   114,     1,   123,     1,   127,     1,   134
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,   107,   108,   109,   109,   109,   109,   109,   109,   109,
     109,   109,   109,   110,   111,   111,   112,   112,   112,   112,
     112,   112,   112,   112,   113,   114,   114,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   115,   115,   115,
     115,   116,   116,   117,   117,   115,   118,   118,   119,   119,
     115,   120,   120,   121,   121,   122,   123,   115,   124,   125,
     125,   126,   127,   115,   128,   128,   129,   129,   130,   115,
     131,   131,   132,   132,   115,   133,   133,   134,   115,   115,
     115,   115,   115,   115,   115,   115,   135,   135,   117,   136,
     119,   137,   137,   137,   123,   138,   138,   139,   139,   127,
     140,   132,   113,   141,   141,   134
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     1,     0,     2,     3,     2,     3,     3,
       3,     3,     1,     1,     1,     0,     2,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     3,     3,     1,     1,     1,     3,     3,     1,     1,
       1,     4,     6,     0,     2,     2,     1,     1,     4,     0,
       2,     2,     1,     1,     4,     6,     0,     2,     2,     1,
       3,     3,     1,     1,     1,     4,     6,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     1,     2,
       1,     2,     2,     2,     1,     2,     2,     2,     2,     1,
       2,     1,     1,     2,     2,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 232 "gram2.y" /* yacc.c:1646  */
    {
        fp2_root = (yyvsp[-1].astptr);
      }
#line 1718 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 239 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].astptr);
      }
#line 1726 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 243 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].astptr);
      }
#line 1734 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 247 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].astptr);
      }
#line 1742 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 251 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].astptr);
      }
#line 1750 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 255 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].astptr);
      }
#line 1758 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 259 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].astptr);
      }
#line 1766 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 263 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].astptr);
      }
#line 1774 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 267 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].astptr);
      }
#line 1782 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 271 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].astptr);
      }
#line 1790 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 275 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].astptr);
      }
#line 1798 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 282 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = gen_GLOBAL((yyvsp[0].astptr));
      }
#line 1806 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 289 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = list_create(AST_NIL);
      }
#line 1814 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 293 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = list_insert_last((yyvsp[-1].astptr), (yyvsp[0].astptr));
      }
#line 1822 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 300 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = (yyvsp[0].endval).label;
        (yyval.astptr) = gen_PROGRAM((yyvsp[-2].statval).part[0], label, (yyvsp[-2].statval).part[1], (yyvsp[-1].astptr));
        if( (yyvsp[0].endval).missing ) 
	{
	  ft_SetParseErrorCode((yyval.astptr), ftt_MISSING_ENDBRACKET);
	}
        setLineTags2((yyval.astptr), (yyvsp[-2].statval), (yyvsp[0].endval));
      }
#line 1838 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 312 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label, name;

        label = (yyvsp[0].endval).label;
        name = gen_IDENTIFIER();
        gen_put_text(name, "main", STR_IDENTIFIER);
        (yyval.astptr) = gen_PROGRAM(AST_NIL, label, name, (yyvsp[-1].astptr));
        if( (yyvsp[0].endval).missing ) 
	{
	  ft_SetParseErrorCode((yyval.astptr), ftt_MISSING_ENDBRACKET);
	}
	ft_SetShow((yyval.astptr),false);		/* no program statement */
      }
#line 1856 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 326 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = (yyvsp[0].endval).label;
        (yyval.astptr) = gen_BLOCK_DATA((yyvsp[-2].statval).part[0], label, (yyvsp[-2].statval).part[1], (yyvsp[-1].astptr));
        if( (yyvsp[0].endval).missing )
	{
	  ft_SetParseErrorCode((yyval.astptr), ftt_MISSING_ENDBRACKET);
	}
        setLineTags2((yyval.astptr), (yyvsp[-2].statval), (yyvsp[0].endval));
      }
#line 1872 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 338 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = (yyvsp[0].endval).label;
	(yyval.astptr) = gen_SUBROUTINE((yyvsp[-2].statval).part[0], label, (yyvsp[-2].statval).part[1], (yyvsp[-2].statval).part[2], (yyvsp[-1].astptr));
        if( (yyvsp[0].endval).missing )
	{
	  ft_SetParseErrorCode((yyval.astptr), ftt_MISSING_ENDBRACKET);
	}
        setLineTags2((yyval.astptr), (yyvsp[-2].statval), (yyvsp[0].endval));
      }
#line 1888 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 350 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = (yyvsp[0].endval).label;
        (yyval.astptr) = gen_FUNCTION((yyvsp[-2].statval).part[0], label, (yyvsp[-2].statval).part[1], (yyvsp[-2].statval).part[2],
                          (yyvsp[-2].statval).part[3], (yyvsp[-1].astptr));
        if( (yyvsp[0].endval).missing )
	{
	  ft_SetParseErrorCode((yyval.astptr), ftt_MISSING_ENDBRACKET);
	}
        setLineTags2((yyval.astptr), (yyvsp[-2].statval), (yyvsp[0].endval));
      }
#line 1905 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 363 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = (yyvsp[0].endval).label;
        (yyval.astptr) = gen_DEBUG((yyvsp[-2].statval).part[0], label, (yyvsp[-2].statval).part[1], (yyvsp[-1].astptr));
        if( (yyvsp[0].endval).missing )
        {
	  ft_SetParseErrorCode((yyval.astptr), ftt_MISSING_ENDBRACKET);
	}
        setLineTags2((yyval.astptr), (yyvsp[-2].statval), (yyvsp[0].endval));
      }
#line 1921 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 375 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 1930 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 380 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).part[0];
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 1939 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 388 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.endval).missing = false;
        (yyval.endval).label   = (yyvsp[0].statval).part[0];
        (yyval.endval).conceal = (yyvsp[0].statval).conceal;
      }
#line 1949 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 397 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = list_create(AST_NIL);
      }
#line 1957 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 401 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = list_insert_last((yyvsp[-1].astptr), (yyvsp[0].astptr));
      }
#line 1965 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 418 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 1974 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 423 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 1983 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 428 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 1992 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 433 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2001 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 438 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2010 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 443 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2019 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 448 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2028 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 453 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2037 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 458 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2046 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 463 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2055 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 468 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2064 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 473 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2073 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 478 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2082 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 483 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2091 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 488 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2100 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 493 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2109 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 498 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2118 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 503 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2127 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 508 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2136 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 513 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2145 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 518 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2154 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 523 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2163 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 528 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2172 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 533 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2181 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 538 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2190 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 543 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2199 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 548 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2208 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 553 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2217 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 558 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2226 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 563 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2235 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 568 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2244 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 573 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2253 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 578 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2262 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 583 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2271 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 588 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2280 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 593 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2289 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 598 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2298 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 603 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2307 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 608 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2316 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 613 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2325 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 618 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2334 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 623 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2343 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 628 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2352 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 633 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2361 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 638 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2370 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 643 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2379 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 648 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2388 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 653 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2397 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 658 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2406 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 663 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2415 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 668 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2424 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 673 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2433 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 678 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2442 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 683 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2451 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 688 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2460 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 693 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2469 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 698 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2478 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 703 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).part[0];
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2487 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 708 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).part[0];
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2496 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 713 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).part[0];
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2505 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 718 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).part[0];
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2514 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 723 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).part[0];
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2523 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 728 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).part[0];
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2532 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 756 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

	label = ( (yyvsp[0].endval).label == IMPLIED  ?  AST_NIL  :  (yyvsp[0].endval).label );
        (yyval.astptr) = gen_DO((yyvsp[-2].statval).part[0], label, (yyvsp[-2].statval).part[1], (yyvsp[-2].statval).part[2], (yyvsp[-1].astptr));
        if( (yyvsp[0].endval).missing )
	{
          ft_SetParseErrorCode((yyval.astptr), ftt_MISSING_ENDBRACKET);
	}
        else if( (yyvsp[0].endval).label == IMPLIED )
          ft_SetParseErrorCode((yyval.astptr), ftt_WRONG_ENDBRACKET);

        setLineTags2((yyval.astptr), (yyvsp[-2].statval), (yyvsp[0].endval));
      }
#line 2551 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 771 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

	label = ( (yyvsp[0].endval).label == IMPLIED  ?  AST_NIL  :  (yyvsp[0].endval).label );
        (yyval.astptr) = gen_DO((yyvsp[-2].statval).part[0], label, (yyvsp[-2].statval).part[1], (yyvsp[-2].statval).part[2], (yyvsp[-1].astptr));
        if( (yyvsp[0].endval).missing )
	{
          ft_SetParseErrorCode((yyval.astptr), ftt_MISSING_ENDBRACKET);
	}
        else if( (yyvsp[0].endval).label != IMPLIED )
          ft_SetParseErrorCode((yyval.astptr), ftt_WRONG_ENDBRACKET);

        setLineTags2((yyval.astptr), (yyvsp[-2].statval), (yyvsp[0].endval));
      }
#line 2570 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 789 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.endval).missing = false;
        (yyval.endval).label   = (yyvsp[0].statval).part[0];
        (yyval.endval).conceal = (yyvsp[0].statval).conceal;
      }
#line 2580 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 795 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.endval).missing = false;
        (yyval.endval).label   = IMPLIED;
        (yyval.endval).conceal = (yyvsp[0].statval).conceal;
      }
#line 2590 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 815 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = (yyvsp[0].endval).label;
        (yyval.astptr) = gen_DO_ALL((yyvsp[-2].statval).part[0], label, (yyvsp[-2].statval).part[1], (yyvsp[-2].statval).part[2], (yyvsp[-1].astptr));
        if( (yyvsp[0].endval).missing )  
        {
	  ft_SetParseErrorCode((yyval.astptr), ftt_MISSING_ENDBRACKET);
	}
        setLineTags2((yyval.astptr), (yyvsp[-2].statval), (yyvsp[0].endval));
      }
#line 2606 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 827 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = ( (yyvsp[0].endval).missing  ||  (yyvsp[0].endval).label != IMPLIED )  ?  (yyvsp[0].endval).label  :  AST_NIL;
        (yyval.astptr) = gen_DO_ALL((yyvsp[-2].statval).part[0], label, (yyvsp[-2].statval).part[1], (yyvsp[-2].statval).part[2], (yyvsp[-1].astptr));
        if( (yyvsp[0].endval).missing )
	{
          ft_SetParseErrorCode((yyval.astptr), ftt_MISSING_ENDBRACKET);
	}
        else if( (yyvsp[0].endval).label != IMPLIED )
          ft_SetParseErrorCode((yyval.astptr), ftt_WRONG_ENDBRACKET);

        setLineTags2((yyval.astptr), (yyvsp[-2].statval), (yyvsp[0].endval));
      }
#line 2625 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 845 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.endval).missing = false;
        (yyval.endval).label   = (yyvsp[0].statval).part[0];
        (yyval.endval).conceal = (yyvsp[0].statval).conceal;
      }
#line 2635 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 851 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.endval).missing = false;
        (yyval.endval).label   = IMPLIED;
        (yyval.endval).conceal = (yyvsp[0].statval).conceal;
      }
#line 2645 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 871 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label, first_guard, guard_list;

        label = (yyvsp[0].endval).label;
        first_guard = gen_GUARD(AST_NIL, (yyvsp[-3].statval).part[1], (yyvsp[-2].astptr));
        guard_list = list_insert_first((yyvsp[-1].astptr), first_guard);

        (yyval.astptr) = gen_IF((yyvsp[-3].statval).part[0], label, guard_list);
        if( (yyvsp[0].endval).missing )
          { 
	    ft_SetParseErrorCode((yyval.astptr), ftt_MISSING_ENDBRACKET);
            ft_SetParseErrorCode(first_guard, ftt_MISSING_ENDBRACKET);
          }

        setLineTags2((yyval.astptr), (yyvsp[-3].statval), (yyvsp[0].endval));
      }
#line 2666 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 888 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label, first_guard, last_guard, guard_list;

        label = (yyvsp[0].endval).label;
	guard_list = (yyvsp[-3].astptr);

        first_guard = gen_GUARD(AST_NIL, (yyvsp[-5].statval).part[1], (yyvsp[-4].astptr));
        guard_list  = list_insert_first(guard_list, first_guard);
        setLineTags2(first_guard, (yyvsp[-5].statval), (yyvsp[0].endval));	/* $6 unused */

	last_guard  = gen_GUARD((yyvsp[-2].statval).part[0], (yyvsp[-2].statval).part[1], (yyvsp[-1].astptr));
        setLineTags2(last_guard, (yyvsp[-2].statval), (yyvsp[0].endval));  /* $6 unused */
        guard_list  = list_insert_last(guard_list, last_guard);

        (yyval.astptr) = gen_IF((yyvsp[-5].statval).part[0], label, guard_list);
        if( (yyvsp[0].endval).missing )
          { 
	    ft_SetParseErrorCode((yyval.astptr), ftt_MISSING_ENDBRACKET);
            ft_SetParseErrorCode(first_guard, ftt_MISSING_ENDBRACKET);
          }
        /* awful kludge -- see 'setConceal2' */
          ft_SetConceal(ftt_fortTree, (yyval.astptr), 2, (yyvsp[0].endval).conceal);

      }
#line 2695 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 916 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = list_create(AST_NIL);
      }
#line 2703 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 920 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = list_insert_last((yyvsp[-1].astptr), (yyvsp[0].astptr));
      }
#line 2711 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 926 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = gen_GUARD((yyvsp[-1].statval).part[0], (yyvsp[-1].statval).part[1], (yyvsp[0].astptr));
        setLineTags2((yyval.astptr), (yyvsp[-1].statval), UNUSED_ENDVAL);
      }
#line 2720 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 934 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.endval).missing = false;
        (yyval.endval).label   = (yyvsp[0].statval).part[0];
        (yyval.endval).conceal = (yyvsp[0].statval).conceal;
      }
#line 2730 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 954 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = (yyvsp[0].endval).label;
        (yyval.astptr) = gen_PARALLEL((yyvsp[-3].statval).part[0], label, AST_NIL, AST_NIL, (yyvsp[-2].astptr), (yyvsp[-1].astptr));
        if( (yyvsp[0].endval).missing )  
	{
	  ft_SetParseErrorCode((yyval.astptr), ftt_MISSING_ENDBRACKET);
	}

        setLineTags2((yyval.astptr), (yyvsp[-3].statval), (yyvsp[0].endval));
      }
#line 2747 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 970 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = list_create(AST_NIL);
      }
#line 2755 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 974 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = list_insert_last((yyvsp[-1].astptr), (yyvsp[0].astptr));
      }
#line 2763 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 981 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = gen_PARALLEL_CASE((yyvsp[-1].statval).part[0], AST_NIL, (yyvsp[0].astptr));
        setLineTags2((yyval.astptr), (yyvsp[-1].statval), UNUSED_ENDVAL);
      }
#line 2772 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 989 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.endval).missing = false;
        (yyval.endval).label   = (yyvsp[0].statval).part[0];
        (yyval.endval).conceal = (yyvsp[0].statval).conceal;
      }
#line 2782 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 1009 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = (yyvsp[0].endval).label;
        (yyval.astptr) = gen_PARALLEL((yyvsp[-3].statval).part[0], label, (yyvsp[-3].statval).part[1], (yyvsp[-3].statval).part[2], (yyvsp[-2].astptr), (yyvsp[-1].astptr));
        if( (yyvsp[0].endval).missing ) 
	{
	  ft_SetParseErrorCode((yyval.astptr), ftt_MISSING_ENDBRACKET);
	}
        setLineTags2((yyval.astptr), (yyvsp[-3].statval), (yyvsp[0].endval));
      }
#line 2798 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 1022 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label, last_case;

        label = (yyvsp[0].endval).label;
        last_case = gen_PARALLEL_CASE((yyvsp[-2].statval).part[0], list_create(AST_NIL), (yyvsp[-1].astptr));
        setLineTags2(last_case, (yyvsp[-2].statval), UNUSED_ENDVAL);
        (yyvsp[-3].astptr) = list_insert_last((yyvsp[-3].astptr), last_case);

        (yyval.astptr) = gen_PARALLEL((yyvsp[-5].statval).part[0], label, (yyvsp[-5].statval).part[1], (yyvsp[-5].statval).part[2], (yyvsp[-4].astptr), (yyvsp[-3].astptr));
        if( (yyvsp[0].endval).missing ) 
	{
	  ft_SetParseErrorCode((yyval.astptr), ftt_MISSING_ENDBRACKET);
	}

        setLineTags2((yyval.astptr), (yyvsp[-5].statval), (yyvsp[0].endval));
      }
#line 2819 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 1042 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = list_create(AST_NIL);
      }
#line 2827 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 1046 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = list_insert_last((yyvsp[-1].astptr), (yyvsp[0].astptr));
      }
#line 2835 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 1053 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = gen_PARALLEL_CASE((yyvsp[-1].statval).part[0], (yyvsp[-1].statval).part[1], (yyvsp[0].astptr));
        setLineTags1((yyval.astptr), (yyvsp[-1].statval));
      }
#line 2844 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 1072 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = (yyvsp[0].endval).label;
        (yyval.astptr) = gen_PARALLELLOOP((yyvsp[-2].statval).part[0], label, (yyvsp[-2].statval).part[1], (yyvsp[-2].statval).part[2], (yyvsp[-1].astptr));
        if( (yyvsp[0].endval).missing ) 
	{
	  ft_SetParseErrorCode((yyval.astptr), ftt_MISSING_ENDBRACKET);
	}

        setLineTags2((yyval.astptr), (yyvsp[-2].statval), (yyvsp[0].endval));
      }
#line 2861 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 1085 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = ((yyvsp[0].endval).missing || (yyvsp[0].endval).label != IMPLIED)  ?  (yyvsp[0].endval).label  :  AST_NIL;
        (yyval.astptr) = gen_PARALLELLOOP((yyvsp[-2].statval).part[0], label, (yyvsp[-2].statval).part[1], (yyvsp[-2].statval).part[2],
                                 (yyvsp[-1].astptr));
        if( (yyvsp[0].endval).missing )
	{
          ft_SetParseErrorCode((yyval.astptr), ftt_MISSING_ENDBRACKET);
	}
        else if( (yyvsp[0].endval).label != IMPLIED )
	{
          ft_SetParseErrorCode((yyval.astptr), ftt_WRONG_ENDBRACKET);
	}
        setLineTags2((yyval.astptr), (yyvsp[-2].statval), (yyvsp[0].endval));
      }
#line 2882 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 1105 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.endval).missing = false;
        (yyval.endval).label   = (yyvsp[0].statval).part[0];
        (yyval.endval).conceal = (yyvsp[0].statval).conceal;
      }
#line 2892 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 1111 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.endval).missing = false;
        (yyval.endval).label   = IMPLIED;
        (yyval.endval).conceal = (yyvsp[0].statval).conceal;
      }
#line 2902 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 1131 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label, first_guard, guard_list;

        label = (yyvsp[0].endval).label;
        first_guard = gen_GUARD(AST_NIL, (yyvsp[-3].statval).part[1], (yyvsp[-2].astptr));
        guard_list = list_insert_first((yyvsp[-1].astptr), first_guard);

        (yyval.astptr) = gen_WHERE_BLOCK((yyvsp[-3].statval).part[0], label, guard_list);
        if( (yyvsp[0].endval).missing )
          { 
	    ft_SetParseErrorCode((yyval.astptr), ftt_MISSING_ENDBRACKET);
            ft_SetParseErrorCode(first_guard, ftt_MISSING_ENDBRACKET);
          }

        setLineTags2((yyval.astptr), (yyvsp[-3].statval), (yyvsp[0].endval));
      }
#line 2923 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 1148 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label, first_guard, last_guard, guard_list;

        label = (yyvsp[0].endval).label;
	guard_list = (yyvsp[-3].astptr);

        first_guard = gen_GUARD(AST_NIL, (yyvsp[-5].statval).part[1], (yyvsp[-4].astptr));
        guard_list  = list_insert_first(guard_list, first_guard);
        setLineTags2(first_guard, (yyvsp[-5].statval), (yyvsp[0].endval));	/* $6 unused */

	last_guard  = gen_GUARD((yyvsp[-2].statval).part[0], (yyvsp[-2].statval).part[1], (yyvsp[-1].astptr));
        setLineTags2(last_guard, (yyvsp[-2].statval), (yyvsp[0].endval));  /* $6 unused */
        guard_list  = list_insert_last(guard_list, last_guard);

        (yyval.astptr) = gen_WHERE_BLOCK((yyvsp[-5].statval).part[0], label, guard_list);
        if( (yyvsp[0].endval).missing )
          { 
	    ft_SetParseErrorCode((yyval.astptr), ftt_MISSING_ENDBRACKET);
            ft_SetParseErrorCode(first_guard, ftt_MISSING_ENDBRACKET);
          }
        /* awful kludge -- see 'setConceal2' */
          ft_SetConceal(ftt_fortTree, (yyval.astptr), 2, (yyvsp[0].endval).conceal);

      }
#line 2952 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 1176 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.endval).missing = false;
        (yyval.endval).label   = (yyvsp[0].statval).part[0];
        (yyval.endval).conceal = (yyvsp[0].statval).conceal;
      }
#line 2962 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 1194 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = (yyvsp[0].statval).tree;
        setLineTags1((yyval.astptr), (yyvsp[0].statval));
      }
#line 2971 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 1209 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = misplaced((yyvsp[0].statval), "ENDDO", "DO");
      }
#line 2979 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 1213 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = misplaced((yyvsp[0].statval), "labeled statement", "DO");
      }
#line 2987 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 1220 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.endval) = missing("ENDDO");
      }
#line 2995 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 1227 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = misplaced((yyvsp[0].statval), "ENDALL", "DOALL");
      }
#line 3003 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 1235 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.endval) = missing("ENDALL");
      }
#line 3011 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 1242 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = misplaced((yyvsp[0].statval), "ELSEIF", "IF");
      }
#line 3019 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1246 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = misplaced((yyvsp[0].statval), "ELSE", "IF");
      }
#line 3027 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 1250 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = misplaced((yyvsp[0].statval), "ENDIF", "IF");
      }
#line 3035 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1257 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.endval) = missing("ENDIF");
      }
#line 3043 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1264 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = misplaced((yyvsp[0].statval), "PARALLEL", "PARBEGIN");
      }
#line 3051 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1268 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = misplaced((yyvsp[0].statval), "PAREND", "PARBEGIN");
      }
#line 3059 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1275 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = misplaced((yyvsp[0].statval), "PARALLEL", "PARBEGIN");
      }
#line 3067 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1279 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = misplaced((yyvsp[0].statval), "OTHER PROCESSES:", "PARBEGIN");
      }
#line 3075 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1287 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.endval) = missing("PAREND");
      }
#line 3083 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1294 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = misplaced((yyvsp[0].statval), "END LOOP", "PARALLEL LOOP");
      }
#line 3091 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1302 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.endval) = missing("END LOOP");
      }
#line 3099 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1309 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.endval) = missing("END");
      }
#line 3107 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1315 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = misplaced((yyvsp[0].statval), "ELSEWHERE", "WHERE");
      }
#line 3115 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1319 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.astptr) = misplaced((yyvsp[0].statval), "ENDWHERE", "WHERE");
      }
#line 3123 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1326 "gram2.y" /* yacc.c:1646  */
    {
        (yyval.endval) = missing("ENDWHERE");
      }
#line 3131 "gram2.tab.c" /* yacc.c:1646  */
    break;


#line 3135 "gram2.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 1336 "gram2.y" /* yacc.c:1906  */







/********************************/
/*  Local subroutines		*/
/********************************/




static
FortTreeNode misplaced(fx_StatToken st, char *what, char *prev)
  //fx_StatToken st;
  //char *what;
  //char *prev;
{
  char complaint[100];
  FortTreeNode commtext, err;

  yyerrok;
  commtext = gen_TEXT();
  (void) sprintf(complaint, "%s not matched with preceeding %s.", what, prev);
  gen_put_text(commtext, complaint, STR_COMMENT_TEXT);
  err = gen_ERROR(commtext, st.tree, st.part[0], st.part[1], st.part[2],
								st.part[3]);
  ft_SetParseErrorCode(err, (short) st.token);
  setLineTags1(err,st);
  return err;
}

/*ARGSUSED*/

static
fx_EndingStat missing(char *what)
  //char *what;
{
  fx_EndingStat endval;

  yyerrok;
  endval.missing = true;
  endval.label = AST_NIL;
  endval.conceal = 0;

  return endval;
}




static void
setLineTags1(FortTreeNode node, fx_StatToken st)
  // FortTreeNode node;
  // fx_StatToken st;
{
  ft_SetConceal(ftt_fortTree, node, 0, st.conceal);
  tt_setTagNode(ftt_textTree, node, st.tt_tag);
}




static void
setLineTags2(FortTreeNode node, fx_StatToken st, fx_EndingStat endval)
  // FortTreeNode node;
  // fx_StatToken st;
  // fx_EndingStat endval;
{
  ft_SetConceal(ftt_fortTree, node, 1, st.conceal);
  tt_setTagNode(ftt_textTree, node, st.tt_tag );

  ft_SetConceal(ftt_fortTree, node, 2, endval.conceal);
}

void yyerror(const char *s)
{
}
