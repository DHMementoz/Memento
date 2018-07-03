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


#ifdef RHHH
#include "randhhh1D.h"
#define INCLUDED_SOMETHING
#endif


#ifndef INCLUDED_SOMETHING
#error Algorithm undefined
#endif



double dblmainmax(double a, double b) {return (a >= b ? a : b);}

int main(int argc, char * argv[]) {
		int m; //number of heavy hitters in output
		int counters = 100;
		int threshold = 1000;
		int n = 100000;
		double time, walltime;
		double epsil;
		HeavyHitter * ans;
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
		FILE * fp = NULL; //the file for the output
		FILE * fsummary = NULL;
		if (getenv("SUMMARYFILE") != NULL) fsummary = fopen(getenv("SUMMARYFILE"), "a");

		if (argc > 1) n = atoi(argv[1]);
		if (argc > 2) counters = atoi(argv[2]);
		if (argc > 3) threshold = atoi(argv[3]);
		if (argc > 4) fp = fopen(argv[4], "w");

		if(n/counters >= threshold) {
			printf("Unacceptable parameters: eps*n >= theshold\n");
			return 0;
		}

		data = (unsigned int *) malloc(sizeof(unsigned int) * n);

		for (i = 0; i < n; i++) {
			scanf("%d%d%d%d", &w, &x, &y, &z);
			ip = (unsigned int)256*((unsigned int)256*((unsigned int)256*w + x) + y) + z;
			data[i] = ip;
		}

		for (j=0; j < NRRUNS; ++j){
			init_HHH1D((double)1/(double)counters, insertProb);
			begint = clock();
			ftime(&begintb);
			for (i = 0; i < n; i++){
				update_hhh1d(data[i], 1);
				printf("IP %u.%u.%u.%u OCUUR %u\n", (data[i]>>24)&0x000000FF, 
											 (data[i]>>16)&0x000000FF, 
											 (data[i]>>8)&0x000000FF, 
											  data[i]&0x000000FF, pointQuery_hhh1d(data[i]&0xFFFFFF00));
			}
			endt = clock();
			ftime(&endtb);

			time = ((double)(endt-begint))/CLK_PER_SEC;
			walltime = ((double) (endtb.time-begintb.time))+((double)endtb.millitm-(double)begintb.millitm)/1000;
			measurements[j] = time;
			avg += time;
			if (j < NRRUNS - 1)
				deinit_hhh1d();
		}
		
		avg /= NRRUNS;
		
		for (j=0; j < NRRUNS; ++j){
			sos += pow(measurements[j] - avg, 2);
		}
		stdev = pow(sos / (NRRUNS - 1), 0.5);
		
		upCI = avg + stdev * t_4 / pow(NRRUNS, 0.5) ;
		dnCI = avg - stdev * t_4 / pow(NRRUNS, 0.5) ;


		free(data);

		printf("%d ips took %fs (%fs-%fs) ", n, avg, dnCI, upCI);

		ans = output(threshold, &m, n);

		printf("%d HHHs\n", m);
		
		deinit_hhh1d();


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

	return 0;
}


