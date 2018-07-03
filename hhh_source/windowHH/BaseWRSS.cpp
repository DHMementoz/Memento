

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "BaseWRSS.hpp"
#include <iostream>


using namespace std;

BaseWRSS::BaseWRSS(unsigned int windowSize, float gamma, unsigned int m, float epsilon)
{
    overflowsNumber = 0;
    tail = 0;
    indexTail = 0;
    this->blockSize = ((windowSize * epsilon)/4); // W/k; k= 4/epsilon
	frameItems = blockSize;
    this->windowSize = windowSize;
    this->blocksNumber = windowSize / blockSize;
	this->frameBlocks = this->blocksNumber;
    this->m = m;
    this->epsilon = epsilon;
    maxOverflows = min(blocksNumber * 2, windowSize);
    indexSize = maxOverflows + blocksNumber;
    head = maxOverflows - 1;
    indexHead = blocksNumber - 1;
    index = new vector<int> (indexSize); // 0 means end of block
    overflowsElements = new unsigned int[maxOverflows];
    rss = new RSS_CPP(epsilon / 4., m, gamma);//y
    threshold = (unsigned int) windowSize*m*epsilon / 4; // W*M/k
	//printf("in WRSS, epsilon = %f, blockSize = %d, theshold = %d, indexSize = %d, maxOverflows = %d\n", epsilon, blockSize, threshold, indexSize, maxOverflows);

    totalOverflows = new unordered_map<unsigned int, unsigned int> (maxOverflows);//B TODO: allocate static?
}

BaseWRSS::~BaseWRSS()
{
    delete(totalOverflows);
    delete(rss);
    delete [] overflowsElements;
    delete(index);
}

void BaseWRSS::incTimestamp(){
	//printf("in incTimestamp\n");
	if (!(--frameItems)) {
        indexTail = (indexTail + 1) % indexSize;
        indexHead = (indexHead + 1) % indexSize;
        index->at(indexHead) = 0;
		frameItems = blockSize;
		--frameBlocks;
    }

    // Remove oldest element in oldest block
	unsigned int oldId = overflowsElements[tail];
    try {
		//printf("1 ");
        if (index->at(indexTail)) {
			//if (frameItems == 78066)
			//	printf("2 frameItems = %d, oldId = %u\n", frameItems, oldId);
			//printf("totalOverflows->at(oldId) = %d ", totalOverflows->at(oldId));
			//printf("before, totalOverflows->at(oldId) = %d", totalOverflows->at(oldId));
			int cnt = totalOverflows->at(oldId);
			//if (frameItems == 78066)
			//	printf("3 frameItems = %d\n", frameItems);			
			totalOverflows->erase(oldId);
			//if (frameItems == 78066)
			//	printf("4 frameItems = %d\n", frameItems);			
            if (cnt - 1)
            {
				//if (frameItems == 78066)
				//	printf("5 frameItems = %d\n", frameItems);
                totalOverflows->insert(make_pair(oldId,cnt - 1));
				//if (frameItems == 78066)
				//	printf("6 frameItems = %d\n", frameItems);				
				//printf("after, totalOverflows->at(oldId) = %d", totalOverflows->at(oldId));
			}
			
			//printf("3 ");
			//if (frameItems == 78066)
			//		printf("7 frameItems = %d\n", frameItems);
            tail = (tail + 1) % maxOverflows;
			//if (frameItems == 78066)
			//		printf("8 frameItems = %d\n", frameItems);
//            --overflowsNumber;
			//printf("4 ");
            indexTail = (indexTail + 1) % indexSize;
			//if (frameItems == 78066)
			//		printf("9 frameItems = %d\n", frameItems);
        }
    } catch (const out_of_range) {
		/*printf("caught it! oldId = %d, tail = %d, overflowsElements[tail] = %d\n", oldId, tail, overflowsElements[tail]);
		for (int i=0; i < maxOverflows; ++i){
			printf("%d ", overflowsElements[i]);
		}*/
		printf("failed with this->epsilon = %lf, frameItems = %d\n", this->epsilon, frameItems);
		//for (auto it : totalOverflows) 
		//	std::cout << " " << it.first << ":" << it.second;
		exit(1);
    }
	
}


void BaseWRSS::update(unsigned int item, int weight)
{
	//printf("in update\n");
	incTimestamp();
	//printf("in update after incTimestamp\n");

	unsigned int prevQuery = this->rss->query(item);
    this->rss->update(item, weight);
	//printf("in update after update, prevQuery=%d, threshold = %d, weight =%d\n", prevQuery, threshold, weight);

    if ( (prevQuery%threshold) + weight >= threshold) {
		//printf("1\n");
        head = (head + 1) % maxOverflows;
		//printf("2\n");
        overflowsElements[head] = item;
		//printf("3\n");
        indexHead = (indexHead + 1) % indexSize;
		//printf("4\n");
        index->at(indexHead) = 1;
		//printf("in update before if\n");
        if (totalOverflows->find(item) == totalOverflows->end())
            totalOverflows->insert(make_pair<unsigned int, unsigned int>((int)item,1));
        else
            totalOverflows->at(item) = totalOverflows->at(item) + 1;
    }
	
	//this->rss->debug();		
	//printf("in update before New frame\n");
    // New frame
    if (!frameBlocks) {
        frameItems = blockSize;
		frameBlocks = blocksNumber;
        rss->clear();
    }	
	//this->rss->debug();		
}

unsigned int BaseWRSS::query(unsigned int item)
{
    int minOverFlows;
    unsigned int rssEstimation = this->rss->query(item);
	
    
	unordered_map<unsigned int, unsigned int>::const_iterator foundedItem = totalOverflows->find(item);
    if (foundedItem == totalOverflows->end()) // item has no oveflows
        minOverFlows = 0;
    else {
        minOverFlows = totalOverflows->at(item);
    }
	//printf("2\n");
	//printf("rssEstimation = %d\n", rssEstimation);
    rssEstimation = rssEstimation % (int) this->threshold; //TODO
	//printf("3\n");
	//if ((this->threshold * (minOverFlows + 2 ) + rssEstimation) > 20)
	//	printf("rssEstimation = %d, this->threshold = %d, minOverFlows = %d\n", rssEstimation, this->threshold, minOverFlows);
    return (this->threshold * (minOverFlows + 2 ) + rssEstimation);
}

/*
void BaseWRSS::sampleUpdate(unsigned int item, int weight, int timeElapsed){
	frameItems += timeElapsed;
	while (frameItems >= blockSize){
		indexTail = (indexTail + 1) % indexSize;
        indexHead = (indexHead + 1) % indexSize;
        index->at(indexHead) = 0;	
	}
    if (((++frameItems) % blockSize) == 0) {
        indexTail = (indexTail + 1) % indexSize;
        indexHead = (indexHead + 1) % indexSize;
        index->at(indexHead) = 0;
		while (true){
			try {
				if (index->at(indexTail)) {
					int oldId = overflowsElements[tail];
					if (!(totalOverflows->at(oldId) - 1))
						totalOverflows->erase(oldId);
					else
						totalOverflows->insert(make_pair(oldId,totalOverflows->at(oldId) - 1));
					tail = (tail + 1) % maxOverflows;
					indexTail = (indexTail + 1) % indexSize;
				}
				} catch (const out_of_range) {
					break;
				}
		}
    }

    // Remove oldest element in oldest block

    

	unsigned int prevQuery = this->rss->query(item);
    this->rss->update(item, weight);

    if ( (prevQuery%threshold) + weight >= threshold) {
        head = (head + 1) % maxOverflows;
        overflowsElements[head] = item;
        indexHead = (indexHead + 1) % indexSize;
        index->at(indexHead) = 1;
        if (totalOverflows->find(item) == totalOverflows->end())
            totalOverflows->insert(make_pair<unsigned int, unsigned int>((int)item,1));
        else
            totalOverflows->at(item) = totalOverflows->at(item) + 1;
    }
	

    // New frame
    if (!frameBlocks) {
        frameItems = blockSize;
		frameBlocks = blocksNumber;
        rss->clear();
    }	
}
*/
