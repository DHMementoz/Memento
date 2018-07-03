

#include <stdlib.h>
#include <stdio.h>

//for debugging purposes only
#include <assert.h> 

#include "ulossycount.h"
#define COUNTERSIZE k
#define COUNTERS items
#define COUNT parentg->count
#include "BaseWRSS_LongId.hpp"

#include "wrandhhh2D.h"

int min(int a, int b) {return (a <= b ? a : b);}
int max(int a, int b) {return (a >= b ? a : b);}

//The counters
BaseWRSS_LongId * counters;
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
void init(double SSepsilon, double prob, unsigned windowSize, double gamma, int m) {
	int i;
	srand(SEED);
#ifdef PROB
	ignoreProb = 1 - prob;
	LogOneMinusP = log(ignoreProb);
	for (int i=0; i<(1<<16); ++i){
		double rnd = __uniform();
		randArr[i] = log(rnd) / LogOneMinusP + 1;
	}	
	packetsTillSample = randArr[++randIdx];
#endif
	counters = new BaseWRSS_LongId(windowSize, gamma, m, SSepsilon / (float) NUM_COUNTERS);
}

//deinitialise
void deinit() {
	free(counters);
}

void update(LCLitem_t item) {
#ifdef  PROB
	if (--packetsTillSample) return;
	short i = rand() % NUM_COUNTERS;
	counters->update(item & masks[i], 1);
	packetsTillSample = randArr[++randIdx];
	//printf("sample!, packetsTillSample = %d\n", packetsTillSample);
#else
	short i = rand() % NUM_COUNTERS;
	counters->update(item & masks[i], 1);
#endif
}
