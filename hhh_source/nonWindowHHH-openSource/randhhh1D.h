
#ifndef RANDHHH1D_H
#define RANDHHH1D_H

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
extern LCLitem_t masks[NUM_COUNTERS];
extern int leveleps[NUM_COUNTERS];

//initialise
void init_HHH1D(double SSepsilon, double prob);

//deinitialise
void deinit_hhh1d();

void update_hhh1d(LCLitem_t item, int count);

unsigned int pointQuery_hhh1d(LCLitem_t item);

//struct to store a heavy hitter output
typedef struct heavyhitter {
	LCLitem_t item;
	int mask; //The item & mask
	int upper, lower; //Upper and lower count bounds
} HeavyHitter;


//the one-dimensional output
HeavyHitter * output(int threshold, int * numhitters, int streamLen);
#endif

