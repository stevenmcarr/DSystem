/* $Id: chars.C,v 1.1 1997/04/28 20:18:07 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/

#include <libs/support/misc/general.h>

#include <libs/frontEnd/ast/strutil.h>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/support/lists/list.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/frontEnd/include/gi.h>
#include <libs/frontEnd/ast/asttree.h>
#include <libs/frontEnd/ast/aphelper.h>
#include <stdio.h>

#include <libs/f2i/ai.h>
#include <libs/f2i/sym.h>
#include <libs/f2i/f2i_label.h>
#include <libs/f2i/char.h>

#include <libs/f2i/mnemonics.h>

/* forward declarations */



/* generate iloc to perform a move character */
/* long of size "len" starting at "dst"      */
void generate_mvcl(int src, int dst, int len)
  // int src, dst, len;
{
  int counter, one, sum, space, srcplus, dstplus, loop, end;

  /* It takes a lot of registers to get this one right.		*/
  /* We load up a counter, initialize it to one, and then loop  */
  /* through the string, one byte at a time, moving characters. */
  /* This is the wrong way to do it... we should at least check */
  /* for word oriented stuff... or call bcopy().		*/

  generate(0, COMMENT, 0, 0, 0, "Start of MVCL");

  one = getIntConstantInRegister("1");

  /* First - make copies of the argument pointers */
  srcplus = TempReg(src, one, iADD, TYPE_INTEGER);
  generate_move(srcplus, src, TYPE_INTEGER);
  src = srcplus;

  dstplus = TempReg(dst, one, iADD, TYPE_INTEGER);
  generate_move(dstplus, dst, TYPE_INTEGER);
  dst = dstplus;


  space	  = StrTempReg("C", src, TYPE_INTEGER);
  counter = StrTempReg("MVCL", src, TYPE_INTEGER);
  sum	  = TempReg(counter, one, iADD, TYPE_INTEGER);
  srcplus = TempReg(src, one, iADD, TYPE_INTEGER);
  dstplus = TempReg(dst, one, iADD, TYPE_INTEGER);

  loop	  = LABEL_NEW;
  end	  = LABEL_NEW;
  
  generate(0, COMMENT, 0, 0, 0, "counter <- 1");
  generate_move(counter, one, TYPE_INTEGER);
  generate_branch(0, LT, len, counter, TYPE_INTEGER,
			 end, loop, "check for len<1");

  generate(loop, NOP, 0, 0, 0, "Get a byte");
  generate_load(space, src, TYPE_CHARACTER, -1, "&unknown");
  generate(0, iADD, src, one, srcplus, "Bump src ptr");
  generate_move(src, srcplus, TYPE_INTEGER);

  generate_store(dst, space, TYPE_INTEGER, -1,"&unknown");
  generate(0, iADD, dst, one, dstplus, "Bump dst ptr");
  generate_move(dst, dstplus, TYPE_INTEGER);

  generate(0, iADD, counter, one, sum, "bump counter");
  generate_move(counter, sum, TYPE_INTEGER);
  generate_branch(0, GE, len, counter, TYPE_INTEGER, loop, end,
		"and go around again");

  generate(end, NOP, 0, 0, 0, "End of MVCL");
} /* generate_mvcl */




/* generate iloc to store the filler into  */
/* "len" bytes in memory starting at "dst" */
void generate_fill(int dst, int filler, int len)
  // int dst, filler, len;
{
  int counter, one, dstplus, sum, loop, end;

  one 	  = getIntConstantInRegister("1");

  /* first, make a copy of the destination pointer */
  dstplus = TempReg(dst, one, iADD, TYPE_INTEGER);
  generate_move(dstplus, dst, TYPE_INTEGER);
  dst = dstplus;

  counter = StrTempReg("FILL", filler, TYPE_INTEGER);
  dstplus = TempReg(dst, one, iADD, TYPE_INTEGER);
  sum	  = TempReg(counter, one, iADD, TYPE_INTEGER);

  loop = LABEL_NEW;
  end  = LABEL_NEW;

  generate(0, NOP, 0, 0, 0, "Start of FILL");
  generate_move(counter, one, TYPE_INTEGER);
  generate_branch(0, LT, len, counter, TYPE_INTEGER, end, loop,
			"Check for len < 0");

  generate(loop, NOP, 0, 0, 0, "Put a byte");
  generate_store(dst, filler, TYPE_CHARACTER, -1,"&unknown");
  generate(0, iADD, dst, one, dstplus, "Bump dst ptr");
  generate_move(dst, dstplus, TYPE_INTEGER);

  generate(0, iADD, counter, one, sum, "Bump counter");
  generate_move(counter, sum, TYPE_INTEGER);

  generate_branch(0, GE, len, counter, TYPE_INTEGER, loop, end, 
			"and around again");

  generate(end, NOP, 0, 0, 0, "End of FILL");
} /* generate_fill */




/* Simulate a character comparison --- ideally, this should */
/* be done by a library call or have its own opcode.	    */
int generate_char_compare(int reg1, int reg2, int len1, int len2, int compare)
     // int reg1, reg2, len1, len2, compare;
{
  int start, end, test1, test2;				/* labels */
  int a1, a2, l1, l2, t1, t2, one, zero, res, spc;	/* registers */
  int tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, cmp2;		/* more registers */

  /* initialize labels */
  start = LABEL_NEW;
  end   = LABEL_NEW;
  test1 = LABEL_NEW;
  test2 = LABEL_NEW;

  /* allocate register numbers */
  spc	= (int) getConstantInRegFromInt(BLANK_CHARACTER);
  one	= (int) getIntConstantInRegister("1");
  zero  = (int) getIntConstantInRegister("0");
  a1 = StrTempReg("CMP*add",reg1,TYPE_INTEGER);
  a2 = StrTempReg("CMP*add",reg2,TYPE_INTEGER);
  l1 = StrTempReg("CMP*len1",one,TYPE_INTEGER);
  l2 = StrTempReg("CMP*len2",one,TYPE_INTEGER);
  t1 = StrTempReg("CMP*chr",a1,TYPE_INTEGER);
  t2 = StrTempReg("CMP*chr",a2,TYPE_INTEGER);
  cmp2 = TempReg(l1,l2,iMAX,TYPE_INTEGER);

  tmp1 = StrTempReg("CMP*tmp",a1,TYPE_CHARACTER);
  tmp2 = StrTempReg("CMP*tmp",a2,TYPE_CHARACTER);
  tmp3 = TempReg(a1,one,iADD,TYPE_INTEGER);
  tmp4 = TempReg(a2,one,iADD,TYPE_INTEGER);
  tmp5 = TempReg(l1,one,iSUB,TYPE_INTEGER);
  tmp6 = TempReg(l2,one,iSUB,TYPE_INTEGER);


  res = StrTempReg("CMP*res",one,TYPE_LOGICAL);

  /* initialize register values */
  generate_move(a1,reg1,TYPE_INTEGER);
  generate_move(a2,reg2,TYPE_INTEGER);
  generate_move(l1,len1,TYPE_INTEGER);
  generate_move(l2,len2,TYPE_INTEGER);
  generate_move(res,zero,TYPE_INTEGER);
 
  /* start of the basic compare loop */
  generate (start, NOP, 0, 0, 0, "Start of compare loop");
  generate_move (t1, spc, TYPE_INTEGER);
  generate_branch(0, NE, l1, zero, TYPE_INTEGER, test1, NO_TARGET, NOCOMMENT);
  generate_load (tmp1, a1, TYPE_CHARACTER, -1, "&unknown");
  generate_move (t1, tmp1, TYPE_INTEGER);
  generate(test1, NOP, 0, 0, 0, NOCOMMENT);
  generate_move (t2, spc, TYPE_INTEGER);
  generate_branch(0, NE, l2, zero, TYPE_INTEGER, test2, NO_TARGET, NOCOMMENT);
  generate_load(tmp2, a2, TYPE_CHARACTER, -1, "&unknown");
  generate_move(t2, tmp2, TYPE_INTEGER);
  
  /* now t1 and t2 are loaded with the next character or a blank if */
  /* one is longer than the other.  Compare them and branch if the  */
  /* test condition does not hold.				    */
  switch (compare)
  {
    case GT:
      generate_branch(test2, LT, t1, t2, TYPE_INTEGER, end, NO_TARGET,
			NOCOMMENT);
      break;
    case GE:
      generate_branch(test2, LE, t1, t2, TYPE_INTEGER, end, NO_TARGET,
			NOCOMMENT);
      break;
    case LT:
      generate_branch(test2, GE, t1, t2, TYPE_INTEGER, end, NO_TARGET,
			NOCOMMENT);
      break;
    case LE:
      generate_branch(test2, GT, t1, t2, TYPE_INTEGER, end, NO_TARGET,
			NOCOMMENT);
      break;
    case EQ:
      generate_branch(test2, NE, t1, t2, TYPE_INTEGER, end, NO_TARGET,
			NOCOMMENT);
      break;
    case NE:
      generate_branch(test2, EQ, t1, t2, TYPE_INTEGER, end, NO_TARGET,
			NOCOMMENT);
      break;
    default:
      ERROR("generate_compare","Unknown comparison type",FATAL);
      break;
  }
  
  /* now increment & decrement the counters */
  generate (0, iADD, a1, one, tmp3, NOCOMMENT);
  generate_move (a1, tmp3, TYPE_INTEGER);
  generate (0, iADD,a2, one, tmp4, NOCOMMENT);
  generate_move (a2, tmp4, TYPE_INTEGER);
  generate (0, iSUB, l1, one, tmp5, NOCOMMENT);
  generate_move (l1, tmp5, TYPE_INTEGER);
  generate (0, iSUB, l2, one, tmp6, NOCOMMENT);
  generate_move (l2, tmp6, TYPE_INTEGER);
  
  /* if characters left, branch to start */
  generate (0, iMAX, l1, l2, cmp2, NOCOMMENT);
  generate_branch(0, EQ, cmp2, zero, TYPE_INTEGER, start,
			NO_TARGET, NOCOMMENT);

  /* if this code is reached, condition is TRUE */
  generate_move (res, one, TYPE_INTEGER);

  /* our end */
  generate (end, NOP, 0, 0, 0, NOCOMMENT);
  return(res);
} /* generate_char_compare */




/* simulate the INDEX intrinsic using a naive algorithm:     */
/* test (s1 == &s2[0]), (s1 == &s2[1]), (s1 == &s2[2]), etc. */
void generate_index(int address1, int address2, int length1, int length2, int result)
  // int address1, address2, length1, length2, result;
{
  int loop, cont, exit;  /* labels */
  
  /* the following are all registers */
  int len1, len2, add1, add2, one, ctr, zero, mlen, a2, tmp1;
  int t1, t2, tmp2, tmp3, tmp4, tmp5, tmp6;
  
  /* initialize labels */
  loop = LABEL_NEW;
  cont = LABEL_NEW;
  exit = LABEL_NEW;

  /* constant registers */
  one = getIntConstantInRegister("1");
  zero = getIntConstantInRegister("0");

  /* targets of moves -- first move from second arg */
  len1 = StrTempReg("INDX*len", length1, TYPE_INTEGER);
  len2 = StrTempReg("INDX*len", length2, TYPE_INTEGER);
  add1 = StrTempReg("INDX*add", address1, TYPE_INTEGER);
  add2 = StrTempReg("INDX*add", address2, TYPE_INTEGER);
  a2   = StrTempReg("INDX*add", add2, TYPE_INTEGER);
  ctr  = StrTempReg("INDX*ctr", one, TYPE_INTEGER);
  mlen = StrTempReg("INDX*ctr", zero, TYPE_INTEGER);
  
  /* targets of ldb */
  t1   = StrTempReg("*INDEX*", add1, TYPE_INTEGER);
  t2   = StrTempReg("*INDEX*", add2, TYPE_INTEGER);

  /* temporaries for adds, compares, etc */
  tmp1 = TempReg(add1, mlen, iADD, TYPE_INTEGER);
  tmp2 = TempReg(mlen, one, iADD, TYPE_INTEGER);
  tmp3 = TempReg(a2, one, iADD, TYPE_INTEGER);
  tmp4 = TempReg(ctr, one, iADD, TYPE_INTEGER);
  tmp5 = TempReg(add1, one, iADD, TYPE_INTEGER);
  tmp6 = TempReg(len1, one, iADD, TYPE_INTEGER);
		 
  /* notice the switch -- add1 and len1 refer to the original, longer	*/
  /* string while add2 and len2 refer to the shorter string (which 	*/
  /* should be inside add1 and len1)					*/

  /* initialize */
  generate_move(len1, length2, TYPE_INTEGER);
  generate_move(len2, length1, TYPE_INTEGER);
  generate_move(add1, address2, TYPE_INTEGER);
  generate_move(add2, address1, TYPE_INTEGER);
  generate_move(ctr, one, TYPE_INTEGER);
  generate_move(mlen, zero, TYPE_INTEGER);
  generate_move(a2, add2, TYPE_INTEGER);

  /* main comparison loop */
  generate(loop, iADD, add1, mlen, tmp1, NOCOMMENT);
  generate_load(t1, tmp1, TYPE_CHARACTER, -1, "&unknown");
  generate_load(t2, a2, TYPE_CHARACTER, -1, "&unknown");
  generate_branch(0, NE, t1, t2, TYPE_INTEGER, cont, NO_TARGET, NOCOMMENT);
  generate(0, iADD, mlen, one, tmp2, NOCOMMENT);
  generate_move(mlen, tmp2, TYPE_INTEGER);
  generate(0, iADD, a2, one, tmp3, NOCOMMENT);
  generate_move(a2, tmp3, TYPE_INTEGER);
  generate_branch(0, NE, len2, mlen, TYPE_INTEGER, loop, NO_TARGET, NOCOMMENT);

  /* found - return ctr */
  generate_move(result, ctr, TYPE_INTEGER);
  generate(0, JMPl, exit, 0, 0, NOCOMMENT);

  /* not found - try next character */
  generate(cont, iADD, ctr, one, tmp4, NOCOMMENT);
  generate_move(ctr, tmp4, TYPE_INTEGER);
  generate(0, iADD, add1, one, tmp5, NOCOMMENT);
  generate_move(add1, tmp5, TYPE_INTEGER);
  generate(0, iSUB, len1, one, tmp6, NOCOMMENT);
  generate_move(len1, tmp6, TYPE_INTEGER);
  generate_move(a2, add2, TYPE_INTEGER);
  generate_move(mlen, zero, TYPE_INTEGER);
  generate_branch(0, LT, len2, len1, TYPE_INTEGER, loop, NO_TARGET, NOCOMMENT);

  /* not found && no more characters -- return 0 */
  generate_move(result, zero, TYPE_INTEGER);
  generate(exit, NOP, 0, 0, 0, NOCOMMENT);
} /* generate_index */

