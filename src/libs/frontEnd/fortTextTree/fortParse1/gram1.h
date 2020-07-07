/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

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
#line 72 "gram1.y" /* yacc.c:1909  */

    FortTreeNode astptr;
    fx_StatToken statval;
    KWD_OPT  kwdval;
    Boolean  boolean;
    

#line 247 "gram1.tab.h" /* yacc.c:1909  */
};

typedef union YY1STYPE YY1STYPE;
# define YY1STYPE_IS_TRIVIAL 1
# define YY1STYPE_IS_DECLARED 1
#endif


extern YY1STYPE yy1lval;

int yy1parse (void);

#endif /* !YY1_YY1_GRAM1_TAB_H_INCLUDED  */
