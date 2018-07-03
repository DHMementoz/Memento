/*
Implementation of the 1-dimensional Hierarchical Heavy Hitters algorithm (HHH) for IP addresses
-Thomas Steinke (tsteinke@seas.harvard.edu) 2010-10-06
*/

#ifndef HHH1_H
#define HHH1_H

#include "lossycount.h"

#ifndef NUM_MASKS
#define NUM_MASKS 5
#endif
#define NUM_COUNTERS NUM_MASKS

//The masks associated with the counters
//Note that we must ensure that they are in increasing order of generality
extern LCLitem_t masks[NUM_COUNTERS];
extern int leveleps[NUM_COUNTERS];

//initialise
void init(double epsilon);

//deinitialise
void deinit();

#ifndef PARALLEL
//update an input
void update(LCLitem_t item, int count);
#else
void update(LCLitem_t * item, int count);
#endif

unsigned int pointQuery(LCLitem_t item);

//struct to store a heavy hitter output
typedef struct heavyhitter {
	LCLitem_t item;
	int mask; //The item & mask
	int upper, lower; //Upper and lower count bounds
} HeavyHitter;


//the one-dimensional output
HeavyHitter * output(int threshold, int * numhitters);
#endif

