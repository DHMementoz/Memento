/*
Implementation of the 1-dimensional Hierarchical Heavy Hitters algorithm (HHH) for IP addresses
-Thomas Steinke (tsteinke@seas.harvard.edu) 2010-10-06
*/

#include <stdlib.h>

#ifdef UNITARY
#include "ulossycount.h"
#define COUNTERSIZE k
#define COUNTERS items
#define COUNT parentg->count
#define LCL_type LCU_type
#define LCL_Init LCU_Init
#define LCL_Destroy LCU_Destroy
#define LCL_Update(x,y,z) LCU_Update(x,y)
//extern int LCL_PointEstUpp(LCL_type *, LCLitem_t);
//extern int LCL_PointEstLow(LCL_type *, LCLitem_t);
#else
#include "lossycount.h"
#define COUNTERSIZE size
#define COUNTERS counters
#define COUNT count
#endif

#include "hashtable.h"
#include "hhh1.h"
#include "alloc.h"

#define P(x...) fprintf(stderr, x)
#define PIP(item) fprintf(stderr, "%3d.%3d.%3d.%3d", (int)(255&((item) >> 24)), (int)(255&((item) >> 16)), (int)(255&((item) >> 8)), (int)(255&((item) >> 0)))

//#define NUM_COUNTERS 4

//The counters
LCL_type * counters[NUM_COUNTERS];
double epsval;
//The masks associated with the counters
//Note that we must ensure that they are in increasing order of generality
/*LCLitem_t masks[NUM_COUNTERS] = {
	0xFFFFFFFF, //255.255.255.255
	0xFFFFFF00, //255.255.255.0
	0xFFFF0000, //255.255.0.0
	0xFF000000,  //255.0.0.0
	0x00000000
};*/

double max(double a, double b) {return (a >= b ? a : b);}

double twototheminus(int k) {
	double ans = 1;
	while (k > 0) {ans /= 2; k--;}
	return ans;
}
//initialise
void init(double epsilon) {
	int i;
	epsval = epsilon;
	//counters[NUM_COUNTERS-1] = LCL_Init(1);
	//counters[NUM_COUNTERS-2] = LCL_Init(0.5);
	//counters[NUM_COUNTERS-3] = LCL_Init(0.25);
	for (i = 0; i < NUM_COUNTERS; i++)
		counters[i] = LCL_Init(max(epsilon, twototheminus(leveleps[i])));
}

//deinitialise
void deinit() {
	int i;
	for (i = 0; i < NUM_COUNTERS; i++)
		LCL_Destroy(counters[i]);
}

#ifndef PARALLEL
//update an input
void update(LCLitem_t item, int count) {
	int i;
	//P("inserting "); PIP(item); P(" [%d]\n", count);
	for (i = 0; i < NUM_COUNTERS; i++) {
		LCL_Update(counters[i], item & masks[i], count);
		//P("update [%2d] ", i); PIP(item & masks[i]); P("\n");
	}
}
#else
//update a sequence of imputs in parallel
void update(LCLitem_t * item, int count) {
	int i, j;
	#pragma omp parallel for private(j)
	for (i = 0; i < NUM_COUNTERS; i++)
		for (j = 0; j < count; j++)
			LCL_Update(counters[i], item[j] & masks[i], 1);
}

#endif

/*/struct to store a heavy hitter output
typedef struct heavyhitter {
	LCLitem_t item, mask; //The item & mask
	int upper, lower; //Upper and lower count bounds
} HeavyHitter;*/

//we want to sort heavyhitters
int cmpHH(const void * lhs, const void * rhs) {
	if (((const HeavyHitter*) lhs)->item > ((const HeavyHitter*) rhs)->item) return 1;
	if (((const HeavyHitter*) lhs)->item < ((const HeavyHitter*) rhs)->item) return -1;
	if (((const HeavyHitter*) lhs)->mask > ((const HeavyHitter*) rhs)->mask) return 1;
	if (((const HeavyHitter*) lhs)->mask < ((const HeavyHitter*) rhs)->mask) return -1;
	if (((const HeavyHitter*) lhs)->upper != ((const HeavyHitter*) rhs)->upper) return ((const HeavyHitter*) lhs)->upper - ((const HeavyHitter*) rhs)->upper;
	return ((const HeavyHitter*) lhs)->lower - ((const HeavyHitter*) rhs)->lower;
}

// Query
unsigned int pointQuery(LCLitem_t item){
	int i;
	for (i = 0; i < NUM_COUNTERS - 1; i++){
		unsigned mask = 0x000000FF << (8*i);
		if (item & mask) {
			/*for (int j = 0; j < counters[i]->COUNTERSIZE; j++) {
				printf("line 149: IP %u.%u.%u.%u OCUUR %u\n", (counters[i]->COUNTERS[j].item>>24)&0x000000FF, 
											 (counters[i]->COUNTERS[j].item>>16)&0x000000FF, 
											 (counters[i]->COUNTERS[j].item>>8)&0x000000FF, 
											  counters[i]->COUNTERS[j].item&0x000000FF, counters[i]->COUNTERS[j].COUNT);
				if (counters[i]->COUNTERS[j].item == item) {
					return counters[i]->COUNTERS[j].COUNT;
				}
			}
					
			printf("Line 148: %d\n", i);*/
			return LCU_PointEstUpp(counters[i], item);
		}
	}
	return LCU_PointEstUpp(counters[NUM_COUNTERS - 1], item);
}


//the one-dimensional output
HeavyHitter * output(int threshold, int * numhitters) {
	LL ** hashtable; //for transferring counts to parents
	int htsize; //size of hash table;
	HeavyHitter * output; //This will be the heavy hitters to output
	int n = 0; //the number of items in output
	int outputspace = 5;
	int i, j;
	LL * tmp;
	int s; //double count

	output = (HeavyHitter *) CALLOC(sizeof(HeavyHitter), outputspace); 

	htsize = (((int)(1.0/epsval)) + 1) | 1;
	hashtable = HT_Init(htsize);
	
	for (i = 0; i < NUM_COUNTERS; i++) {
		#ifndef UNITARY
		for (j = 1; j <= counters[i]->COUNTERSIZE; j++) {
		#else
		for (j = 0; j < counters[i]->COUNTERSIZE; j++) {
		#endif
			//P("mask=%2d counter=%3d item=", i, j); PIP(counters[i]->counters[j].item); P(" count=%3d delta=%3d\n", counters[i]->counters[j].count, counters[i]->counters[j].delta);
			if (counters[i]->COUNTERS[j].item != LCL_NULLITEM) {
				//P("count("); PIP(counters[i]->COUNTERS[j].item); P(")[%d] = [%d, %d]\n", i, counters[i]->COUNTERS[j].COUNT - counters[i]->COUNTERS[j].delta, counters[i]->COUNTERS[j].COUNT);
				//Now we just have to check that the counts are sufficient
				//first calculate the doublecount s
				tmp = HT_Find(hashtable, ((int64__t) counters[i]->COUNTERS[j].item) << 32 | ((int64__t)i), htsize);
				s = (tmp ? tmp->val : 0); //if there are no doublecounts recoreded assume s=0
				//now erase tmp, we don't need it again
				if (tmp) {
					if (tmp->next) tmp->next->prev = tmp->prev;

					if (tmp->prev) tmp->prev->next = tmp->next; //if prev just update it
					else *(HT_FindEntry(hashtable, ((int64__t)counters[i]->COUNTERS[j].item) << 32 | ((int64__t)i), htsize)) = tmp->next; //else update the start of the list
					FREE(tmp);
				}

				//now compare to the threshold
				if (counters[i]->COUNTERS[j].COUNT - s >= threshold) {
					while (outputspace <= n) {outputspace *= 2; output = (HeavyHitter *) REALLOC(output, sizeof(HeavyHitter) * outputspace);}
					//Add this item to our list of heavy hitters
					output[n].item = counters[i]->COUNTERS[j].item;
					output[n].mask = i;
					output[n].upper = counters[i]->COUNTERS[j].COUNT;
					output[n].lower = output[n].upper - counters[i]->COUNTERS[j].delta;
					
					//update s
					s = output[n].lower;
					n++;
				}

				//passs up the doublecount s
				if (s != 0 && i + 1 < NUM_COUNTERS) { //check that there is a positive doublecount and someone to pass up to
					tmp = HT_Insert(hashtable, ((int64__t)(counters[i]->COUNTERS[j].item & masks[i+1])) << 32 | ((int64__t) (i+1)), 0, htsize);
					tmp->val += s;
				}
			}
		}
	}

	HT_Clear(hashtable, htsize);
	
	//now clean up the output
	output = (HeavyHitter *) REALLOC(output, n * sizeof(HeavyHitter));
	qsort(output, n, sizeof(HeavyHitter), &cmpHH);

	*numhitters = n;

	return output;
}

/*int main() {
		int m; //number of heavy hitters in output
		int counters, threshold, n;
		scanf("%d%d%d", &counters, &threshold, &n);
		HeavyHitter * ans;
		int i;
		unsigned int wa, xa, ya, za;
		unsigned int wb, xb, yb, zb;
		unsigned int w, x, y, z;
		unsigned int a, b, ip;

		init((double)1/(double)counters);

		for (i = 0; i < n; i++) {
			scanf("%u%u%u%u", &w, &x, &y, &z);
			ip = (unsigned int)256*((unsigned int)256*((unsigned int)256*w + x) + y) + z;
			update(ip, 1);
		}

		ans = output1(threshold, &m);

		deinit();

		for (i = 0; i < m; i++) {
			//output ans[i]

			//break up the ip
			a = ans[i].item;
			za = a % 256; a /= 256;
			ya = a % 256; a /= 256;
			xa = a % 256; a /= 256;
			wa = a % 256; a /= 256;

			//break up the mask
			b = ans[i].mask;
			zb = b % 256; b /= 256;
			yb = b % 256; b /= 256;
			xb = b % 256; b /= 256;
			wb = b % 256; b /= 256;

			//output ip&mask
			if (wb != 0) printf("%u.", wa); else printf("*.");
			if (xb != 0) printf("%u.", xa); else printf("*.");
			if (yb != 0) printf("%u.", ya); else printf("*.");
			if (zb != 0) printf("%u", za); else printf("*");

			//output counts
			printf("	[%d, %d]\n",  ans[i].lower, ans[i].upper);
		}

		FREE(ans);

	return 0;
}*/

