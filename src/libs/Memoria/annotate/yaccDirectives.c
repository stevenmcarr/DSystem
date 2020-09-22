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

/* All symbols defined below should begin with a2i_ or YY, to avoid
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


/* Substitute the variable and function names.  */
#define a2i_parse         a2i_parse
#define a2i_lex           a2i_lex
#define a2i_error         a2i_error
#define a2i_debug         a2i_debug
#define a2i_nerrs         a2i_nerrs

#define a2i_lval          a2i_lval
#define a2i_char          a2i_char

/* Copy the first part of user declarations.  */
#line 7 "yaccDirectives.y" /* yacc.c:339  */

#include <stdlib.h>
#include <libs/support/misc/general.h>
#include <libs/frontEnd/ast/ast.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/astlist.h>
#include <include/frontEnd/astcons.h>
#include <include/frontEnd/astnode.h>
#include <include/frontEnd/astsel.h>
#include <libs/frontEnd/ast/asttree.h>
#include <include/frontEnd/astrec.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/ast/forttypes.h>
#include <libs/frontEnd/include/walk.h>
#include <libs/frontEnd/fortTree/fortsym.h>

#include <libs/Memoria/annotate/DirectivesInclude.h>

void a2i_error(char*);

char *a2i_DirectiveString;

Directive a2i_Directive;

Boolean a2i_IsDirective;

extern int a2i_lex();
extern void a2i__scan_string(char *);


#line 105 "yaccDirectives.c" /* yacc.c:339  */

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
   by #include "yaccDirectives.h".  */
#ifndef YY_A2I_YACCDIRECTIVES_H_INCLUDED
# define YY_A2I_YACCDIRECTIVES_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int a2i_debug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum a2i_tokentype
  {
    CDIR = 258,
    PREFETCH = 259,
    FLUSH = 260,
    DEP = 261,
    CLUSTER = 262,
    NAME = 263,
    ICONST = 264,
    LPAR = 265,
    RPAR = 266,
    COMMA = 267,
    PLUS = 268,
    MINUS = 269,
    TIMES = 270,
    DIVIDE = 271
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 39 "yaccDirectives.y" /* yacc.c:355  */

   Instruction ival;
   char       *cval;
   Directive   dval;
   AST_INDEX   aval;
  

#line 170 "yaccDirectives.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE a2i_lval;

int a2i_parse (void);

#endif /* !YY_A2I_YACCDIRECTIVES_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 187 "yaccDirectives.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 a2i_type_uint8;
#else
typedef unsigned char a2i_type_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 a2i_type_int8;
#else
typedef signed char a2i_type_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 a2i_type_uint16;
#else
typedef unsigned short int a2i_type_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 a2i_type_int16;
#else
typedef short int a2i_type_int16;
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
/* Suppress an incorrect diagnostic about a2i_lval being uninitialized.  */
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


#if ! defined a2i_overflow || YYERROR_VERBOSE

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
#endif /* ! defined a2i_overflow || YYERROR_VERBOSE */


#if (! defined a2i_overflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union a2i_alloc
{
  a2i_type_int16 a2i_ss_alloc;
  YYSTYPE a2i_vs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union a2i_alloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (a2i_type_int16) + sizeof (YYSTYPE)) \
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
        YYSIZE_T a2i_newbytes;                                            \
        YYCOPY (&a2i_ptr->Stack_alloc, Stack, a2i_size);                    \
        Stack = &a2i_ptr->Stack_alloc;                                    \
        a2i_newbytes = a2i_stacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        a2i_ptr += a2i_newbytes / sizeof (*a2i_ptr);                          \
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
          YYSIZE_T a2i_i;                         \
          for (a2i_i = 0; a2i_i < (Count); a2i_i++)   \
            (Dst)[a2i_i] = (Src)[a2i_i];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  8
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   45

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  17
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  9
/* YYNRULES -- Number of rules.  */
#define YYNRULES  20
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  44

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by a2i_lex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   271

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? a2i_translate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by a2i_lex, without out-of-bounds checking.  */
static const a2i_type_uint8 a2i_translate[] =
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
      15,    16
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const a2i_type_uint8 a2i_rline[] =
{
       0,    60,    60,    66,    72,    77,    83,    90,    96,   100,
     107,   109,   113,   118,   123,   128,   133,   138,   142,   148,
     155
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const a2i_tname[] =
{
  "$end", "error", "$undefined", "CDIR", "PREFETCH", "FLUSH", "DEP",
  "CLUSTER", "NAME", "ICONST", "LPAR", "RPAR", "COMMA", "PLUS", "MINUS",
  "TIMES", "DIVIDE", "$accept", "directive", "command", "subvar",
  "subscript_list", "subscript", "expr", "constexpr", "id", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const a2i_type_uint16 a2i_toknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271
};
# endif

#define YYPACT_NINF -23

#define a2i_pact_value_is_default(Yystate) \
  (!!((Yystate) == (-23)))

#define YYTABLE_NINF -1

#define a2i_table_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const a2i_type_int8 a2i_pact[] =
{
       1,    19,     6,    -1,     7,    -1,    -1,   -23,   -23,   -23,
      20,     3,    30,   -23,     3,   -23,    32,    34,     3,   -23,
     -23,    27,   -23,    27,   -23,    29,   -23,    14,   -23,    34,
       5,   -23,   -23,    27,    27,    27,    27,    27,   -23,   -23,
      23,    23,   -23,   -23
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const a2i_type_uint8 a2i_defact[] =
{
       0,     0,     0,     0,     0,     0,     0,     2,     1,    19,
       0,     0,     0,     6,     0,    20,     0,     0,     0,     3,
       4,     0,     5,     0,    10,     0,     9,    11,    18,    17,
       0,    17,     7,     0,     0,     0,     0,     0,    16,     8,
      12,    13,    14,    15
};

  /* YYPGOTO[NTERM-NUM].  */
static const a2i_type_int8 a2i_pgoto[] =
{
     -23,   -23,   -23,    -9,   -23,    12,   -22,    28,   -11
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const a2i_type_int8 a2i_defgoto[] =
{
      -1,     2,     7,    24,    25,    26,    27,    28,    31
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const a2i_type_uint8 a2i_table[] =
{
      17,    30,    16,    17,     1,    19,     8,    17,     9,    22,
      29,    15,    40,    41,    42,    43,    38,    11,    34,    35,
      36,    37,    29,     3,     4,     5,     6,    34,    35,    36,
      37,    10,    14,    12,    13,    15,     9,    23,    36,    37,
      32,    33,    18,    20,    21,    39
};

static const a2i_type_uint8 a2i_check[] =
{
      11,    23,    11,    14,     3,    14,     0,    18,     9,    18,
      21,     8,    34,    35,    36,    37,    11,    10,    13,    14,
      15,    16,    33,     4,     5,     6,     7,    13,    14,    15,
      16,     3,    12,     5,     6,     8,     9,    10,    15,    16,
      11,    12,    12,    11,    10,    33
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const a2i_type_uint8 a2i_stos[] =
{
       0,     3,    18,     4,     5,     6,     7,    19,     0,     9,
      24,    10,    24,    24,    12,     8,    20,    25,    12,    20,
      11,    10,    20,    10,    20,    21,    22,    23,    24,    25,
      23,    25,    11,    12,    13,    14,    15,    16,    11,    22,
      23,    23,    23,    23
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const a2i_type_uint8 a2i_r1[] =
{
       0,    17,    18,    19,    19,    19,    19,    20,    21,    21,
      22,    22,    23,    23,    23,    23,    23,    23,    23,    24,
      25
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const a2i_type_uint8 a2i_r2[] =
{
       0,     2,     2,     4,     4,     4,     2,     4,     3,     1,
       1,     1,     3,     3,     3,     3,     3,     1,     1,     1,
       1
};


#define a2i_errok         (a2i_errstatus = 0)
#define a2i_clearin       (a2i_char = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto a2i_acceptlab
#define YYABORT         goto a2i_abortlab
#define YYERROR         goto a2i_errorlab


#define YYRECOVERING()  (!!a2i_errstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (a2i_char == YYEMPTY)                                        \
    {                                                           \
      a2i_char = (Token);                                         \
      a2i_lval = (Value);                                         \
      YYPOPSTACK (a2i_len);                                       \
      a2i_state = *a2i_ssp;                                         \
      goto a2i_backup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      a2i_error (YY_("syntax error: cannot back up")); \
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
  if (a2i_debug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (a2i_debug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      a2i__symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
a2i__symbol_value_print (FILE *a2i_output, int a2i_type, YYSTYPE const * const a2i_valuep)
{
  FILE *a2i_o = a2i_output;
  YYUSE (a2i_o);
  if (!a2i_valuep)
    return;
# ifdef YYPRINT
  if (a2i_type < YYNTOKENS)
    YYPRINT (a2i_output, a2i_toknum[a2i_type], *a2i_valuep);
# endif
  YYUSE (a2i_type);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
a2i__symbol_print (FILE *a2i_output, int a2i_type, YYSTYPE const * const a2i_valuep)
{
  YYFPRINTF (a2i_output, "%s %s (",
             a2i_type < YYNTOKENS ? "token" : "nterm", a2i_tname[a2i_type]);

  a2i__symbol_value_print (a2i_output, a2i_type, a2i_valuep);
  YYFPRINTF (a2i_output, ")");
}

/*------------------------------------------------------------------.
| a2i__stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
a2i__stack_print (a2i_type_int16 *a2i_bottom, a2i_type_int16 *a2i_top)
{
  YYFPRINTF (stderr, "Stack now");
  for (; a2i_bottom <= a2i_top; a2i_bottom++)
    {
      int a2i_bot = *a2i_bottom;
      YYFPRINTF (stderr, " %d", a2i_bot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (a2i_debug)                                                  \
    a2i__stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
a2i__reduce_print (a2i_type_int16 *a2i_ssp, YYSTYPE *a2i_vsp, int a2i_rule)
{
  unsigned long int a2i_lno = a2i_rline[a2i_rule];
  int a2i_nrhs = a2i_r2[a2i_rule];
  int a2i_i;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             a2i_rule - 1, a2i_lno);
  /* The symbols being reduced.  */
  for (a2i_i = 0; a2i_i < a2i_nrhs; a2i_i++)
    {
      YYFPRINTF (stderr, "   $%d = ", a2i_i + 1);
      a2i__symbol_print (stderr,
                       a2i_stos[a2i_ssp[a2i_i + 1 - a2i_nrhs]],
                       &(a2i_vsp[(a2i_i + 1) - (a2i_nrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (a2i_debug)                          \
    a2i__reduce_print (a2i_ssp, a2i_vsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int a2i_debug;
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

# ifndef a2i_strlen
#  if defined __GLIBC__ && defined _STRING_H
#   define a2i_strlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
a2i_strlen (const char *a2i_str)
{
  YYSIZE_T a2i_len;
  for (a2i_len = 0; a2i_str[a2i_len]; a2i_len++)
    continue;
  return a2i_len;
}
#  endif
# endif

# ifndef a2i_stpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define a2i_stpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
a2i_stpcpy (char *a2i_dest, const char *a2i_src)
{
  char *a2i_d = a2i_dest;
  const char *a2i_s = a2i_src;

  while ((*a2i_d++ = *a2i_s++) != '\0')
    continue;

  return a2i_d - 1;
}
#  endif
# endif

# ifndef a2i_tnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for a2i_error.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from a2i_tname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
a2i_tnamerr (char *a2i_res, const char *a2i_str)
{
  if (*a2i_str == '"')
    {
      YYSIZE_T a2i_n = 0;
      char const *a2i_p = a2i_str;

      for (;;)
        switch (*++a2i_p)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++a2i_p != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (a2i_res)
              a2i_res[a2i_n] = *a2i_p;
            a2i_n++;
            break;

          case '"':
            if (a2i_res)
              a2i_res[a2i_n] = '\0';
            return a2i_n;
          }
    do_not_strip_quotes: ;
    }

  if (! a2i_res)
    return a2i_strlen (a2i_str);

  return a2i_stpcpy (a2i_res, a2i_str) - a2i_res;
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
a2i_syntax_error (YYSIZE_T *a2i_msg_alloc, char **a2i_msg,
                a2i_type_int16 *a2i_ssp, int a2i_token)
{
  YYSIZE_T a2i_size0 = a2i_tnamerr (YY_NULLPTR, a2i_tname[a2i_token]);
  YYSIZE_T a2i_size = a2i_size0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *a2i_format = YY_NULLPTR;
  /* Arguments of a2i_format. */
  char const *a2i_arg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int a2i_count = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in a2i_char) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated a2i_char.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (a2i_token != YYEMPTY)
    {
      int a2i_n = a2i_pact[*a2i_ssp];
      a2i_arg[a2i_count++] = a2i_tname[a2i_token];
      if (!a2i_pact_value_is_default (a2i_n))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int a2i_xbegin = a2i_n < 0 ? -a2i_n : 0;
          /* Stay within bounds of both a2i_check and a2i_tname.  */
          int a2i_checklim = YYLAST - a2i_n + 1;
          int a2i_xend = a2i_checklim < YYNTOKENS ? a2i_checklim : YYNTOKENS;
          int a2i_x;

          for (a2i_x = a2i_xbegin; a2i_x < a2i_xend; ++a2i_x)
            if (a2i_check[a2i_x + a2i_n] == a2i_x && a2i_x != YYTERROR
                && !a2i_table_value_is_error (a2i_table[a2i_x + a2i_n]))
              {
                if (a2i_count == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    a2i_count = 1;
                    a2i_size = a2i_size0;
                    break;
                  }
                a2i_arg[a2i_count++] = a2i_tname[a2i_x];
                {
                  YYSIZE_T a2i_size1 = a2i_size + a2i_tnamerr (YY_NULLPTR, a2i_tname[a2i_x]);
                  if (! (a2i_size <= a2i_size1
                         && a2i_size1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  a2i_size = a2i_size1;
                }
              }
        }
    }

  switch (a2i_count)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        a2i_format = S;                       \
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
    YYSIZE_T a2i_size1 = a2i_size + a2i_strlen (a2i_format);
    if (! (a2i_size <= a2i_size1 && a2i_size1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    a2i_size = a2i_size1;
  }

  if (*a2i_msg_alloc < a2i_size)
    {
      *a2i_msg_alloc = 2 * a2i_size;
      if (! (a2i_size <= *a2i_msg_alloc
             && *a2i_msg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *a2i_msg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *a2i_p = *a2i_msg;
    int a2i_i = 0;
    while ((*a2i_p = *a2i_format) != '\0')
      if (*a2i_p == '%' && a2i_format[1] == 's' && a2i_i < a2i_count)
        {
          a2i_p += a2i_tnamerr (a2i_p, a2i_arg[a2i_i++]);
          a2i_format += 2;
        }
      else
        {
          a2i_p++;
          a2i_format++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
a2i_destruct (const char *a2i_msg, int a2i_type, YYSTYPE *a2i_valuep)
{
  YYUSE (a2i_valuep);
  if (!a2i_msg)
    a2i_msg = "Deleting";
  YY_SYMBOL_PRINT (a2i_msg, a2i_type, a2i_valuep, a2i_locationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (a2i_type);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int a2i_char;

/* The semantic value of the lookahead symbol.  */
YYSTYPE a2i_lval;
/* Number of syntax errors so far.  */
int a2i_nerrs;


/*----------.
| a2i_parse.  |
`----------*/

int
a2i_parse (void)
{
    int a2i_state;
    /* Number of tokens to shift before error messages enabled.  */
    int a2i_errstatus;

    /* The stacks and their tools:
       'a2i_ss': related to states.
       'a2i_vs': related to semantic values.

       Refer to the stacks through separate pointers, to allow a2i_overflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    a2i_type_int16 a2i_ssa[YYINITDEPTH];
    a2i_type_int16 *a2i_ss;
    a2i_type_int16 *a2i_ssp;

    /* The semantic value stack.  */
    YYSTYPE a2i_vsa[YYINITDEPTH];
    YYSTYPE *a2i_vs;
    YYSTYPE *a2i_vsp;

    YYSIZE_T a2i_stacksize;

  int a2i_n;
  int a2i_result;
  /* Lookahead token as an internal (translated) token number.  */
  int a2i_token = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE a2i_val;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char a2i_msgbuf[128];
  char *a2i_msg = a2i_msgbuf;
  YYSIZE_T a2i_msg_alloc = sizeof a2i_msgbuf;
#endif

#define YYPOPSTACK(N)   (a2i_vsp -= (N), a2i_ssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int a2i_len = 0;

  a2i_ssp = a2i_ss = a2i_ssa;
  a2i_vsp = a2i_vs = a2i_vsa;
  a2i_stacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  a2i_state = 0;
  a2i_errstatus = 0;
  a2i_nerrs = 0;
  a2i_char = YYEMPTY; /* Cause a token to be read.  */
  goto a2i_setstate;

/*------------------------------------------------------------.
| a2i_newstate -- Push a new state, which is found in a2i_state.  |
`------------------------------------------------------------*/
 a2i_newstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  a2i_ssp++;

 a2i_setstate:
  *a2i_ssp = a2i_state;

  if (a2i_ss + a2i_stacksize - 1 <= a2i_ssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T a2i_size = a2i_ssp - a2i_ss + 1;

#ifdef a2i_overflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *a2i_vs1 = a2i_vs;
        a2i_type_int16 *a2i_ss1 = a2i_ss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if a2i_overflow is a macro.  */
        a2i_overflow (YY_("memory exhausted"),
                    &a2i_ss1, a2i_size * sizeof (*a2i_ssp),
                    &a2i_vs1, a2i_size * sizeof (*a2i_vsp),
                    &a2i_stacksize);

        a2i_ss = a2i_ss1;
        a2i_vs = a2i_vs1;
      }
#else /* no a2i_overflow */
# ifndef YYSTACK_RELOCATE
      goto a2i_exhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= a2i_stacksize)
        goto a2i_exhaustedlab;
      a2i_stacksize *= 2;
      if (YYMAXDEPTH < a2i_stacksize)
        a2i_stacksize = YYMAXDEPTH;

      {
        a2i_type_int16 *a2i_ss1 = a2i_ss;
        union a2i_alloc *a2i_ptr =
          (union a2i_alloc *) YYSTACK_ALLOC (YYSTACK_BYTES (a2i_stacksize));
        if (! a2i_ptr)
          goto a2i_exhaustedlab;
        YYSTACK_RELOCATE (a2i_ss_alloc, a2i_ss);
        YYSTACK_RELOCATE (a2i_vs_alloc, a2i_vs);
#  undef YYSTACK_RELOCATE
        if (a2i_ss1 != a2i_ssa)
          YYSTACK_FREE (a2i_ss1);
      }
# endif
#endif /* no a2i_overflow */

      a2i_ssp = a2i_ss + a2i_size - 1;
      a2i_vsp = a2i_vs + a2i_size - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) a2i_stacksize));

      if (a2i_ss + a2i_stacksize - 1 <= a2i_ssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", a2i_state));

  if (a2i_state == YYFINAL)
    YYACCEPT;

  goto a2i_backup;

/*-----------.
| a2i_backup.  |
`-----------*/
a2i_backup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  a2i_n = a2i_pact[a2i_state];
  if (a2i_pact_value_is_default (a2i_n))
    goto a2i_default;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (a2i_char == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      a2i_char = a2i_lex ();
    }

  if (a2i_char <= YYEOF)
    {
      a2i_char = a2i_token = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      a2i_token = YYTRANSLATE (a2i_char);
      YY_SYMBOL_PRINT ("Next token is", a2i_token, &a2i_lval, &a2i_lloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  a2i_n += a2i_token;
  if (a2i_n < 0 || YYLAST < a2i_n || a2i_check[a2i_n] != a2i_token)
    goto a2i_default;
  a2i_n = a2i_table[a2i_n];
  if (a2i_n <= 0)
    {
      if (a2i_table_value_is_error (a2i_n))
        goto a2i_errlab;
      a2i_n = -a2i_n;
      goto a2i_reduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (a2i_errstatus)
    a2i_errstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", a2i_token, &a2i_lval, &a2i_lloc);

  /* Discard the shifted token.  */
  a2i_char = YYEMPTY;

  a2i_state = a2i_n;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++a2i_vsp = a2i_lval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto a2i_newstate;


/*-----------------------------------------------------------.
| a2i_default -- do the default action for the current state.  |
`-----------------------------------------------------------*/
a2i_default:
  a2i_n = a2i_defact[a2i_state];
  if (a2i_n == 0)
    goto a2i_errlab;
  goto a2i_reduce;


/*-----------------------------.
| a2i_reduce -- Do a reduction.  |
`-----------------------------*/
a2i_reduce:
  /* a2i_n is the number of a rule to reduce with.  */
  a2i_len = a2i_r2[a2i_n];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  a2i_val = a2i_vsp[1-a2i_len];


  YY_REDUCE_PRINT (a2i_n);
  switch (a2i_n)
    {
        case 2:
#line 61 "yaccDirectives.y" /* yacc.c:1646  */
    {
	      a2i_Directive = (a2i_vsp[0].dval);
	     }
#line 1285 "yaccDirectives.c" /* yacc.c:1646  */
    break;

  case 3:
#line 67 "yaccDirectives.y" /* yacc.c:1646  */
    {
	     (a2i_val.dval).Instr = PrefetchInstruction;
	     (a2i_val.dval).Subscript = (a2i_vsp[0].aval);
	     (a2i_val.dval).DirectiveNumber = atoi(gen_get_text((a2i_vsp[-2].aval)));
	    }
#line 1295 "yaccDirectives.c" /* yacc.c:1646  */
    break;

  case 4:
#line 73 "yaccDirectives.y" /* yacc.c:1646  */
    {
	     (a2i_val.dval).Instr = FlushInstruction;
	     (a2i_val.dval).Subscript = (a2i_vsp[-1].aval);
	    }
#line 1304 "yaccDirectives.c" /* yacc.c:1646  */
    break;

  case 5:
#line 78 "yaccDirectives.y" /* yacc.c:1646  */
    {
	      (a2i_val.dval).Instr = Dependence;
	      (a2i_val.dval).Subscript = (a2i_vsp[0].aval);
	      (a2i_val.dval).DirectiveNumber = atoi(gen_get_text((a2i_vsp[-2].aval)));
	    }
#line 1314 "yaccDirectives.c" /* yacc.c:1646  */
    break;

  case 6:
#line 84 "yaccDirectives.y" /* yacc.c:1646  */
    {
	      (a2i_val.dval).Instr = Cluster;
	      (a2i_val.dval).Cluster = atoi(gen_get_text((a2i_vsp[0].aval)));
	    }
#line 1323 "yaccDirectives.c" /* yacc.c:1646  */
    break;

  case 7:
#line 91 "yaccDirectives.y" /* yacc.c:1646  */
    {
   (a2i_val.aval) = gen_SUBSCRIPT((a2i_vsp[-3].aval),(a2i_vsp[-1].aval));
  }
#line 1331 "yaccDirectives.c" /* yacc.c:1646  */
    break;

  case 8:
#line 97 "yaccDirectives.y" /* yacc.c:1646  */
    {
		   (a2i_val.aval) = list_insert_last((a2i_vsp[-2].aval),(a2i_vsp[0].aval));
		  }
#line 1339 "yaccDirectives.c" /* yacc.c:1646  */
    break;

  case 9:
#line 101 "yaccDirectives.y" /* yacc.c:1646  */
    {
		   (a2i_val.aval) = list_create((a2i_vsp[0].aval));
		  }
#line 1347 "yaccDirectives.c" /* yacc.c:1646  */
    break;

  case 10:
#line 108 "yaccDirectives.y" /* yacc.c:1646  */
    {(a2i_val.aval) = (a2i_vsp[0].aval);}
#line 1353 "yaccDirectives.c" /* yacc.c:1646  */
    break;

  case 11:
#line 110 "yaccDirectives.y" /* yacc.c:1646  */
    {(a2i_val.aval) = (a2i_vsp[0].aval);}
#line 1359 "yaccDirectives.c" /* yacc.c:1646  */
    break;

  case 12:
#line 114 "yaccDirectives.y" /* yacc.c:1646  */
    {
          (a2i_val.aval) = gen_BINARY_PLUS((a2i_vsp[-2].aval),(a2i_vsp[0].aval));
         }
#line 1367 "yaccDirectives.c" /* yacc.c:1646  */
    break;

  case 13:
#line 119 "yaccDirectives.y" /* yacc.c:1646  */
    {
          (a2i_val.aval) = gen_BINARY_MINUS((a2i_vsp[-2].aval),(a2i_vsp[0].aval));
         }
#line 1375 "yaccDirectives.c" /* yacc.c:1646  */
    break;

  case 14:
#line 124 "yaccDirectives.y" /* yacc.c:1646  */
    {
          (a2i_val.aval) = gen_BINARY_TIMES((a2i_vsp[-2].aval),(a2i_vsp[0].aval));
         }
#line 1383 "yaccDirectives.c" /* yacc.c:1646  */
    break;

  case 15:
#line 129 "yaccDirectives.y" /* yacc.c:1646  */
    {
          (a2i_val.aval) = gen_BINARY_DIVIDE((a2i_vsp[-2].aval),(a2i_vsp[0].aval));
         }
#line 1391 "yaccDirectives.c" /* yacc.c:1646  */
    break;

  case 16:
#line 134 "yaccDirectives.y" /* yacc.c:1646  */
    {
          (a2i_val.aval) = (a2i_vsp[-1].aval);
         }
#line 1399 "yaccDirectives.c" /* yacc.c:1646  */
    break;

  case 17:
#line 139 "yaccDirectives.y" /* yacc.c:1646  */
    { 
	(a2i_val.aval) =(a2i_vsp[0].aval);
       }
#line 1407 "yaccDirectives.c" /* yacc.c:1646  */
    break;

  case 18:
#line 143 "yaccDirectives.y" /* yacc.c:1646  */
    { 
	(a2i_val.aval) =(a2i_vsp[0].aval);
       }
#line 1415 "yaccDirectives.c" /* yacc.c:1646  */
    break;

  case 19:
#line 149 "yaccDirectives.y" /* yacc.c:1646  */
    {
	(a2i_val.aval) = gen_CONSTANT();
	gen_put_text((a2i_val.aval),(a2i_vsp[0].cval),STR_CONSTANT_INTEGER); 
       }
#line 1424 "yaccDirectives.c" /* yacc.c:1646  */
    break;

  case 20:
#line 156 "yaccDirectives.y" /* yacc.c:1646  */
    { 
	(a2i_val.aval) = gen_IDENTIFIER();
	gen_put_text((a2i_val.aval),(a2i_vsp[0].cval),STR_IDENTIFIER); 
       }
#line 1433 "yaccDirectives.c" /* yacc.c:1646  */
    break;


#line 1437 "yaccDirectives.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter a2i_char, and that requires
     that a2i_token be updated with the new translation.  We take the
     approach of translating immediately before every use of a2i_token.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering a2i_char or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", a2i_r1[a2i_n], &a2i_val, &a2i_loc);

  YYPOPSTACK (a2i_len);
  a2i_len = 0;
  YY_STACK_PRINT (a2i_ss, a2i_ssp);

  *++a2i_vsp = a2i_val;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  a2i_n = a2i_r1[a2i_n];

  a2i_state = a2i_pgoto[a2i_n - YYNTOKENS] + *a2i_ssp;
  if (0 <= a2i_state && a2i_state <= YYLAST && a2i_check[a2i_state] == *a2i_ssp)
    a2i_state = a2i_table[a2i_state];
  else
    a2i_state = a2i_defgoto[a2i_n - YYNTOKENS];

  goto a2i_newstate;


/*--------------------------------------.
| a2i_errlab -- here on detecting error.  |
`--------------------------------------*/
a2i_errlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  a2i_token = a2i_char == YYEMPTY ? YYEMPTY : YYTRANSLATE (a2i_char);

  /* If not already recovering from an error, report this error.  */
  if (!a2i_errstatus)
    {
      ++a2i_nerrs;
#if ! YYERROR_VERBOSE
      a2i_error (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR a2i_syntax_error (&a2i_msg_alloc, &a2i_msg, \
                                        a2i_ssp, a2i_token)
      {
        char const *a2i_msgp = YY_("syntax error");
        int a2i_syntax_error_status;
        a2i_syntax_error_status = YYSYNTAX_ERROR;
        if (a2i_syntax_error_status == 0)
          a2i_msgp = a2i_msg;
        else if (a2i_syntax_error_status == 1)
          {
            if (a2i_msg != a2i_msgbuf)
              YYSTACK_FREE (a2i_msg);
            a2i_msg = (char *) YYSTACK_ALLOC (a2i_msg_alloc);
            if (!a2i_msg)
              {
                a2i_msg = a2i_msgbuf;
                a2i_msg_alloc = sizeof a2i_msgbuf;
                a2i_syntax_error_status = 2;
              }
            else
              {
                a2i_syntax_error_status = YYSYNTAX_ERROR;
                a2i_msgp = a2i_msg;
              }
          }
        a2i_error (a2i_msgp);
        if (a2i_syntax_error_status == 2)
          goto a2i_exhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (a2i_errstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (a2i_char <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (a2i_char == YYEOF)
            YYABORT;
        }
      else
        {
          a2i_destruct ("Error: discarding",
                      a2i_token, &a2i_lval);
          a2i_char = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto a2i_errlab1;


/*---------------------------------------------------.
| a2i_errorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
a2i_errorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label a2i_errorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto a2i_errorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (a2i_len);
  a2i_len = 0;
  YY_STACK_PRINT (a2i_ss, a2i_ssp);
  a2i_state = *a2i_ssp;
  goto a2i_errlab1;


/*-------------------------------------------------------------.
| a2i_errlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
a2i_errlab1:
  a2i_errstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      a2i_n = a2i_pact[a2i_state];
      if (!a2i_pact_value_is_default (a2i_n))
        {
          a2i_n += YYTERROR;
          if (0 <= a2i_n && a2i_n <= YYLAST && a2i_check[a2i_n] == YYTERROR)
            {
              a2i_n = a2i_table[a2i_n];
              if (0 < a2i_n)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (a2i_ssp == a2i_ss)
        YYABORT;


      a2i_destruct ("Error: popping",
                  a2i_stos[a2i_state], a2i_vsp);
      YYPOPSTACK (1);
      a2i_state = *a2i_ssp;
      YY_STACK_PRINT (a2i_ss, a2i_ssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++a2i_vsp = a2i_lval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", a2i_stos[a2i_n], a2i_vsp, a2i_lsp);

  a2i_state = a2i_n;
  goto a2i_newstate;


/*-------------------------------------.
| a2i_acceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
a2i_acceptlab:
  a2i_result = 0;
  goto a2i_return;

/*-----------------------------------.
| a2i_abortlab -- YYABORT comes here.  |
`-----------------------------------*/
a2i_abortlab:
  a2i_result = 1;
  goto a2i_return;

#if !defined a2i_overflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| a2i_exhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
a2i_exhaustedlab:
  a2i_error (YY_("memory exhausted"));
  a2i_result = 2;
  /* Fall through.  */
#endif

a2i_return:
  if (a2i_char != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      a2i_token = YYTRANSLATE (a2i_char);
      a2i_destruct ("Cleanup: discarding lookahead",
                  a2i_token, &a2i_lval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (a2i_len);
  YY_STACK_PRINT (a2i_ss, a2i_ssp);
  while (a2i_ssp != a2i_ss)
    {
      a2i_destruct ("Cleanup: popping",
                  a2i_stos[*a2i_ssp], a2i_vsp);
      YYPOPSTACK (1);
    }
#ifndef a2i_overflow
  if (a2i_ss != a2i_ssa)
    YYSTACK_FREE (a2i_ss);
#endif
#if YYERROR_VERBOSE
  if (a2i_msg != a2i_msgbuf)
    YYSTACK_FREE (a2i_msg);
#endif
  return a2i_result;
}
#line 161 "yaccDirectives.y" /* yacc.c:1906  */


void a2i_error(s) 

  char *s;

  {
   a2i_IsDirective = false;
  }

static int SetIndexExprNodeType(node,ft_SymTable)

  AST_INDEX node;
  SymDescriptor ft_SymTable;

  {
   int type;

     if (is_identifier(node))
       type = fst_GetField(ft_SymTable,gen_get_text(node),SYMTAB_TYPE);
     else
       type = TYPE_INTEGER;
     gen_put_real_type(node,type);
     gen_put_converted_type(node,TYPE_INTEGER);
     return(WALK_CONTINUE);
  }

static void SetTypes(Subscript,ft_SymTable)

  AST_INDEX Subscript;
  SymDescriptor ft_SymTable;

  {
   AST_INDEX name;
   int type;

     name = gen_SUBSCRIPT_get_name(Subscript);
     type = fst_GetField(ft_SymTable,gen_get_text(name),SYMTAB_TYPE);
     gen_put_real_type(Subscript,type);
     gen_put_converted_type(Subscript,type);
     gen_put_real_type(name,type);
     gen_put_converted_type(name,type);
     walk_expression(gen_SUBSCRIPT_get_rvalue_LIST(Subscript),SetIndexExprNodeType,
		     NOFUNC,(Generic)ft_SymTable);
  }

Boolean a2i_string_parse (str,Dir,symtab)

  char *str;
  Directive *Dir;
  SymDescriptor symtab;

  {
   /* tell flex to scan the string rather than
      a file */

   a2i__scan_string(str); 
   a2i_IsDirective = true;
   a2i_parse();
   if (a2i_IsDirective)
     {
      Dir->Instr = a2i_Directive.Instr;
      Dir->Subscript = a2i_Directive.Subscript;
      Dir->DirectiveNumber = a2i_Directive.DirectiveNumber;
      if (Dir->Instr != Cluster)
        SetTypes(Dir->Subscript,symtab);
      else
	Dir->Cluster = a2i_Directive.Cluster;
      return true;
     }
   else
     return false;
  }
