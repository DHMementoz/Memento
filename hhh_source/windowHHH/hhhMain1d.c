
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include <sys/timeb.h>

#ifndef NRRUNS
#define NRRUNS 5
#endif

#ifndef VMULT
#define VMULT 1
#endif

#if VMULT>1
#define PROB
#endif

#ifndef CLK_PER_SEC
#ifdef CLOCKS_PER_SEC
#define CLK_PER_SEC CLOCKS_PER_SEC
#endif
#endif

#include "wrandhhh1D.h"

/*
#if NUM_MASKS==5
LCLitem_t wrhhh1d_masks[NUM_MASKS] = {
	0xFFFFFFFFu,
	0xFFFFFF00u,
	0xFFFF0000u,
	0xFF000000u,
	0x00000000u
};
int wrhhh1d_leveleps[NUM_MASKS] = {
	32,
	24,
	16,
	8,
	0
};
#endif
 

#if NUM_MASKS==33
LCLitem_t wrhhh1d_masks[NUM_MASKS] = {
	0xFFFFFFFFu << 0,
	0xFFFFFFFFu << 1,
	0xFFFFFFFFu << 2,
	0xFFFFFFFFu << 3,
	0xFFFFFFFFu << 4,
	0xFFFFFFFFu << 5,
	0xFFFFFFFFu << 6,
	0xFFFFFFFFu << 7,
	0xFFFFFFFFu << 8,
	0xFFFFFFFFu << 9,
	0xFFFFFFFFu << 10,
	0xFFFFFFFFu << 11,
	0xFFFFFFFFu << 12,
	0xFFFFFFFFu << 13,
	0xFFFFFFFFu << 14,
	0xFFFFFFFFu << 15,
	0xFFFFFFFFu << 16,
	0xFFFFFFFFu << 17,
	0xFFFFFFFFu << 18,
	0xFFFFFFFFu << 19,
	0xFFFFFFFFu << 20,
	0xFFFFFFFFu << 21,
	0xFFFFFFFFu << 22,
	0xFFFFFFFFu << 23,
	0xFFFFFFFFu << 24,
	0xFFFFFFFFu << 25,
	0xFFFFFFFFu << 26,
	0xFFFFFFFFu << 27,
	0xFFFFFFFFu << 28,
	0xFFFFFFFFu << 29,
	0xFFFFFFFFu << 30,
	0xFFFFFFFFu << 31,
	0x00000000u
};
int wrhhh1d_leveleps[NUM_MASKS] = {
	                     32,31,30,
	29,28,27,26,25,24,23,22,21,20,
	19,18,17,16,15,14,13,12,11,10,
	 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};
#endif
*/

double dblmainmax(double a, double b) {return (a >= b ? a : b);}

int main(int argc, char * argv[]) {
		int m; //number of heavy hitters in output
		int counters = 100;
		int n = 100000;
		double time, walltime;
		double epsil;
		int i,j;
		unsigned int w, x, y, z;
		clock_t begint, endt;
		struct timeb begintb, endtb;
		unsigned int ip;
		unsigned int * data;
		double insertProb = 1. / VMULT;	
		double measurements[NRRUNS] = {0};
		double t_4 = 2.572; //Change if NRRUNS!=5
		double avg = 0, stdev = 0, upCI = 0, dnCI= 0, sos = 0;
		
		//unsigned windowSize = 1<<20;
		unsigned windowSize;
		float gamma=0.;
		unsigned M=1;
		
		FILE * fp = NULL; //the file for the output
		FILE * fsummary = NULL;
		if (getenv("SUMMARYFILE") != NULL) fsummary = fopen(getenv("SUMMARYFILE"), "a");

		if (argc > 1) n = atoi(argv[1]);
		if (argc > 2) windowSize = atoi(argv[2]);
		if (argc > 3) counters = atoi(argv[3]);		
		if (argc > 4) insertProb = atof(argv[4]);
		if (argc > 5) fp = fopen(argv[5], "w");

		data = (unsigned int *) malloc(sizeof(unsigned int) * n);

		for (i = 0; i < n; i++) {
			scanf("%d%d%d%d", &w, &x, &y, &z);
			ip = (unsigned int)256*((unsigned int)256*((unsigned int)256*w + x) + y) + z;
			data[i] = ip;
		}

		for (j=0; j < NRRUNS; ++j){
			wrhhh1d_init((double)1/(double)counters, insertProb, windowSize, gamma, M);
			begint = clock();
			ftime(&begintb);
			for (i = 0; i < n; i++){
				wrhhh1d_update(data[i]);
			}
			endt = clock();
			ftime(&endtb);

			time = ((double)(endt-begint))/CLK_PER_SEC;
			walltime = ((double) (endtb.time-begintb.time))+((double)endtb.millitm-(double)begintb.millitm)/1000;
			measurements[j] = time;
			avg += time;
			if (j < NRRUNS - 1)
				wrhhh1d_deinit();
		}
		
		avg /= NRRUNS;
		
		for (j=0; j < NRRUNS; ++j){
			sos += pow(measurements[j] - avg, 2);
		}
		stdev = pow(sos / (NRRUNS - 1), 0.5);
		
		upCI = avg + stdev * t_4 / pow(NRRUNS, 0.5) ;
		dnCI = avg - stdev * t_4 / pow(NRRUNS, 0.5) ;


		free(data);

		printf("%d ips, windowSize = %d, counters=%d, prob = %f, took %fs (%fs-%fs)\n", n, windowSize, counters, insertProb, avg, dnCI, upCI);
		/*
		ans = output(threshold, &m, n);

		printf("%d HHHs\n", m);
		
		deinit();


		if (fp != NULL) {
			fprintf(fp, "%s %d %d %d %d %lf\n", argv[0], n, counters, threshold, m, time);
			for (i = 0; i < m; i++) {
				fprintf(fp, "%d %d.%d.%d.%d %d %d\n",
				ans[i].mask,
				(int)((ans[i].item >> 24) & (LCLitem_t)255),
				(int)((ans[i].item >> 16) & (LCLitem_t)255),
				(int)((ans[i].item >> 8) & (LCLitem_t)255),
				(int)((ans[i].item >> 0) & (LCLitem_t)255),
				ans[i].lower * ( (int) (NUM_MASKS / insertProb)), ans[i].upper * ( (int) (NUM_MASKS / insertProb)));				
			}
			fclose(fp);
		}
		
		epsil = -1;
		for (i = 0; i < m; i++) {
			epsil = dblmainmax(epsil, ((double)(ans[i].upper-ans[i].lower))/n );
		}

		free(ans);

		if (fsummary != NULL) {
			fprintf(fsummary, "check=false algorithm=%-14s nitems=%-10d counters=%-5d threshold=%-9d  outputsize=%-3d time=%lf walltime=%lf epsilon=%lf",
			                   argv[0],        n,           counters,     threshold,      m,              time,  walltime, epsil);
			fprintf(fsummary, "\n");
			fclose(fsummary);       
		}

	return 0;*/
}


