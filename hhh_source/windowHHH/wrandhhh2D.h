

#ifndef RANDHHH2D_H
#define RANDHHH2D_H


#define NUM_COUNTERS 25 //number of masks
#define MAX_DEPTH 5 //depth of masks lattice
#define MAX_DESCENDANTS 512 //maximum number of direct descendents of a given ip pair

#include "prng.h"

#ifndef SEED
#define SEED 3421
#endif

#if VMULT>1
#define PROB
#endif

//still define things as in lossycount.h
#define LCLitem_t uint64__t

//The masks associated with the counters
//Note that we must ensure that they are in increasing order of generality
extern LCLitem_t masks[NUM_COUNTERS];

//initialise
void init(double SSepsilon, double prob, unsigned windowSize, double gamma, int m);

//deinitialise
void deinit();
void update(LCLitem_t item);


#endif


