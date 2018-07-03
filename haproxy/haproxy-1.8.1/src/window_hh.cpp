#include <stdlib.h>
#include <stdio.h>

#include "../../../hhh_source/windowHH/BaseWRSS.hpp"

extern "C" void window_hh_update(unsigned int ip);
extern "C" void window_hh_init(unsigned int window_size, float eps);
extern "C" unsigned int window_hh_query(unsigned int ip);

BaseWRSS * window_hh_alg;
	
void window_hh_update(unsigned int ip) {

	window_hh_alg->update(ip, 1);

}

void window_hh_init(unsigned int window_size, float eps) {
	
	window_hh_alg = new BaseWRSS(window_size, 0, 1, eps);
	
}

unsigned int window_hh_query(unsigned int ip) {
	
	return window_hh_alg->query(ip);

}


