#include <stdlib.h>
#include <stdio.h>
#include "lossycount.h"
//#include "prng.h"
#include "alloc.h"

#ifdef TESTLOSSY
#define P(x...) fprintf(stderr, x)
#define PIP(item) fprintf(stderr, "%3d.%3d.%3d.%3d", (int)(255&((item) >> 24)), (int)(255&((item) >> 16)), (int)(255&((item) >> 8)), (int)(255&((item) >> 0)))
#else
#define P(x...)
#define PIP(item)
#endif

/********************************************************************
Implementation of Lazy Lossy Counting algorithm to Find Frequent Items
Based on the paper of Manku and Motwani, 2002
And Metwally, Agrwawal and El Abbadi, 2005
Implementation by G. Cormode 2002, 2003, 2005
This implements the space saving algorithm, which 
guarantees 1/epsilon space. 
This implementation uses a heap to track which is the current smallest count

Original Code: 2002-11
This version: 2002,2003,2005,2008

This work is licensed under the Creative Commons
Attribution-NonCommercial License. To view a copy of this license,
visit http://creativecommons.org/licenses/by-nc/1.0/ or send a letter
to Creative Commons, 559 Nathan Abbott Way, Stanford, California
94305, USA.
*********************************************************************/

LCL_type * LCL_Init(float fPhi)
{
	int i;
	int k = 1 + (int) 1.0/fPhi;

	LCL_type *result = (LCL_type *) CALLOC(1,sizeof(LCL_type));
	// needs to be odd so that the heap always has either both children or 
	// no children present in the data structure

	result->size = (1 + k) | 1; // ensure that size is odd
	//fprintf(stderr, "phi=%f, size=%d\n", fPhi, result->size);
	result->hashsize = LCL_HASHMULT*result->size;
	result->hashtable=(LCLCounter **) CALLOC(result->hashsize,sizeof(LCLCounter*));
	result->counters=(LCLCounter*) CALLOC(1+result->size,sizeof(LCLCounter));
	// indexed from 1, so add 1

	result->hasha=151261303;
	result->hashb=6722461; // hard coded constants for the hash table,
	//should really generate these randomly
	result->n=(LCLweight_t) 0;

	for (i=1; i<=result->size;i++)
	{
		result->counters[i].next=NULL;
		result->counters[i].prev=NULL;
		result->counters[i].item=LCL_NULLITEM;
		result->counters[i].hash=-1;
		// initialize items and counters to zero
	}
	result->root=&result->counters[1]; // put in a pointer to the top of the heap
	return(result);
}

void LCL_Destroy(LCL_type * lcl)
{
	FREE(lcl->hashtable);
	FREE(lcl->counters);
	FREE(lcl);
}

void LCL_RebuildHash(LCL_type * lcl)
{
	// rebuild the hash table and linked list pointers based on current
	// contents of the counters array
	int i;
	LCLCounter * pt;

	for (i=0; i<lcl->hashsize;i++)
		lcl->hashtable[i]=0;
	// first, reset the hash table
	for (i=1; i<=lcl->size;i++) {
		lcl->counters[i].next=NULL;
		lcl->counters[i].prev=NULL;
	}
	// empty out the linked list
	for (i=1; i<=lcl->size;i++) { // for each item in the data structure
		pt=&lcl->counters[i];
		pt->next=lcl->hashtable[lcl->counters[i].hash];
		if (pt->next)
			pt->next->prev=pt;
		lcl->hashtable[lcl->counters[i].hash]=pt;
	}
}

void Heapify(LCL_type * lcl, int ptr)
{ // restore the heap condition in case it has been violated
	LCLCounter tmp;
	LCLCounter * cpt, *minchild;
	int mc;

	while(1)
	{
		if ((ptr<<1) + 1>lcl->size) break;
		// if the current node has no children

		cpt=&lcl->counters[ptr]; // create a current pointer
		mc=(ptr<<1)+
			(((ptr<<1)+1==lcl->size)||(lcl->counters[ptr<<1].count<lcl->counters[(ptr<<1)+1].count)? 0 : 1);
		minchild=&lcl->counters[mc];
		// compute which child is the lesser of the two

		if (cpt->count < minchild->count) break;
		// if the parent is less than the smallest child, we can stop

		P("\tswapping %d[%d,%d] %d[%d,%d]\n", ptr, cpt->count, cpt->hash, mc, minchild->count, minchild->hash);
		tmp=*cpt;
		*cpt=*minchild;
		*minchild=tmp;
		// else, swap the parent and child in the heap

		if (cpt->hash==minchild->hash)
			// test if the hash value of a parent is the same as the 
			// hash value of its child
		{ 
			// swap the prev and next pointers back. 
			// if the two items are in the same linked list
			// this avoids some nasty buggy behaviour
			minchild->prev=cpt->prev;
			cpt->prev=tmp.prev;
			minchild->next=cpt->next;
			cpt->next=tmp.next;
		} else { // ensure that the pointers in the linked list are correct
			// check: hashtable has correct pointer (if prev ==0)
			if (!cpt->prev) { // if there is no previous pointer
				if (cpt->item!=LCL_NULLITEM)
					lcl->hashtable[cpt->hash]=cpt; // put in pointer from hashtable
			} else
				cpt->prev->next=cpt;
			if (cpt->next) 
				cpt->next->prev=cpt; // place in linked list

			if (!minchild->prev) // also fix up the child
				lcl->hashtable[minchild->hash]=minchild; 
			else
				minchild->prev->next=minchild; 
			if (minchild->next)
				minchild->next->prev=minchild;
		}
		ptr=mc;
		// continue on with the heapify from the child position
	} 
}

LCLCounter * LCL_FindItem(LCL_type * lcl, LCLitem_t item)
{ // find a particular item in the date structure and return a pointer to it
	LCLCounter * hashptr;
	int hashval;

	hashval=(int) hash31(lcl->hasha, lcl->hashb,item) % lcl->hashsize;
	hashptr=lcl->hashtable[hashval];
	// compute the hash value of the item, and begin to look for it in 
	// the hash table

	while (hashptr) {
		if (hashptr->item==item)
			break;
		else hashptr=hashptr->next;
	}
	return hashptr;
	// returns NULL if we do not find the item
}

void LCL_Update(LCL_type * lcl, LCLitem_t item, LCLweight_t value)
{
	int hashval;
	LCLCounter * hashptr;
	// find whether new item is already stored, if so store it and add one
	// update heap property if necessary

	lcl->n+=value;
	lcl->counters->item=0; // mark data structure as 'dirty'

	hashval=(int) hash31(lcl->hasha, lcl->hashb,item) % lcl->hashsize;
	hashptr=lcl->hashtable[hashval];
	// compute the hash value of the item, and begin to look for it in 
	// the hash table

	while (hashptr) {
		if (hashptr->item==item) {
			P("\tIn hashtable\n");
			hashptr->count+=value; // increment the count of the item
			Heapify(lcl,hashptr-lcl->counters); // and fix up the heap
			return;
		}
		else hashptr=hashptr->next;
	}
	P("\tNot in hashtable\n");
	// if control reaches here, then we have failed to find the item
	// so, overwrite smallest heap item and reheapify if necessary
	// fix up linked list from hashtable
	if (!lcl->root->prev) // if it is first in its list
		lcl->hashtable[lcl->root->hash]=lcl->root->next;
	else
		lcl->root->prev->next=lcl->root->next;
	if (lcl->root->next) // if it is not last in the list
		lcl->root->next->prev=lcl->root->prev;
	// update the hash table appropriately to remove the old item

	// slot new item into hashtable
	hashptr=lcl->hashtable[hashval];
	lcl->root->next=hashptr;
	if (hashptr)
		hashptr->prev=lcl->root;
	lcl->hashtable[hashval]=lcl->root;
	// we overwrite the smallest item stored, so we look in the root
	lcl->root->prev=NULL;
	lcl->root->item=item;
	lcl->root->hash=hashval;
	lcl->root->delta=lcl->root->count;
	// update the implicit lower bound on the items frequency
	//  value+=lcl->root->delta;
	// update the upper bound on the items frequency
	lcl->root->count=value+lcl->root->delta;
	Heapify(lcl,1); // restore heap property if needed
	// return value;
}

LCLweight_t LCL_Size(LCL_type * lcl)
{ // return the size of the data structure in bytes
	return sizeof(LCL_type) + (lcl->hashsize * sizeof(int)) + 
		(lcl->size*sizeof(LCLCounter));
}

LCLweight_t LCL_PointEstUpp(LCL_type * lcl, LCLitem_t item)
{ // estimate the count of a particular item upper bound
	LCLCounter * i;
	i=LCL_FindItem(lcl,item);
	if (i)
		return(i->count);
	else
		return lcl->root->count;
}

LCLweight_t LCL_PointEstLow(LCL_type * lcl, LCLitem_t item)
{ // estimate the count of a particular item lower bound
	LCLCounter * i;
	i=LCL_FindItem(lcl,item);
	if (i)
		return(i->count)-(i->delta);
	else
		return 0;
}

