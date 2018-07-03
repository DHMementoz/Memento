#ifndef ALLOC_H
#define ALLOC_H
	#ifdef MEMORY_STATS
		#define MALLOC(x) memmalloc(x)
		#define CALLOC(x, y) memcalloc(x, y)
		#define REALLOC(x, y) memrealloc(x, y)
		#define FREE(x) memfree(x)
		void * memmalloc(int x);
		void * memcalloc(int x, int y);
		void * memrealloc(void * x, int y);
		void memfree(void * x);
	#else
		#define MALLOC(x) malloc(x)
		#define CALLOC(x, y) calloc(x, y)
		#define REALLOC(x, y) realloc(x, y)
		#define FREE(x) free(x)
	#endif
	int maxmemusage();
#endif
