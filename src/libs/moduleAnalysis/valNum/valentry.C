/* $Id: valentry.C,v 1.11 1997/03/11 14:36:22 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
#include <libs/moduleAnalysis/valNum/val.i>
#include <libs/frontEnd/ast/astutil.h>
#include <libs/frontEnd/ast/gen.h>
#include <libs/support/strings/rn_string.h>
#include <libs/support/file/FormattedFile.h>

inline int abs(int n) { return ((n < 0)? -n : n); }

static int sizesByType[] = {3,	// TOP_TYPE
			    4,	// CONST
			    3,	// TEXT -- characters of string starting at 3!
			    8,	// RANGE
			    5,	// IVAR
			    4,	// VARIANT
			    7,	// ENTRY
			    8,  // RETURN
			    7,	// IP_BASE
			    -5,	// OP        (size is |sizesByType| + arity)
			    -5,	// INTRINSIC (size is |sizesByType| + arity)
			    -5,	// PURE_FN   (size is |sizesByType| + arity)
			    -5,	// PHI	     (size is |sizesByType| + arity)
			    -5,	// GAMMA     (size is |sizesByType| + arity)
			    -5,	// LIST      (size is |sizesByType| + arity)
			    6,	// TEST
			    5,	// ETA
			    5,	// OK_MU
			    6,	// VAR_MU
			    3,	// MU_PH
			    7,  // DUMMY
			    6,  // ARRAY
			    3};	// BOT_TYPE

/**************************************************************************** 
 * ValEntry Support Routines                                March 1993      *
 * Author: Paul Havlak                                                      *
 *                                                                          *
 * This includes hash and compare routines for use in the ValTable hash     *
 * table.  Ultimately will include overloaded operators for value numbers.  *
 *                                                                          *
 * Copyright 1991, 1993 Rice University, as part of the Rn/ParaScope        *
 * Programming Environment Project.                                         *
 *                                                                          *
 ****************************************************************************/

static Boolean is_ip_type(ValType t)
{
    switch(t)
    {
      case VAL_ENTRY:
      case VAL_IP_BASE:
      case VAL_RETURN:
      case VAL_DUMMY:
	return true;
      default:
	return false;
    }
}
static Boolean is_ip_value(ValEntry &ve)
{
    return is_ip_type(ve_type(ve));
}

static int num_unhashed(ValEntry &ve)	// returns # of trailing unhashed fds
{
    switch(ve_type(ve))
    {
      case VAL_RETURN: return 2;
      case VAL_ENTRY:
      case VAL_IP_BASE:
      case VAL_VAR_MU:
      case VAL_DUMMY:  return 1;
      default:         return 0;
    }
};

static unsigned int num_fields(ValEntry &ve)
{
    int result = sizesByType[ve_type(ve)];

    if (result < 0)
    {
	/*
	 *  Variable number of operands
	 */
	result = ve_arity(ve) + abs(result);
    }
    return (unsigned int) result;
}

static unsigned int num_hashed(ValEntry &ve)
{
    int result = num_fields(ve);

    result -= num_unhashed(ve);

    return (unsigned int) result;
}

unsigned int ValEntry::hash(int size)
{
    unsigned int nhashed = num_hashed(*this);
    int first_hash = 0;

    if (is_ip_value(*this))
    {
	first_hash = 2;
    }

    unsigned int result = 0;
    unsigned int bits = sizeof(unsigned int)*8;

    for (unsigned int i = first_hash; i < nhashed; i++)
    {
	unsigned int rshift = (i*5) & (bits - 1);
	unsigned int lshift = bits - rshift;
	result ^= ((field[i] << lshift) | (field[i] >> rshift));
    }
    if (ve_type(*this) == VAL_TEXT)
    {
	result += hash_string(ve_string(*this), size);
    }
    return (result % size);
}

int ValEntry::compare(ValEntry &v2)
{
    unsigned int nhashed = num_hashed(*this);
    int result;

    if (nhashed != num_hashed(v2))
    {
	return -1;		// any non-zero will do for inequality
    }
    else
    {
	result = memcmp(this->field, v2.field, sizeof(ValField)*nhashed);
    }
    if ((result == 0) && (ve_type(*this) == VAL_TEXT))
    {
	result = strcmp(ve_string(*this), ve_string(v2));
    }
    return result;
}

ValEntry *val_new(ValType type, int arity)
{
    ValEntry *result;

    if ((type < VAL_TOP_TYPE) &&
	(type > VAL_BOT_TYPE)) die_with_message("val_new: bogus type\n");

    int nfields = sizesByType[type];

    if (nfields < 0)	nfields = abs(nfields) + arity;
    /*
     *  One field slot already exists in the standard ValEntry
     */
    int size = sizeof(ValEntry) + (nfields-1)*sizeof(ValField);
    /*
     *  VAL_TEXT also needs space for string, size passed as arity
     */
    if (type == VAL_TEXT)	size += (arity + 1);	// leave room for null

    result = (ValEntry *)get_mem(size, "val_new: new value table entry");

    result->constRange   = VAL_NIL;
    ve_type(*result)    = type;
    ve_expType(*result) = TYPE_UNKNOWN;
    if ((type <= VAL_TEXT) ||		// TOP_TYPE, CONST, or TEXT
	(type == VAL_ENTRY) ||
	(type == VAL_IP_BASE))
    {
	ve_level(*result) = 0;
    }
    else
    {
	ve_level(*result) = -1;		// almost guaranteed to choke
    }
    /*
     *  Initialization of the rest is up to the caller
     */
    return result;
}

void val_delete(ValEntry *ve)
{
    free_mem((void *)ve);
}

/*---------------------------------------------------------------------------
 * -- val_print_entry
 *
 *       This function prints out the contents of a entry in value table 
 *--------------------------------------------------------------------------*/
void val_print_entry(Values Vp, ValNumber valNum)
{
    ValEntry* valNode;
    int arity;

    // cout << "value number\t" << valNum;
    printf("value number\t%d\n", valNum);
    if (valNum == VAL_TOP)
    {
	// cout << "\n\tTOP\n\n";
	printf("\tTOP\n\n");
	return;
    }
    if (valNum == VAL_BOTTOM)
    {
	// cout << "\n\t BOTTOM\n\n";
	printf("\tBOTTOM\n\n");
	return;
    }
    if (VAL_bogus((*Vp)[valNum]))
	return;

    valNode = &((*Vp)[valNum]);

    // cout << "\n\ttype:\t" << valType[(int) ve_type(*valNode)];
    // cout << "\n\texptype:\t" << gen_type_get_text(ve_expType(*valNode));
    // cout << "\n\tlevel:\t" << ve_level(*valNode);
    // cout << "\n\tconstRange:\t" << valNode->constRange;

    printf("\ttype:\t%s\n", valType[(int) ve_type(*valNode)]);
    printf("\texptype:\t%s\n", gen_type_get_text(ve_expType(*valNode)));
    printf("\tlevel:\t%d\n", ve_level(*valNode));
    printf("\tconstRange:\t%d\n", ve_cRange(*valNode));

    switch(ve_type(*valNode))
    {
      case VAL_CONST:
        // cout << "\n\tamount:\t" << ve_const(*valNode);
	printf("\tamount:\t%d\n", ve_const(*valNode));
	break;

      case VAL_TEXT:
	// cout << "\n\ttext:\t" << ve_string(*valNode);
	printf("\ttext:\t%s\n", ve_string(*valNode));
	break;

      case VAL_RANGE:
        // cout << "\n\tlo:\t" << ve_lo(*valNode);
        // cout << "\n\thi:\t" << ve_hi(*valNode);
        // cout << "\n\talign:\t" << ve_align(*valNode);
        // cout << "\n\tstep:\t" << ve_step(*valNode);
        // cout << "\n\tsimple:\t" << ve_simple(*valNode);
	printf("\tlo:\t%d\n", ve_lo(*valNode));
	printf("\thi:\t%d\n", ve_hi(*valNode));
	printf("\talign:\t%d\n", ve_align(*valNode));
	printf("\tstep:\t%d\n", ve_step(*valNode));
	printf("\tsimple:\t%d\n", ve_simple(*valNode));
	break;

      case VAL_IVAR:
        // cout << "\n\tflipped:\t" << ve_flipped(*valNode);
        // cout << "\n\tbounds:\t" << ve_bounds(*valNode);
	printf("\tflipped:\t%d\n", ve_flipped(*valNode));
	printf("\tbounds:\t%d\n", ve_bounds(*valNode));
	break;
      case VAL_VARIANT:
        // cout << "\n\toccur (sn):\t" << ve_occur(*valNode);
	printf("\toccur:\t%d\n", ve_occur(*valNode));
	break;

      case VAL_ENTRY:
        // cout << "\n\t" << ve_name(*valNode) << "\t@\t" << ve_entry(*valNode);
        // cout << "\n\toffset:\t" << ve_offset(*valNode);
        // cout << "\n\tlength:\t" << ve_length(*valNode);
	printf("\tinval\t%d@%d\n", ve_name(*valNode), ve_entry(*valNode));
	printf("\toffset:\t%d\n", ve_offset(*valNode));
	printf("\tlength:\t%d\n", ve_length(*valNode));
	break;

      case VAL_IP_BASE:
        // cout << "\n\t" << ve_name(*valNode) << "\t@\t" << ve_entry(*valNode);
        // cout << "\n\toffset:\t" << ve_offset(*valNode);
        // cout << "\n\tlength:\t" << ve_length(*valNode);
	printf("\tinval\t%d@%d\n", ve_name(*valNode), ve_entry(*valNode));
	printf("\toffset:\t%d\n", ve_offset(*valNode));
	printf("\tlength:\t%d\n", ve_length(*valNode));
	break;

      case VAL_RETURN:
        // cout << "\n\t" << ve_name(*valNode) << "\t@\t" << ve_call(*valNode);
        // cout << "\n\toffset:\t" << ve_offset(*valNode);
	// cout << "\n\tlength:\t" << ve_length(*valNode);
        // cout << "\n\tpassed value:\t" << ve_input(*valNode);
	printf("\tretval\t%d@%d\n", ve_name(*valNode), ve_entry(*valNode));
	printf("\toffset:\t%d\n", ve_offset(*valNode));
	printf("\tlength:\t%d\n", ve_length(*valNode));
	printf("\tpassed val:\t%d\n", ve_input(*valNode));
	break;

      case VAL_OP:
	// cout << "\n\top:\t" << valOpType[(int) ve_opType(*valNode)];
	printf("\top:\t%s\n", valOpType[(int) ve_opType(*valNode)]);
	break;

      case VAL_INTRINSIC:
      case VAL_PURE_FN:
	// cout << "\n\tintrinsic:\t" << ve_entry(*valNode);
	printf("\tintrinsic:\t%d\n", ve_entry(*valNode));
	break;

      case VAL_PHI:
        // cout << "\n\tstmt:\t" << ve_stmt(*valNode);
	printf("\tstmt:\t%d\n", ve_stmt(*valNode));
	break;

      case VAL_GAMMA:
        // cout << "\n\ttest:\t" << ve_test(*valNode);
	printf("\ttest:\t%d\n", ve_test(*valNode));
	break;

      case VAL_TEST:
        // cout << "\n\toccur (sn):\t" << ve_occur(*valNode);
        // cout << "\n\tvalue:\t" << ve_itest(*valNode);
        // cout << "\n\ttype:\t" << cd_branch_text(ve_testType(*valNode));
	printf("\toccur:\t%d\n", ve_occur(*valNode));
	printf("\tvalue:\t%d\n", ve_itest(*valNode));
	//
	// Using cd_branch_text causes a link ordering problem...
	// printf("\ttype:\t%s\n", cd_branch_text(ve_testType(*valNode)));
	printf("\ttype:\t%d\n", ve_testType(*valNode));
	break;

      case VAL_ETA:
        // cout << "\n\ttest:\t" << ve_test(*valNode);
        // cout << "\n\tfinal:\t" << ve_final(*valNode);
	printf("\ttest:\t%d\n", ve_test(*valNode));
	printf("\tfinal:\t%d\n", ve_final(*valNode));
	break;

      case VAL_OK_MU:
        // cout << "\n\tinit:\t" << ve_init(*valNode);
        // cout << "\n\titer:\t" << ve_iter(*valNode);
	printf("\tinit:\t%d\n", ve_init(*valNode));
	printf("\titer:\t%d\n", ve_iter(*valNode));
	break;

      case VAL_VAR_MU:
        // cout << "\n\toccur:\t" << ve_occur(*valNode);
        // cout << "\n\tinit:\t" << ve_init(*valNode);
        // cout << "\n\tvarIter:\t" << ve_varIter(*valNode);
	printf("\toccur:\t%d\n", ve_occur(*valNode));
	printf("\tinit:\t%d\n", ve_init(*valNode));
	printf("\tvarIter:\t%d\n", ve_varIter(*valNode));
	break;

      case VAL_DUMMY:
        // cout << "\n\tdummy def:\t" << ve_state(*valNode);
        // cout << "\n\tvariable:\t" << ve_name(*valNode);
        // cout << "\n\toffset:\t" << ve_offset(*valNode);
        // cout << "\n\tlength:\t" << ve_length(*valNode);
	printf("\tdummy def:\t%d\n", ve_state(*valNode));
	printf("\tvariable\t%d\n", ve_name(*valNode));
	printf("\toffset:\t%d\n", ve_offset(*valNode));
	printf("\tlength:\t%d\n", ve_length(*valNode));
	break;

      case VAL_ARRAY:
        // cout << "\n\tstate:\t" << ve_state(*valNode);
        // cout << "\n\trhs:\t" << ve_rhs(*valNode);
        // cout << "\n\tsubscripts:\t" << ve_subs(*valNode);
	printf("\tstate:\t%d\n", ve_state(*valNode));
	printf("\trhs:\t%d\n", ve_rhs(*valNode));
	printf("\tsubscripts:\t%d\n", ve_subs(*valNode));
	break;

      case VAL_LIST:
      case VAL_MU_PH:
      default:
	break;
    }
    if (sizesByType[ve_type(*valNode)] < 0)
    {
	// cout << "\n\tkids:";
	printf("\tkids:");

	arity = ve_arity(*valNode);
	for (int i = 0; i < arity; i++)
	{
	    // cout << "\t" << ve_kid(*valNode,i);
	    printf("\t%d", ve_kid(*valNode,i));
	}
	printf("\n");
    }
    // cout << "\n\n" << flush;
    printf("\n");
    fflush(stdout);

}  /* end of val_print_entry() */

void ValEntry::Write(FormattedFile & port)
{
    int nfields = num_fields(*this);

    port.Write("V");

    port.Write(ve_type(*this));		// field[0]
    port.Write(nfields);

    if (ve_type(*this) == VAL_TEXT)	port.Write(ve_string(*this));

    for (int i = 0; i < nfields; i++)	port.Write(field[i]);

    port.Write(this->constRange);
}
ValEntry *ValEntry::Read(FormattedFile & port)
{
    ValEntry *result;
    char buff[255];
    int nfields;
    int minFields;
    int i;

    if (port.Read(buff) || (buff[0] != 'V')) return NULL;

    port.Read(i);
    ValType type = (ValType) i;
    port.Read(nfields);

    if (type == VAL_TEXT)	port.Read(buff);
    else			buff[0] = '\0';

    minFields = sizesByType[type];
    if (minFields < 0)		// if variable arity minFields is negated
    {
	result = val_new(type, nfields + minFields);	// pass in arity
    }
    else 			// num of fields is exactly nfields
    {
	result = val_new(type, strlen(buff));	// pass in size of text string
    }

    for (i = 0; i < nfields; i++)	port.Read(result->field[i]);

    if (type == VAL_TEXT)	strcpy(ve_string(*result), buff);

    port.Read(result->constRange);

    return result;
}
