

#ifndef BaseWRSS_LongId_hpp
#define BaseWRSS_LongId_hpp

#include <stdio.h>
#include <unordered_map>
#include <vector>

#include "RSS_CPP_LongId.hpp"

using namespace std;

class BaseWRSS_LongId {
public:
	BaseWRSS_LongId(unsigned int windowSize, float gamma, unsigned int m, float epsilon);
    ~BaseWRSS_LongId();
    void update(ID item, int weight);
	void sampleUpdate(ID item, int weight, int timeElapsed);
	void incTimestamp();
    unsigned int query(ID item);
	unsigned int threshold;
	//std::vector<std::pair<unsigned int, LCUWT_LongId>>& allItems(); 
private:
    unsigned int frameItems;
	unsigned int frameBlocks;
	unsigned int blocksNumber;
    unsigned int blockSize;
    unsigned int windowSize;
    vector<char> *index;
    unsigned int tail;
    unsigned int overflowsNumber;
    unsigned int indexTail;
    unsigned int head;
    unsigned int indexHead;
    unordered_map<unsigned int, unsigned  int> *totalOverflows; //B
    ID *overflowsElements;// b
    RSS_CPP_LongId *rss;
    unsigned int indexSize;
    unsigned int maxOverflows;
    double epsilon;
    int m; //maximal value of an element in the stream
    //double alpha = 0.2;
    int computeOverflowCount(unsigned int item);
};

#endif /* BaseWRSS_LongId_hpp */
