/* $Id: Metrics.h,v 1.1 1997/03/11 14:29:12 carr Exp $ */
/******************************************************************************/
/*        Copyright (c) 1990, 1991, 1992, 1993, 1994 Rice University          */
/*                           All Rights Reserved                              */
/******************************************************************************/
// $Id: Metrics.h,v 1.1 1997/03/11 14:29:12 carr Exp $ -*-c++-*-
//**************************************************************************
// Declarations for data structures representing various performance metrics:
//
// class PerfMetrics	:	detailed individual execution time metrics
// 				   for procedure, loop or message
//
// class PerfStats	:	A vector of values, with associated stats.
//
// class SetOfNamedFractions :	A vector of fractions adding to one, meant
//				   to be displayed in a bargraph or pie-chart
//				   The fractions have associated labels. The
//				   object includes the absolute total value.
//**************************************************************************

#ifndef Metrics_h
#define Metrics_h

#ifndef _MATH_H
#include <math.h>
#endif
#ifndef general_h				// For typedef enum Boolean
#include <libs/support/misc/general.h>
#endif


//----------------------------------------------------------------------
//
// class SetOfNamedFractions:	A vector of fractions adding to one,
//				for display in a bargraph or pie-chart.
//
// The fractions have associated labels. The object includes the 
// absolute total value, which can be used to determine the height
// of the bar.
//
// The operator() defines an iterator over the list:
// while (orderedList()) {
//     frac  = orderedList.CurFraction();
//     label = orderedList.CurLabel();
// }

class SetOfNamedFractions
{
  private:
    enum	{ FRACTION_SET_INIT_SIZE = 8 };
  
  public:
    SetOfNamedFractions(Boolean _sorted = false,
			int initSize = FRACTION_SET_INIT_SIZE);	// constructor
    SetOfNamedFractions(SetOfNamedFractions& setNF); 		// copy ''
    SetOfNamedFractions& operator=(SetOfNamedFractions& setNF);	// assignment
    ~SetOfNamedFractions();			     		// destructor
    
    
    void	AddToList(double fraction, const char* label);
    void 	SetTotal(double total);
    double	Total();
    int		NumValues();
    
    Boolean	operator() ();	// reset and return false when list exhausted
    double	CurFraction();	// Returns "current" fraction.
    char*	CurLabel();	// Returns label of "current" fraction.
    
    void	Print();	// For testing

  private:
    Boolean	getSorted;
    Boolean	sortingDone;
    double	total;
    double*	fractions;
    char**	labels;
    int		numValues;
    int		curSize;
    int		curIndex;
    
    void	GrowArrays(int newSize);
    void	SortInDecreasingOrder();
};

inline double SetOfNamedFractions::Total()	 { return total; }
inline int    SetOfNamedFractions::NumValues()	 { return numValues; }
inline double SetOfNamedFractions::CurFraction() { return fractions[curIndex];}
inline char*  SetOfNamedFractions::CurLabel()    { return labels[curIndex]; }


//----------------------------------------------------------------------

// class PerfStats  :  A vector of values, with associated statistics.
//
// The empty vector is treated as not defined. Passing it to =, +=, + or -
//    or applying +, - or ComputeStats() on it is an error. Retrieving a
//    field is an error, but is not checked.
// The zero vector of size NumValues() > 0 is represented with a flag
//    but no actual values are stored.  IsZero() tests if the flag is set.
//    WARNING:  IsZero() will return false for an explicit vector of all
//    zeroes.  In fact, any operation that allocates space for the vector 
//    will clear the zero flag.
// Both operands of the +=, + and - operators must be of the same size.
//    (except that the left operand of += can be empty).

class PerfStats {
  public:
    PerfStats();
    PerfStats(const int numValues, const double* const indivStats);
    PerfStats(const PerfStats& stats);
    ~PerfStats();
    
    void	SetSize(const int numValues);	// Call before the next func
    void	SetValueOnProcessor(const int procNum, const double newValue);
    void	SetZero(const int  numValues);	// Special case; no array used.
    void 	ComputeStats();			// Computes statistics after
						//   individual values are set
    PerfStats&	operator= (const PerfStats& stats);
    PerfStats&	operator+=(const PerfStats& stats);
    PerfStats&	operator+ (const PerfStats& stats) const;
    PerfStats&	operator- (const PerfStats& stats) const;
    
    double	Mean() const;
    double	Min() const;
    double	Max() const;
    double	Var() const;
    double	StdDev() const;
    double	CV() const;		// Coeff. of Variation = StdDev/Mean
    double	OnProcessor(const int procNum) const;
    int		NumValues() const;
    
    Boolean	IsZero() const;
    void	Print() const;

  private:
    struct PerfStats_Repr_struct {	// Space usage for P processors:
	Boolean zero;			// (11 + 8P) * 4 bytes
	double	mean;
	double	min;
	double	max;
	double	var;	// stddev is harder to compute than var, so store var
	double	*valueForProcessor;
	int	numValues;
    } repr;

    static PerfStats	_newValue;	// For members that create a new value.
					// Borrowed from Pablo.
    void 	ReallocArray(int newNumValues);
};
    
// Inline the functions setting individual fields
inline void PerfStats::SetSize(const int numValues) {ReallocArray(numValues);}

inline void PerfStats::SetValueOnProcessor(const int procNum,
					   const double newValue)
{
    assert(procNum >= 0 && procNum < repr.numValues);
    repr.valueForProcessor[procNum] = newValue;
}
    
// Inline the field extractions
inline int	PerfStats::NumValues() const	{ return repr.numValues; }
inline double	PerfStats::Mean() const		{ return repr.mean; }
inline double	PerfStats::Min() const		{ return repr.min; }
inline double	PerfStats::Max() const		{ return repr.max; }
inline double	PerfStats::Var() const		{ return repr.var; }
inline double	PerfStats::StdDev() const	{ return sqrt(repr.var); }
inline double	PerfStats::CV() const
			{ return (Mean() == 0)? 0.0 : StdDev()/fabs(Mean()); }
inline double	PerfStats::OnProcessor(const int procNum) const
{
    assert(procNum >= 0 && procNum < repr.numValues);
    return repr.valueForProcessor[procNum];
}

inline Boolean	PerfStats::IsZero() const	{ return repr.zero; }


//----------------------------------------------------------------------
// Space ~= 9 * sizeof(PerfStats) ~= 36 * (8*P + 11) bytes

class PerfMetrics
{
  private:
    int		numProcs;		// The number of processors
    
    PerfStats	numInvocations;		// #Times this code unit was invoked
    PerfStats	totalTime;		// Total time in the code unit
					// 	= busyTime + idleTime
    PerfStats	busyTime;		// Time doing useful computation
    PerfStats	idleTime;		// Time not doing useful computation
					// 	= sum of the following four:
    PerfStats	recvOvhdIdleTime;	// Waiting for a message
    PerfStats	sendOvhdIdleTime;	// Overhead for sending a message
    PerfStats	noWorkIdleTime;		// Idle due to insufficient parallelism
    PerfStats	redundantWorkIdleTime;	// "Idle" due to redundant computation

    PerfStats	totalMesgVolume;	// Total #bytes sent during execution
    
  public:
    PerfMetrics() {assert(false);}		// Disable default constructor 
    PerfMetrics(int numProcs);			// Most common constructor 
    PerfMetrics(PerfMetrics& fromMetrics);	// Copy constructor
    virtual ~PerfMetrics();			// Destructor
    
    // Get the individual costs
    int		NumProcs		(); // Ok, so this isn't a cost. So?
    PerfStats&	NumInvocations		();
    PerfStats&	TotalTime		();
    PerfStats&	BusyTime		();
    PerfStats&	IdleTime		();
    PerfStats&	RecvOvhdIdleTime	();
    PerfStats&	SendOvhdIdleTime	();
    PerfStats&	NoWorkIdleTime		();
    PerfStats&	RedundantWorkIdleTime	();
    PerfStats&	TotalMesgVolume		();
    
    // Set all cost entries to 0
    void	ClearAllValues		();
    
    // Add all the costs of the given metrics object into this one
    PerfMetrics& operator +=		(PerfMetrics& fromMetrics);
    
    // Add only the individual idle time cost fields
    void	AddIndivIdleTimeCosts	(PerfMetrics& fromMetrics);
    
    // Add only the message-passing idle time cost fields
    void	AddMesgCosts		(PerfMetrics& fromMetrics);
    
    // Add only the poor parallelism idle time fields
    void	AddPoorPrlCosts		(PerfMetrics& fromMetrics);
    
    // Add the costs of each individual field from given perf stats
    void	AddNumInvocations	(PerfStats& fromStats);
    void	AddTotalTime		(PerfStats& fromStats);
    void	AddBusyTime		(PerfStats& fromStats);
    void	AddIdleTime		(PerfStats& fromStats);
    void	AddRecvOvhdIdleTime	(PerfStats& fromStats);
    void	AddSendOvhdIdleTime	(PerfStats& fromStats);
    void	AddNoWorkIdleTime	(PerfStats& fromStats);
    void	AddRedundantWorkIdleTime(PerfStats& fromStats);
    void	AddTotalMesgVolume	(PerfStats& fromStats);
    
    // Set the individual costs from given perf stats
    void	SetNumInvocations	(const PerfStats& fromStats);
    void	SetTotalTime		(const PerfStats& fromStats);
    void	SetBusyTime		(const PerfStats& fromStats);
    void	SetIdleTime		(const PerfStats& fromStats);
    void	SetRecvOvhdIdleTime	(const PerfStats& fromStats);
    void	SetSendOvhdIdleTime	(const PerfStats& fromStats);
    void	SetNoWorkIdleTime	(const PerfStats& fromStats);
    void	SetRedundantWorkIdleTime(const PerfStats& fromStats);
    void	SetTotalMesgVolume	(const PerfStats& fromStats);
    
    // Print out the contents of this object to stdout:
    void	Print() const;
};

// Inline the field extractions

inline int	  PerfMetrics::NumProcs()	  { return numProcs; }
inline PerfStats& PerfMetrics::NumInvocations()	  { return numInvocations;}
inline PerfStats& PerfMetrics::TotalTime()	  { return totalTime; }
inline PerfStats& PerfMetrics::BusyTime()	  { return busyTime; }
inline PerfStats& PerfMetrics::IdleTime()	  { return idleTime; }
inline PerfStats& PerfMetrics::RecvOvhdIdleTime() { return recvOvhdIdleTime;}
inline PerfStats& PerfMetrics::SendOvhdIdleTime() { return sendOvhdIdleTime;}
inline PerfStats& PerfMetrics::NoWorkIdleTime()	  { return noWorkIdleTime;}
inline PerfStats& PerfMetrics::RedundantWorkIdleTime()
					      { return redundantWorkIdleTime; }
inline PerfStats& PerfMetrics::TotalMesgVolume()  { return totalMesgVolume; }

// Inline the functions to set the individual cost fields

inline void PerfMetrics::SetNumInvocations(const PerfStats& fromStats)
					{ numInvocations = fromStats; }
inline void PerfMetrics::SetTotalTime(const PerfStats& fromStats)
					{ totalTime = fromStats; }
inline void PerfMetrics::SetBusyTime(const PerfStats& fromStats)
					{ busyTime = fromStats; }
inline void PerfMetrics::SetIdleTime(const PerfStats& fromStats)
					{ idleTime = fromStats; }
inline void PerfMetrics::SetRecvOvhdIdleTime(const PerfStats& fromStats)
					{ recvOvhdIdleTime = fromStats; }
inline void PerfMetrics::SetSendOvhdIdleTime(const PerfStats& fromStats)
					{ sendOvhdIdleTime = fromStats; }
inline void PerfMetrics::SetNoWorkIdleTime(const PerfStats& fromStats)
					{ noWorkIdleTime = fromStats; }
inline void PerfMetrics::SetRedundantWorkIdleTime(const PerfStats& fromStats)
					{ redundantWorkIdleTime = fromStats; }
inline void PerfMetrics::SetTotalMesgVolume(const PerfStats& fromStats)
					{ totalMesgVolume = fromStats; }


// Inline the individual cost field additions

inline void PerfMetrics::AddNumInvocations(PerfStats& fromStats)
					{ numInvocations += fromStats; }
inline void PerfMetrics::AddTotalTime(PerfStats& fromStats)
					{ totalTime += fromStats; }
inline void PerfMetrics::AddBusyTime(PerfStats& fromStats)
					{ busyTime += fromStats; }
inline void PerfMetrics::AddIdleTime(PerfStats& fromStats)
					{ idleTime += fromStats; }
inline void PerfMetrics::AddRecvOvhdIdleTime(PerfStats& fromStats)
					{ recvOvhdIdleTime += fromStats; }
inline void PerfMetrics::AddSendOvhdIdleTime(PerfStats& fromStats)
					{ sendOvhdIdleTime += fromStats; }
inline void PerfMetrics::AddNoWorkIdleTime(PerfStats& fromStats)
					{ noWorkIdleTime += fromStats; }
inline void PerfMetrics::AddRedundantWorkIdleTime(PerfStats& fromStats)
					{ redundantWorkIdleTime += fromStats; }
inline void PerfMetrics::AddTotalMesgVolume(PerfStats& fromStats)
					{ totalMesgVolume += fromStats; }


//----------------------------------------------------------------------

#endif /* Metrics_h */
