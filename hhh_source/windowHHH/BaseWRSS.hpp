

#ifndef BaseWRSS_hpp
#define BaseWRSS_hpp

#include <stdio.h>
#include <unordered_map>
#include <vector>

#include "RSS_CPP.hpp"

using namespace std;

class BaseWRSS {
public:
	BaseWRSS(unsigned int windowSize, float gamma, unsigned int m, float epsilon);
    ~BaseWRSS();
    void update(unsigned int item, int weight);
	void sampleUpdate(unsigned int item, int weight, int timeElapsed);
	void incTimestamp();
    unsigned int query(unsigned int item);
	unsigned int threshold;
	//std::vector<std::pair<unsigned int, LCUWT_RANA>>& allItems(); 
private:
    unsigned int frameItems;
	unsigned int frameBlocks;
	unsigned int blocksNumber;
    unsigned int blockSize;
    unsigned int windowSize;
    vector<int> *index;
    unsigned int tail;
    unsigned int overflowsNumber;
    unsigned int indexTail;
    unsigned int head;
    unsigned int indexHead;
    unordered_map<unsigned int, unsigned  int> *totalOverflows; //B
    unsigned int *overflowsElements;// b
    RSS_CPP *rss;
    unsigned int indexSize;
    unsigned int maxOverflows;
    double epsilon;
    int m; //maximal value of an element in the stream
    //double alpha = 0.2;
    int computeOverflowCount(unsigned int item);
};

#endif /* BaseWRSS_hpp */
