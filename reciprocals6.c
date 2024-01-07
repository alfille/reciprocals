// Reciprocals
// Find sums of reciprocals tht add up to 1
// use floating point -- so some loss of precision

// Paul H Alfille 2023
// https://github.com/alfille/reciprocals
// MIT licence

// Second version (first version in C) -- directly solve last position
// Third version -- directly solve last 2 positions
// 4th --last 2 doesn't work, only last
// 5th more selective use of gcd and use stein's instead
// 6th -- swith to difference rather than sum

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <inttypes.h>
#include <getopt.h>
#include <time.h>

#define MAXLENGTH 1000000

int Gshow_counter ;
int Gshow_sequence ;
uint64_t Glength;
uint64_t Gcounter ;
uint64_t Ginteger ;
int Gtimer_mode = 0 ;
struct timespec Gtime ;

uint64_t normalize_threshold = 1000000000000 ;

struct fraction{
    uint64_t val ; // 1/val
    uint64_t num ; // cumulative diff numerator
    uint64_t den ; // cumulative diff denominator
} G[ MAXLENGTH ] ;

typedef enum { no_more=0, yes_more = 1, error_more = 2 } search_more ;

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
    printf("\t\t--t\t --timer \t timer mode -- expects specified values to follow as regular arguemtns\n");
    printf("\t-h\t--help\tthis help\n");
    printf("\nSee https://github.com/alfille/reciprocals for full exposition\n\n");
    exit(1);
}

search_more Latesummer( struct fraction * pfrac_old ) {
    // Last fraction -- easier calculation because either it a reciprocal or not. No search required
    // not "summing to one" case
    struct fraction * pfrac_new = pfrac_old + 1 ;
    uint64_t den = pfrac_old->den ;
    uint64_t num = pfrac_old->num ;
    uint64_t init_val = pfrac_old->val + Ginteger ;
    uint64_t val = ( den / ( num * Ginteger ) ) * Ginteger ;
    
    if ( val < init_val ) {
        return no_more ;
    }
    if ( val * num == den ) {
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
    return val != init_val ? yes_more : no_more ;
}
        
search_more Latesummer_for_1( struct fraction * pfrac_old ) {
    // Last fraction -- easier calculation because either it a reciprocal or not. No search required
    // Special for "summing to one"
    //printf("late entry index=%" PRIu64 ", val=%" PRIu64 ", num=%" PRIu64 ", den=%" PRIu64 "\n",Glength-1,pfrac_old->val,pfrac_old->num,pfrac_old->den);
    struct fraction * pfrac_new = pfrac_old + 1 ;
    uint64_t den = pfrac_old->den ;
    uint64_t num = pfrac_old->num ;
    uint64_t init_val = pfrac_old->val + 1 ;
    uint64_t val = den / num ;
    
    if ( val < init_val ) {
        return no_more ;
    }
    if ( val * num == den ) {
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
    return val != init_val ? yes_more : no_more ; 
}
        
search_more Midsummer( uint64_t index, struct fraction * pfrac_old ) {
    // Middle fraction -- figure out low start and recursion will signal end
    // Not  "summing to one" case
    //printf("mid entry index=%" PRIu64 ", val=%" PRIu64 ", num=%" PRIu64 ", den=%" PRIu64 "\n",index,pfrac_old->val,pfrac_old->num,pfrac_old->den);
    struct fraction * pfrac_new = pfrac_old + 1 ;
    uint64_t pre_num = pfrac_old->num ;
    uint64_t pre_den = pfrac_old->den ;
    
    if ( pre_den > normalize_threshold ) {
        // calculate gdc
        // use https://www.geeksforgeeks.org/steins-algorithm-for-finding-gcd/
        uint64_t a = pre_num ;
        uint64_t b = pre_den ;
        int common2s ;
        for ( common2s = 0 ; ((a | b) & 1 ) == 0; ++common2s ) {
            a >>= 1 ;
            b >>= 1 ;
        }
        while ( (a & 1) == 0 ) {
            a >>= 1 ;
        }
        do {
            while ((b & 1) == 0 ) {
                b >>= 1 ;
            }
            if ( a > b ) {
                uint64_t c = b ;
                b = a - b ;
                a = c;
            } else {
                b -= a ;
            }
        } while ( b != 0 ) ;
        a <<= common2s ;
        // reduce sum
        pre_num = pre_num / a ;
        pre_den = pre_den / a ;
        pfrac_old->num = pre_num ;
        pfrac_old->den = pre_den ;
        //printf("nml entry index=%" PRIu64 ", val=%" PRIu64 ", num=%" PRIu64 ", den=%" PRIu64 "\n",index,pfrac_old->val,pfrac_old->num,pfrac_old->den);
    }
    
    uint64_t init_val = ( pre_den / ( Ginteger * pre_num)) * Ginteger + Ginteger ;

        //printf("inc calc init_val=%" PRIu64 "\n",init_val);
        if ( init_val <= pfrac_old->val ) {
            init_val = pfrac_old->val + Ginteger ;
        }
        //printf("pre calc init_val=%" PRIu64 "\n",init_val);
        uint64_t val = init_val ;
        
    while (1) {
        pfrac_new->num = pre_num * val - pre_den ;
        pfrac_new->den = pre_den * val ;
        pfrac_new->val = val ;

        if ( ((index<Glength-2)?Midsummer( index+1, pfrac_new ):Latesummer(pfrac_new)) == no_more ) {
            return val != init_val ? yes_more : no_more ;
        }

        val += Ginteger;
    }
}

search_more Midsummer_for_1( uint64_t index, struct fraction * pfrac_old ) {
    // Middle fraction -- figure out low start and recursion will signal end
    // Special for "summing to one"
    // printf("mid entry index=%" PRIu64 ", val=%" PRIu64 ", num=%" PRIu64 ", den=%" PRIu64 "\n",index,pfrac_old->val,pfrac_old->num,pfrac_old->den);
    struct fraction * pfrac_new = pfrac_old + 1 ;
    uint64_t pre_num = pfrac_old->num ;
    uint64_t pre_den = pfrac_old->den ;
    
    if ( pre_den > normalize_threshold ) {
        // calculate gdc
        // use https://www.geeksforgeeks.org/steins-algorithm-for-finding-gcd/
        uint64_t a = pre_num ;
        uint64_t b = pre_den ;
        int common2s ;
        for ( common2s = 0 ; ((a | b) & 1 ) == 0; ++common2s ) {
            a >>= 1 ;
            b >>= 1 ;
        }
        while ( (a & 1) == 0 ) {
            a >>= 1 ;
        }
        do {
            while ((b & 1) == 0 ) {
                b >>= 1 ;
            }
            if ( a > b ) {
                uint64_t c = b ;
                b = a - b ;
                a = c;
            } else {
                b -= a ;
            }
        } while ( b != 0 ) ;
        a <<= common2s ;
        // reduce sum
        pre_num = pre_num / a ;
        pre_den = pre_den / a ;
        pfrac_old->num = pre_num ;
        pfrac_old->den = pre_den ;
        //printf("nml entry index=%" PRIu64 ", val=%" PRIu64 ", num=%" PRIu64 ", den=%" PRIu64 "\n",index,pfrac_old->val,pfrac_old->num,pfrac_old->den);
    }
    
    uint64_t init_val = pre_den / pre_num +1 ;
        if ( init_val <= pfrac_old->val ) {
                init_val = pfrac_old->val + 1 ;
        }
        //printf("pre calc init_val=%" PRIu64 "\n",init_val);
        uint64_t val = init_val ;
        
    while (1) {
        pfrac_new->num = pre_num * val - pre_den ;
        pfrac_new->den = pre_den * val ;
        pfrac_new->val = val ;

        if ( ((index<Glength-2)?Midsummer_for_1( index+1, pfrac_new ):Latesummer_for_1(pfrac_new)) == no_more ) {
            return val != init_val ? yes_more : no_more ;
        }

        val += 1;
    }
}

search_more Summer( void ) {
    struct fraction * pfrac = & G[0] ;
    uint64_t val = 2 * Ginteger ; // first value always
    //printf("Glength=%"PRIu64"\n",Glength);
    
    do {
        //printf("top index=%" PRIu64 ", val=%" PRIu64 "\n",0l,val);
        pfrac->num = val-1 ;
        pfrac->den = val ;
        pfrac->val = val ;
        val += Ginteger ;
    } while( ( (Ginteger==1) ? Midsummer_for_1( 1, pfrac ) : Midsummer( 1, pfrac ) ) == yes_more ) ;
    return no_more ;
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
    {"timer"   ,   no_argument,       &Gtimer_mode   , 1},       
    {"help"    ,   no_argument,       0, 'h'},
    {0, 0, 0, 0}
};

void Reset( uint64_t length ) {
    Glength = length ;
    Gcounter = 0 ;
    clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &Gtime ) ;
}

search_more Solve( uint64_t start, uint64_t end ) {
    // loop through all lengths (nmumber of terms in reciprocal)
    search_more ret ;
    for ( uint64_t length=start ; length <= end ; ++ length ) {
        Reset( length ) ;
        //printf("Computing %" PRIu64 "\n",length);
        ret = Summer() ;
        if ( Gshow_counter ) {
            printf("Length= %" PRIu64 ", Count= %" PRIu64 "\n",length,Gcounter);
        }
    }
    return ret ;
}

search_more Add_preset( uint64_t index, uint64_t val ) {
    // add a preset value for timing
    if ( val % Ginteger != 0 ) {
        // bad val -- not divisible by Ginteger
        fprintf( stderr, "Error: preset value %", PRIu64 " not divisible by %" PRIu64 "\n", val, Ginteger );
        return error_more ;
    }
    
    if ( index == 0 ) {
        if ( val < 2 * Ginteger ) {
            fprintf( stderr, "Error: first preset value %", PRIu64 " not minimum %" PRIu64 "\n", val, 2 * Ginteger );
            return error_more ;
        }
        G[index].num = val-1 ;
        G[index].den = val ;
        G[index].val = val ;
    } else {
        if ( val < G[index-1].val + Ginteger ) {
            // test that values are increasing
            fprintf( stderr, "Error: preset value %", PRIu64 " not greater than previous %" PRIu64 "\n", val, G[index-1].val );
            return error_more ;
        }
        G[index].num = G[index-1].num * val ;
        if ( G[index].num < G[index-1].den ) {
            // test that difference is still positive
            fprintf( stderr, "Error: sum at preset value %", PRIu64 " greater than 1\n", val );
            return error_more ;
        }
        G[index].num -= G[index-1].den ;
        G[index].den = G[index-1].den * val ;
        G[index].val = val ;
    }
    return yes_more ;
}

search_more Timer_out( search_more status ) {
    /* For "--timer" mode
     * print 3 comma-separared values
     *   1. time elapsed
     *       seconds, (double)
     *   2. count of solutions
     *       integer (uint64_t)
     *   3. completion mode
     *       string, in quotes 
     *       "yes_more"   -- more solutions possible available incrementing last preset value
     *       "no_more"    -- no more solutions possible available incrementing last preset value
     *       "error_more" -- error in presets -- see stderr
     *
     * Even with error, the 3 values will be shown
     * */

    // calc time
    struct timespec now;
    clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &now ) ;
    double elapsed = now.tv_sec - Gtime.tv_sec + 10E-9 * ( now.tv_nsec - Gtime.tv_nsec );
    
    // print results
    switch ( status ) {
        case error_more:
            printf( "%g, %" PRIu64 ",\"%s\"\n", elapsed, Gcounter, "error_more" ) ;
            break ;
        case yes_more:
            printf( "%g, %" PRIu64 ",\"%s\"\n", elapsed, Gcounter, "yes_more" ) ;
            break ;
        case no_more:
            printf( "%g, %" PRIu64 ",\"%s\"\n", elapsed, Gcounter, "no_more" ) ;
            break ;
    }
    return status ;
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
    int option_index ;
    while ( (c = getopt_long( argc, argv, "ths:e:i:cq", long_options, &option_index )) != -1 ) {
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
            case 't':
                Gtimer_mode = 1;
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

    // now run calculations
    if ( Gtimer_mode == 1 ) {
        // timer mode with preset values
        uint64_t index = 0 ;
        Reset( start ) ;
        if ( optind >= argc ) {
            // no presets given
            return Timer_out( Summer() ) ;
        } else {
            // Add presets first
            for ( int i = optind; i < argc; ++i) {
                ++ index ;
                if ( Add_preset( index-1, atoll(argv[i]) ) == error_more ) {
                    return Timer_out(error_more) ;
                }
            }
            return Timer_out( (Ginteger==1) ? Midsummer_for_1( index, &G[index-1] ) : Midsummer( index, &G[index-1] ) );
        }
    } else {
        // solve directly for a range of sequence lengths
        Solve( start, end ) ;
    }

    return 0 ;
}

