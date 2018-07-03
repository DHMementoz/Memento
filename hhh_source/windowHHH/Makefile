CC=g++ -std=c++11 -ansi -Wall -O2

all: hhh1d hhh2d
	
hhh1d: hhhMain1d.c wrandhhh1D.c BaseWRSS.cpp RSS_CPP.cpp prng.c
	$(CC) hhhMain1d.c wrandhhh1D.c BaseWRSS.cpp RSS_CPP.cpp prng.c -o hhh1d -std=c++11
	
hhh2d: hhhMain2d.c wrandhhh2D.c BaseWRSS_LongId.cpp RSS_CPP_LongId.cpp prng.c
	$(CC) hhhMain2d.c wrandhhh2D.c BaseWRSS_LongId.cpp RSS_CPP_LongId.cpp prng.c -o hhh2d -std=c++11	
	
