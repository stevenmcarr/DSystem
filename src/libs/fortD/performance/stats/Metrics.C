/* $Id: Metrics.C,v 1.2 2001/09/17 00:13:28 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: Metrics.C,v 1.2 2001/09/17 00:13:28 carr Exp $ -*-c++-*-
//**************************************************************************
// Definitions for class PerfMetrics and associated data structures
//**************************************************************************

#include <stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <libs/fortD/performance/stats/Metrics.h>

#include <values.h>
#ifndef MININT				// For some reason, file values.h
#define MININT -(MAXINT - 1)		// defines MAXINT but not MININT
#endif


//--------------------------------------------------------------------------
// EXTERNAL VARIABLES AND FUNCTIONS
//--------------------------------------------------------------------------

// Defined here but EXTERN'ed to get naming right for linkage.
EXTERN(void, TestPrint,	(SetOfNamedFractions& snf));


//**************************************************************************
// MEMBER FUNCTION DEFINITIONS FOR class SetOfNamedFractions
//**************************************************************************

SetOfNamedFractions::SetOfNamedFractions(Boolean _sorted, int initSize) :
    getSorted(_sorted)
{
    fractions = (double*) NULL;
    labels = (char**) NULL;
    GrowArrays(initSize);
    total = 0.0;
    numValues = 0;
    curIndex = -1;
}

SetOfNamedFractions::SetOfNamedFractions(SetOfNamedFractions& setNF) :
    getSorted(setNF.getSorted),
    sortingDone(setNF.sortingDone),
    total(setNF.total),
    curIndex(setNF.curIndex)
{
    assert(setNF.curSize > 0);
    assert(setNF.fractions != (double*) NULL);
    assert(setNF.labels != (char**) NULL);
    
    GrowArrays(setNF.curSize);
    for (int i = 0; i < setNF.NumValues(); i++) {
	fractions[i] = setNF.fractions[i];
	if (setNF.labels[i] != (char*) NULL) {
	    labels[i] = new char[1 + strlen(setNF.labels[i])];
	    strcpy(labels[i], setNF.labels[i]);
	}
    }
    numValues = setNF.numValues;
}

SetOfNamedFractions&
SetOfNamedFractions::operator= (SetOfNamedFractions& setNF)
{
    assert(setNF.curSize > 0);
    assert(setNF.fractions != (double*) NULL);
    assert(setNF.labels != (char**) NULL);
    
    if (this != &setNF) {			// check for S = S
	getSorted = setNF.getSorted;
	sortingDone = setNF.sortingDone;
	total = setNF.total;
	curIndex = setNF.curIndex;
	
	for (int i = 0; i < NumValues(); i++)	// must delete these
	    delete[] labels[i];
	
	if (curSize != setNF.curSize) {		// cannot reuse old space
	    delete[] fractions;
	    fractions = (double*) NULL;
	    delete[] labels;
	    labels = (char**) NULL;
	    GrowArrays(setNF.curSize);
	}
	
	for (int i = 0; i < setNF.NumValues(); i++) { // Copy over all old values
	    fractions[i] = setNF.fractions[i];
	    if (setNF.labels[i] != (char*) NULL) {
		labels[i] = new char[1 + strlen(setNF.labels[i])];
		strcpy(labels[i], setNF.labels[i]);
	    }
	}
	numValues = setNF.numValues;
    }
    return *this;
}

SetOfNamedFractions::~SetOfNamedFractions()
{
    delete[] fractions;
    for (int i = 0; i < NumValues(); i++)
	delete[] labels[i];
    delete[] labels;
}

void SetOfNamedFractions::GrowArrays(int newSize)
{
    double* newFractions = new double[newSize];
    char**  newLabels    = new char*[newSize];
    if (fractions != (double*) NULL) {
	assert (labels != (char**) NULL);
	memcpy(newFractions, fractions, NumValues() * sizeof(double));
	memcpy(newLabels, labels, NumValues() * sizeof(char*));
	delete[] fractions;
	delete[] labels;		// Dont delete the strings themselves
    }
    fractions = newFractions;
    labels = newLabels;
    curSize = newSize;
}

void SetOfNamedFractions::AddToList(double fraction, const char* label)
{
    if (NumValues() == curSize - 1)
	GrowArrays(curSize * 2);
    assert(NumValues() < curSize-1);
    fractions[numValues] = fraction;
    labels[numValues] = (char*) new char[1 + strlen(label)];
    strcpy(labels[numValues], label);
    ++numValues;
    sortingDone = false;
}

void SetOfNamedFractions::SetTotal(double _total) { total = _total; }

Boolean SetOfNamedFractions::operator() ()
{
    if (getSorted && !sortingDone)
	SortInDecreasingOrder();
    if (curIndex < NumValues() - 1)
	 { ++ curIndex; return true ; }
    else { curIndex = -1; return false; }
}    

void SetOfNamedFractions::Print()
{
    if (NumValues() == 0) return;
    int i, j, nextNum;
    double dsum = 0.0;
    
    if (getSorted && !sortingDone)
	SortInDecreasingOrder();
    
    for (i = 0; i < NumValues(); i++) dsum += fractions[i];
    assert(dsum > 0.999 && dsum < 1.001);
    
    printf("Total Value Of Set = %g\n", Total());
    
    for (i=0, j=0, nextNum = MIN(NumValues(), 5); i < NumValues();
	 nextNum = MIN(NumValues(), nextNum + 5))
    {
	for ( ; i < nextNum; i++) printf("\t%s", labels[i]); printf("\n");
	for ( ; j < nextNum; j++) printf("\t%.4g", fractions[j]);
	printf("\n\n");
    }
}

// To test iterators, and accessing values via interface:
void TestPrint(SetOfNamedFractions& snf)
{
    if (snf.NumValues() == 0) return;
    
    double dsum = 0.0;
    while (snf()) dsum += snf.CurFraction();
    assert(dsum > 0.999 && dsum < 1.001);
    
    printf("Total Value Of Set = %g\n", snf.Total());
    
    while (snf()) printf("\t%s", snf.CurLabel()); printf("\n");
    while (snf()) printf("\t%.4g", snf.CurFraction());
    printf("\n\n");
}

// Typically, size will be very small. Use insertion sort.
void SetOfNamedFractions::SortInDecreasingOrder()
{
    double val;
    char* label;
    int j;
    for (int i = 1; i < NumValues(); i++) {
	val = fractions[i]; label = labels[i];
	for (j = i; j >= 1 && fractions[j-1] < val; j--) {
	    fractions[j] = fractions[j-1]; labels[j] = labels[j-1];
	}
	fractions[j] = val; labels[j] = label;
    }
    sortingDone = true;
}



//**************************************************************************
// MEMBER FUNCTION DEFINITIONS FOR class PerfMetrics
//**************************************************************************

PerfMetrics::PerfMetrics(int _numProcs) : numProcs(_numProcs)
{
}
    
PerfMetrics::PerfMetrics(PerfMetrics& fromMetrics) // copy constructor
    : numProcs(fromMetrics.NumProcs())
{
    ClearAllValues();
    *this += fromMetrics;
}

PerfMetrics::~PerfMetrics()
{
}

// Add only the message-passing idle time cost fields
inline void PerfMetrics::AddMesgCosts(PerfMetrics& fromMetrics)
{
    if (fromMetrics.RecvOvhdIdleTime().NumValues() > 0)
	AddRecvOvhdIdleTime(fromMetrics.RecvOvhdIdleTime());
    if (fromMetrics.SendOvhdIdleTime().NumValues() > 0)
	AddSendOvhdIdleTime(fromMetrics.SendOvhdIdleTime());
    if (fromMetrics.TotalMesgVolume().NumValues() > 0)
	AddTotalMesgVolume(fromMetrics.TotalMesgVolume());
}

// Add only the poor parallelism idle time fields
inline void PerfMetrics::AddPoorPrlCosts(PerfMetrics& fromMetrics)
{
    if (fromMetrics.NoWorkIdleTime().NumValues() > 0)
	AddNoWorkIdleTime(fromMetrics.NoWorkIdleTime());
    if (fromMetrics.RedundantWorkIdleTime().NumValues() > 0)
	AddRedundantWorkIdleTime(fromMetrics.RedundantWorkIdleTime());
}

// Add only the individual idle time cost fields
inline void PerfMetrics::AddIndivIdleTimeCosts(PerfMetrics& fromMetrics)
{
    AddMesgCosts(fromMetrics);
    AddPoorPrlCosts(fromMetrics);
}
    
// Add all the costs of the given metrics object into this one
PerfMetrics& PerfMetrics::operator += (PerfMetrics& fromMetrics)
{
    AddTotalTime(fromMetrics.TotalTime()); // Assume these 3 fields
    AddBusyTime(fromMetrics.BusyTime());   // are valid.  operator+=
    AddIdleTime(fromMetrics.IdleTime());   // will test that and die in
    AddIndivIdleTimeCosts(fromMetrics);	   // agonies if not.
    return *this;
}

void PerfMetrics::ClearAllValues()
{
    numInvocations.SetZero(numProcs);
    totalTime.SetZero(numProcs);
    
    busyTime.SetZero(numProcs);
    idleTime.SetZero(numProcs);
    
    recvOvhdIdleTime.SetZero(numProcs);
    sendOvhdIdleTime.SetZero(numProcs);
    noWorkIdleTime.SetZero(numProcs);
    redundantWorkIdleTime.SetZero(numProcs);
}

void PerfMetrics::Print() const
{
    printf("\nNumInvocations: "); numInvocations.Print();
    printf("\nTotalTime: "); totalTime.Print();
    printf("\nBusyTime: "); busyTime.Print();
    printf("\nIdleTime: "); idleTime.Print();
    if (recvOvhdIdleTime.NumValues() > 0)
	{ printf("\nRecvOvhdIdleTime: "); recvOvhdIdleTime.Print(); }
    if (sendOvhdIdleTime.NumValues() > 0)
	{ printf("\nSendOvhdIdleTime: "); sendOvhdIdleTime.Print(); }
    if (noWorkIdleTime.NumValues() > 0)
	{ printf("\nnoWorkIdleTime: "); noWorkIdleTime.Print(); }
    if (redundantWorkIdleTime.NumValues() > 0)
	{ printf("\nredundantWorkIdleTime: "); redundantWorkIdleTime.Print(); }
    if (totalMesgVolume.NumValues() > 0)
	{ printf("\ntotalMesgVolume: "); totalMesgVolume.Print(); }
}


//**************************************************************************
// MEMBER FUNCTION DEFINITIONS FOR class PerfStats 
//**************************************************************************

PerfStats::PerfStats()
{
    repr.numValues = 0;
    repr.zero = false;
    repr.valueForProcessor = (double*) NULL;
}

PerfStats::PerfStats(const int numValues, const double* const indivStats)
{
    repr.numValues = 0;
    repr.zero = false;
    repr.valueForProcessor = (double*) NULL;
    if (numValues > 0) {
	ReallocArray(numValues);
	memcpy(repr.valueForProcessor, indivStats, numValues * sizeof(double));
	ComputeStats();
    }
    else {
	repr.numValues = 0;
	repr.valueForProcessor = (double*) NULL;
    }
}

PerfStats::PerfStats(const PerfStats& stats)
{
    repr.numValues = 0;
    repr.zero = false;
    repr.valueForProcessor = (double*) NULL;
    
    // printf("\nCOPYING stats (0x%x) to this (0x%x)\n", &stats, this);
    // printf("stats: "); stats.Print();
    // printf("stats.repr.valueForProcessor = 0x%x\n",
    // 	   stats.repr.valueForProcessor);

    if (stats.NumValues() > 0) {	// Otherwise nothing to do.
	if (stats.IsZero())		// No allocation or copy needed
	    SetZero(stats.NumValues());
	else {
	    ReallocArray(stats.NumValues()); // Wont be any previous array
	    repr.mean = stats.repr.mean;     //   but use ReallocArray() anyway
	    repr.min  = stats.repr.min;
	    repr.max  = stats.repr.max;
	    repr.var  = stats.repr.var;
	    memcpy(repr.valueForProcessor, stats.repr.valueForProcessor,
		   NumValues() * sizeof(double));
	}
    }
    // printf("this->repr.valueForProcessor = 0x%x\n", repr.valueForProcessor);
}    

PerfStats::~PerfStats()
{
    if (repr.valueForProcessor != (double*) NULL)
	delete[] repr.valueForProcessor;
}

void PerfStats::SetZero(const int numValues)
{
    if (repr.valueForProcessor != (double*) NULL) {
	delete[] repr.valueForProcessor;
	repr.valueForProcessor = (double*) NULL;
    }
    repr.zero = true;
    repr.numValues = numValues;
    repr.mean = repr.var = repr.min = repr.max = 0.0;
}

// Computes statistics for the individual values in repr.valueForProcessor[]
void PerfStats::ComputeStats()
{
    assert (NumValues() > 0);
    if (IsZero()) {
	SetZero(NumValues());
	return;
    }
    register double dval, sum = 0.0, sumsq = 0.0;
    repr.min = MAXDOUBLE;
    repr.max = MINDOUBLE;
    for (int i = 0; i < NumValues(); i++) {
	dval = OnProcessor(i);
	sum += dval;
	sumsq += dval * dval;
	if (repr.min > dval) repr.min = dval;
	if (repr.max < dval) repr.max = dval;
    }
    repr.mean = sum / NumValues();
    repr.var = (sum == 0)? 0 : (sumsq/NumValues()) - repr.mean * repr.mean;
}

PerfStats& PerfStats::operator= (const PerfStats& stats)
{
    // printf("\nASSIGNING stats (0x%x) to this (0x%x)\n", &stats, this);
    // printf("stats: "); stats.Print();
    // printf("stats.repr.valueForProcessor = 0x%x\n",
    //    stats.repr.valueForProcessor);

    if (&repr != &stats.repr		// Special check for S = S.
	&& stats.NumValues() > 0)	// Otherwise nothing to do.
    {
	if (stats.IsZero())
	    SetZero(stats.NumValues());
	else {
	    ReallocArray(stats.NumValues());
	    repr.mean = stats.repr.mean;
	    repr.min  = stats.repr.min;
	    repr.max  = stats.repr.max;
	    repr.var  = stats.repr.var;
	    memcpy(repr.valueForProcessor, stats.repr.valueForProcessor,
		   NumValues() * sizeof(double));
	}
    }
    // printf("this->repr.valueForProcessor = 0x%x\n", repr.valueForProcessor);
    return *this;
}

// This operator adds the corresponding values (pairwise) in the two
// arrays this->valueForProcessor and stats.valueForProcessor and then
// recomputes the various statistics

PerfStats& PerfStats::operator += (const PerfStats& stats)
{
    assert(stats.NumValues() > 0);		// stats value is valid?
    if (stats.IsZero()) {			// basically, nothing to do
	if (NumValues() == 0)
	    SetZero(stats.NumValues());
    }
    else {
	if (NumValues() == 0) {
	    ReallocArray(stats.NumValues());	// Doesn't initialize values
	    for (int i = 0; i < NumValues(); i++) // so set all to zero
		repr.valueForProcessor[i] = 0;
	}
	assert(NumValues() == stats.NumValues()); // pairwise + makes sense?
	if (IsZero())
	    *this = stats;			// Just use = operator
	else {
	    for (int i = 0; i < NumValues(); i++)
		repr.valueForProcessor[i] += stats.OnProcessor(i);
	    ComputeStats();
	}
    }
    return *this;
}

PerfStats& PerfStats::operator + (const PerfStats& stats) const
{
    assert(NumValues() > 0);			// both operands valid and
    assert(NumValues() == stats.NumValues());	// pairwise + makes sense?
    
    if (IsZero())			// if either operand is zero
	_newValue = stats;		// return the other
    else if (stats.IsZero() == 0)
	_newValue = *this;
    else {
	_newValue.ReallocArray(NumValues()); // Space freed by next Realloc
	for (int i = 0; i < NumValues(); i++) // Copy
	    _newValue.repr.valueForProcessor[i] =
		repr.valueForProcessor[i] + stats.OnProcessor(i);
	_newValue.ComputeStats();	// Not as efficient as it could be.
    }
    return _newValue;
}


PerfStats& PerfStats::operator - (const PerfStats& stats) const
{
    assert(NumValues() > 0);
    assert(NumValues() == stats.NumValues()); // So pairwise - makes sense
    
    if (stats.IsZero())				   // i.e, stats == [0, 0, ...]
	_newValue = *this;
    else if (IsZero()) {			   // i.e, *this == [0, 0, ...]
	_newValue.ReallocArray(stats.NumValues()); // then _newValue = - stats
	_newValue.repr.mean = - stats.repr.mean;
	_newValue.repr.min  = stats.repr.max;
	_newValue.repr.max  = stats.repr.min;
	_newValue.repr.var  = stats.repr.var;
	for (int i = 0; i < _newValue.NumValues(); i++)
	    _newValue.repr.valueForProcessor[i] =
		- stats.OnProcessor(i);
    }
    else {
	_newValue.ReallocArray(NumValues()); // Space freed by next Realloc
	for (int i = 0; i < repr.numValues; i++) // Copy
	    _newValue.repr.valueForProcessor[i] =
		repr.valueForProcessor[i] - stats.OnProcessor(i);
	_newValue.ComputeStats();	// Not as efficient as it could be.
    }
    return _newValue;
}

void PerfStats::ReallocArray(int newNumValues)
{
    if (repr.valueForProcessor == (double*) NULL)
	repr.valueForProcessor = new double[newNumValues];
    else {
	assert(NumValues() > 0);
	if (NumValues() != newNumValues) {	// then cannot reuse space, so 
	    delete[] repr.valueForProcessor;	// discard and reallocate
	    repr.valueForProcessor = new double[newNumValues];
	}
    }
    repr.numValues = newNumValues;
    repr.zero = false;
}

void PerfStats::Print() const
{
    if (NumValues() == 0) printf("No data available\n");
    else {
	printf("MEAN = %g, CV = %g, [MIN,MAX] = %g,%g, STDDEV = %g\n",
	       Mean(), CV(), Min(), Max(), StdDev());
	printf("\tVALUES[1..%d] :", NumValues());
	if (IsZero()) printf(" == 0.0 (all values are zero)\n");
	else {
	    for (int i=0; i < NumValues(); i++)
		printf(" %g", OnProcessor(i));
	    printf("\n");
	}
    }
}

// Definition of the static data:
PerfStats PerfStats::_newValue;


//**************************************************************************
