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
// Update -- add timeout and clean up options

// To compile:
// gcc -O3 -o reciprocal reciprocal.c -lm

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <math.h>
#include <inttypes.h>
#include <getopt.h>
#include <time.h>

#define MAXLENGTH 1000000

int Gshow_sequence ;
int Glength = 4;
uint64_t Gcounter ;
uint64_t Gsum = 1 ;
uint64_t Guntil = 0 ;
struct timespec Gtime ;
int Gtimeout = 0 ;
uint64_t Gfrom ;
uint64_t Gto ;

uint64_t normalize_threshold = 1000000000000 ;

struct fraction{
    uint64_t val ; // 1/val
    uint64_t num ; // cumulative diff numerator
    uint64_t den ; // cumulative diff denominator
} G[ MAXLENGTH ] ;

typedef enum { no_more=0, yes_more = 1, error_more = 2 } search_more ;

search_more Lastterm( struct fraction * pfrac_old ) {
    // Last fraction -- easier calculation because either it a reciprocal or not. No search required
    // not "summing to one" case
    struct fraction * pfrac_new = pfrac_old + 1 ;
    uint64_t den = pfrac_old->den ;
    uint64_t num = pfrac_old->num ;
    uint64_t init_val = pfrac_old->val + Gsum ;
    uint64_t val = ( den / ( num * Gsum ) ) * Gsum ;
    
    if ( val < init_val ) {
        return no_more ;
    }
    if ( val * num == den ) {
        // solution found
        ++Gcounter ;
        pfrac_new->val = val ;
        if (Gshow_sequence) {
            printf("[ ");
            for ( int i=0 ; i<Glength ; ++i ) {
                printf("%" PRIu64 " ",G[i].val);
            }
            printf("]\n");
        }
    }
    return val != init_val ? yes_more : no_more ;
}
        
search_more Lastterm_for_1( struct fraction * pfrac_old ) {
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

uint64_t gcd( uint64_t a, uint64_t b ) {
    // calculate gcd
    // use https://www.geeksforgeeks.org/steins-algorithm-for-finding-gcd/
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
    return a << common2s ;
}
        
search_more Midterm( uint64_t index, struct fraction * pfrac_old ) {
    // Middle fraction -- figure out low start and recursion will signal end
    // Not  "summing to one" case
    //printf("mid entry index=%" PRIu64 ", val=%" PRIu64 ", num=%" PRIu64 ", den=%" PRIu64 "\n",index,pfrac_old->val,pfrac_old->num,pfrac_old->den);
    struct fraction * pfrac_new = pfrac_old + 1 ;
    uint64_t pre_num = pfrac_old->num ;
    uint64_t pre_den = pfrac_old->den ;
    
    if ( pre_den > normalize_threshold ) {
        // calculate gcd
        uint64_t a = gcd( pre_num, pre_den ) ;
        // reduce sum
        pre_num = pre_num / a ;
        pre_den = pre_den / a ;
        pfrac_old->num = pre_num ;
        pfrac_old->den = pre_den ;
        //printf("nml entry index=%" PRIu64 ", val=%" PRIu64 ", num=%" PRIu64 ", den=%" PRIu64 "\n",index,pfrac_old->val,pfrac_old->num,pfrac_old->den);
    }
    
    uint64_t init_val = ( pre_den / ( Gsum * pre_num)) * Gsum + Gsum ;

        //printf("inc calc init_val=%" PRIu64 "\n",init_val);
        if ( init_val <= pfrac_old->val ) {
            init_val = pfrac_old->val + Gsum ;
        }
        //printf("pre calc init_val=%" PRIu64 "\n",init_val);
        uint64_t val = init_val ;
        
    while (1) {
        pfrac_new->num = pre_num * val - pre_den ;
        pfrac_new->den = pre_den * val ;
        pfrac_new->val = val ;

        if ( ((index<Glength-2)?Midterm( index+1, pfrac_new ):Lastterm(pfrac_new)) == no_more ) {
            return val != init_val ? yes_more : no_more ;
        }

        val += Gsum;
    }
}

search_more Midterm_for_1( uint64_t index, struct fraction * pfrac_old ) {
    // Middle fraction -- figure out low start and recursion will signal end
    // Special for "summing to one"
    // printf("mid entry index=%" PRIu64 ", val=%" PRIu64 ", num=%" PRIu64 ", den=%" PRIu64 "\n",index,pfrac_old->val,pfrac_old->num,pfrac_old->den);
    struct fraction * pfrac_new = pfrac_old + 1 ;
    uint64_t pre_num = pfrac_old->num ;
    uint64_t pre_den = pfrac_old->den ;
    
    if ( pre_den > normalize_threshold ) {
        // calculate gcd
        uint64_t a = gcd( pre_num, pre_den ) ;
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

        if ( ((index<Glength-2)?Midterm_for_1( index+1, pfrac_new ):Lastterm_for_1(pfrac_new)) == no_more ) {
            return val != init_val ? yes_more : no_more ;
        }

        val += 1;
    }
}

search_more Add_preset( uint64_t index, uint64_t val ) {
    // add a preset value for timing
    if ( val % Gsum != 0 ) {
        // bad val -- not divisible by Gsum
        fprintf( stderr, "Error: preset value %" PRIu64 " not divisible by %" PRIu64 "\n", val, Gsum );
        return error_more ;
    }
    
    if ( index == 0 ) {
        if ( val < 2 * Gsum ) {
            fprintf( stderr, "Error: first preset value %" PRIu64 " not minimum %" PRIu64 "\n", val, 2 * Gsum );
            return error_more ;
        }
        G[index].num = val-1 ;
        G[index].den = val ;
        G[index].val = val ;
    } else {
        if ( val < G[index-1].val + Gsum ) {
            // test that values are increasing
            fprintf( stderr, "Error: preset value %" PRIu64 " not greater than prior preset %" PRIu64 "\n", val, G[index-1].val );
            return error_more ;
        }
        G[index].num = G[index-1].num * val ;
        if ( G[index].num <= G[index-1].den ) {
            // test that difference is still positive
            fprintf( stderr, "Error: sum at preset value %" PRIu64 " not less than than 1\n", val );
            return error_more ;
        }
        G[index].num -= G[index-1].den ;
        G[index].den = G[index-1].den * val ;
        if ( G[index].den > normalize_threshold ) {
            uint64_t a = gcd( G[index].num, G[index].den ) ;
            G[index].num /= a;
            G[index].den /= a;
        }
        G[index].val = val ;
    }
    //printf("Preset add: index=%" PRIu64 ", num=%" PRIu64 ", den=%" PRIu64 ", val=%" PRIu64 "\n",index,G[index].num,G[index].den,G[index].val);
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

search_more Range_search( int index, uint64_t from, uint64_t to ) {
    // now run through laast preset level
    for ( uint64_t v = from; v < to + Gsum ; v += Gsum ) {
        Add_preset( index , v ) ;
        switch ((Gsum==1) ? Midterm_for_1( index+1, &G[index] ) : Midterm( index+1, &G[index] ) ) {
            case yes_more:
                break ;
            case no_more:
                return Timer_out( no_more ) ;
            case error_more:
                return Timer_out( error_more ) ;
        }
    }
    return Timer_out(yes_more) ;
}

void Range_out( uint64_t from, uint64_t to ) {
    //printf("%" PRIu64 ", %" PRIu64 "\n" , from, to );
    if ( (from==0) || (from>to) ) {
        to = 0 ;
        from = 0 ;
    } 
    printf("%" PRIu64 ", %" PRIu64 "\n" , from, to );
}

void Timeout_handler( int sig ) {
    printf("%" PRIu64 ", %" PRIu64 ", timeout\n",Gfrom, Gto ) ;
    exit(1);
}

void help() {
    printf("Reciprocals -- find sequences of integers where reciprocals sum to 1 (e.g. [2,3,6])\n");
    printf("\tShows all solution sequences of a given length\n");
    printf("\tby Paul H Alfille 2023 -- MIT Licence\n");
    printf("\tSee https://github.com/alfille/reciprocaln");
    printf("\n");
    printf("reciprocal [options] [v1, v2, ...]\n");
    printf("\twhere v1, v2 are prrset terms\n");
    printf("\nnote that v1, v2 must be distinct, increasing, and less than the target sum\n");
    printf("Options\n");
    printf("\t-n%d\t--number\tnumber of terms in the sum (default %d)\n",Glength,Glength);
    printf("\t\tAll terms will be of form 1/v and distinct\n");
    printf("\t-s%d\t--sum\ttarget sum (default %d)\n",Gsum,Gsum);
    printf("\t-q\t--seq\tshow full sequences (default off)\n"); 
    printf("\t-t\t--timelimit \t stop calculation after specified seconds (default none)\n");
    printf("\t-r\t--range \t increase last preset to this value\n");
    printf("\t-h\t--help\tthis help\n");
    printf("\nSee https://github.com/alfille/reciprocals for full exposition\n\n");
    exit(1);
}

struct option long_options[] =
{
    {"seq"      ,   no_argument,       &Gshow_sequence, 1},
    {"number"   ,   required_argument, 0, 'n'},
    {"sum"      ,   required_argument, 0, 's'},
    {"timelimit",   required_argument, 0, 't'},       
    {"range"    ,   required_argument, 0, 'r'},       
    {"help"     ,   no_argument,       0, 'h'},
    {0          ,   0          ,       0,   0}
};

int main( int argc, char * argv[] ) {
    
    // defaults
    Gshow_sequence = 0 ;
    Gsum = 1 ;

    // Parse command line
    int c;
    int option_index ;
    while ( (c = getopt_long( argc, argv, "t:r:hs:n:q", long_options, &option_index )) != -1 ) {
        //printf("opt=%c, index=%d, val=%s\n",c,option_index, long_options[option_index].name);
        switch (c) {
            case 0:
                break ;
            case 'h':
                help();
                break ;
            case 'q':
                Gshow_sequence = 1 ;
                break ;
            case 'n':
                Glength = (uint64_t) atoi(optarg);
                break ;
            case 'r':
                Guntil = (uint64_t) atoi(optarg);
                break ;
            case 's':
                Gsum = (uint64_t) atoi(optarg);
                break ;
            case 't':
                Gtimeout = atoi(optarg);
                break ;
            default:
                help() ;
                break ;
            }
    }
        
    // test parameters
    if ( Glength < 3 ) {
        Glength = 3 ;
    }
    if ( Glength > MAXLENGTH-1 ) {
        Glength = MAXLENGTH-1 ;
    }
    if ( Gsum < 1 ) {
        Gsum = 1 ;
    }

    printf("Find sets of %d unique reciprocals that sum to %" PRIu64 "\n", Glength, Gsum );

    // Timer handler
    signal( SIGALRM, Timeout_handler);
    if ( Gtimeout > 0 ) {
        alarm( Gtimeout ) ;
    }

    // total timer
    Gcounter = 0 ;

    // start of elapsed time
    clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &Gtime ) ;

    int nPresets = argc - optind ;
    if (nPresets <= 0) { 
        // no presets
        Gfrom = 2 ;
        Gto = Gsum * ( (uint64_t) ( 1.1 + (Glength) / (exp(Gsum)-1) ) ) ;

        // Solve
        if ( Guntil == 0 ) {
            Range_search( 0, Gfrom, Gto ) ;
        } else {
            Range_search( 0, Gfrom, Guntil ) ;
        }
        Timer_out(yes_more) ;

    } else {
        // Add presets first
        int index = -1 ;
        for ( int i = 0; i < nPresets; ++i) {
            ++ index ;
            if ( index == Glength-2 ) {
                --index ;
                fprintf(stderr, "Too many preset values -- will only use first %" PRIu64 "\n",Glength-2);
                Guntil = 0 ;
            }
            if ( Add_preset( index, atoll(argv[i]) ) == error_more ) {
                return Timer_out(error_more) ;
            }
        }

        // estimage range of next level after presets
        Gfrom = ( G[index].den / ( Gsum * G[index].num)) * Gsum + Gsum ;
        if ( Gfrom < G[index].val + Gsum ) {
            Gfrom = G[index].val + Gsum ;
        }
        Gto = Gsum * ( (uint64_t) ( 1.1 + (Glength-index-1) / (exp(((double) Gsum * G[index].num)/(G[index].den))-1) ) );

        // Solve
        if ( Guntil < G[index].val ) {
            Range_search( index, G[index].val, G[index].val ) ;
        } else {
            Range_search( index, G[index].val, Guntil ) ;
        }
        Timer_out(yes_more) ;
    }

    return 0 ;
}

