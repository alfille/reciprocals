// Reciprocals
// Find sums of reciprocals tht add up to 1
// use floating point -- so some loss of precision

// Paul H Alfille 2023
// https://github.com/alfille/reciprocals
// MIT licence

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <inttypes.h>

#define MAXLENGTH 1000

int Gshow_counter ;
int Gshow_sequence ;
uint64_t Glength;
uint64_t Gcounter ;
uint64_t Ginteger ;

uint64_t Gval[MAXLENGTH];
uint64_t Gnum[MAXLENGTH]; // numerator
uint64_t Gden[MAXLENGTH]; // denominator

void help() {
	printf("Reciprocals -- find sequences of integers where reciprocals sum to 1 (e.g. [2,3,6])\n");
	printf("\tShows all solution sequences of a given length\n");
	printf("\tby Paul H Alfille 2023 -- MIT Licence\n\n");
	printf("Options\n");
	printf("\t-s3\tstarting length (default 3)\n");
	printf("\t-e3\tending length (default same as start)\n");
	printf("\t-i1\tinteger to sum to (default 1)\n");
	printf("\t-c\ttoggle show counts (default on)\n");
	printf("\t-q\ttoggle show full sequences (default off)\n"); 
	printf("\t-h\tthis help\n");
	printf("\nSee https://github.com/alfille/reciprocals for full exposition\n\n");
	exit(1);
}

int endsummer( uint64_t index, uint64_t init_val ) {
	uint64_t num = Gnum[Glength-2] ;
	uint64_t den = Gden[Glength-2] ;
	if ( (num + 1 == den) && (den >= init_val) && (den % Ginteger == 0) ) {
		// solution found
		++Gcounter ;
		Gval[Glength-1] = den ;
		if (Gshow_sequence) {
			int i ;
			printf("[ ");
			for ( i=0 ; i<Glength ; ++i ) {
				printf("%" PRIu64 " ",Gval[i]);
			}
			printf("]\n");
		}
		return den != init_val ;
	}
	// too big or too small
	return den >= (den-num)*init_val ; 
}
		
int midsummer( uint64_t index, uint64_t init_val ) {
	if ( index == Glength-1 ) {
		return endsummer( index, init_val ) ;
	}
	uint64_t val = init_val ;
	uint64_t pre_num = Gnum[index-1] ;
	uint64_t pre_den = Gden[index-1] ;
	while (1) {
		//printf("mid index=%" PRIu64 ", val=%" PRIu64 "\n",index,val);
		// calculate sum -- raw rational
		uint64_t num = pre_num*val+pre_den ;
		uint64_t den = pre_den*val ;
		if ( num < den ) {
			// calculate gdc
			uint64_t a = num ;
			uint64_t g = den ;
			while ( a != 0 ) {
				uint64_t c = a ;
				a = g % a ;
				g = c ;
			}
			// reduce sum
			Gnum[index] = num / g ;
			Gden[index] = den / g ;
			Gval[index] = val ;
			if ( ! midsummer( index+1, val+Ginteger ) ) {
				return val != init_val ;
			}
		}
		val += Ginteger;
	}
}

void summer( void ) {
	uint64_t val = 2 * Ginteger ; // first value always
	do {
		//printf("top index=%" PRIu64 ", val=%" PRIu64 "\n",0l,val);
		Gval[0] = val ;
		Gnum[0] = 1 ;
		Gden[0] = val ;
		val += Ginteger ;
	} while( midsummer( 1, val ) ) ;
}

int main( int argc, char * argv[] ) {
	uint64_t start = 3;
	uint64_t end = 3;
	
	// defaults
	Gshow_counter = 0 ;
	Gshow_sequence = 0 ;
	Ginteger = 1 ;

	// Parse command line
    int c;
    while ( (c = getopt( argc, argv, "hs:e:i:cq" )) != -1 ) {
		switch (c) {
			case 'h':
				help();
				break ;
			case 'c':
				Gshow_counter ^= 1 ;
				break ;
			case 'q':
				Gshow_sequence ^= 1 ;
				break ;
			case 's':
				start = (uint64_t) atoi(optarg);
				break ;
			case 'e':
				end = (uint64_t) atoi(optarg);
				break ;
			case 'i':
				Ginteger = (uint64_t) atoi(optarg);
				break ;
			default:
				help() ;
				break ;
			}
		}
	
	
	// test parameters
	if ( start < 3 ) {
		start = 3 ;
	}
	if ( start > MAXLENGTH-1 ) {
		start = MAXLENGTH-1 ;
	}
	if ( end < start ) {
		end = start ;
	}
	if ( end > MAXLENGTH-1 ) {
		end = MAXLENGTH-1 ;
	}
	if ( Gshow_counter==0 && Gshow_sequence==0 ) {
		Gshow_counter = 1 ;
	}
	if ( Ginteger < 1 ) {
		Ginteger = 1 ;
	}	
	
	// loop through
	uint64_t length ;
	for ( length=start ; length <= end ; ++ length ) {
		Glength = length ;
		Gcounter = 0 ;
		//printf("Computing %" PRIu64 "\n",length);
		summer() ;
		if ( Gshow_counter ) {
			printf("Length= %" PRIu64 ", Count= %" PRIu64 "\n",length,Gcounter);
		}
	}
	return 0 ;
}

