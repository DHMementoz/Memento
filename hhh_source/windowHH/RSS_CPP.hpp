#pragma once
#pragma once
#ifndef RSS_CPP_h_rana
#define RSS_CPP_h_rana
#include "ulossycount.h"
#define RSS_HASHMULT 3
#define UNDER_ESTIMATOR 0

typedef struct rss_item_rana RSSITEM_RANA;
typedef struct rss_group_rana RSSGROUP_RANA;

typedef long long LCUWT_RANA;

struct rss_group_rana
{
	LCUWT_RANA count;
	RSSITEM_RANA *items;
	RSSGROUP_RANA*previousg, *nextg;
};

struct rss_item_rana
{
	unsigned int item;
	int hash;
	LCUWT delta;
	int remainder;
	RSSGROUP_RANA *parentg;
	RSSITEM_RANA *previousi, *nexti;
	RSSITEM_RANA *nexting, *previousing;
};

class RSS_CPP {
public:
	RSS_CPP(float fPhi, int M, float gamma);
	~RSS_CPP();
	void update(unsigned item, int weight);
	int const size();
	void clear();
	unsigned int const query(unsigned int item);
	//RSSGROUP_RANA * getRootNode(){return m_root;}
	//std::vector<std::pair<unsigned int, LCUWT_RANA>> allItems();
	//void debug();
private:
	void recycleGroup(RSSGROUP_RANA& oldgroup);
	inline void connectToGroup(RSSITEM_RANA &newi, RSSGROUP_RANA &tmpg);
	void const showGroups();
	void insertIntoHashtable(RSSITEM_RANA &newi, int i, unsigned int newitem);
	RSSITEM_RANA & takoverMinimal(unsigned int item, int h);
	RSSITEM_RANA * getCounter(unsigned int item, int h);
	RSSITEM_RANA * getNewCounter(unsigned int nrIncs);
	inline int const getHash(unsigned int item);
	void advanceCounter(RSSITEM_RANA &il, unsigned int nrIncs);
	void removeFromHash(RSSITEM_RANA & il);
	RSSGROUP_RANA & getLastGroup(RSSGROUP_RANA &oldgroup, unsigned int goalVal);
	void putInNewGroup(RSSITEM_RANA &newi, RSSGROUP_RANA & tmpg);
	void moveGroup(RSSITEM_RANA &il, RSSGROUP_RANA &group, unsigned int goalVal);
	void addToGroup(RSSGROUP_RANA & group, RSSITEM_RANA & il);
	void newGroup(RSSITEM_RANA &il, RSSGROUP_RANA &group, unsigned int goalVal);
	void disconnectCounter(RSSITEM_RANA & il);
	void computeNrIncsAndAdvance(int newVal, RSSITEM_RANA &il);
	//LCUWT m_n;
	int m_gpt;
	int m_tblsz;
	int m_maxRootVictim;
	long long m_a, m_b;
	RSSGROUP_RANA * m_root;
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
	RSSITEM_RANA  *m_items;
	RSSGROUP_RANA *m_groups;
	RSSGROUP_RANA **m_freegroups;
	RSSITEM_RANA  **m_hashtable;

#endif
};

#endif
