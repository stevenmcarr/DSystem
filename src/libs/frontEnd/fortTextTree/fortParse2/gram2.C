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

/* All symbols defined below should begin with yy2 or YY2, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YY2BISON 1

/* Bison version.  */
#define YY2BISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YY2SKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YY2PURE 0

/* Push parsers.  */
#define YY2PUSH 0

/* Pull parsers.  */
#define YY2PULL 1




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

# ifndef YY2_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY2_NULLPTR nullptr
#  else
#   define YY2_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YY2ERROR_VERBOSE
# undef YY2ERROR_VERBOSE
# define YY2ERROR_VERBOSE 1
#else
# define YY2ERROR_VERBOSE 0
#endif

/* In a future release of Bison, this section will be replaced
   by #include "gram2.tab.h".  */
#ifndef YY2_YY2_GRAM2_TAB_H_INCLUDED
# define YY2_YY2_GRAM2_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YY2DEBUG
# define YY2DEBUG 0
#endif
#if YY2DEBUG
extern int yy2debug;
#endif

/* Token type.  */
#ifndef YY2TOKENTYPE
# define YY2TOKENTYPE
  enum yy2tokentype
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
#if ! defined YY2STYPE && ! defined YY2STYPE_IS_DECLARED

union YY2STYPE
{
#line 79 "gram2.y" /* yacc.c:355  */

    FortTreeNode astptr;
    fx_StatToken statval;
    fx_EndingStat endval;
    

#line 251 "gram2.tab.c" /* yacc.c:355  */
};

typedef union YY2STYPE YY2STYPE;
# define YY2STYPE_IS_TRIVIAL 1
# define YY2STYPE_IS_DECLARED 1
#endif


extern YY2STYPE yy2lval;

int yy2parse (void);

#endif /* !YY2_YY2_GRAM2_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 268 "gram2.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YY2TYPE_UINT8
typedef YY2TYPE_UINT8 yy2type_uint8;
#else
typedef unsigned char yy2type_uint8;
#endif

#ifdef YY2TYPE_INT8
typedef YY2TYPE_INT8 yy2type_int8;
#else
typedef signed char yy2type_int8;
#endif

#ifdef YY2TYPE_UINT16
typedef YY2TYPE_UINT16 yy2type_uint16;
#else
typedef unsigned short int yy2type_uint16;
#endif

#ifdef YY2TYPE_INT16
typedef YY2TYPE_INT16 yy2type_int16;
#else
typedef short int yy2type_int16;
#endif

#ifndef YY2SIZE_T
# ifdef __SIZE_TYPE__
#  define YY2SIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YY2SIZE_T size_t
# elif ! defined YY2SIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YY2SIZE_T size_t
# else
#  define YY2SIZE_T unsigned int
# endif
#endif

#define YY2SIZE_MAXIMUM ((YY2SIZE_T) -1)

#ifndef YY2_
# if defined YY2ENABLE_NLS && YY2ENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY2_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY2_
#  define YY2_(Msgid) Msgid
# endif
#endif

#ifndef YY2_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY2_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY2_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY2_ATTRIBUTE_PURE
# define YY2_ATTRIBUTE_PURE   YY2_ATTRIBUTE ((__pure__))
#endif

#ifndef YY2_ATTRIBUTE_UNUSED
# define YY2_ATTRIBUTE_UNUSED YY2_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY2_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY2USE(E) ((void) (E))
#else
# define YY2USE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yy2lval being uninitialized.  */
# define YY2_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY2_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY2_INITIAL_VALUE(Value) Value
#endif
#ifndef YY2_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY2_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY2_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY2_INITIAL_VALUE
# define YY2_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yy2overflow || YY2ERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YY2STACK_USE_ALLOCA
#  if YY2STACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YY2STACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YY2STACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YY2STACK_ALLOC alloca
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

# ifdef YY2STACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YY2STACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YY2STACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YY2STACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YY2STACK_ALLOC YY2MALLOC
#  define YY2STACK_FREE YY2FREE
#  ifndef YY2STACK_ALLOC_MAXIMUM
#   define YY2STACK_ALLOC_MAXIMUM YY2SIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YY2MALLOC || defined malloc) \
             && (defined YY2FREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YY2MALLOC
#   define YY2MALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YY2SIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YY2FREE
#   define YY2FREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yy2overflow || YY2ERROR_VERBOSE */


#if (! defined yy2overflow \
     && (! defined __cplusplus \
         || (defined YY2STYPE_IS_TRIVIAL && YY2STYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yy2alloc
{
  yy2type_int16 yy2ss_alloc;
  YY2STYPE yy2vs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YY2STACK_GAP_MAXIMUM (sizeof (union yy2alloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YY2STACK_BYTES(N) \
     ((N) * (sizeof (yy2type_int16) + sizeof (YY2STYPE)) \
      + YY2STACK_GAP_MAXIMUM)

# define YY2COPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YY2SIZE and YY2STACKSIZE give the old and new number of
   elements in the stack, and YY2PTR gives the new location of the
   stack.  Advance YY2PTR to a properly aligned location for the next
   stack.  */
# define YY2STACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YY2SIZE_T yy2newbytes;                                            \
        YY2COPY (&yy2ptr->Stack_alloc, Stack, yy2size);                    \
        Stack = &yy2ptr->Stack_alloc;                                    \
        yy2newbytes = yy2stacksize * sizeof (*Stack) + YY2STACK_GAP_MAXIMUM; \
        yy2ptr += yy2newbytes / sizeof (*yy2ptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YY2COPY_NEEDED && YY2COPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YY2COPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YY2COPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YY2COPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YY2SIZE_T yy2i;                         \
          for (yy2i = 0; yy2i < (Count); yy2i++)   \
            (Dst)[yy2i] = (Src)[yy2i];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YY2COPY_NEEDED */

/* YY2FINAL -- State number of the termination state.  */
#define YY2FINAL  120
/* YY2LAST -- Last index in YY2TABLE.  */
#define YY2LAST   1071

/* YY2NTOKENS -- Number of terminals.  */
#define YY2NTOKENS  107
/* YY2NNTS -- Number of nonterminals.  */
#define YY2NNTS  35
/* YY2NRULES -- Number of rules.  */
#define YY2NRULES  155
/* YY2NSTATES -- Number of states.  */
#define YY2NSTATES  210

/* YY2TRANSLATE[YY2X] -- Symbol number corresponding to YY2X as returned
   by yy2lex, with out-of-bounds checking.  */
#define YY2UNDEFTOK  2
#define YY2MAXUTOK   361

#define YY2TRANSLATE(YY2X)                                                \
  ((unsigned int) (YY2X) <= YY2MAXUTOK ? yy2translate[YY2X] : YY2UNDEFTOK)

/* YY2TRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yy2lex, without out-of-bounds checking.  */
static const yy2type_uint8 yy2translate[] =
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

#if YY2DEBUG
  /* YY2RLINE[YY2N] -- Source line where rule number YY2N was defined.  */
static const yy2type_uint16 yy2rline[] =
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

#if YY2DEBUG || YY2ERROR_VERBOSE || 0
/* YY2TNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YY2NTOKENS, nonterminals.  */
static const char *const yy2tname[] =
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
  "parallel_loop_misplaced", "where_misplaced", YY2_NULLPTR
};
#endif

# ifdef YY2PRINT
/* YY2TOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yy2type_uint16 yy2toknum[] =
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

#define YY2PACT_NINF -172

#define yy2pact_value_is_default(Yystate) \
  (!!((Yystate) == (-172)))

#define YY2TABLE_NINF -14

#define yy2table_value_is_error(Yytable_value) \
  0

  /* YY2PACT[STATE-NUM] -- Index in YY2TABLE of the portion describing
     STATE-NUM.  */
static const yy2type_int16 yy2pact[] =
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

  /* YY2DEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YY2TABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yy2type_uint8 yy2defact[] =
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

  /* YY2PGOTO[NTERM-NUM].  */
static const yy2type_int16 yy2pgoto[] =
{
    -172,  -172,  -172,  -172,  -172,    13,    32,   -14,    20,    29,
    -100,    37,   -84,    52,   -96,  -172,  -137,    57,  -172,  -172,
    -171,    58,  -172,  -172,    74,   -75,    63,  -121,  -172,  -172,
    -172,  -172,  -172,  -172,  -172
};

  /* YY2DEFGOTO[NTERM-NUM].  */
static const yy2type_int16 yy2defgoto[] =
{
      -1,    10,    11,    12,    13,    21,   130,    22,   131,   105,
     163,   106,   168,   107,   170,   183,   184,   108,   172,   192,
     189,   109,   171,   190,   110,   176,   111,   197,   112,   113,
     114,   115,   116,   117,   118
};

  /* YY2TABLE[YY2PACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YY2TABLE_NINF, syntax error.  */
static const yy2type_int16 yy2table[] =
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

static const yy2type_int16 yy2check[] =
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

  /* YY2STOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yy2type_uint8 yy2stos[] =
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

  /* YY2R1[YY2N] -- Symbol number of symbol that rule YY2N derives.  */
static const yy2type_uint8 yy2r1[] =
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

  /* YY2R2[YY2N] -- Number of symbols on the right hand side of rule YY2N.  */
static const yy2type_uint8 yy2r2[] =
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

int yy2errstatus;
#define yy2errok         (yy2errstatus = 0)
#define yy2clearin       (yy2char = YY2EMPTY)
#define YY2EMPTY         (-2)
#define YY2EOF           0

#define YY2ACCEPT        goto yy2acceptlab
#define YY2ABORT         goto yy2abortlab
#define YY2ERROR         goto yy2errorlab


#define YY2RECOVERING()  (!!yy2errstatus)

#define YY2BACKUP(Token, Value)                                  \
do                                                              \
  if (yy2char == YY2EMPTY)                                        \
    {                                                           \
      yy2char = (Token);                                         \
      yy2lval = (Value);                                         \
      YY2POPSTACK (yy2len);                                       \
      yy2state = *yy2ssp;                                         \
      goto yy2backup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yy2error (YY2_("syntax error: cannot back up")); \
      YY2ERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YY2TERROR        1
#define YY2ERRCODE       256



/* Enable debugging if requested.  */
#if YY2DEBUG

# ifndef YY2FPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YY2FPRINTF fprintf
# endif

# define YY2DPRINTF(Args)                        \
do {                                            \
  if (yy2debug)                                  \
    YY2FPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY2_LOCATION_PRINT
# define YY2_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY2_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yy2debug)                                                            \
    {                                                                     \
      YY2FPRINTF (stderr, "%s ", Title);                                   \
      yy2_symbol_print (stderr,                                            \
                  Type, Value); \
      YY2FPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YY2OUTPUT.  |
`----------------------------------------*/

static void
yy2_symbol_value_print (FILE *yy2output, int yy2type, YY2STYPE const * const yy2valuep)
{
  FILE *yy2o = yy2output;
  YY2USE (yy2o);
  if (!yy2valuep)
    return;
# ifdef YY2PRINT
  if (yy2type < YY2NTOKENS)
    YY2PRINT (yy2output, yy2toknum[yy2type], *yy2valuep);
# endif
  YY2USE (yy2type);
}


/*--------------------------------.
| Print this symbol on YY2OUTPUT.  |
`--------------------------------*/

static void
yy2_symbol_print (FILE *yy2output, int yy2type, YY2STYPE const * const yy2valuep)
{
  YY2FPRINTF (yy2output, "%s %s (",
             yy2type < YY2NTOKENS ? "token" : "nterm", yy2tname[yy2type]);

  yy2_symbol_value_print (yy2output, yy2type, yy2valuep);
  YY2FPRINTF (yy2output, ")");
}

/*------------------------------------------------------------------.
| yy2_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy2_stack_print (yy2type_int16 *yy2bottom, yy2type_int16 *yy2top)
{
  YY2FPRINTF (stderr, "Stack now");
  for (; yy2bottom <= yy2top; yy2bottom++)
    {
      int yy2bot = *yy2bottom;
      YY2FPRINTF (stderr, " %d", yy2bot);
    }
  YY2FPRINTF (stderr, "\n");
}

# define YY2_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yy2debug)                                                  \
    yy2_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YY2RULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy2_reduce_print (yy2type_int16 *yy2ssp, YY2STYPE *yy2vsp, int yy2rule)
{
  unsigned long int yy2lno = yy2rline[yy2rule];
  int yy2nrhs = yy2r2[yy2rule];
  int yy2i;
  YY2FPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yy2rule - 1, yy2lno);
  /* The symbols being reduced.  */
  for (yy2i = 0; yy2i < yy2nrhs; yy2i++)
    {
      YY2FPRINTF (stderr, "   $%d = ", yy2i + 1);
      yy2_symbol_print (stderr,
                       yy2stos[yy2ssp[yy2i + 1 - yy2nrhs]],
                       &(yy2vsp[(yy2i + 1) - (yy2nrhs)])
                                              );
      YY2FPRINTF (stderr, "\n");
    }
}

# define YY2_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yy2debug)                          \
    yy2_reduce_print (yy2ssp, yy2vsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yy2debug;
#else /* !YY2DEBUG */
# define YY2DPRINTF(Args)
# define YY2_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY2_STACK_PRINT(Bottom, Top)
# define YY2_REDUCE_PRINT(Rule)
#endif /* !YY2DEBUG */


/* YY2INITDEPTH -- initial size of the parser's stacks.  */
#ifndef YY2INITDEPTH
# define YY2INITDEPTH 200
#endif

/* YY2MAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YY2STACK_ALLOC_MAXIMUM < YY2STACK_BYTES (YY2MAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YY2MAXDEPTH
# define YY2MAXDEPTH 10000
#endif


#if YY2ERROR_VERBOSE

# ifndef yy2strlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yy2strlen strlen
#  else
/* Return the length of YY2STR.  */
static YY2SIZE_T
yy2strlen (const char *yy2str)
{
  YY2SIZE_T yy2len;
  for (yy2len = 0; yy2str[yy2len]; yy2len++)
    continue;
  return yy2len;
}
#  endif
# endif

# ifndef yy2stpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yy2stpcpy stpcpy
#  else
/* Copy YY2SRC to YY2DEST, returning the address of the terminating '\0' in
   YY2DEST.  */
static char *
yy2stpcpy (char *yy2dest, const char *yy2src)
{
  char *yy2d = yy2dest;
  const char *yy2s = yy2src;

  while ((*yy2d++ = *yy2s++) != '\0')
    continue;

  return yy2d - 1;
}
#  endif
# endif

# ifndef yy2tnamerr
/* Copy to YY2RES the contents of YY2STR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yy2error.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YY2STR is taken from yy2tname.  If YY2RES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YY2SIZE_T
yy2tnamerr (char *yy2res, const char *yy2str)
{
  if (*yy2str == '"')
    {
      YY2SIZE_T yy2n = 0;
      char const *yy2p = yy2str;

      for (;;)
        switch (*++yy2p)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yy2p != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yy2res)
              yy2res[yy2n] = *yy2p;
            yy2n++;
            break;

          case '"':
            if (yy2res)
              yy2res[yy2n] = '\0';
            return yy2n;
          }
    do_not_strip_quotes: ;
    }

  if (! yy2res)
    return yy2strlen (yy2str);

  return yy2stpcpy (yy2res, yy2str) - yy2res;
}
# endif

/* Copy into *YY2MSG, which is of size *YY2MSG_ALLOC, an error message
   about the unexpected token YY2TOKEN for the state stack whose top is
   YY2SSP.

   Return 0 if *YY2MSG was successfully written.  Return 1 if *YY2MSG is
   not large enough to hold the message.  In that case, also set
   *YY2MSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yy2syntax_error (YY2SIZE_T *yy2msg_alloc, char **yy2msg,
                yy2type_int16 *yy2ssp, int yy2token)
{
  YY2SIZE_T yy2size0 = yy2tnamerr (YY2_NULLPTR, yy2tname[yy2token]);
  YY2SIZE_T yy2size = yy2size0;
  enum { YY2ERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yy2format = YY2_NULLPTR;
  /* Arguments of yy2format. */
  char const *yy2arg[YY2ERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yy2count = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yy2char) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yy2char.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yy2token != YY2EMPTY)
    {
      int yy2n = yy2pact[*yy2ssp];
      yy2arg[yy2count++] = yy2tname[yy2token];
      if (!yy2pact_value_is_default (yy2n))
        {
          /* Start YY2X at -YY2N if negative to avoid negative indexes in
             YY2CHECK.  In other words, skip the first -YY2N actions for
             this state because they are default actions.  */
          int yy2xbegin = yy2n < 0 ? -yy2n : 0;
          /* Stay within bounds of both yy2check and yy2tname.  */
          int yy2checklim = YY2LAST - yy2n + 1;
          int yy2xend = yy2checklim < YY2NTOKENS ? yy2checklim : YY2NTOKENS;
          int yy2x;

          for (yy2x = yy2xbegin; yy2x < yy2xend; ++yy2x)
            if (yy2check[yy2x + yy2n] == yy2x && yy2x != YY2TERROR
                && !yy2table_value_is_error (yy2table[yy2x + yy2n]))
              {
                if (yy2count == YY2ERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yy2count = 1;
                    yy2size = yy2size0;
                    break;
                  }
                yy2arg[yy2count++] = yy2tname[yy2x];
                {
                  YY2SIZE_T yy2size1 = yy2size + yy2tnamerr (YY2_NULLPTR, yy2tname[yy2x]);
                  if (! (yy2size <= yy2size1
                         && yy2size1 <= YY2STACK_ALLOC_MAXIMUM))
                    return 2;
                  yy2size = yy2size1;
                }
              }
        }
    }

  switch (yy2count)
    {
# define YY2CASE_(N, S)                      \
      case N:                               \
        yy2format = S;                       \
      break
      YY2CASE_(0, YY2_("syntax error"));
      YY2CASE_(1, YY2_("syntax error, unexpected %s"));
      YY2CASE_(2, YY2_("syntax error, unexpected %s, expecting %s"));
      YY2CASE_(3, YY2_("syntax error, unexpected %s, expecting %s or %s"));
      YY2CASE_(4, YY2_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YY2CASE_(5, YY2_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YY2CASE_
    }

  {
    YY2SIZE_T yy2size1 = yy2size + yy2strlen (yy2format);
    if (! (yy2size <= yy2size1 && yy2size1 <= YY2STACK_ALLOC_MAXIMUM))
      return 2;
    yy2size = yy2size1;
  }

  if (*yy2msg_alloc < yy2size)
    {
      *yy2msg_alloc = 2 * yy2size;
      if (! (yy2size <= *yy2msg_alloc
             && *yy2msg_alloc <= YY2STACK_ALLOC_MAXIMUM))
        *yy2msg_alloc = YY2STACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yy2p = *yy2msg;
    int yy2i = 0;
    while ((*yy2p = *yy2format) != '\0')
      if (*yy2p == '%' && yy2format[1] == 's' && yy2i < yy2count)
        {
          yy2p += yy2tnamerr (yy2p, yy2arg[yy2i++]);
          yy2format += 2;
        }
      else
        {
          yy2p++;
          yy2format++;
        }
  }
  return 0;
}
#endif /* YY2ERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yy2destruct (const char *yy2msg, int yy2type, YY2STYPE *yy2valuep)
{
  YY2USE (yy2valuep);
  if (!yy2msg)
    yy2msg = "Deleting";
  YY2_SYMBOL_PRINT (yy2msg, yy2type, yy2valuep, yy2locationp);

  YY2_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY2USE (yy2type);
  YY2_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yy2char;

/* The semantic value of the lookahead symbol.  */
YY2STYPE yy2lval;
/* Number of syntax errors so far.  */
int yy2nerrs;


/*----------.
| yy2parse.  |
`----------*/

int
yy2parse (void)
{
    int yy2state;
    /* Number of tokens to shift before error messages enabled.  */
    int yy2errstatus;

    /* The stacks and their tools:
       'yy2ss': related to states.
       'yy2vs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yy2overflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yy2type_int16 yy2ssa[YY2INITDEPTH];
    yy2type_int16 *yy2ss;
    yy2type_int16 *yy2ssp;

    /* The semantic value stack.  */
    YY2STYPE yy2vsa[YY2INITDEPTH];
    YY2STYPE *yy2vs;
    YY2STYPE *yy2vsp;

    YY2SIZE_T yy2stacksize;

  int yy2n;
  int yy2result;
  /* Lookahead token as an internal (translated) token number.  */
  int yy2token = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YY2STYPE yy2val;

#if YY2ERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yy2msgbuf[128];
  char *yy2msg = yy2msgbuf;
  YY2SIZE_T yy2msg_alloc = sizeof yy2msgbuf;
#endif

#define YY2POPSTACK(N)   (yy2vsp -= (N), yy2ssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yy2len = 0;

  yy2ssp = yy2ss = yy2ssa;
  yy2vsp = yy2vs = yy2vsa;
  yy2stacksize = YY2INITDEPTH;

  YY2DPRINTF ((stderr, "Starting parse\n"));

  yy2state = 0;
  yy2errstatus = 0;
  yy2nerrs = 0;
  yy2char = YY2EMPTY; /* Cause a token to be read.  */
  goto yy2setstate;

/*------------------------------------------------------------.
| yy2newstate -- Push a new state, which is found in yy2state.  |
`------------------------------------------------------------*/
 yy2newstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yy2ssp++;

 yy2setstate:
  *yy2ssp = yy2state;

  if (yy2ss + yy2stacksize - 1 <= yy2ssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YY2SIZE_T yy2size = yy2ssp - yy2ss + 1;

#ifdef yy2overflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YY2STYPE *yy2vs1 = yy2vs;
        yy2type_int16 *yy2ss1 = yy2ss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yy2overflow is a macro.  */
        yy2overflow (YY2_("memory exhausted"),
                    &yy2ss1, yy2size * sizeof (*yy2ssp),
                    &yy2vs1, yy2size * sizeof (*yy2vsp),
                    &yy2stacksize);

        yy2ss = yy2ss1;
        yy2vs = yy2vs1;
      }
#else /* no yy2overflow */
# ifndef YY2STACK_RELOCATE
      goto yy2exhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YY2MAXDEPTH <= yy2stacksize)
        goto yy2exhaustedlab;
      yy2stacksize *= 2;
      if (YY2MAXDEPTH < yy2stacksize)
        yy2stacksize = YY2MAXDEPTH;

      {
        yy2type_int16 *yy2ss1 = yy2ss;
        union yy2alloc *yy2ptr =
          (union yy2alloc *) YY2STACK_ALLOC (YY2STACK_BYTES (yy2stacksize));
        if (! yy2ptr)
          goto yy2exhaustedlab;
        YY2STACK_RELOCATE (yy2ss_alloc, yy2ss);
        YY2STACK_RELOCATE (yy2vs_alloc, yy2vs);
#  undef YY2STACK_RELOCATE
        if (yy2ss1 != yy2ssa)
          YY2STACK_FREE (yy2ss1);
      }
# endif
#endif /* no yy2overflow */

      yy2ssp = yy2ss + yy2size - 1;
      yy2vsp = yy2vs + yy2size - 1;

      YY2DPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yy2stacksize));

      if (yy2ss + yy2stacksize - 1 <= yy2ssp)
        YY2ABORT;
    }

  YY2DPRINTF ((stderr, "Entering state %d\n", yy2state));

  if (yy2state == YY2FINAL)
    YY2ACCEPT;

  goto yy2backup;

/*-----------.
| yy2backup.  |
`-----------*/
yy2backup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yy2n = yy2pact[yy2state];
  if (yy2pact_value_is_default (yy2n))
    goto yy2default;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YY2CHAR is either YY2EMPTY or YY2EOF or a valid lookahead symbol.  */
  if (yy2char == YY2EMPTY)
    {
      YY2DPRINTF ((stderr, "Reading a token: "));
      yy2char = yy2lex ();
    }

  if (yy2char <= YY2EOF)
    {
      yy2char = yy2token = YY2EOF;
      YY2DPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yy2token = YY2TRANSLATE (yy2char);
      YY2_SYMBOL_PRINT ("Next token is", yy2token, &yy2lval, &yy2lloc);
    }

  /* If the proper action on seeing token YY2TOKEN is to reduce or to
     detect an error, take that action.  */
  yy2n += yy2token;
  if (yy2n < 0 || YY2LAST < yy2n || yy2check[yy2n] != yy2token)
    goto yy2default;
  yy2n = yy2table[yy2n];
  if (yy2n <= 0)
    {
      if (yy2table_value_is_error (yy2n))
        goto yy2errlab;
      yy2n = -yy2n;
      goto yy2reduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yy2errstatus)
    yy2errstatus--;

  /* Shift the lookahead token.  */
  YY2_SYMBOL_PRINT ("Shifting", yy2token, &yy2lval, &yy2lloc);

  /* Discard the shifted token.  */
  yy2char = YY2EMPTY;

  yy2state = yy2n;
  YY2_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yy2vsp = yy2lval;
  YY2_IGNORE_MAYBE_UNINITIALIZED_END

  goto yy2newstate;


/*-----------------------------------------------------------.
| yy2default -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yy2default:
  yy2n = yy2defact[yy2state];
  if (yy2n == 0)
    goto yy2errlab;
  goto yy2reduce;


/*-----------------------------.
| yy2reduce -- Do a reduction.  |
`-----------------------------*/
yy2reduce:
  /* yy2n is the number of a rule to reduce with.  */
  yy2len = yy2r2[yy2n];

  /* If YY2LEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YY2VAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YY2VAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YY2VAL may be used uninitialized.  */
  yy2val = yy2vsp[1-yy2len];


  YY2_REDUCE_PRINT (yy2n);
  switch (yy2n)
    {
        case 2:
#line 232 "gram2.y" /* yacc.c:1646  */
    {
        fp2_root = (yy2vsp[-1].astptr);
      }
#line 1718 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 3:
#line 239 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].astptr);
      }
#line 1726 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 4:
#line 243 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].astptr);
      }
#line 1734 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 247 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].astptr);
      }
#line 1742 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 6:
#line 251 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].astptr);
      }
#line 1750 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 7:
#line 255 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].astptr);
      }
#line 1758 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 259 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].astptr);
      }
#line 1766 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 9:
#line 263 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].astptr);
      }
#line 1774 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 267 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].astptr);
      }
#line 1782 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 271 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].astptr);
      }
#line 1790 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 275 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].astptr);
      }
#line 1798 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 13:
#line 282 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = gen_GLOBAL((yy2vsp[0].astptr));
      }
#line 1806 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 289 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = list_create(AST_NIL);
      }
#line 1814 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 293 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = list_insert_last((yy2vsp[-1].astptr), (yy2vsp[0].astptr));
      }
#line 1822 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 300 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = (yy2vsp[0].endval).label;
        (yy2val.astptr) = gen_PROGRAM((yy2vsp[-2].statval).part[0], label, (yy2vsp[-2].statval).part[1], (yy2vsp[-1].astptr));
        if( (yy2vsp[0].endval).missing ) 
	{
	  ft_SetParseErrorCode((yy2val.astptr), ftt_MISSING_ENDBRACKET);
	}
        setLineTags2((yy2val.astptr), (yy2vsp[-2].statval), (yy2vsp[0].endval));
      }
#line 1838 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 312 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label, name;

        label = (yy2vsp[0].endval).label;
        name = gen_IDENTIFIER();
        gen_put_text(name, "main", STR_IDENTIFIER);
        (yy2val.astptr) = gen_PROGRAM(AST_NIL, label, name, (yy2vsp[-1].astptr));
        if( (yy2vsp[0].endval).missing ) 
	{
	  ft_SetParseErrorCode((yy2val.astptr), ftt_MISSING_ENDBRACKET);
	}
	ft_SetShow((yy2val.astptr),false);		/* no program statement */
      }
#line 1856 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 326 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = (yy2vsp[0].endval).label;
        (yy2val.astptr) = gen_BLOCK_DATA((yy2vsp[-2].statval).part[0], label, (yy2vsp[-2].statval).part[1], (yy2vsp[-1].astptr));
        if( (yy2vsp[0].endval).missing )
	{
	  ft_SetParseErrorCode((yy2val.astptr), ftt_MISSING_ENDBRACKET);
	}
        setLineTags2((yy2val.astptr), (yy2vsp[-2].statval), (yy2vsp[0].endval));
      }
#line 1872 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 338 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = (yy2vsp[0].endval).label;
	(yy2val.astptr) = gen_SUBROUTINE((yy2vsp[-2].statval).part[0], label, (yy2vsp[-2].statval).part[1], (yy2vsp[-2].statval).part[2], (yy2vsp[-1].astptr));
        if( (yy2vsp[0].endval).missing )
	{
	  ft_SetParseErrorCode((yy2val.astptr), ftt_MISSING_ENDBRACKET);
	}
        setLineTags2((yy2val.astptr), (yy2vsp[-2].statval), (yy2vsp[0].endval));
      }
#line 1888 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 20:
#line 350 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = (yy2vsp[0].endval).label;
        (yy2val.astptr) = gen_FUNCTION((yy2vsp[-2].statval).part[0], label, (yy2vsp[-2].statval).part[1], (yy2vsp[-2].statval).part[2],
                          (yy2vsp[-2].statval).part[3], (yy2vsp[-1].astptr));
        if( (yy2vsp[0].endval).missing )
	{
	  ft_SetParseErrorCode((yy2val.astptr), ftt_MISSING_ENDBRACKET);
	}
        setLineTags2((yy2val.astptr), (yy2vsp[-2].statval), (yy2vsp[0].endval));
      }
#line 1905 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 363 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = (yy2vsp[0].endval).label;
        (yy2val.astptr) = gen_DEBUG((yy2vsp[-2].statval).part[0], label, (yy2vsp[-2].statval).part[1], (yy2vsp[-1].astptr));
        if( (yy2vsp[0].endval).missing )
        {
	  ft_SetParseErrorCode((yy2val.astptr), ftt_MISSING_ENDBRACKET);
	}
        setLineTags2((yy2val.astptr), (yy2vsp[-2].statval), (yy2vsp[0].endval));
      }
#line 1921 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 375 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 1930 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 380 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).part[0];
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 1939 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 388 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.endval).missing = false;
        (yy2val.endval).label   = (yy2vsp[0].statval).part[0];
        (yy2val.endval).conceal = (yy2vsp[0].statval).conceal;
      }
#line 1949 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 397 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = list_create(AST_NIL);
      }
#line 1957 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 401 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = list_insert_last((yy2vsp[-1].astptr), (yy2vsp[0].astptr));
      }
#line 1965 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 418 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 1974 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 423 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 1983 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 428 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 1992 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 433 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2001 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 438 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2010 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 443 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2019 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 448 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2028 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 453 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2037 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 458 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2046 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 463 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2055 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 468 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2064 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 473 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2073 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 478 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2082 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 483 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2091 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 488 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2100 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 493 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2109 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 498 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2118 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 503 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2127 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 508 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2136 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 513 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2145 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 518 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2154 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 523 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2163 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 528 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2172 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 533 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2181 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 538 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2190 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 543 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2199 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 548 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2208 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 553 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2217 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 558 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2226 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 563 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2235 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 568 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2244 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 573 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2253 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 578 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2262 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 583 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2271 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 588 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2280 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 593 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2289 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 598 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2298 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 603 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2307 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 608 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2316 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 613 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2325 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 618 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2334 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 623 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2343 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 628 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2352 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 633 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2361 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 638 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2370 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 643 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2379 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 648 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2388 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 653 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2397 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 658 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2406 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 663 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2415 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 668 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2424 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 673 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2433 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 678 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2442 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 683 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2451 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 688 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2460 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 693 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2469 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 698 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2478 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 703 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).part[0];
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2487 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 708 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).part[0];
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2496 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 713 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).part[0];
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2505 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 718 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).part[0];
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2514 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 723 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).part[0];
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2523 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 728 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).part[0];
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2532 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 756 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

	label = ( (yy2vsp[0].endval).label == IMPLIED  ?  AST_NIL  :  (yy2vsp[0].endval).label );
        (yy2val.astptr) = gen_DO((yy2vsp[-2].statval).part[0], label, (yy2vsp[-2].statval).part[1], (yy2vsp[-2].statval).part[2], (yy2vsp[-1].astptr));
        if( (yy2vsp[0].endval).missing )
	{
          ft_SetParseErrorCode((yy2val.astptr), ftt_MISSING_ENDBRACKET);
	}
        else if( (yy2vsp[0].endval).label == IMPLIED )
          ft_SetParseErrorCode((yy2val.astptr), ftt_WRONG_ENDBRACKET);

        setLineTags2((yy2val.astptr), (yy2vsp[-2].statval), (yy2vsp[0].endval));
      }
#line 2551 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 92:
#line 771 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

	label = ( (yy2vsp[0].endval).label == IMPLIED  ?  AST_NIL  :  (yy2vsp[0].endval).label );
        (yy2val.astptr) = gen_DO((yy2vsp[-2].statval).part[0], label, (yy2vsp[-2].statval).part[1], (yy2vsp[-2].statval).part[2], (yy2vsp[-1].astptr));
        if( (yy2vsp[0].endval).missing )
	{
          ft_SetParseErrorCode((yy2val.astptr), ftt_MISSING_ENDBRACKET);
	}
        else if( (yy2vsp[0].endval).label != IMPLIED )
          ft_SetParseErrorCode((yy2val.astptr), ftt_WRONG_ENDBRACKET);

        setLineTags2((yy2val.astptr), (yy2vsp[-2].statval), (yy2vsp[0].endval));
      }
#line 2570 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 93:
#line 789 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.endval).missing = false;
        (yy2val.endval).label   = (yy2vsp[0].statval).part[0];
        (yy2val.endval).conceal = (yy2vsp[0].statval).conceal;
      }
#line 2580 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 795 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.endval).missing = false;
        (yy2val.endval).label   = IMPLIED;
        (yy2val.endval).conceal = (yy2vsp[0].statval).conceal;
      }
#line 2590 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 96:
#line 815 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = (yy2vsp[0].endval).label;
        (yy2val.astptr) = gen_DO_ALL((yy2vsp[-2].statval).part[0], label, (yy2vsp[-2].statval).part[1], (yy2vsp[-2].statval).part[2], (yy2vsp[-1].astptr));
        if( (yy2vsp[0].endval).missing )  
        {
	  ft_SetParseErrorCode((yy2val.astptr), ftt_MISSING_ENDBRACKET);
	}
        setLineTags2((yy2val.astptr), (yy2vsp[-2].statval), (yy2vsp[0].endval));
      }
#line 2606 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 827 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = ( (yy2vsp[0].endval).missing  ||  (yy2vsp[0].endval).label != IMPLIED )  ?  (yy2vsp[0].endval).label  :  AST_NIL;
        (yy2val.astptr) = gen_DO_ALL((yy2vsp[-2].statval).part[0], label, (yy2vsp[-2].statval).part[1], (yy2vsp[-2].statval).part[2], (yy2vsp[-1].astptr));
        if( (yy2vsp[0].endval).missing )
	{
          ft_SetParseErrorCode((yy2val.astptr), ftt_MISSING_ENDBRACKET);
	}
        else if( (yy2vsp[0].endval).label != IMPLIED )
          ft_SetParseErrorCode((yy2val.astptr), ftt_WRONG_ENDBRACKET);

        setLineTags2((yy2val.astptr), (yy2vsp[-2].statval), (yy2vsp[0].endval));
      }
#line 2625 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 845 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.endval).missing = false;
        (yy2val.endval).label   = (yy2vsp[0].statval).part[0];
        (yy2val.endval).conceal = (yy2vsp[0].statval).conceal;
      }
#line 2635 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 99:
#line 851 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.endval).missing = false;
        (yy2val.endval).label   = IMPLIED;
        (yy2val.endval).conceal = (yy2vsp[0].statval).conceal;
      }
#line 2645 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 871 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label, first_guard, guard_list;

        label = (yy2vsp[0].endval).label;
        first_guard = gen_GUARD(AST_NIL, (yy2vsp[-3].statval).part[1], (yy2vsp[-2].astptr));
        guard_list = list_insert_first((yy2vsp[-1].astptr), first_guard);

        (yy2val.astptr) = gen_IF((yy2vsp[-3].statval).part[0], label, guard_list);
        if( (yy2vsp[0].endval).missing )
          { 
	    ft_SetParseErrorCode((yy2val.astptr), ftt_MISSING_ENDBRACKET);
            ft_SetParseErrorCode(first_guard, ftt_MISSING_ENDBRACKET);
          }

        setLineTags2((yy2val.astptr), (yy2vsp[-3].statval), (yy2vsp[0].endval));
      }
#line 2666 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 888 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label, first_guard, last_guard, guard_list;

        label = (yy2vsp[0].endval).label;
	guard_list = (yy2vsp[-3].astptr);

        first_guard = gen_GUARD(AST_NIL, (yy2vsp[-5].statval).part[1], (yy2vsp[-4].astptr));
        guard_list  = list_insert_first(guard_list, first_guard);
        setLineTags2(first_guard, (yy2vsp[-5].statval), (yy2vsp[0].endval));	/* $6 unused */

	last_guard  = gen_GUARD((yy2vsp[-2].statval).part[0], (yy2vsp[-2].statval).part[1], (yy2vsp[-1].astptr));
        setLineTags2(last_guard, (yy2vsp[-2].statval), (yy2vsp[0].endval));  /* $6 unused */
        guard_list  = list_insert_last(guard_list, last_guard);

        (yy2val.astptr) = gen_IF((yy2vsp[-5].statval).part[0], label, guard_list);
        if( (yy2vsp[0].endval).missing )
          { 
	    ft_SetParseErrorCode((yy2val.astptr), ftt_MISSING_ENDBRACKET);
            ft_SetParseErrorCode(first_guard, ftt_MISSING_ENDBRACKET);
          }
        /* awful kludge -- see 'setConceal2' */
          ft_SetConceal(ftt_fortTree, (yy2val.astptr), 2, (yy2vsp[0].endval).conceal);

      }
#line 2695 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 103:
#line 916 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = list_create(AST_NIL);
      }
#line 2703 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 920 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = list_insert_last((yy2vsp[-1].astptr), (yy2vsp[0].astptr));
      }
#line 2711 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 926 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = gen_GUARD((yy2vsp[-1].statval).part[0], (yy2vsp[-1].statval).part[1], (yy2vsp[0].astptr));
        setLineTags2((yy2val.astptr), (yy2vsp[-1].statval), UNUSED_ENDVAL);
      }
#line 2720 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 934 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.endval).missing = false;
        (yy2val.endval).label   = (yy2vsp[0].statval).part[0];
        (yy2val.endval).conceal = (yy2vsp[0].statval).conceal;
      }
#line 2730 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 954 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = (yy2vsp[0].endval).label;
        (yy2val.astptr) = gen_PARALLEL((yy2vsp[-3].statval).part[0], label, AST_NIL, AST_NIL, (yy2vsp[-2].astptr), (yy2vsp[-1].astptr));
        if( (yy2vsp[0].endval).missing )  
	{
	  ft_SetParseErrorCode((yy2val.astptr), ftt_MISSING_ENDBRACKET);
	}

        setLineTags2((yy2val.astptr), (yy2vsp[-3].statval), (yy2vsp[0].endval));
      }
#line 2747 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 970 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = list_create(AST_NIL);
      }
#line 2755 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 974 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = list_insert_last((yy2vsp[-1].astptr), (yy2vsp[0].astptr));
      }
#line 2763 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 981 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = gen_PARALLEL_CASE((yy2vsp[-1].statval).part[0], AST_NIL, (yy2vsp[0].astptr));
        setLineTags2((yy2val.astptr), (yy2vsp[-1].statval), UNUSED_ENDVAL);
      }
#line 2772 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 989 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.endval).missing = false;
        (yy2val.endval).label   = (yy2vsp[0].statval).part[0];
        (yy2val.endval).conceal = (yy2vsp[0].statval).conceal;
      }
#line 2782 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 1009 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = (yy2vsp[0].endval).label;
        (yy2val.astptr) = gen_PARALLEL((yy2vsp[-3].statval).part[0], label, (yy2vsp[-3].statval).part[1], (yy2vsp[-3].statval).part[2], (yy2vsp[-2].astptr), (yy2vsp[-1].astptr));
        if( (yy2vsp[0].endval).missing ) 
	{
	  ft_SetParseErrorCode((yy2val.astptr), ftt_MISSING_ENDBRACKET);
	}
        setLineTags2((yy2val.astptr), (yy2vsp[-3].statval), (yy2vsp[0].endval));
      }
#line 2798 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 1022 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label, last_case;

        label = (yy2vsp[0].endval).label;
        last_case = gen_PARALLEL_CASE((yy2vsp[-2].statval).part[0], list_create(AST_NIL), (yy2vsp[-1].astptr));
        setLineTags2(last_case, (yy2vsp[-2].statval), UNUSED_ENDVAL);
        (yy2vsp[-3].astptr) = list_insert_last((yy2vsp[-3].astptr), last_case);

        (yy2val.astptr) = gen_PARALLEL((yy2vsp[-5].statval).part[0], label, (yy2vsp[-5].statval).part[1], (yy2vsp[-5].statval).part[2], (yy2vsp[-4].astptr), (yy2vsp[-3].astptr));
        if( (yy2vsp[0].endval).missing ) 
	{
	  ft_SetParseErrorCode((yy2val.astptr), ftt_MISSING_ENDBRACKET);
	}

        setLineTags2((yy2val.astptr), (yy2vsp[-5].statval), (yy2vsp[0].endval));
      }
#line 2819 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 1042 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = list_create(AST_NIL);
      }
#line 2827 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 1046 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = list_insert_last((yy2vsp[-1].astptr), (yy2vsp[0].astptr));
      }
#line 2835 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 1053 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = gen_PARALLEL_CASE((yy2vsp[-1].statval).part[0], (yy2vsp[-1].statval).part[1], (yy2vsp[0].astptr));
        setLineTags1((yy2val.astptr), (yy2vsp[-1].statval));
      }
#line 2844 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 1072 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = (yy2vsp[0].endval).label;
        (yy2val.astptr) = gen_PARALLELLOOP((yy2vsp[-2].statval).part[0], label, (yy2vsp[-2].statval).part[1], (yy2vsp[-2].statval).part[2], (yy2vsp[-1].astptr));
        if( (yy2vsp[0].endval).missing ) 
	{
	  ft_SetParseErrorCode((yy2val.astptr), ftt_MISSING_ENDBRACKET);
	}

        setLineTags2((yy2val.astptr), (yy2vsp[-2].statval), (yy2vsp[0].endval));
      }
#line 2861 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 1085 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label;

        label = ((yy2vsp[0].endval).missing || (yy2vsp[0].endval).label != IMPLIED)  ?  (yy2vsp[0].endval).label  :  AST_NIL;
        (yy2val.astptr) = gen_PARALLELLOOP((yy2vsp[-2].statval).part[0], label, (yy2vsp[-2].statval).part[1], (yy2vsp[-2].statval).part[2],
                                 (yy2vsp[-1].astptr));
        if( (yy2vsp[0].endval).missing )
	{
          ft_SetParseErrorCode((yy2val.astptr), ftt_MISSING_ENDBRACKET);
	}
        else if( (yy2vsp[0].endval).label != IMPLIED )
	{
          ft_SetParseErrorCode((yy2val.astptr), ftt_WRONG_ENDBRACKET);
	}
        setLineTags2((yy2val.astptr), (yy2vsp[-2].statval), (yy2vsp[0].endval));
      }
#line 2882 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 1105 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.endval).missing = false;
        (yy2val.endval).label   = (yy2vsp[0].statval).part[0];
        (yy2val.endval).conceal = (yy2vsp[0].statval).conceal;
      }
#line 2892 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 1111 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.endval).missing = false;
        (yy2val.endval).label   = IMPLIED;
        (yy2val.endval).conceal = (yy2vsp[0].statval).conceal;
      }
#line 2902 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 1131 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label, first_guard, guard_list;

        label = (yy2vsp[0].endval).label;
        first_guard = gen_GUARD(AST_NIL, (yy2vsp[-3].statval).part[1], (yy2vsp[-2].astptr));
        guard_list = list_insert_first((yy2vsp[-1].astptr), first_guard);

        (yy2val.astptr) = gen_WHERE_BLOCK((yy2vsp[-3].statval).part[0], label, guard_list);
        if( (yy2vsp[0].endval).missing )
          { 
	    ft_SetParseErrorCode((yy2val.astptr), ftt_MISSING_ENDBRACKET);
            ft_SetParseErrorCode(first_guard, ftt_MISSING_ENDBRACKET);
          }

        setLineTags2((yy2val.astptr), (yy2vsp[-3].statval), (yy2vsp[0].endval));
      }
#line 2923 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 1148 "gram2.y" /* yacc.c:1646  */
    {
        FortTreeNode label, first_guard, last_guard, guard_list;

        label = (yy2vsp[0].endval).label;
	guard_list = (yy2vsp[-3].astptr);

        first_guard = gen_GUARD(AST_NIL, (yy2vsp[-5].statval).part[1], (yy2vsp[-4].astptr));
        guard_list  = list_insert_first(guard_list, first_guard);
        setLineTags2(first_guard, (yy2vsp[-5].statval), (yy2vsp[0].endval));	/* $6 unused */

	last_guard  = gen_GUARD((yy2vsp[-2].statval).part[0], (yy2vsp[-2].statval).part[1], (yy2vsp[-1].astptr));
        setLineTags2(last_guard, (yy2vsp[-2].statval), (yy2vsp[0].endval));  /* $6 unused */
        guard_list  = list_insert_last(guard_list, last_guard);

        (yy2val.astptr) = gen_WHERE_BLOCK((yy2vsp[-5].statval).part[0], label, guard_list);
        if( (yy2vsp[0].endval).missing )
          { 
	    ft_SetParseErrorCode((yy2val.astptr), ftt_MISSING_ENDBRACKET);
            ft_SetParseErrorCode(first_guard, ftt_MISSING_ENDBRACKET);
          }
        /* awful kludge -- see 'setConceal2' */
          ft_SetConceal(ftt_fortTree, (yy2val.astptr), 2, (yy2vsp[0].endval).conceal);

      }
#line 2952 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 1176 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.endval).missing = false;
        (yy2val.endval).label   = (yy2vsp[0].statval).part[0];
        (yy2val.endval).conceal = (yy2vsp[0].statval).conceal;
      }
#line 2962 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 1194 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = (yy2vsp[0].statval).tree;
        setLineTags1((yy2val.astptr), (yy2vsp[0].statval));
      }
#line 2971 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 136:
#line 1209 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = misplaced((yy2vsp[0].statval), "ENDDO", "DO");
      }
#line 2979 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 137:
#line 1213 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = misplaced((yy2vsp[0].statval), "labeled statement", "DO");
      }
#line 2987 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 138:
#line 1220 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.endval) = missing("ENDDO");
      }
#line 2995 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 139:
#line 1227 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = misplaced((yy2vsp[0].statval), "ENDALL", "DOALL");
      }
#line 3003 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 140:
#line 1235 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.endval) = missing("ENDALL");
      }
#line 3011 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 141:
#line 1242 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = misplaced((yy2vsp[0].statval), "ELSEIF", "IF");
      }
#line 3019 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 142:
#line 1246 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = misplaced((yy2vsp[0].statval), "ELSE", "IF");
      }
#line 3027 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 143:
#line 1250 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = misplaced((yy2vsp[0].statval), "ENDIF", "IF");
      }
#line 3035 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 144:
#line 1257 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.endval) = missing("ENDIF");
      }
#line 3043 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 145:
#line 1264 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = misplaced((yy2vsp[0].statval), "PARALLEL", "PARBEGIN");
      }
#line 3051 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 146:
#line 1268 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = misplaced((yy2vsp[0].statval), "PAREND", "PARBEGIN");
      }
#line 3059 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 147:
#line 1275 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = misplaced((yy2vsp[0].statval), "PARALLEL", "PARBEGIN");
      }
#line 3067 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 148:
#line 1279 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = misplaced((yy2vsp[0].statval), "OTHER PROCESSES:", "PARBEGIN");
      }
#line 3075 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 149:
#line 1287 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.endval) = missing("PAREND");
      }
#line 3083 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 150:
#line 1294 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = misplaced((yy2vsp[0].statval), "END LOOP", "PARALLEL LOOP");
      }
#line 3091 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 151:
#line 1302 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.endval) = missing("END LOOP");
      }
#line 3099 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 152:
#line 1309 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.endval) = missing("END");
      }
#line 3107 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 153:
#line 1315 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = misplaced((yy2vsp[0].statval), "ELSEWHERE", "WHERE");
      }
#line 3115 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 154:
#line 1319 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.astptr) = misplaced((yy2vsp[0].statval), "ENDWHERE", "WHERE");
      }
#line 3123 "gram2.tab.c" /* yacc.c:1646  */
    break;

  case 155:
#line 1326 "gram2.y" /* yacc.c:1646  */
    {
        (yy2val.endval) = missing("ENDWHERE");
      }
#line 3131 "gram2.tab.c" /* yacc.c:1646  */
    break;


#line 3135 "gram2.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yy2char, and that requires
     that yy2token be updated with the new translation.  We take the
     approach of translating immediately before every use of yy2token.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YY2ABORT, YY2ACCEPT, or YY2ERROR immediately after altering yy2char or
     if it invokes YY2BACKUP.  In the case of YY2ABORT or YY2ACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YY2ERROR or YY2BACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY2_SYMBOL_PRINT ("-> $$ =", yy2r1[yy2n], &yy2val, &yy2loc);

  YY2POPSTACK (yy2len);
  yy2len = 0;
  YY2_STACK_PRINT (yy2ss, yy2ssp);

  *++yy2vsp = yy2val;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yy2n = yy2r1[yy2n];

  yy2state = yy2pgoto[yy2n - YY2NTOKENS] + *yy2ssp;
  if (0 <= yy2state && yy2state <= YY2LAST && yy2check[yy2state] == *yy2ssp)
    yy2state = yy2table[yy2state];
  else
    yy2state = yy2defgoto[yy2n - YY2NTOKENS];

  goto yy2newstate;


/*--------------------------------------.
| yy2errlab -- here on detecting error.  |
`--------------------------------------*/
yy2errlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yy2token = yy2char == YY2EMPTY ? YY2EMPTY : YY2TRANSLATE (yy2char);

  /* If not already recovering from an error, report this error.  */
  if (!yy2errstatus)
    {
      ++yy2nerrs;
#if ! YY2ERROR_VERBOSE
      yy2error (YY2_("syntax error"));
#else
# define YY2SYNTAX_ERROR yy2syntax_error (&yy2msg_alloc, &yy2msg, \
                                        yy2ssp, yy2token)
      {
        char const *yy2msgp = YY2_("syntax error");
        int yy2syntax_error_status;
        yy2syntax_error_status = YY2SYNTAX_ERROR;
        if (yy2syntax_error_status == 0)
          yy2msgp = yy2msg;
        else if (yy2syntax_error_status == 1)
          {
            if (yy2msg != yy2msgbuf)
              YY2STACK_FREE (yy2msg);
            yy2msg = (char *) YY2STACK_ALLOC (yy2msg_alloc);
            if (!yy2msg)
              {
                yy2msg = yy2msgbuf;
                yy2msg_alloc = sizeof yy2msgbuf;
                yy2syntax_error_status = 2;
              }
            else
              {
                yy2syntax_error_status = YY2SYNTAX_ERROR;
                yy2msgp = yy2msg;
              }
          }
        yy2error (yy2msgp);
        if (yy2syntax_error_status == 2)
          goto yy2exhaustedlab;
      }
# undef YY2SYNTAX_ERROR
#endif
    }



  if (yy2errstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yy2char <= YY2EOF)
        {
          /* Return failure if at end of input.  */
          if (yy2char == YY2EOF)
            YY2ABORT;
        }
      else
        {
          yy2destruct ("Error: discarding",
                      yy2token, &yy2lval);
          yy2char = YY2EMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yy2errlab1;


/*---------------------------------------------------.
| yy2errorlab -- error raised explicitly by YY2ERROR.  |
`---------------------------------------------------*/
yy2errorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YY2ERROR and the label yy2errorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yy2errorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YY2ERROR.  */
  YY2POPSTACK (yy2len);
  yy2len = 0;
  YY2_STACK_PRINT (yy2ss, yy2ssp);
  yy2state = *yy2ssp;
  goto yy2errlab1;


/*-------------------------------------------------------------.
| yy2errlab1 -- common code for both syntax error and YY2ERROR.  |
`-------------------------------------------------------------*/
yy2errlab1:
  yy2errstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yy2n = yy2pact[yy2state];
      if (!yy2pact_value_is_default (yy2n))
        {
          yy2n += YY2TERROR;
          if (0 <= yy2n && yy2n <= YY2LAST && yy2check[yy2n] == YY2TERROR)
            {
              yy2n = yy2table[yy2n];
              if (0 < yy2n)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yy2ssp == yy2ss)
        YY2ABORT;


      yy2destruct ("Error: popping",
                  yy2stos[yy2state], yy2vsp);
      YY2POPSTACK (1);
      yy2state = *yy2ssp;
      YY2_STACK_PRINT (yy2ss, yy2ssp);
    }

  YY2_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yy2vsp = yy2lval;
  YY2_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY2_SYMBOL_PRINT ("Shifting", yy2stos[yy2n], yy2vsp, yy2lsp);

  yy2state = yy2n;
  goto yy2newstate;


/*-------------------------------------.
| yy2acceptlab -- YY2ACCEPT comes here.  |
`-------------------------------------*/
yy2acceptlab:
  yy2result = 0;
  goto yy2return;

/*-----------------------------------.
| yy2abortlab -- YY2ABORT comes here.  |
`-----------------------------------*/
yy2abortlab:
  yy2result = 1;
  goto yy2return;

#if !defined yy2overflow || YY2ERROR_VERBOSE
/*-------------------------------------------------.
| yy2exhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yy2exhaustedlab:
  yy2error (YY2_("memory exhausted"));
  yy2result = 2;
  /* Fall through.  */
#endif

yy2return:
  if (yy2char != YY2EMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yy2token = YY2TRANSLATE (yy2char);
      yy2destruct ("Cleanup: discarding lookahead",
                  yy2token, &yy2lval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YY2ABORT or YY2ACCEPT.  */
  YY2POPSTACK (yy2len);
  YY2_STACK_PRINT (yy2ss, yy2ssp);
  while (yy2ssp != yy2ss)
    {
      yy2destruct ("Cleanup: popping",
                  yy2stos[*yy2ssp], yy2vsp);
      YY2POPSTACK (1);
    }
#ifndef yy2overflow
  if (yy2ss != yy2ssa)
    YY2STACK_FREE (yy2ss);
#endif
#if YY2ERROR_VERBOSE
  if (yy2msg != yy2msgbuf)
    YY2STACK_FREE (yy2msg);
#endif
  return yy2result;
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

  yy2errok;
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

  yy2errok;
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

void yy2error(const char *s)
{
}
