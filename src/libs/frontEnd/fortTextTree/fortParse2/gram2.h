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
#line 79 "gram2.y" /* yacc.c:1909  */

    FortTreeNode astptr;
    fx_StatToken statval;
    fx_EndingStat endval;
    

#line 168 "gram2.tab.h" /* yacc.c:1909  */
};

typedef union YY2STYPE YY2STYPE;
# define YY2STYPE_IS_TRIVIAL 1
# define YY2STYPE_IS_DECLARED 1
#endif


extern YY2STYPE yy2lval;

int yy2parse (void);

#endif /* !YY2_YY2_GRAM2_TAB_H_INCLUDED  */
