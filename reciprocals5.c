// Reciprocals
// Find sums of reciprocals tht add up to 1
// use floating point -- so some loss of precision

// Paul H Alfille 2023
// https://github.com/alfille/reciprocals
// MIT licence

// Second version (first version in C) -- directly solve last position
// Third version -- directly solve last 2 positions

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <inttypes.h>
#include <getopt.h>

#define MAXLENGTH 1000

int Gshow_counter ;
int Gshow_sequence ;
uint64_t Glength;
uint64_t Gcounter ;
uint64_t Ginteger ;

uint64_t normalize_threshold = 1000000000000 ;

struct fraction{
	uint64_t val ; // 1/val
	uint64_t num ; // cumulative sum numerator
	uint64_t den ; // cumulative sum denominator
} G[ MAXLENGTH ] ;

void help() {
    printf("Reciprocals -- find sequences of integers where reciprocals sum to 1 (e.g. [2,3,6])\n");
    printf("\tShows all solution sequences of a given length\n");
    printf("\tby Paul H Alfille 2023 -- MIT Licence\n\n");
    printf("Options\n");
    printf("\t-s3\t--start\tstarting length (default 3)\n");
    printf("\t-e3\t--end\tending length (default same as start)\n");
    printf("\t-i1\t--sum\tinteger to sum to (default 1)\n");
    printf("\t-c\t\ttoggle show counts (default on)\n");
    printf("\t\t--count\tshow counts YES\n");
    printf("\t\t--no_count\tshow counts NO\n");
    printf("\t-q\t\ttoggle show full sequences (default off)\n"); 
    printf("\t\t--seq\tshow sequences YES\n");
    printf("\t\t--no_seq\tshow sequences NO\n");
    printf("\t-h\t--help\tthis help\n");
    printf("\nSee https://github.com/alfille/reciprocals for full exposition\n\n");
    exit(1);
}

int latesummer( uint64_t index, struct fraction * pfrac_old ) {
	struct fraction * pfrac_new = pfrac_old + 1 ;
    uint64_t den = pfrac_old->den ;
    uint64_t diff = den - pfrac_old->num ;
    uint32_t val = den / diff ;
    
    if ( val < pfrac_old->val + Ginteger ) {
		return 0 ;
	}
	if ( den % diff != 0 ) {
		return 1 ;
	}
	if ( val % Ginteger == 0 ) {
        // solution found
        ++Gcounter ;
        pfrac_new->val = val ;
        if (Gshow_sequence) {
            int i ;
            printf("[ ");
            for ( i=0 ; i<Glength ; ++i ) {
                printf("%" PRIu64 " ",G[i].val);
            }
            printf("]\n");
        }
    }
    return 1 ; 
}
        
int latesummer1( uint64_t index, struct fraction * pfrac_old ) {
	struct fraction * pfrac_new = pfrac_old + 1 ;
    uint64_t den = pfrac_old->den ;
    uint64_t diff = den - pfrac_old->num ;
    uint32_t val = den / diff ;
    
    if ( val < pfrac_old->val + 1 ) {
		return 0 ;
	}
	if ( den % diff == 0 ) {
		// solution found
		++Gcounter ;
		pfrac_new->val = val ;
		if (Gshow_sequence) {
			int i ;
			printf("[ ");
			for ( i=0 ; i<Glength ; ++i ) {
				printf("%" PRIu64 " ",G[i].val);
			}
			printf("]\n");
		}
	}
    return 1 ; 
}
        
int midsummer( uint64_t index, struct fraction * pfrac_old ) {
	//printf("mid entry index=%" PRIu64 ", val=%" PRIu64 ", num=%" PRIu64 ", den=%" PRIu64 "\n",index,pfrac_old->val,pfrac_old->num,pfrac_old->den);
    if ( index == Glength-1 ) {
        return latesummer( index, pfrac_old ) ;
    }
    struct fraction * pfrac_new = pfrac_old + 1 ;
    uint64_t pre_num = pfrac_old->num ;
    uint64_t pre_den = pfrac_old->den ;
    
    if ( pre_den > normalize_threshold ) {
		// calculate gdc
		uint64_t a = pre_num ;
		uint64_t g = pre_den ;
		while ( a != 0 ) {
			uint64_t c = a ;
			a = g % a ;
			g = c ;
		}
		// reduce sum
		pre_num /= g ;
		pre_den /= g ;
		pfrac_old->num = pre_num ;
		pfrac_old->den = pre_den ;
	}
    
    uint64_t init_val = pre_den / ( pre_den - pre_num ) ;
	//printf("raw calc init_val=%" PRIu64 "\n",init_val);
    if ( Ginteger == 1 ) {
		init_val += 1 ;
	} else {
		init_val += Ginteger - init_val % Ginteger ;
	}
	//printf("inc calc init_val=%" PRIu64 "\n",init_val);
	if ( init_val <= pfrac_old->val ) {
		init_val = pfrac_old->val + Ginteger ;
	}
	//printf("pre calc init_val=%" PRIu64 "\n",init_val);
	uint64_t val = init_val ;
	
    while (1) {
		pfrac_new->num = pre_num * val + pre_den ;
		pfrac_new->den = pre_den * val ;
		pfrac_new->val = val ;

		if ( ! midsummer( index+1, pfrac_new ) ) {
			return val != init_val ;
		}

        val += Ginteger;
    }
}

int midsummer1( uint64_t index, struct fraction * pfrac_old ) {
	// Ginteger==1 case
	//printf("mid entry index=%" PRIu64 ", val=%" PRIu64 ", num=%" PRIu64 ", den=%" PRIu64 "\n",index,pfrac_old->val,pfrac_old->num,pfrac_old->den);
    if ( index == Glength-1 ) {
        return latesummer1( index, pfrac_old ) ;
    }
    struct fraction * pfrac_new = pfrac_old + 1 ;
    uint64_t pre_num = pfrac_old->num ;
    uint64_t pre_den = pfrac_old->den ;
    
    if ( pre_den > normalize_threshold ) {
		// calculate gdc
		uint64_t a = pre_num ;
		uint64_t g = pre_den ;
		while ( a != 0 ) {
			uint64_t c = a ;
			a = g % a ;
			g = c ;
		}
		// reduce sum
		pre_num /= g ;
		pre_den /= g ;
		pfrac_old->num = pre_num ;
		pfrac_old->den = pre_den ;
	}
    
    uint64_t init_val = pre_den / ( pre_den - pre_num ) +1 ;
	if ( init_val <= pfrac_old->val ) {
		init_val = pfrac_old->val + 1 ;
	}
	//printf("pre calc init_val=%" PRIu64 "\n",init_val);
	uint64_t val = init_val ;
	
    while (1) {
		pfrac_new->num = pre_num * val + pre_den ;
		pfrac_new->den = pre_den * val ;
		pfrac_new->val = val ;

		if ( ! midsummer1( index+1, pfrac_new ) ) {
			return val != init_val ;
		}

        val += 1;
    }
}

void summer( void ) {
	struct fraction * pfrac = & G[0] ;
    uint64_t val = 2 * Ginteger ; // first value always
    //printf("Glength=%"PRIu64"\n",Glength);
    do {
        printf("top index=%" PRIu64 ", val=%" PRIu64 "\n",0l,val);
        pfrac->num = 1 ;
        pfrac->den = val ;
        pfrac->val = val ;
        val += Ginteger ;
    } while( (Ginteger==1) ? midsummer1( 1, pfrac ) : midsummer( 1, pfrac ) ) ;
}

struct option long_options[] =
{
    {"count"   ,   no_argument,       &Gshow_counter,  1},
    {"no_count",   no_argument,       &Gshow_counter,  0},
    {"seq"     ,   no_argument,       &Gshow_sequence, 1},
    {"no_seq"  ,   no_argument,       &Gshow_sequence, 0},
    {"start"   ,   required_argument, 0, 's'},
    {"end"     ,   required_argument, 0, 'e'},
    {"sum"     ,   required_argument, 0, 'i'},
    {"help"    ,   no_argument,       0, 'h'},
    {0, 0, 0, 0}
};

int main( int argc, char * argv[] ) {
    uint64_t start = 3;
    uint64_t end = 3;
    
    // defaults
    Gshow_counter = 0 ;
    Gshow_sequence = 0 ;
    Ginteger = 1 ;

    // Parse command line
    int c;
    int option_index ;
    while ( (c = getopt_long( argc, argv, "hs:e:i:cq", long_options, &option_index )) != -1 ) {
        //printf("opt=%d, index=%d, val=%s\n",c,option_index, long_options[option_index].name);
        switch (c) {
            case 0:
                break ;
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

