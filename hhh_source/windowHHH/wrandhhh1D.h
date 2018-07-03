

#ifndef WRANDHHH1D_H
#define WRANDHHH1D_H

#include "ulossycount.h"

#ifndef SEED
#define SEED 3421
#endif

//#define FASTRAND 1

#ifndef NUM_MASKS
#define NUM_MASKS 5
#endif
#define NUM_COUNTERS NUM_MASKS

//The masks associated with the counters
//Note that we must ensure that they are in increasing order of generality
extern LCLitem_t wrhhh1d_masks[NUM_COUNTERS];
extern int wrhhh1d_leveleps[NUM_COUNTERS];

//initialise
extern "C" void wrhhh1d_init(double SSepsilon, double prob, unsigned windowSize, double gamma, int m);

//deinitialise
extern "C" void wrhhh1d_deinit();

extern "C" void wrhhh1d_update(LCLitem_t item);

extern "C" void wrhhh1d_updateLBSample(LCLitem_t item);

extern "C" void wrhhh1d_updateLBFinishedInterval(LCLitem_t intervalSize);
/*
LB1:
x
y
z
FinishedInterval (97)

LB2:
q
FinishedInterval (99)

LB3:
FinishedInterval (100)

LB4:
FinishedInterval (100)

LB5: 
r 
p 
FinishedInterval (100)

LB6:
FinishedInterval (100)

LB7:
FinishedInterval (100)

LB8:
FinishedInterval (100)
*/

extern "C" unsigned int wrhhh1d_pointQuery(LCLitem_t item);
extern "C" unsigned int wrhhh1d_pointQuery_over_estimation(LCLitem_t item, int window, double sam_prob);

#endif

