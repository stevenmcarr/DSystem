/* $Id: SymConstraints.C,v 1.1 1997/03/11 14:35:20 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
//
//  Build linear equality relations (c1 * a + c2 * b = c3, with
//  all the ci constant) for interprocedural propagation.
//
//					Paul Havlak
//					September 1993
#include <libs/moduleAnalysis/valNum/val.i>
#include <libs/moduleAnalysis/valNum/val_pass.h>
#include <libs/moduleAnalysis/valNum/val_ip.h>
#include <libs/support/numerical/ExtendedGCD.h>
#include <libs/ipAnalysis/callGraph/CallGraph.h>
#include <libs/ipAnalysis/callGraph/CallGraphNodeEdge.h>
#include <libs/ipAnalysis/callGraph/CallGraphIterators.h>
// #include <ScalarModRefAnnot.h>
#include <libs/ipAnalysis/problems/symbolic/SymValAnnot.h>
#include <libs/ipAnalysis/problems/symbolic/SymConAnnot.h>
#include <libs/ipAnalysis/problems/symbolic/SymTrans.h>
#include <libs/ipAnalysis/problems/symbolic/SymConstraints.i>
#include <libs/ipAnalysis/problems/symbolic/IntList.h>
#include <libs/ipAnalysis/problems/symbolic/SymConList.h>
#include <libs/ipAnalysis/problems/symbolic/IpVarList.h>

//  Vector of these is used to build entry values, based on incoming
//  relations, in SymGetEntryVals
//
class ValEntryExpr {
  public:
    ValEntryExpr() : 
        parent(UNUSED), baseVal(VAL_NIL), 
	coeff(1), addend(0),
	pCoeff(0), pAddend(0)
        {};
    ValEntryExpr(int par, ValNumber bv, int co, int add, int pc, int pa) :
        parent(par), baseVal(bv), 
	coeff(co), addend(add), 
        pCoeff(pc), pAddend(pa)
        {};
    int parent;
    ValNumber baseVal;
    int coeff;
    int addend;
    int pCoeff;
    int pAddend;
};


//  For each call site,
//      For each referenced variable v1
//          find the base value (b in v1 = c1 * b + c2)
//          lookup base in table
//          if base is associated with another variable v2
//              enter linear constraint of v1 on v2
//          else
//              enter base in table associated with v1

static void extract(ValTable &V, ValNumber *valp, 
		    int *constp, ValOpType opType)
{
    ValEntry *v = &(V[*valp]);

    if ((ve_type(*v) == VAL_OP) &&
	(ve_opType(*v) == opType))
    {
	ValNumber left, right;
	left = ve_left(*v);
	right = ve_right(*v);

	/*
	 *  constants now guaranteed to be on left
	 */
	if (ve_type(V[left]) == VAL_CONST)
	{
	    *constp = ve_const(V[left]);
	    *valp   = right;
	}
    }
}

extern char *Rn_sym_level;

static void parse(BaseMap &BM, ValTable &V, ValNumber val, 
		  ConstraintItem &con, int index)
{
    BaseMapEntry *old;
    ValNumber base;
    int coeff, addend;
    int symLevel;
    sscanf(Rn_sym_level, "%d", &symLevel);

    con.parent  = UNUSED;
    con.pCoeff  = 0;

    if ((val == VAL_BOTTOM) ||
	(val == VAL_TOP) ||
	(val == VAL_NIL))
    {
	con.myCoeff = 0;		// parent NIL, myCoeff 0 --> root
	con.addend  = 0;
	return;
    }
    else if (ve_type(V[val]) == VAL_CONST)
    {
	con.myCoeff = 1;		// parent NIL, myCoeff 1 --> constant
	con.addend  = ve_const(V[val]);
	return;
    }
    else if (symLevel == 1)		// only handle constants
    {
	con.myCoeff = 0;		// parent NIL, myCoeff 0 --> root
	con.addend  = 0;
	return;
    }

    base   = val;
    coeff  = 1;
    addend = 0;
    /*
     *  Extract constant additive part if possible
     */
    if (symLevel > 1) extract(V, &base, &addend, VAL_OP_PLUS);
    /*
     *  Extract constant coefficient if possible
     */
    if (symLevel > 2) extract(V, &base, &coeff, VAL_OP_TIMES);

    old = BM[base];

    if (old)
    {
	//  Let
	//  b  = base
	int v1 = old->index;
	int c1 = old->coeff;
	int a1 = old->addend;
	int v2 = index;
	int c2 = coeff;
	int a2 = addend;

	//  old constraint: v1 = c1 * b + a1
	//  new constraint: v2 = c2 * b + a2
	//
	//  cancel out b and generate two-variable constraint:
	//
	//  c2 * v1 == c2 * c1 * b + c2 * a1;
	//  c1 * v2 == c1 * c2 * b + c1 * a2;
	//  c2 * v1 +(-c1)* v2 == c2 * a1 - c1 * a2

	//  introduce gcd to divide by in normalizing
	//
	int g = Gcd(c1, c2);

	//  ensure positive coefficient for parent relation
	//
	if (c2 < 0) g = -g;

	//  this gives us new equation
	//
	//  (c2/g)*v1 + (-c1/g)*v2 == (c2*a1 - c1*a2)/g

	//  v1 is the parent variable
	//
	con.parent = v1;
	con.pCoeff = c2/g;

	//  v2 is newly related to this parent
	//
	con.myCoeff = -c1/g;

	//  additive part
	//
	con.addend  = (c2*a1 - c1*a2)/g;
    }
    else
    {
	//  Not related to previously encountered vars
	//
	BM.add_entry(base, index, coeff, addend);

	con.myCoeff = 0;	// parent NIL, myCoeff 0 --> root
	con.addend  = 0;
    }
}

static void relate(ConstraintItem *cons, int kid, int newRoot,
		   int &parC, int &myC, int &add)
{
    //  Input:  two variables with same parent
    //  Output: relation between the two

    if (cons[kid].parent == newRoot)
    {
	parC = cons[kid].pCoeff;
	myC  = cons[kid].myCoeff;
	add  = cons[kid].addend;
	return;
    }
    else if ((cons[kid].parent != cons[newRoot].parent) ||
	     (cons[kid].parent == UNUSED))
    {
	parC = myC = add = 0;
	return;
    }

    //  Kid constraint:		ak + br = c
    //
    int a = cons[kid].myCoeff;
    int b = cons[kid].pCoeff;
    int c = cons[kid].addend;
	
    //  newRoot constraint:	dn + er = f
    //
    int d = cons[newRoot].myCoeff;
    int e = cons[newRoot].pCoeff;
    int f = cons[newRoot].addend;

    //  cancel out r
    //
    //  bdn + ber = bf
    //  eak + ebr = ec
    //  --------------
    //  bdn - eak = bf - ec
    //
    //  Build new constraint with newRoot as parent: pn + qk = s
    //  Guarantee that
    //      * p is positive
    //      * gcd(p, q, s) == 1
    //
    int g;
    g = Gcd(b*d, e*a);
    g = Gcd(g, b*f - e*c);

    if (b*d < 0)
    {
	g = -g;
    }
    /* p */ parC = (b*d)/g;
    /* q */ myC  = -(e*a)/g;
    /* s */ add  = (b*f - e*c)/g;
}

static void connect(ConstraintItem *consA, ConstraintItem *consB,
		    int kid, int newRoot,
		    int &parC, int &myC, int &add)
{
    //  Input:  two variables which both have different constant values
    //		in the two constraint sets
    //
    //  Output: linear relation between the two

    //  Kid constraints:
    //
    int k1 = consA[kid].myCoeff;
    int k2 = consB[kid].myCoeff;
    int dk = k1 - k2;
	
    //  newRoot constraints:
    //
    int r1 = consA[newRoot].myCoeff;
    int r2 = consB[newRoot].myCoeff;
    int dr = r1 - r2;

    //  New relationship:
    //		want the form
    //			ar + bk = c, a positive
    //
    //		(r - r1)/dr = (k - k1)/dk
    //		
    //		dk(r - r1) = dr(k - k1)
    //
    //		dk*r + (-dr)*k = dk*r1 - dr*k1
    //
    //		g = gcd(dk, dr, (dk*r1 - dr*k1))
    //
    //		if dk > 0
    //			divide by g
    //		else
    //			divide by -g
    //		
    int g;
    g = Gcd(dk, dr);
    g = Gcd(g, dk*r1 - dr*k1);

    if (dr < 0)
    {
	g = -g;
    }
    /* a */ parC = dk/g;
    /* b */ myC  = -dr/g;
    /* c */ add  = (dk*r1 - dr*k1)/g;
}


//  Rewrite value number from passed table using entry values provided
//  by inVals->pass.
//
static ValNumber update(ValTable *OV, ValNumber ov, 
			ValIP *inVals, ValNumber *cache)
{
    ValTable *NV = inVals->values;

    if (ov == VAL_NIL)		return VAL_NIL;
    if (cache[ov] != VAL_NIL)	return cache[ov];

    ValEntry *vp = &((*OV)[ov]);
    ValType   vt = ve_type(*vp);
    ValNumber nv = VAL_BOTTOM;
    ValNumber *kids = NULL;
    ValNumber initVal, input;

    switch(vt)
    {
      case VAL_OP:
      case VAL_INTRINSIC:
      case VAL_PURE_FN:
      case VAL_PHI:
      case VAL_GAMMA:
      case VAL_LIST:
      {
	  kids = new ValNumber[ve_arity(*vp)];

	  for (int i = 0; i < ve_arity(*vp); i++)
	  {
	      kids[i] = update(OV, ve_kid(*vp, i), inVals, cache);
	  }
	  break;
      }
      default:
	break;
    }

    switch(vt)
    {
      case VAL_BOT_TYPE:
      case VAL_TOP_TYPE:
	nv = ov;
	break;

      case VAL_CONST:
	nv = val_lookup_const(*NV, ve_expType(*vp), ve_const(*vp));
	break;

      case VAL_TEXT:
	nv = val_lookup_text(*NV, ve_expType(*vp), val_get_const_text(OV, ov));
	break;

      case VAL_RANGE:
	nv = val_lookup_range(*NV, 
			      update(OV, ve_lo(*vp),    inVals, cache),
			      update(OV, ve_hi(*vp),    inVals, cache),
			      update(OV, ve_align(*vp), inVals, cache), 
			      update(OV, ve_step(*vp),  inVals, cache));
	break;

      case VAL_IVAR:
	//  Could this be a problem with something symbolic bounds 
	//  becoming constant and being flipped?
	//
	nv = val_lookup_ivar(*NV, ve_level(*vp),
			     update(OV, ve_bounds(*vp), inVals, cache),
			     ve_flipped(*vp));
	break;

      case VAL_VARIANT:
	nv = val_lookup_variant(*NV, ve_expType(*vp),
				ve_occur(*vp), ve_level(*vp));
	break;

      case VAL_OP:
	nv = val_lookup_op(*NV, ve_expType(*vp),
			   ve_opType(*vp), ve_arity(*vp), kids);
	break;

      case VAL_INTRINSIC:
	nv = val_lookup_intrinsic(*NV, ve_expType(*vp),
				  val_get_const_text(OV, ve_entry(*vp)),
				  ve_arity(*vp), kids);
	break;

      case VAL_PURE_FN:
	nv = val_lookup_pure(*NV, ve_expType(*vp),
			     val_get_const_text(OV, ve_entry(*vp)),
			     ve_arity(*vp), kids);
	break;

      case VAL_PHI:
	nv = val_lookup_phi(*NV, ve_expType(*vp), ve_stmt(*vp),
			    ve_arity(*vp), kids);
	break;

      case VAL_GAMMA:
	nv = val_lookup_gamma(*NV, ve_expType(*vp),
			      update(OV, ve_test(*vp), inVals, cache),
			      ve_arity(*vp), kids);
	break;

      case VAL_LIST:
	nv = val_lookup_list(*NV, ve_arity(*vp), kids);
	break;

      case VAL_TEST:
	nv = val_lookup_test(*NV,
			     update(OV, ve_itest(*vp), inVals, cache),
			     ve_testType(*vp), ve_occur(*vp));
	break;

      case VAL_ETA:
	nv = val_lookup_eta(*NV,
			    update(OV, ve_test(*vp),  inVals, cache),
			    update(OV, ve_final(*vp), inVals, cache));
	break;

      case VAL_OK_MU:
	nv = val_lookup_ok_mu(*NV,
			      update(OV, ve_init(*vp), inVals, cache),
			      update(OV, ve_iter(*vp), inVals, cache),
			      ve_level(*vp));
	break;

      case VAL_VAR_MU:
	//
	//  Try to collapse cycles
	//
	initVal = update(OV, ve_init(*vp), inVals, cache);

	nv = val_lookup_var_mu(*NV, ve_occur(*vp), initVal, ve_level(*vp));
	//
	//  Add this early to thwart infinite recursion
	//
	cache[ov] = nv;

	input = update(OV, ve_varIter(*vp), inVals, cache);

	//  Could try some aux. i.v. recognition here
	//
	if (input == nv) nv = initVal;
	else
	    ve_varIter((*NV)[nv]) = input;
	break;

      case VAL_MU_PH:
	nv = val_lookup_mu_ph(*NV, ve_level(*vp));
	break;

      case VAL_ARRAY:
	nv = val_lookup_array(*NV, ve_expType(*vp),
			      update(OV, ve_state(*vp), inVals, cache),
			      update(OV, ve_rhs(*vp),   inVals, cache),
			      update(OV, ve_subs(*vp),  inVals, cache));
	break;

      case VAL_ENTRY:
	nv = inVals->pass->query_entry(val_get_const_text(OV, ve_name(*vp)),
				       ve_offset(*vp));
	if (nv != VAL_NIL)
	{
	    //  Try to head off type foulups
	    //
	    nv = val_convert(NV, nv, ve_expType(*vp));
	}
	else
	{
	    nv = val_lookup_entry(*NV, ve_expType(*vp),
				  val_get_const_text(OV, ve_entry(*vp)),
				  val_get_const_text(OV, ve_name(*vp)),
				  ve_offset(*vp), ve_length(*vp));
	}
	break;

      case VAL_RETURN:
	nv = val_lookup_return(*NV, ve_expType(*vp), ve_level(*vp), 
			       ve_call(*vp), 
			       val_get_const_text(OV, ve_name(*vp)),
			       ve_offset(*vp), ve_length(*vp),
			       /* input is unused here */ VAL_NIL);
	break;

      case VAL_DUMMY:
	nv = val_lookup_dummy(*NV, ve_expType(*vp), 
			      update(OV, ve_state(*vp), inVals, cache),
			      val_get_const_text(OV, ve_name(*vp)),
			      ve_offset(*vp), ve_length(*vp));
	break;
      default:
	die_with_message("update: disallowed type\n");
	break;
    }
    if (kids) delete kids;
    if (nv <= VAL_NIL) nv = VAL_BOTTOM;
    cache[ov] = nv;
    return nv;
}


ValIP *SymGetEntryCfgVals(void *passed_cg, char *name)
{
    // fprintf(stderr, "In SymGetEntryCfgVals for %s\n", name);

    //  look up cg node and call SymGetEntryVals
    //
    CallGraph *cg = (CallGraph *) passed_cg;

    return SymGetEntryVals(cg->LookupNode(name));
}

ValIP *SymGetEntryVals(CallGraphNode *node)
{
    // fprintf(stderr, "In SymGetEntryVals for %s\n", node->procName);

    ValIP      *vipp   = new ValIP;
    ValTable   *Vp     = vipp->values = new ValTable;
    ValPassMap *inVals = vipp->pass   = new ValPassMap;
    char *name;
    int offset, length;
    //  VarScope vscope;

    //  ScalarModRefAnnot *ref =
    //	    (ScalarModRefAnnot *) node->GetAnnotation(SCALAR_REF_ANNOT, true);
    //  ScalarModRefAnnotIterator refit(ref);

    int vCount;
    //  vCount = 0;
    int i;

    //  Count the referenced ip variables
    //
    //  for (refit.Reset();
    //	     name = refit.Current(vscope, offset, length);
    //       refit++)

    SymValAnnot *valInfo = (SymValAnnot *) 
	node->GetAnnotation(SYMVAL_ANNOT, true);

    vCount = valInfo->refScalars()->Count();

    if (vCount == 0) return vipp;

    //  Allocate space for information
    //
    ValEntryExpr *inExprs = new ValEntryExpr[vCount];

    ConstraintItem *cons = ((SymConAnnot *) 
			    node->GetAnnotation(SYMCON_ANNOT, 
						true))->hidden;

    //  for (refit.Reset(), i = 0;
    //       name = refit.Current(vscope, offset, length);
    //       refit++)

    i = 0;

    for (IpVarListIter scalars(valInfo->refScalars());
	 name = scalars.Current(offset, length);
	 scalars++)
    {
	if (cons[i].parent == UNUSED)
	{
	    //  this is a root
	    //
	    if (cons[i].myCoeff == 1)
	    {
		//  a constant
		//
		inExprs[i].baseVal = val_const(Vp, cons[i].addend);
	    }
	    //  default initialization is fine for other roots,
	    //  all they need is addend = 0, coeff = 1
	}
	else // a kid
	{
	    //  Express relation			ax + by = c
	    //  (x is root, y is kid)
	    //  in terms of new base value i:	x = b*i + x1
	    //					y = -a*i + y1
	    //
	    int x = cons[i].parent;
	    int a = cons[i].pCoeff;
	    int b = cons[i].myCoeff;
	    int c = cons[i].addend;

	    //  Find g == gcd(a,b) and x0 and y0 such that
	    //
	    //	a*x0 + b*y0 == g
	    //
	    int x0, y0;
	    int g = EGcd(a, b, &x0, &y0);

	    //  For properly built constraints, g divides c
	    //
	    //	(a*x0 + b*y0)*c/g == c
	    //	ax + by = (a*x0 + b*y0)*c/g
	    //	ax + by = a*x0*c/g + b*y0*c/g
	    //	a(x-x0*c/g) + b(y-y0*c/g) = 0
	    //
	    int x1 = x0 * c/g;
	    int y1 = y0 * c/g;

	    //	a(x-x1) + b(y-y1) = 0
	    //	(x-x1)/b == (y-y1)/(-a) == i
	    //	x == b*i + x1
	    //	y == -a*i + y1
	    //
	    inExprs[i].parent  = x;
	    inExprs[i].coeff   = -a;
	    inExprs[i].addend  = y1;
	    inExprs[i].pCoeff  = b;
	    inExprs[i].pAddend = x1;

	    //  update root information which says x == e*j + x2
	    int e  = inExprs[x].coeff;
	    int x2 = inExprs[x].addend;

	    //  e*j + x2 = b*i + x1 = x
	    //  b*i - e*j = x1 - x2
	    //
	    int i0, j0;
	    int g2 = EGcd(b, -e, &i0, &j0);

	    //  For properly built constraint sets, g2 divides (x1-x2)
	    //
	    int i1 = i0 * (x1 - x2)/g2;
	    int x3 = x1 + b*i1;

	    //  new expression: x = x3 + lcm(b,e)*k
	    //
	    inExprs[x].addend = x3;
	    inExprs[x].coeff  = Lcm(b,e);
	}
	i++;	//  increment index only for ip vars
    }

    //  Build value numbers for everything
    //
    //  for (refit.Reset(), i = 0;
    //       name = refit.Current(vscope, offset, length);
    //       refit++)

    for (scalars.Reset(), i = 0;
	 name = scalars.Current(offset, length);
	 scalars++)
    {
	ValNumber addend, coeff;

	if (inExprs[i].parent == UNUSED)	// root or constant
	{
	    if (inExprs[i].baseVal == VAL_NIL)
	    {
		//  Not a constant
		//
		if ((inExprs[i].coeff  == 1) &&
		    (inExprs[i].addend == 0))
		{
		    //  Don't need to make up a base, it would be the
			//  same as the entry value.
		    inExprs[i].baseVal = val_lookup_entry(*Vp, 
							  TYPE_INTEGER,
							  node->procName,
							  name, 
							  offset, length);
		}
		else
		{
		    inExprs[i].baseVal = val_lookup_ip_base(*Vp, 
							    TYPE_INTEGER,
							    node->procName,
							    name, 
							    offset, length);
		}
	    }
	    coeff  = val_const(Vp, inExprs[i].coeff);
	    addend = val_const(Vp, inExprs[i].addend);

	    inVals->add_entry(name, offset, 
			      val_binary(Vp, VAL_OP_PLUS, addend,
					 val_binary(Vp, VAL_OP_TIMES, coeff,
						    inExprs[i].baseVal)));
	}
	else //  a kid
	{
	    //  update kid value number from root value number, then save
	    //
	    //  root info is
	    //	x = x0 + dj
	    //
	    int x  = inExprs[i].parent;
	    int x0 = inExprs[x].addend;
	    int d  = inExprs[x].coeff;

	    //  old kid info is
	    //      x = x1 + bi
	    //	y = y1 + ai	(originally -a, but rename here)
	    //
	    int x1 = inExprs[i].pAddend;
	    int b  = inExprs[i].pCoeff;
	    int y1 = inExprs[i].addend;
	    int a  = inExprs[i].coeff;

	    //  derive
	    //	x0 = x1 + b*i0
	    //
	    int i0 = (x0 - x1)/b;	// must divide evenly

	    int y0 = y1 + a*i0;

	    //  j varies as d/b times as quickly as i
	    //
	    //  y  = y0 + ajd/b
	    //
	    addend = val_const(Vp, y0);
	    coeff  = val_const(Vp, a*d/b);

	    inVals->add_entry(name, offset,
			      val_binary(Vp, VAL_OP_PLUS, addend,
					 val_binary(Vp, VAL_OP_TIMES, coeff,
						    inExprs[x].baseVal)));
	}
	i++;	// increment only for ip variables
    }
    // printf("Entry values for %s\n", node->procName);
    // vipp->Dump();
    return vipp;
}

EXTERN( void, int_set, (int *array, int size, int value) );

void SymFwdEdge(CallGraphNode *node)
{
    // fprintf(stderr, "In SymFwdEdge for %s\n", node->procName);

    /////////////////////////////////////////////////////////////////////
    //
    //  Go through all the call sites, get REF annotations, and
    //  save passed values
    //
    CallGraphEdge *edge;
    CallGraphNode *sink;
    ConstraintItem *constraints = NULL;
    ValNumber pv;	// passed value
    char *name;
    // VarScope vscope;
    int offset, length;

    ValIP *oldSym = ((SymValAnnot *)node->GetAnnotation(SYMVAL_ANNOT, 
							true))->valIp();
    ValIP *inVals = SymGetEntryVals(node);

    ValNumber *updateCache = new ValNumber[val_table_max(oldSym->values)];

    int_set(updateCache, val_table_max(oldSym->values), VAL_NIL);
   
    for (CallGraphNodeOutEdgeIterator out(node);
         edge = out.Current();
         out++)
    {
	BaseMap BM;
	int count = 0;
	sink = edge->Callee();

	// fprintf(stderr, "\tforwarding edge to %s\n", sink->procName);

        //  Record passed values of global variables...
        //
        //  ScalarModRefAnnot *ref =
	//    (ScalarModRefAnnot *) sink->GetAnnotation(SCALAR_REF_ANNOT, true);
	//  ScalarModRefAnnotIterator refit(ref);

	SymConAnnot *eAnnot = new SymConAnnot(edge);
	constraints = eAnnot->hidden;

	count = 0;

        //  for (refit.Reset();
	//       name = refit.Current(vscope, offset, length);
	//       refit++)

	SymValAnnot *valInfo = (SymValAnnot *) 
	    sink->GetAnnotation(SYMVAL_ANNOT, true);

	for (IpVarListIter scalars(valInfo->refScalars());
	     name = scalars.Current(offset, length);
	     scalars++)
	{
	    if (sink->LookupFormal(name))
	    {
		int fid = sink->FormalNameToPosition(name);

		pv = oldSym->pass->query_entry(edge->c_id, "", fid);
	    }
	    else
	    {
		pv = oldSym->pass->query_entry(edge->c_id, name, offset);
	    }

	    //  Rewrite this value using new knowledge of input values.
	    //  This information is used *only* to compute constraints
	    //  for passing on, so this ValIP need not be saved.
	    //
	    ValNumber nv = update(oldSym->values, pv, inVals, updateCache);

	    parse(BM, *(inVals->values), nv, constraints[count], count);
	    count++;
        }
	//  save constraint set here
	//
	edge->PutAnnotation(eAnnot);
	//  fprintf(stderr, "SymFwdEdge from %s to %s\n", 
	//		node->procName, sink->procName);
	printf("Edge from %s, ", node->procName);
	eAnnot->Dump(sink);
    }
    delete inVals;
    delete updateCache;
}

SymConAnnot *SymFwdMerge(CallGraphNode *node)
{
    printf("Merging edges into %s\n", node->procName);

    //  Take the constraints along all the input edges and combine them
    //
    CallGraphEdge *edge, *first;
    CallGraphNodeInEdgeIterator inedge(node);
    SymConAnnot *rtn = new SymConAnnot(node);
    ConstraintItem *fCons, *cons;
    // char *name;
    // VarScope vscope;
    // int offset, length, index;
    int vCount, i;
    IntList roots;
    SymConAnnot *results = new SymConAnnot(node);

    //  ScalarModRefAnnot *ref =
    //	   (ScalarModRefAnnot *) node->GetAnnotation(SCALAR_REF_ANNOT, true);
    //  ScalarModRefAnnotIterator refit(ref);

    //  vCount = 0;

    //  for (refit.Reset();
    //       name = refit.Current(vscope, offset, length);
    //       refit++)

    SymValAnnot *valInfo = (SymValAnnot *) 
	node->GetAnnotation(SYMVAL_ANNOT, true);

    vCount = valInfo->refScalars()->Count();

    if (!(first = inedge.Current()))
    {
	//  assignment to first intentional;
	//  only continue if there are inedges
	//
	return results;
    }
    inedge++;

    //  fprintf(stderr, "\ttaking first inedge from %s\n", 
    //		first->Caller()->procName);

    //  Build accumulation list of roots and constraints.
    //  No convenient representation for "top".
    //
    fCons = ((SymConAnnot *) first->GetAnnotation(SYMCON_ANNOT, true))->hidden;

    SymConList *accum = new SymConList[vCount+1];

    //  Constants go in extra list... put this list in vector with the 
    //  others for simpler handling at the very end.
    //
    SymConList *constants = &(accum[vCount]);

    for (i = 0; i < vCount; i++)
    {
	if (fCons[i].parent == UNUSED)
	{
	    //  this is a root
	    //
	    if (fCons[i].myCoeff == 0)
	    {
		//  not a constant
		//
		roots.Append(i);
	    }
	    else if (fCons[i].myCoeff == 1)
	    {
		//  Add to list for constants.
		//  Put kid in "parent" field but leave coeffs unchanged.
		//
		constants->Append(i, 1, 0, fCons[i].addend);
	    }
	    else
	    {
		die_with_message("Bogus constraint in SymFwdMerge (ip sym)\n");
	    }
	}
	else
	{
	    //  This is a constraint with some already-encountered root
	    //
	    //  Put kid in "parent" field but leave coeffs unchanged.
	    //
	    accum[fCons[i].parent].Append(i, fCons[i].myCoeff, 
					  fCons[i].pCoeff, fCons[i].addend);
	}
    }

    //  For each other edge,
    //      merge information with this stuff for first edge
    //
    SinglyLinkedListIterator rootIter(roots);

    for (;
         edge = inedge.Current();
         inedge++)
    {
	//  fprintf(stderr, "\ttaking inedge from %s\n", 
	//	edge->Caller()->procName);

	cons = ((SymConAnnot *) edge->GetAnnotation(SYMCON_ANNOT, 
						    true))->hidden;

	for (rootIter.Reset();
	     rootIter.Current();
	     rootIter++)
	{
	    int thisRoot = ((IntListEntry *)rootIter.Current())->item;
	    int newRoot = UNUSED;

	    SymConList *kids = &(accum[thisRoot]);

	    SinglyLinkedListIterator kidIter(kids);

	    for (kidIter.Reset();
		 kidIter.Current();
		 /* kidIter++ : have to work around delete */)
	    {
		ConstraintItem *thisCon = ((SymConListEntry *)
					   kidIter.Current())->con;

		// if this constraint also exists at current edge,
		// continue, else move this entry to a different root
		//
		int kid     = thisCon->parent;	// nonstandard use of field
		int kCoeff1 = thisCon->myCoeff;
		int rCoeff1 = thisCon->pCoeff;
		int addend1 = thisCon->addend;

		//  Order matters in calls to relate...
		//  parents must always come before their kids in
		//  ip list of referenced variables.
		//
		int kCoeff2, rCoeff2, addend2;
		relate(cons, kid, thisRoot, rCoeff2, kCoeff2, addend2);

		if (rCoeff2 != 0)
		{
		    //  Linearly related in both constraint sets
		    //
		    if ((kCoeff1 == kCoeff2) &&
			(rCoeff1 == rCoeff2) &&
			(addend1 == addend2))
		    {
			//  Same constraint, keep
			//
			kidIter++;
			continue;
		    }
		    else
		    {
			//  Different linear relations, discard
			//
			SymConListEntry *killMe = (SymConListEntry *)
			    kidIter.Current();
			kidIter++;
			kids->Delete(killMe);

			if (newRoot == UNUSED)
			{
			    //  No newRoot, promote kid to newRoot
			    //  -- this is the place to initialize
			    //  bounds of new root from bounds of
			    //  its roots in the two incoming sets.
			    //
			    roots.Append(kid);
			    newRoot = kid;
			}
			else
			{
			    //  newRoot exists, move kid to newRoot's list
			    //  (newRoot is another var that depended on
			    //   same root... find relation in first 
			    //   constraint set between newRoot and kid,
			    //   we'll come back and merge with new 
			    //   constraint between them)
			    //
			    relate(fCons, kid, newRoot, 
				   rCoeff1, kCoeff1, addend1);
			    accum[newRoot].Append(kid, kCoeff1, 
						  rCoeff1, addend1);
			}
		    }
		}
		else
		{
		    //  Linearly related in accumulated set, but not
		    //  in current set.
		    //
		    if ((cons[kid].parent  == UNUSED) &&
			(cons[kid].myCoeff == 1) &&
			(cons[thisRoot].parent  == UNUSED) &&
			(cons[thisRoot].myCoeff == 1))
		    {
			//  Both kid and thisRoot are constant in current set
			//  -- see if point is consistent with accumulated 
			//  linear relation...
			//
			int kConst = cons[kid].addend;
			int rConst = cons[thisRoot].addend;
			int eval = kCoeff1*kConst + rCoeff1*rConst;

			if (eval = addend1)
			{
			    // consistent, so keep the linear constraint
			    //
			    kidIter++;
			    continue;
			}
		    }
		    //  Not constants or constants not consistent with
		    //  accumulated linear relation... discard.
		    //
		    SymConListEntry *killMe = (SymConListEntry *)
			kidIter.Current();
		    kidIter++;
		    kids->Delete(killMe);

		    if (newRoot == UNUSED)
		    {
			//  No newRoot, promote kid to newRoot
			//  -- this is the place to initialize
			//  bounds of new root from bounds of
			//  its roots in the two incoming sets.
			//
			roots.Append(kid);
			newRoot = kid;
		    }
		    else
		    {
			//  newRoot exists, move kid to newRoot's list
			//  (newRoot is another var that depended on
			//   same root... find relation in first 
			//   constraint set between newRoot and kid,
			//   we'll come back and merge with new 
			//   constraint between them)
			//
			relate(fCons, kid, newRoot, 
			       rCoeff1, kCoeff1, addend1);
			accum[newRoot].Append(kid, kCoeff1, 
					      rCoeff1, addend1);
		    }
		}
	    } // end forall kids
	} // end forall roots

	//  Now there are only the constants to worry about
	//  (variables that are still constant despite merging
	//  previous edges)
	//

	// prev constant
	//	new also constant
	//		same constant
	//			continue
	//		different constants
	//			make a line?  need another var in same sit.
	//	new not constant
	//		check for consistency

	//  For now, just handle identical constants
	//
	SinglyLinkedListIterator constIter(constants);

	for (constIter.Reset();
	     constIter.Current();
	     /* constIter++ : have to work around the delete */)
	{
	    ConstraintItem *currConst = ((SymConListEntry *)
					 constIter.Current())->con;

	    int constVar = currConst->parent;	// nonstandard use of field
	    int const1 = currConst->addend;

	    if ((cons[constVar].parent  == UNUSED) &&	// a root
		(cons[constVar].myCoeff == 1) &&	// a constant
		(cons[constVar].addend  == const1))	// the same constant
	    {
		constIter++;
		continue;	// keep the constraint
	    }
	    else	// different -- punt completely for now
	    {
		SymConListEntry *dead = (SymConListEntry *)constIter.Current();
		constIter++;
		constants->Delete(dead);
		roots.Append(constVar);
	    }

	    //  This fragment may have bearing on ultimately deriving lines
	    //  from constants
	    //
	    //  foreach constraint item C
	    //      if a root, roots += C
	    //	else members[C.parent] += (C,f) 
	    //  while roots nonempty
	    //	R = removed from roots
	    //      N = nil
	    //	foreach (C,f) in members[R]
	    //	    if have same relation ...
	}
    }

    //  Now we need to rebuild a constraint vector...
    //
    //  Stuff the index of the constants list in the root list.
    //
    roots.Append(vCount);

    cons = results->hidden;	// the empty result vector;
    unsigned int total = 0;

    SinglyLinkedListIterator rootIter2(roots);

    for (rootIter2.Reset();
	 rootIter2.Current();
	 rootIter2++)
    {
	//  we can leave the constraint for the root alone,
	//  until we have constant bounds to deal with
	//
	int thisRoot = ((IntListEntry *)rootIter2.Current())->item;
	SinglyLinkedListIterator kidIter(accum[thisRoot]);

	if (thisRoot == vCount)
	{
	    printf("\tIncoming constants: %d\n", accum[thisRoot].Count());
	    thisRoot = UNUSED;	// constants list
	}
	else
	{
	    //  total up linear constraints:
	    //  n^2 + n (would be n^2 - n, but list size doesn't
	    //      count the root it's listed under here)
	    //
	    int curSize = accum[thisRoot].Count();
	    total += (curSize * curSize) + curSize;
	}

	for (kidIter.Reset();
	     kidIter.Current();
	     kidIter++)
	{
	    ConstraintItem *thisCon = ((SymConListEntry *)
				       kidIter.Current())->con;

	    int kid = thisCon->parent;	// nonstandard use of field
	    cons[kid].parent  = thisRoot;
	    cons[kid].myCoeff = thisCon->myCoeff;
	    cons[kid].pCoeff  = thisCon->pCoeff;
	    cons[kid].addend  = thisCon->addend;
	}
    }
    //  Need to divide total by two (pulled division out of loop)
    //
    total /= 2;
    printf("\tIncoming relations: %d\n", total);

    results->Dump(node);
    return results;
}
