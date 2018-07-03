#include "RSS_CPP.hpp"

#include <stdlib.h>
#include <stdio.h>
#include <stdexcept>

#include "prng.h"

long hash31(int64__t a, int64__t b, int64__t x);


RSS_CPP::RSS_CPP(float fPhi, int M, float gamma)
{
	int i;
	//int k = 1 + (int) 1.0 / fPhi;
	m_nrCounters = ceil((int) 1.0 / fPhi);

	//RSS_type* result = (RSS_type*)calloc(1, sizeof(RSS_type));
	//std::cout << "M = " << M << std::endl;
	m_M = M;
	m_gamma = gamma;
	m_stepSize =  1 + ceil(M* gamma/ 2.) ;
	//printf("m_stepSize = %d, gamma=%f\n",m_stepSize, gamma);
	//exit(1);
	m_stepSizeMinusOne = m_stepSize - 1;
	//if (m_stepSize <= 0)
		//throw std::exception::exception("bad config, make sure gamma >= 0");
	assert(m_stepSize);
	//assert(gamma > 0);
	m_nrCounters *= (1 + gamma);
	//printf("in ctor, m_nrCounters = %d, fPhi = %f\n", m_nrCounters, fPhi);
	
	m_a = (long long)698124007;
	m_b = (long long)5125833;
	assert(m_nrCounters > 0);
	//m_n = 0;
#if UNDER_ESTIMATOR
	m_maxRootVictim = 0;
#else
	m_maxRootVictim = m_stepSizeMinusOne;
#endif

	m_tblsz = RSS_HASHMULT*m_nrCounters;
	m_hashtable = (RSSITEM_RANA**)calloc(m_tblsz, sizeof(RSSITEM_RANA *));
	m_groups = (RSSGROUP_RANA *)calloc(m_nrCounters, sizeof(RSSGROUP_RANA));
	m_items = (RSSITEM_RANA *)calloc(m_nrCounters, sizeof(RSSITEM_RANA));
	m_freegroups = (RSSGROUP_RANA **)calloc(m_nrCounters, sizeof(RSSGROUP_RANA*));

	for (i = 0; i<m_tblsz; i++)
		m_hashtable[i] = NULL;

	m_root = m_groups;
	m_groups->count = 0;
	m_groups->nextg = NULL;
	m_groups->previousg = NULL;

	m_groups->items = m_items;
	for (i = 0; i<m_nrCounters; i++)
		m_freegroups[i] = &m_groups[i];
	m_gpt = 1; // initialize list of free groups

	for (i = 0; i<m_nrCounters; i++)
	{
		m_items[i].item = 0;
		m_items[i].delta = -1;
		m_items[i].hash = 0;
		m_items[i].nexti = NULL;
		m_items[i].previousi = NULL;  // initialize values

		m_items[i].parentg = m_groups;
		m_items[i].nexting = &(m_items[i + 1]);
		m_items[i].previousing = &(m_items[i - 1]);
		// create doubly linked list
	}
	m_items[0].previousing = &(m_items[m_nrCounters - 1]);
	m_items[m_nrCounters - 1].nexting = &(m_items[0]);
	// fix start and end of linked list

	m_nrFree = m_nrCounters;
}

RSS_CPP::~RSS_CPP()
{
	free(m_items);
	free(m_groups);
	free(m_freegroups);
	free(m_hashtable);
}
//RSSGROUP_RANA *dbg=NULL;

void RSS_CPP::update(unsigned newitem, int weight) {
	/*if (dbg){
		printf("dbg count = %u, newitem=%u\n", dbg->count, newitem);
		if (newitem == 22652416){
			printf("asdasfsdgdafhgbdfbnjkdbndkjb\n");
			printf("%p:%u->%p:%u->%p:%u\n",dbg,dbg->count,dbg->nextg,dbg->nextg->count,dbg->nextg->nextg,dbg->nextg->nextg->count);
		}			
	}*/
	int h = getHash(newitem);
	RSSITEM_RANA *til = getCounter(newitem, h);
	RSSITEM_RANA &il = (til ? *til : takoverMinimal(newitem, h));
	/*if (dbg){
		printf("2:dbg count = %u, newitem=%u\n", dbg->count, newitem);		
	}*/	
#if UNDER_ESTIMATOR
	if (!(til || m_nrFree))
			il.delta = m_root->count;
	//il.parentg->items = il.parentg->items->nexting;
#endif
	int newVal = weight + (til ? il.remainder : m_maxRootVictim);
	unsigned int nrIncs = newVal / m_stepSize;
	/*if (dbg){
		printf("3:dbg count = %u, newitem=%u\n", dbg->count, newitem);		
	}
	il.remainder = newVal % m_stepSize; //TODO: consider changing % to shifts
	if (dbg){
		printf("4:dbg count = %u, newitem=%u\n", dbg->count, newitem);		
	}*/	
	//printf("nrIncs = %d, m_nrFree = %d, til = %p\n", nrIncs, m_nrFree, til);
	if (nrIncs && (!m_nrFree || til)) {
		advanceCounter(il, nrIncs);
		/*if (dbg){
			printf("5:dbg count = %u, newitem=%u\n", dbg->count, newitem);		
		}		
		if ((newitem==23106048) && (il.parentg->count>=1399)){
			printf("qqq:23106048 is here, pg=%p, count = %u\n", &il.parentg, il.parentg->count);
			dbg = il.parentg->nextg;
			if (il.parentg->count == 0){
				printf("ASDASGF\n");
				exit(1);
			}
			if (il.parentg->nextg){
				printf("nextg=%p:%u, item=%u\n",il.parentg->nextg,il.parentg->nextg->count,il.parentg->nextg->items->item);
			}
		}		
		if (dbg){
			printf("6:dbg count = %u, newitem=%u\n", dbg->count, newitem);		
		}*/		
		return;
	}
	if (m_nrFree && !til)
	{ // We deliberately use duplicated code as this `else' is rarely accessed
		newVal = weight;
		nrIncs = newVal / m_stepSize;
		--m_nrFree;
		il.parentg->items = il.parentg->items->nexting;
		il.remainder = newVal % m_stepSize; //TODO: consider changing % to shifts
		if (nrIncs)
			advanceCounter(il, nrIncs);
	}
	/*if (newitem==23106048){
		printf("23106048 is here, count = %u\n", il.parentg->count);	
		if (il.parentg->count == 0){
			printf("ASDASGF\n");
			exit(1);
		}
	}*/	
	/*printf("m_root = %p, m_root->count = %d\n", m_root, m_root->count);
	if (m_root->nextg)
		printf("m_root = %p, m_root->count = %d, m_root->nextg = %p, m_root->nextg->count = %d\n", m_root, m_root->count, m_root->nextg, m_root->nextg->count);
	*/
}

inline int const RSS_CPP::getHash(unsigned int item) {
	return hash31(m_a, m_b, item) % m_tblsz;
}

inline RSSITEM_RANA * RSS_CPP::getCounter(unsigned int item, int h) {
	RSSITEM_RANA * il = m_hashtable[h];
	while (il && (il->item != item))
		il = il->nexti;
	return il;
}

inline RSSITEM_RANA & RSS_CPP::takoverMinimal(unsigned int item, int h) {
	RSSITEM_RANA & il = *(m_root->items);
	removeFromHash(il);
	insertIntoHashtable(il, h, item);
#if UNDER_ESTIMATOR
		if (m_maxRootVictim < il.remainder)
		{
			m_maxRootVictim = il.remainder;
		}
#endif
	return il;
}

void RSS_CPP::advanceCounter(RSSITEM_RANA &il, unsigned int nrIncs) {
	//RSSGROUP_RANA *g = il.parentg;
	/*if (((RSSGROUP_RANA *)0x791050)->count == 0)
		printf("zerrrrro\n");*/
	/*if (g->count >= 807){
		while (g->nextg){
			assert(g->count != g->nextg->count);
			g = g->nextg;
		}
	}
	assert(!(g->nextg) || 
				( (g->count != g->nextg->count) && (!(g->nextg->nextg) || (g->nextg->count != g->nextg->nextg->count)) )
			);
	if (g->count >= 1){
		while (g){
			//printf("xxx: %p, %u ", g, g->count);
			//assert(g->count != 0);
			if (!(g->count)){
				g = il.parentg;
				while (g){
					g = g->nextg;
				}
				int h = getHash(23106048);
				RSSITEM_RANA *til = getCounter(23106048, h);	
			}
			g = g->nextg;
		}
	}*/
	RSSGROUP_RANA &oldgroup = *(il.parentg);
	unsigned int goalVal = oldgroup.count + nrIncs;
	RSSGROUP_RANA &group = getLastGroup(oldgroup, goalVal);
	/*if ((goalVal>=1398) && (il.item == 23106048)){
		printf("000: %u, %p, %u, %p, %u, %p\n", il.item, il.parentg, il.parentg->count, &group, group.count, group.nextg);
		if (il.parentg->nextg){
			printf("111: %p, %u\n", il.parentg->nextg, il.parentg->nextg->count);
			if(il.parentg->count == il.parentg->nextg->count){
				printf("1:WTF???\n");
				exit(1);
			}				
			if (il.parentg->nextg->nextg){
				printf("222: %p, %u\n", il.parentg->nextg->nextg, il.parentg->nextg->nextg->count);
				if (il.parentg->nextg->nextg->count == 0){
					printf("WTF???\n");
				}
				if(il.parentg->nextg->count == il.parentg->nextg->nextg->count){
					printf("2:WTF???\n");
					exit(1);
				}					
				if (il.parentg->nextg->nextg->nextg){
					printf("333: %p, %u\n", il.parentg->nextg->nextg->nextg, il.parentg->nextg->nextg->nextg->count);
					if (il.parentg->nextg->nextg->nextg->count == 0){
						printf("333:WTF???\n");
						exit(1);
					}
					if(il.parentg->nextg->nextg->count == il.parentg->nextg->nextg->nextg->count){
						printf("3:WTF???\n");
						exit(1);
					}					
					if (il.parentg->nextg->nextg->nextg->nextg){
						printf("444: %p, %u\n", il.parentg->nextg->nextg->nextg->nextg, il.parentg->nextg->nextg->nextg->nextg->count);
						if (il.parentg->nextg->nextg->nextg->nextg->count == 0){
							printf("444:WTF???\n");
							exit(1);
						}
						if(il.parentg->nextg->nextg->nextg->count == il.parentg->nextg->nextg->nextg->nextg->count){
							printf("4:WTF???\n");
							exit(1);
						}						
					}						
				}				
			}
		}
	}*/

	if ((il.nexting) == &il) {      // if the counter is the only one in its group

		if (&group == &oldgroup) {    // if we can simply increase the oldgroup's count
			oldgroup.count = goalVal;
			assert(goalVal > 0);
			return;
		}
		if (group.count == goalVal) { // if there exists a group with count = goalVal
			putInNewGroup(il, group);
			return;
		}
		moveGroup(il, group, goalVal);
		return;
	}
	disconnectCounter(il);
	assert(group.count <= goalVal);

	if (group.count == goalVal) {
		putInNewGroup(il, group); // if the goal group exists	
		return;
	}
	newGroup(il, group, goalVal);
}

inline void RSS_CPP::disconnectCounter(RSSITEM_RANA & il) { //disconnect only if there exists other counters in the same group
	if (il.parentg->items == &il) {
		il.parentg->items = il.nexting;
	}
	//il.parentg->items = (il.parentg->items == &il) ? il.nexting: il.parentg->items);

	assert(il.previousing);
	assert(il.nexting);
	il.previousing->nexting = il.nexting;
	il.nexting->previousing = il.previousing;
	il.nexting = &il;
	il.previousing = &il;
}

inline RSSGROUP_RANA & RSS_CPP::getLastGroup(RSSGROUP_RANA & oldgroup, unsigned int goalVal) {
	RSSGROUP_RANA *group = &oldgroup;
	while (group->nextg && (group->nextg->count <= goalVal)) {
		group = group->nextg;
	}
	assert((group->items->nexting == group->items) == (group->items->previousing == group->items));
	return *group;
}

inline void RSS_CPP::removeFromHash(RSSITEM_RANA & il) {
	// if il was first - the chain will point on the next item.
	if (m_hashtable[il.hash] == &il)
		m_hashtable[il.hash] = il.nexti;
	// if there is another node with the same hash. 
	if (il.nexti)
	{
		// next item - previous - points to my previous.
		il.nexti->previousi = il.previousi;

	}
	if (il.previousi) {
		//prev item next - points to my next.
		il.previousi->nexti = il.nexti;
	}
}

inline void RSS_CPP::addToGroup(RSSGROUP_RANA & group, RSSITEM_RANA & il) {
	il.previousing = group.items->previousing;
	il.nexting = group.items;
	il.parentg = &group;
	group.items->previousing->nexti = &il;
	group.items->previousing = &il;
}



inline void const RSS_CPP::showGroups() {
	RSSGROUP_RANA *g;
	RSSITEM_RANA *i, *first;
	int n, wt;

	g = m_groups;
	wt = 0;
	n = 0;
	while (g != NULL)
	{
		printf("Group %lld :", g->count);
		first = g->items;
		i = first;
		if (i != NULL)
			do
			{
				printf("%d -> ", i->item);
				i = i->nexting;
				wt += g->count;
				n++;
			} while (i != first);
		else printf(" empty");
		printf(")");
		g = g->nextg;
		if ((g != NULL) && (g->previousg->nextg != g))
			printf("Badly linked");
		printf("\n");
	}
	printf("In total, %d items, with a total count of %d\n", n, wt);
}

void RSS_CPP::insertIntoHashtable(RSSITEM_RANA &newi, int i, unsigned int newitem) {
	newi.nexti = m_hashtable[i];
	newi.item = newitem; // overwrite the old item
	newi.hash = i;
	newi.previousi = NULL;
	// insert item into the hashtable
	if (m_hashtable[i])
		m_hashtable[i]->previousi = &newi;
	m_hashtable[i] = &newi;
}

unsigned int const RSS_CPP::query(unsigned int x) {
	/*if (dbg){
		printf("QUERY: dbg count = %u, x=%u\n", dbg->count, x);
		if (x == 22652416){
			if (dbg->nextg && dbg->nextg->nextg)
			printf("%p:%u->%p:%u->%p:%u\n",dbg,dbg->count,dbg->nextg,dbg->nextg->count,dbg->nextg->nextg,dbg->nextg->nextg->count);
		}			
	}*/	
	int h;
	RSSITEM_RANA *il;
	h = hash31(m_a, m_b, x) % m_tblsz;
	il = m_hashtable[h];
	// if x has a counter.
	while (il && il->item != x)
		il = il->nexti;
	if (il){
		if (il->delta == -1)
			return il->parentg->count * m_stepSize + il->remainder;
		return il->parentg->count * m_stepSize + il->remainder -(il->delta + 1)*m_stepSize - 1;
	}
	if (UNDER_ESTIMATOR)
		return 0;
	int minCount = m_root->count;
	// if all counters are used. 
	if (minCount)
		return minCount * m_stepSize - 1;
	// if there are unused counters and x does not have a counter, we are certain that x never arrived. 
	return 0;
}


inline void RSS_CPP::connectToGroup(RSSITEM_RANA &newi, RSSGROUP_RANA &tmpg) {
	newi.nexting = tmpg.items;
	newi.previousing = tmpg.items->previousing;
	assert((newi.previousing != &newi) && (newi.nexting != &newi));
	newi.previousing->nexting = &newi;
	newi.nexting->previousing = &newi;
}

inline void RSS_CPP::putInNewGroup(RSSITEM_RANA &newi, RSSGROUP_RANA & tmpg) {
	RSSGROUP_RANA * oldgroup = newi.parentg;
	assert(oldgroup != &tmpg);
	// put item in the tmpg group
	newi.parentg = &tmpg;
	assert((newi.previousing == &newi) && (newi.nexting == &newi));
	if (oldgroup->items == &newi) { // group will be empty
		recycleGroup(*oldgroup);
	}
	connectToGroup(newi, tmpg);
}

inline void RSS_CPP::moveGroup(RSSITEM_RANA &il, RSSGROUP_RANA &group, unsigned int goalVal) {
	//if (il.parentg->nextg && il.parentg->nextg->count >= goalVal){
	//	printf("%p, %u, %p, %u, %p, %u, %u\n", il.parentg, il.parentg->count, il.parentg->nextg, il.parentg->nextg->count, &group, group.count, goalVal);
	//	showGroups();
	//}
	assert(il.parentg->nextg && il.parentg->nextg->count < goalVal);
	RSSGROUP_RANA &tmpg = *(il.parentg);

	if (&tmpg == m_root) // might fail if we allocate only a single counter
	{
		m_root = tmpg.nextg;
		m_root->previousg = NULL;
#if UNDER_ESTIMATOR
		m_maxRootVictim = 0;
#endif
	}
	assert(!(m_root->previousg));
	if (tmpg.previousg)
		tmpg.previousg->nextg = tmpg.nextg;
	assert(tmpg.nextg);
	tmpg.nextg->previousg = tmpg.previousg;
	if (group.nextg) {
		group.nextg->previousg = &tmpg;
	}
	tmpg.nextg = group.nextg;
	tmpg.previousg = &group;
	tmpg.count = goalVal;
	assert(goalVal > 0);
	group.nextg = &tmpg;
	assert(m_root->nextg != m_root);
}

inline void RSS_CPP::newGroup(RSSITEM_RANA &il, RSSGROUP_RANA &group, unsigned int goalVal) {
	assert(group.count < goalVal);
	//std::cout << m_gpt << ',' << m_n << std::endl;
	assert(il.parentg->items != &il);
	// Create a new group for newi
	// RSS_AddNewGroupAfter(rss, newi,group);
	RSSGROUP_RANA &newgroup = *(m_freegroups[m_gpt++]); //get new group
	newgroup.count = goalVal; // set count to the actual value of the new group
	assert(goalVal > 0);
	newgroup.items = &il;
	//newgroup->previousg = group;
	if (group.nextg) { // if there is another group
		group.nextg->previousg = &newgroup;
	}
	newgroup.nextg = group.nextg;
	newgroup.previousg = &group;
	group.nextg = &newgroup;
	il.parentg = &newgroup;
	assert(m_root->nextg != m_root);
}

int const RSS_CPP::size() {
	return 0;
	//return sizeof(RSS_CPP) + (m_tblsz) * sizeof(RSSITEM*) +
		//(m_nrCounters)*(sizeof(RSSITEM) + sizeof(RSSGROUP) + sizeof(RSSITEM*));
}

inline void RSS_CPP::recycleGroup(RSSGROUP_RANA & oldgroup)
{
	if (oldgroup.nextg) // there is another group
		oldgroup.nextg->previousg = oldgroup.previousg;
	if (oldgroup.previousg)
		oldgroup.previousg->nextg = oldgroup.nextg;
	if (m_root == &oldgroup) // this is the first group
	{
		assert(!(oldgroup.nextg->previousg));
		m_root = oldgroup.nextg;
	}

	//recycle oldgroup.
	m_freegroups[--m_gpt] = &oldgroup;
	// if we have created an empty group, remove it 
	assert(m_root->nextg != m_root);
}

void RSS_CPP::clear()
{
	int i;
	/*for (i = 0; i<m_tblsz; i++)
		m_hashtable[i] = NULL;

	m_groups->count = 0;
	m_groups->nextg = NULL;
	m_groups->previousg = NULL;

	m_groups->items = m_items;// TODO: check if there is more counters to reset
	m_root = m_groups;
	m_nrFree = m_nrCounters;*/
	for (i = 0; i<m_tblsz; i++)
		m_hashtable[i] = NULL;

	m_root = m_groups;
	m_groups->count = 0;
	m_groups->nextg = NULL;
	m_groups->previousg = NULL;

	m_groups->items = m_items;
	for (i = 0; i<m_nrCounters; i++)
		m_freegroups[i] = &m_groups[i];
	m_gpt = 1; // initialize list of free groups

	for (i = 0; i<m_nrCounters; i++)
	{
		m_items[i].item = 0;
		//m_items[i].delta = -1;
		m_items[i].hash = 0;
		m_items[i].nexti = NULL;
		m_items[i].previousi = NULL;  // initialize values

		m_items[i].parentg = m_groups;
		m_items[i].nexting = &(m_items[i + 1]);
		m_items[i].previousing = &(m_items[i - 1]);
		// create doubly linked list
	}
	m_items[0].previousing = &(m_items[m_nrCounters - 1]);
	m_items[m_nrCounters - 1].nexting = &(m_items[0]);
	// fix start and end of linked list

	m_nrFree = m_nrCounters;
	
}
/*
void RSS_CPP::debug(){
	unsigned x = 22652416;
	if (dbg){
		printf("DEBUG: dbg count = %u, x=%u\n", dbg->count, x);
		if (x == 22652416){
			if (dbg->nextg && dbg->nextg->nextg)
			printf("%p:%u->%p:%u->%p:%u\n",dbg,dbg->count,dbg->nextg,dbg->nextg->count,dbg->nextg->nextg,dbg->nextg->nextg->count);
		}			
	}	
}
*/

long hash31(int64__t a, int64__t b, int64__t x)
{

  int64__t result;
  long lresult;

  // return a hash of x using a and b mod (2^31 - 1)
// may need to do another mod afterwards, or drop high bits
// depending on d, number of bad guys
// 2^31 - 1 = 2147483647

  //  result = ((int64__t) a)*((int64__t) x)+((int64__t) b);
  result=(a * x) + b;
  result = ((result >> HL) + result) & MOD;
  lresult=(long) result;

  return(lresult);
}

