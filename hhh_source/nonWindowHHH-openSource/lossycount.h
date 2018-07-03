// lossycount.h -- header file for Lossy Counting
// see Manku & Motwani, VLDB 2002 for details
// implementation by Graham Cormode, 2002,2003

#ifndef LOSSYCOUNTING_h
#define LOSSYCOUNTING_h

#include "prng.h"

// lossycount.h -- header file for Lossy Counting
// see Manku & Motwani, VLDB 2002 for details
// implementation by Graham Cormode, 2002,2003

// lclazy.h -- header file for Lazy Lossy Counting
// see Manku & Motwani, VLDB 2002 for details
// implementation by Graham Cormode, 2002,2003, 2005

/////////////////////////////////////////////////////////
#define LCLweight_t long long
//#define LCL_SIZE 101 // size of k, for the summary
// if not defined, then it is dynamically allocated based on user parameter
////////////////////////////////////////////////////////

//The type being counted
//This depends on the algorithm
#ifdef DIMENSION2
#define LCLitem_t uint64__t
#else
#define LCLitem_t uint32_t
#endif

//A null value thereof
#define LCL_NULLITEM 0x7FFFFFFF
	// 2^31 -1 as a special character

typedef struct lclcounter_t LCLCounter;

struct lclcounter_t
{
#ifdef TESTLOSSY
	int intable;
#endif
  LCLitem_t item; // item identifier
  int hash; // its hash value
  LCLweight_t count; // (upper bound on) count for the item
  LCLweight_t delta; // max possible error in count for the value
  LCLCounter *prev, *next; // pointers in doubly linked list for hashtable
}; // 32 bytes

#define LCL_HASHMULT 3  // how big to make the hashtable of elements:
  // multiply 1/eps by this amount
  // about 3 seems to work well

//#ifdef LCL_SIZE
#define LCL_SPACE (LCL_HASHMULT*LCL_SIZE)
//#endif

typedef struct LCL_type
{
  LCLweight_t n;
  int hasha, hashb, hashsize;
  int size;
  LCLCounter *root;
/*#ifdef LCL_SIZE
  LCLCounter counters[LCL_SIZE+1]; // index from 1
  LCLCounter *hashtable[LCL_SPACE]; // array of pointers to items in 'counters'
  // 24 + LCL_SIZE*(32 + LCL_HASHMULT*4) + 8
            // = 24 + 102*(32+12)*4 = 4504
            // call it 4520 for luck...
#else*/
  LCLCounter *counters;
  LCLCounter ** hashtable; // array of pointers to items in 'counters'
//#endif
} LCL_type;

extern LCL_type * LCL_Init(float fPhi);
extern void LCL_Destroy(LCL_type *);
extern void LCL_Update(LCL_type *, LCLitem_t, LCLweight_t);
extern LCLweight_t LCL_Size(LCL_type *);
extern LCLweight_t LCL_PointEstUpp(LCL_type *, LCLitem_t);
extern LCLweight_t LCL_PointEstLow(LCL_type *, LCLitem_t);
//extern std::map<uint32_t, uint32_t> LCL_Output(LCL_type *,int);

#endif
