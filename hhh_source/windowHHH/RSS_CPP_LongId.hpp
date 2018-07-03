#pragma once
#pragma once
#ifndef RSS_CPP_h_LongId
#define RSS_CPP_h_LongId
#include "ulossycount.h"
#define RSS_HASHMULT 3
#define UNDER_ESTIMATOR 0

typedef unsigned long long ID;

typedef struct rss_item_LongId RSSITEM_LongId;
typedef struct rss_group_LongId RSSGROUP_LongId;

typedef long long LCUWT_LongId;

struct rss_group_LongId
{
	LCUWT_LongId count;
	RSSITEM_LongId *items;
	RSSGROUP_LongId*previousg, *nextg;
};

struct rss_item_LongId
{
	ID item;
	int hash;
	LCUWT delta;
	int remainder;
	RSSGROUP_LongId *parentg;
	RSSITEM_LongId *previousi, *nexti;
	RSSITEM_LongId *nexting, *previousing;
};

class RSS_CPP_LongId {
public:
	RSS_CPP_LongId(float fPhi, int M, float gamma);
	~RSS_CPP_LongId();
	void update(ID item, int weight);
	int const size();
	void clear();
	unsigned int const query(ID item);
	//RSSGROUP_LongId * getRootNode(){return m_root;}
	//std::vector<std::pair<unsigned int, LCUWT_LongId>> allItems();
	//void debug();
private:
	void recycleGroup(RSSGROUP_LongId& oldgroup);
	inline void connectToGroup(RSSITEM_LongId &newi, RSSGROUP_LongId &tmpg);
	void const showGroups();
	void insertIntoHashtable(RSSITEM_LongId &newi, int i, ID newitem);
	RSSITEM_LongId & takoverMinimal(ID item, int h);
	RSSITEM_LongId * getCounter(ID item, int h);
	RSSITEM_LongId * getNewCounter(unsigned int nrIncs);
	inline int const getHash(ID item);
	void advanceCounter(RSSITEM_LongId &il, unsigned int nrIncs);
	void removeFromHash(RSSITEM_LongId & il);
	RSSGROUP_LongId & getLastGroup(RSSGROUP_LongId &oldgroup, unsigned int goalVal);
	void putInNewGroup(RSSITEM_LongId &newi, RSSGROUP_LongId & tmpg);
	void moveGroup(RSSITEM_LongId &il, RSSGROUP_LongId &group, unsigned int goalVal);
	void addToGroup(RSSGROUP_LongId & group, RSSITEM_LongId & il);
	void newGroup(RSSITEM_LongId &il, RSSGROUP_LongId &group, unsigned int goalVal);
	void disconnectCounter(RSSITEM_LongId & il);
	void computeNrIncsAndAdvance(int newVal, RSSITEM_LongId &il);
	//LCUWT m_n;
	int m_gpt;
	int m_tblsz;
	int m_maxRootVictim;
	long long m_a, m_b;
	RSSGROUP_LongId * m_root;
	int m_stepSize;
	int m_stepSizeMinusOne;
	int m_M;
	float m_gamma;
	int m_nrCounters;
	unsigned int m_nrFree;
#ifdef LCU_SIZE
	LCUITEM items[LCU_SIZE];
	LCUGROUP groups[LCU_SIZE];
	LCUGROUP *freegroups[LCU_SIZE];
	LCUITEM* hashtable[LCU_TBLSIZE];
#else
	RSSITEM_LongId  *m_items;
	RSSGROUP_LongId *m_groups;
	RSSGROUP_LongId **m_freegroups;
	RSSITEM_LongId  **m_hashtable;

#endif
};

#endif
