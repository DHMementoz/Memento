

#include <stdlib.h>

#include "ulossycount.h"
#include <math.h>
#define COUNTERSIZE k
#define COUNTERS items
#define COUNT parentg->count


#ifndef NUM_MASKS
#define NUM_MASKS 5
#endif 

#include "hashtable.h"
#include "wrandhhh1D.h"
#include "BaseWRSS.hpp"
#include <iostream>

#if NUM_MASKS==5
LCLitem_t wrhhh1d_masks[NUM_MASKS] = {
	0xFFFFFFFFu,
	0xFFFFFF00u,
	0xFFFF0000u,
	0xFF000000u,
	0x00000000u
};
int wrhhh1d_leveleps[NUM_MASKS] = {
	32,
	24,
	16,
	8,
	0
};
#endif
#if NUM_MASKS==33
LCLitem_t wrhhh1d_masks[NUM_MASKS] = {
	0xFFFFFFFFu << 0,
	0xFFFFFFFFu << 1,
	0xFFFFFFFFu << 2,
	0xFFFFFFFFu << 3,
	0xFFFFFFFFu << 4,
	0xFFFFFFFFu << 5,
	0xFFFFFFFFu << 6,
	0xFFFFFFFFu << 7,
	0xFFFFFFFFu << 8,
	0xFFFFFFFFu << 9,
	0xFFFFFFFFu << 10,
	0xFFFFFFFFu << 11,
	0xFFFFFFFFu << 12,
	0xFFFFFFFFu << 13,
	0xFFFFFFFFu << 14,
	0xFFFFFFFFu << 15,
	0xFFFFFFFFu << 16,
	0xFFFFFFFFu << 17,
	0xFFFFFFFFu << 18,
	0xFFFFFFFFu << 19,
	0xFFFFFFFFu << 20,
	0xFFFFFFFFu << 21,
	0xFFFFFFFFu << 22,
	0xFFFFFFFFu << 23,
	0xFFFFFFFFu << 24,
	0xFFFFFFFFu << 25,
	0xFFFFFFFFu << 26,
	0xFFFFFFFFu << 27,
	0xFFFFFFFFu << 28,
	0xFFFFFFFFu << 29,
	0xFFFFFFFFu << 30,
	0xFFFFFFFFu << 31,
	0x00000000u
};
int wrhhh1d_leveleps[NUM_MASKS] = {
	                     32,31,30,
	29,28,27,26,25,24,23,22,21,20,
	19,18,17,16,15,14,13,12,11,10,
	 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};
#endif

#ifndef VMULT
#define VMULT 1
#endif

#if VMULT>1
#define PROB
#endif

//The counters
BaseWRSS * wrandhhh1d_counters;
#ifdef PROB
double ignoreProb;
double LogOneMinusP;
unsigned int randArr[1<<16];
unsigned short randIdx;
unsigned int packetsTillSample;
#endif



double __uniform() {
	unsigned long long x = 0;
	int i;
	for (i = 0; i < 8; ++i) {
		x <<= 8;
		x += rand() & 0xff;
	}
	return (double)x / (-1ull);
}


double twototheminus(int k) {
	double ans = 1;
	while (k > 0) {ans /= 2; k--;}
	return ans;
}
//initialise
void wrhhh1d_init(double SSepsilon, double prob, unsigned windowSize, double gamma, int m) {
	int i;
	srand(SEED);
#ifdef PROB
	ignoreProb = 1 - prob;
	LogOneMinusP = log(ignoreProb);
	for (i=0; i<(1<<16); ++i){
		double rnd = __uniform();
		randArr[i] = log(rnd) / LogOneMinusP + 1;
	}	
	packetsTillSample = randArr[++randIdx];
#endif
	wrandhhh1d_counters = new BaseWRSS(windowSize, gamma, m, SSepsilon / (float) NUM_COUNTERS);
}

//deinitialise
void wrhhh1d_deinit() {
	free(wrandhhh1d_counters);
}

//update an input
void wrhhh1d_update(LCLitem_t item) {
#ifdef  PROB
	if (--packetsTillSample) {
		wrandhhh1d_counters->incTimestamp();
		return;
	}
	short i = rand() % NUM_COUNTERS;
	wrandhhh1d_counters->update(item & masks[i], 1);
	packetsTillSample = randArr[++randIdx];
#else
	short i = rand() % NUM_COUNTERS;
	wrandhhh1d_counters->update(item & wrhhh1d_masks[i], 1);
#endif
}

void wrhhh1d_updateLBSample(LCLitem_t item){
	/*for (int i=0; i < waitTime; ++i )
		wrandhhh1d_counters->incTimestamp();*/
	short i = rand() % NUM_COUNTERS;
	wrandhhh1d_counters->update(item & wrhhh1d_masks[i], 1);
}

void wrhhh1d_updateLBFinishedInterval(LCLitem_t intervalSize){
	for (int i=0; i < intervalSize; ++i)
		wrandhhh1d_counters->incTimestamp();
}


unsigned int wrhhh1d_pointQuery(LCLitem_t item){
	return wrandhhh1d_counters->query(item)*NUM_COUNTERS;
}

// overestimation to prevent false negatives
unsigned int wrhhh1d_pointQuery_over_estimation(LCLitem_t item, int window, double sam_prob){
	return wrandhhh1d_counters->query(item)*NUM_COUNTERS + 4*sqrt(window*NUM_COUNTERS/sam_prob);
}
















