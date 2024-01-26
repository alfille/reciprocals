// Reciprocals
// Find sums of reciprocals tht add up to 1
// use floating point -- so some loss of precision

// Paul H Alfille 2023
// https://github.com/alfille/reciprocals
// MIT licence

// Second version (first version in C) -- directly solve last position
// Third version -- directly solve last 2 positions
// 4th --last 2 doesn't work, only last
// 5th more selective use of W_gcd and use stein's instead
// 6th -- swith to difference rather than sum
// Update -- add timeout and clean up options

// To compile:
// apt install openmpi-bin
// apt install libopenmpi-dev
// mpicc -O3 -o reciprocal_mpi reciprocal_mpi.c -lm

// MPI version
// documentation from OpenMPI https://www.open-mpi.org/

// messages
// To workers:
//   1. Setup
//      Gterms number of terms
//      Gsum target sum
//      Gtimeout timeout length
//   2. Job
//      Presets
//      Preset length
//      Guntil

// To Master
//   Rank
//   Guntil
//   Status
//   Count
//   Range[2]
//   Elapsed


#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <setjmp.h>
#include <math.h>
#include <inttypes.h>
#include <getopt.h>
#include <time.h>
#include <mpi.h>

struct sParameters {
    uint64_t terms ;
    uint32_t sum ;
    uint32_t timeout ;
} xParameters ;
MPI_Datatype Parameters_t ;

struct sJob {
    int      nPresets ;
    uint64_t until ;
    uint64_t presets[0] ; // Gterms, actually
} ;
struct sJob * pJob ;
MPI_Datatype Job_t ;

struct sResponse {
    int      rank ;
    int      status ;
    uint64_t from ;
    uint64_t to ;
    uint64_t count ;
    double_t elapsed ;
} xResponse ;
MPI_Datatype Response_t ;

#define MAXTERMS 1000000

int        Gterms = 4;
uint64_t   Gcounter ;
uint64_t   Gsum = 1 ;
uint64_t   Guntil = 0 ;
struct timespec Gstart ;
int        Gtimeout = 0 ;
uint64_t   Gfrom ;
uint64_t   Gto ;
int        Grank ; // MPI rank 
int        Gsize ; // MPI size
int        Groot ;
int        Gworkers ;
sigjmp_buf Gmark_spot ;
double     Gtime_good ;
double     Gtime_total ;

#define GROOTS 1

uint64_t normalize_threshold = 1000000000000 ;

struct fraction{
    uint64_t val ; // 1/val
    uint64_t num ; // cumulative diff numerator
    uint64_t den ; // cumulative diff denominator
    uint64_t count ;// for this set of terms
} G[ MAXTERMS ] ;

typedef enum { eNo = 0, eYes = 1, eError = 2, eTimeout = 3 } search_more ;

typedef enum { mJob, mResponse } mMessage ;

void B_ParametersStruct(void) {
    // BOTH ROOT and WORKER
    // Setup before number of terms is known
    // Create the message struct as a defined MPI Datatype

    // Setup
    int sParameters_count = 3 ;
    int sParameters_blocklen[] = {1,1,1} ;
    MPI_Aint sParameters_offset[] = {
        offsetof(struct sParameters, terms),
        offsetof(struct sParameters, sum),
        offsetof(struct sParameters, timeout)
    } ;
    MPI_Datatype Parameters_ts[] = { MPI_UINT64_T, MPI_UINT32_T, MPI_UINT32_T } ;
    MPI_Type_create_struct( sParameters_count, sParameters_blocklen, sParameters_offset, Parameters_ts, &Parameters_t ) ;
    MPI_Type_commit( &Parameters_t ) ;
}

void B_JobStruct(void) {
    // BOTH ROOT and WORKER
    // Setup after number of terms is known
    // Create the message struct as a defined MPI Datatype

    // Job 
    pJob = malloc( sizeof( struct sJob ) + Gterms * sizeof( uint64_t ) ) ;
        
    int sJob_count = 3 ;
    int sJob_blocklen[] = {1,1,Gterms-1} ; // note that presets are at most Gterms-1 since last term is pre-determined
    MPI_Aint sJob_offset[] = {
        offsetof(struct sJob, nPresets),
        offsetof(struct sJob, until),
        offsetof(struct sJob, presets)
    } ;
    MPI_Datatype Job_ts[] = { MPI_INT, MPI_UINT64_T, MPI_UINT64_T } ;
    MPI_Type_create_struct( sJob_count, sJob_blocklen, sJob_offset, Job_ts, &Job_t ) ; 
    MPI_Type_commit( &Job_t ) ;
}

void B_ResponseStruct(void) {
    // Response
    int sResponse_count = 6 ;
    int sResponse_blocklen[] = {1,1,1,1,1,1} ;
    MPI_Aint sResponse_offset[] = {
        offsetof(struct sResponse, rank),
        offsetof(struct sResponse, status),
        offsetof(struct sResponse, from),
        offsetof(struct sResponse, to),
        offsetof(struct sResponse, count),
        offsetof(struct sResponse, elapsed)
    } ;
    MPI_Datatype Response_ts[] = { MPI_INT, MPI_INT, MPI_UINT64_T, MPI_UINT64_T, MPI_UINT64_T, MPI_DOUBLE } ;
    MPI_Type_create_struct( sResponse_count, sResponse_blocklen, sResponse_offset, Response_ts, &Response_t ) ;
    MPI_Type_commit( &Response_t ) ;
}    

void B_CommunicationSetup( void ) {
    // BOTH ROOT and WORKER
    // Broadcast parameters to all workers
    B_ParametersStruct() ; // Doesn't require variable sized presets
    B_ResponseStruct() ; // Doesn't require variable sized presets

    // Load struct
    xParameters.terms   = Gterms ;
    xParameters.sum     = Gsum ;
    xParameters.timeout = Gtimeout ;
    
    MPI_Bcast( &xParameters, 1, Parameters_t, Groot, MPI_COMM_WORLD ) ;

    // Unload Struct
    Gterms   = xParameters.terms ;
    Gsum     = xParameters.sum ;
    Gtimeout = xParameters.timeout ;

    //printf("Broadcast rank %d, terms = %d\n", Grank, Gterms) ;
    B_JobStruct() ; // Now know Gterms for presets size
}

search_more W_Lastterm( struct fraction * pfrac_old ) {
    // WORKER ONLY
    // Last fraction -- easier calculation because either it a reciprocal or not. No search required
    // not "summing to one" case
    struct fraction * pfrac_new = pfrac_old + 1 ;
    uint64_t den = pfrac_old->den ;
    uint64_t num = pfrac_old->num ;
    uint64_t init_val = pfrac_old->val + Gsum ;
    uint64_t val = ( den / ( num * Gsum ) ) * Gsum ;
    
    if ( val < init_val ) {
        return eNo ;
    }
    if ( val * num == den ) {
        // solution found
        ++Gcounter ;
        pfrac_new->val = val ;
        for ( int term = 0 ; term < Gterms-2; ++term ) {
            ++G[term].count ;
        }
    }
    return val != init_val ? eYes : eNo ;
}
        
search_more W_Lastterm_for_1( struct fraction * pfrac_old ) {
    // WORKER ONLY
    // Last fraction -- easier calculation because either it a reciprocal or not. No search required
    // Special for "summing to one"
    //printf("late entry index=%" PRIu64 ", val=%" PRIu64 ", num=%" PRIu64 ", den=%" PRIu64 "\n",Gterms-1,pfrac_old->val,pfrac_old->num,pfrac_old->den);
    struct fraction * pfrac_new = pfrac_old + 1 ;
    uint64_t den = pfrac_old->den ;
    uint64_t num = pfrac_old->num ;
    uint64_t init_val = pfrac_old->val + 1 ;
    uint64_t val = den / num ;
    
    if ( val < init_val ) {
        return eNo ;
    }
    if ( val * num == den ) {
        // solution found
        ++Gcounter ;
        pfrac_new->val = val ;
        for ( int term = 0 ; term < Gterms-2; ++term ) {
            ++G[term].count ;
        }
    }
    return val != init_val ? eYes : eNo ; 
}

uint64_t W_gcd( uint64_t a, uint64_t b ) {
    // WORKER ONLY
    // calculate W_gcd
    // use https://www.geeksforgeeks.org/steins-algorithm-for-finding-W_gcd/
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
        
void W_PrintCSV( uint64_t index ) {
    if ( index < Gterms-3 && G[index].count > 0 ) {
        printf( "%g,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 "\n", ((double)G[index].num)/((double)G[index].den), Gterms-index-1, G[index].val, G[index].count, G[index-1].val ) ;
    }
}
        
void W_PrintCSV_TO( uint64_t index ) {
    if ( index < Gterms-3 && G[index].count > 0 ) {
        printf( "%g,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 "\n", ((double)G[index].num)/((double)G[index].den), Gterms-index-1, G[index].val, 10000000, G[index-1].val ) ;
    }
}
        
search_more W_Midterm( uint64_t index, struct fraction * pfrac_old ) {
    // WORKER ONLY
    // Middle fraction -- figure out low start and recursion will signal end
    // Not  "summing to one" case
    //printf("mid entry index=%" PRIu64 ", val=%" PRIu64 ", num=%" PRIu64 ", den=%" PRIu64 "\n",index,pfrac_old->val,pfrac_old->num,pfrac_old->den);
    struct fraction * pfrac_new = pfrac_old + 1 ;
    uint64_t pre_num = pfrac_old->num ;
    uint64_t pre_den = pfrac_old->den ;
    
    if ( pre_den > normalize_threshold ) {
        // calculate W_gcd
        uint64_t a = W_gcd( pre_num, pre_den ) ;
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
        pfrac_new->count = 0 ;

        if ( ((index<Gterms-2)?W_Midterm( index+1, pfrac_new ):W_Lastterm(pfrac_new)) == eNo ) {
            W_PrintCSV( index ) ;
            return val != init_val ? eYes : eNo ;
        }

		W_PrintCSV( index ) ;
        val += Gsum;
    }
}

search_more W_Midterm_for_1( uint64_t index, struct fraction * pfrac_old ) {
    // WORKER ONLY
    // Middle fraction -- figure out low start and recursion will signal end
    // Special for "summing to one"
    // printf("mid entry index=%" PRIu64 ", val=%" PRIu64 ", num=%" PRIu64 ", den=%" PRIu64 "\n",index,pfrac_old->val,pfrac_old->num,pfrac_old->den);
    struct fraction * pfrac_new = pfrac_old + 1 ;
    uint64_t pre_num = pfrac_old->num ;
    uint64_t pre_den = pfrac_old->den ;
    
    if ( pre_den > normalize_threshold ) {
        // calculate W_gcd
        uint64_t a = W_gcd( pre_num, pre_den ) ;
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
        pfrac_new->count = 0 ;

        if ( ((index<Gterms-2)?W_Midterm_for_1( index+1, pfrac_new ):W_Lastterm_for_1(pfrac_new)) == eNo ) {
            W_PrintCSV( index ) ;
            return val != init_val ? eYes : eNo ;
        }

		W_PrintCSV( index ) ;
        val += 1;
    }
}

search_more W_Add_preset( uint64_t index, uint64_t val ) {
    // WORKER ONLY
    // add a preset value for timing
    if ( val % Gsum != 0 ) {
        // bad val -- not divisible by Gsum
        fprintf( stderr, "Error: preset value %" PRIu64 " not divisible by %" PRIu64 "\n", val, Gsum );
        return eError ;
    }
    
    if ( index == 0 ) {
        if ( val < 2 * Gsum ) {
            fprintf( stderr, "Error: first preset value %" PRIu64 " not minimum %" PRIu64 "\n", val, 2 * Gsum );
            return eError ;
        }
        G[index].num = val-1 ;
        G[index].den = val ;
        G[index].val = val ;
    } else {
        if ( val < G[index-1].val + Gsum ) {
            // test that values are increasing
            fprintf( stderr, "Error: preset value %" PRIu64 " not greater than prior preset %" PRIu64 "\n", val, G[index-1].val );
            return eError ;
        }
        G[index].num = G[index-1].num * val ;
        if ( G[index].num <= G[index-1].den ) {
            // test that difference is still positive
            fprintf( stderr, "Error: sum at preset value %" PRIu64 " not less than than 1\n", val );
            return eError ;
        }
        G[index].num -= G[index-1].den ;
        G[index].den = G[index-1].den * val ;
        if ( G[index].den > normalize_threshold ) {
            uint64_t a = W_gcd( G[index].num, G[index].den ) ;
            G[index].num /= a;
            G[index].den /= a;
        }
        G[index].val = val ;
    }
    //printf("Preset add: index=%" PRIu64 ", num=%" PRIu64 ", den=%" PRIu64 ", val=%" PRIu64 "\n",index,G[index].num,G[index].den,G[index].val);
    return eYes ;
}

void W_SendResponse( search_more status ) {
    // WORKER ONLY
    // Clear alarm
    alarm( 0 ) ;

    // calc time
    struct timespec now;
    clock_gettime( CLOCK_THREAD_CPUTIME_ID, &now ) ;
    
    // Full struct
    xResponse.rank    = Grank ;
    xResponse.count   = Gcounter ;
    xResponse.status  = status ;
    xResponse.from    = Gfrom ;
    xResponse.to      = Gto ;
    xResponse.elapsed = (double) (now.tv_sec - Gstart.tv_sec) + 1E-9 * (double) ( now.tv_nsec - Gstart.tv_nsec );
    fprintf(stderr,"Worker send rank %d, counter %" PRIu64 ", status %d, from %" PRIu64 ", to %" PRIu64 ",elapsed %g\n",
        xResponse.rank,xResponse.count,xResponse.status,xResponse.from,xResponse.to,xResponse.elapsed);
    MPI_Send( &xResponse, 1, Response_t, Groot, mResponse, MPI_COMM_WORLD ) ;
}

search_more W_Range_search( int index, uint64_t from, uint64_t to ) {
    // WORKER ONLY
    // now run through last preset level
    for ( uint64_t v = from; v < to + Gsum ; v += Gsum ) {
        W_Add_preset( index , v ) ;
        switch ((Gsum==1) ? W_Midterm_for_1( index+1, &G[index] ) : W_Midterm( index+1, &G[index] ) ) {
            case eYes:
                break ;
            case eNo:
                return eNo ;
            case eError:
                return eError ;
        }
    }
    return eYes ;
}

void W_Timeout_handler( int sig ) {
    // WORKER ONLY
    siglongjmp( Gmark_spot, -1 ) ;
}

int W_GetJob( void ) {
    // WORKER only
    //printf("Worker %d waiting\n",Grank);
    MPI_Recv( pJob, 1, Job_t, MPI_ANY_SOURCE, mJob, MPI_COMM_WORLD, MPI_STATUS_IGNORE ) ;
    
    int nps = pJob->nPresets ;
    // total counter
    Gcounter = 0 ;

    // start of elapsed time
    clock_gettime( CLOCK_THREAD_CPUTIME_ID, &Gstart ) ;

    if ( nps == Gterms ) {
        // Trigger for end of run
        //printf("Worker %d told to close.\n",Grank);
        return 0 ;
    }

    // not death notice
    /*
    printf("Worker %d got job: nPresets %d, \t",Grank,nps);
    for ( int i=0 ; i<nps ; ++i ) {
        printf(" %" PRIu64, pJob->presets[i] );
    }
    printf(" - %" PRIu64 "\n", pJob->until);
    */
    if ( sigsetjmp( Gmark_spot, 1 ) != 0 ) {
        //printf("Worker %d timeout\n",Grank);
		W_PrintCSV_TO( nps ) ;
        W_SendResponse( eTimeout ) ;
        return 1 ;
    }

    // Timer handler
    signal( SIGALRM, W_Timeout_handler);
    alarm( Gtimeout ) ;

    
    if ( nps == 0) { 
        // no presets
        Gfrom = 2 ;
        Gto = Gsum * ( (uint64_t) ( 1.1 + (Gterms) / (exp(Gsum)-1) ) ) ;

        // Solve
        if ( Guntil == 0 ) {
            W_SendResponse( W_Range_search( 0, Gfrom, Gto ) ) ;
        } else {
            W_SendResponse( W_Range_search( 0, Gfrom, Guntil ) ) ;
        }
        return 1 ;
    } else {
        // Add presets first
        if (  nps > Gterms-2 ) {
            // last 2 positions should not be pre set
            // or timed
            nps = Gterms - 2 ;
            alarm(0) ;        int index = -1 ;
        }
        int index = -1 ;
        for ( int i = 0; i < nps; ++i) {
            ++ index ;
            if ( W_Add_preset( index, pJob->presets[i] ) == eError ) {
                W_SendResponse( eError ) ;
                return 1 ;
            }
        }

        // estimate range of next level after presets
        Gfrom = ( G[index].den / ( Gsum * G[index].num)) * Gsum + Gsum ;
        if ( Gfrom < G[index].val + Gsum ) {
            Gfrom = G[index].val + Gsum ;
        }
        Gto = Gsum * ( (uint64_t) ( 1.1 + (Gterms-index-1) / (exp(((double) Gsum * G[index].num)/(G[index].den))-1) ) );

        // Solve
        if ( Guntil < G[index].val ) {
            W_SendResponse( W_Range_search( index, G[index].val, G[index].val ) );
        } else {
            W_SendResponse( W_Range_search( index, G[index].val, Guntil ) ) ;
        }
        return 1 ;
    }
}        

void R_help() {
    // ROOT only
    fprintf(stderr,"Reciprocals -- find sequences of integers where reciprocals sum to 1 (e.g. [2,3,6])\n");
    fprintf(stderr,"\tShows all solution sequences of a given length\n");
    fprintf(stderr,"\tby Paul H Alfille 2023 -- MIT Licence\n");
    fprintf(stderr,"\tSee https://github.com/alfille/reciprocaln");
    fprintf(stderr,"\n");
    fprintf(stderr,"reciprocal [options] [v1, v2, ...]\n");
    fprintf(stderr,"\twhere v1, v2 are prrset terms\n");
    fprintf(stderr,"\nnote that v1, v2 must be distinct, increasing, and less than the target sum\n");
    fprintf(stderr,"Options\n");
    fprintf(stderr,"\t-n%d\t--number\tnumber of terms in the sum (default %d)\n",Gterms,Gterms);
    fprintf(stderr,"\t\tAll terms will be of form 1/v and distinct\n");
    fprintf(stderr,"\t-s%" PRIu64 "\t--sum\ttarget sum (default %" PRIu64 ")\n",Gsum,Gsum);
    fprintf(stderr,"\t-t\t--timelimit \t stop calculation after specified seconds (default none)\n");
    fprintf(stderr,"\t-r\t--range \t increase last preset to this value\n");
    fprintf(stderr,"\t-h\t--R_help\tthis R_help\n");
    fprintf(stderr,"\nSee https://github.com/alfille/reciprocals for full exposition\n\n");
    exit(1);
}

struct option long_options[] =
{
    {"number"   ,   required_argument, 0, 'n'},
    {"sum"      ,   required_argument, 0, 's'},
    {"timelimit",   required_argument, 0, 't'},       
    {"range"    ,   required_argument, 0, 'r'},       
    {"R_help"     ,   no_argument,       0, 'h'},
    {0          ,   0          ,       0,   0}
};

void R_ParseCommandLine( int argc, char * argv[] ) {
    // ROOT only
    // defaults
    Gsum = 1 ;

    // Parse command line
    int c;
    int option_index ;
    while ( (c = getopt_long( argc, argv, "t:r:hs:n:", long_options, &option_index )) != -1 ) {
        //printf("opt=%c, index=%d, val=%s\n",c,option_index, long_options[option_index].name);
        switch (c) {
            case 0:
                break ;
            case 'h':
                R_help();
                break ;
            case 'n':
                Gterms = (uint64_t) atoi(optarg);
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
                R_help() ;
                break ;
            }
    }
        
    // test parameters
    if ( Gterms < 3 ) {
        Gterms = 3 ;
    }
    if ( Gterms > MAXTERMS-1 ) {
        Gterms = MAXTERMS-1 ;
    }
    if ( Gsum < 1 ) {
        Gsum = 1 ;
    }
}

void * vWorkerJob ;
int * pFreeWorker ;
int nFW ;

void R_allocFreeWorker(void) {
    // ROOT only
    nFW = 0 ;
    pFreeWorker = calloc( Gworkers, sizeof( int ) ) ;
    for ( int rank = 0 ; rank < Gworkers ; ++rank ) {
        pFreeWorker[nFW] = rank ;
        ++ nFW ;
    }
}

void R_freeFreeWorker( void ) {
    // ROOT only
    free(pFreeWorker) ;
}
    
void * vScratchJob ;    
void * vRankJobs ;  
void * vWorkQueue ;
int    nWQ ;
#define WORKQUEUE_MAX 1000000

void R_allocJobSpace(void) {
    // ROOT only
    struct wJob {
        int      nPresets ;
        uint64_t until ;
        uint64_t presets[Gterms-1];
    } ;
    struct wJob * pScratch = ( struct wJob *) calloc( 1 + Gworkers + WORKQUEUE_MAX, sizeof( struct wJob ) ) ;
    vScratchJob = pScratch ;
    struct wJob * pRankJobs = pScratch + 1 ;
    vRankJobs = pRankJobs ;
    struct wJob * pWorkQueue = pRankJobs + Gworkers ;
    vWorkQueue = pWorkQueue ;
    nWQ = 0 ;
}

void R_freeJobSpace( void ) {
    // ROOT only
    free(vScratchJob) ;
}

void R_SendRank( int rank ) {
    // ROOT only
    struct wJob {
        int      nPresets ;
        uint64_t until ;
        uint64_t presets[Gterms-1];
    } ;
    struct wJob * pRankJobs = vRankJobs ;
    MPI_Send( pRankJobs + rank, 1, Job_t, rank, mJob, MPI_COMM_WORLD ) ;
    //printf("Root send to rank %d, nPresets=%d\n",rank, pRankJobs[rank].nPresets) ;
}

void R_addFreeWorker( int rank ) {
    // ROOT only
    pFreeWorker[ nFW ] = rank ;
    ++ nFW ;
}

int R_getFreeWorker( void ) {
    // ROOT only
    --nFW ;
    return pFreeWorker[nFW] ;
}

void R_Load_SendRank( int rank, void * vjob ) {
    // ROOT only
    struct wJob {
        int      nPresets ;
        uint64_t until ;
        uint64_t presets[Gterms-1];
    } ;

    struct wJob * pjob = vjob ;
    struct wJob * pRankJobs = vRankJobs ;
    memcpy( pRankJobs + rank, vjob, sizeof( struct wJob ) ) ;
    R_SendRank( rank ) ;
}

void R_addQueue( void * vjob ) {
    //printf("Add to QUEUE\n");
    // ROOT only
    struct wJob {
        int      nPresets ;
        uint64_t until ;
        uint64_t presets[Gterms-1];
    } ;
    
    struct wJob * pjob = vjob ;
    struct wJob * pWorkQueue = vWorkQueue ;
    
    if ( nWQ >= WORKQUEUE_MAX ) {
        fprintf(stderr, "Max out work que -- too many pending jobs.(>%d)\n",WORKQUEUE_MAX);
        exit(1) ;
    }
    memcpy( pWorkQueue + nWQ, vjob, sizeof( struct wJob ) ) ;
    ++ nWQ ; ;
}
    
void R_SendOrQueue( void * vjob ) {
    // ROOT only
    if ( nFW > 0 ) {
        int rank = R_getFreeWorker() ;
        //printf("Root send to worker %d\n",rank);
        R_Load_SendRank( rank, vjob ) ;
    } else {
        //printf("Root send to queue %d\n",nFW);
        R_addQueue( vjob ) ;
    }
}
    
void R_useQueue( int rank ) {
    //printf("Get From QUEUE\n");
    // ROOT only
    struct wJob {
        int      nPresets ;
        uint64_t until ;
        uint64_t presets[Gterms-1];
    } ;
    struct wJob * pWorkQueue = vWorkQueue ;
    -- nWQ ;
    R_Load_SendRank( rank, pWorkQueue + nWQ ) ;
}

void R_SplitJob( int rank ) {
    // ROOT only
    //Use xResponse data too
    struct wJob {
        int      nPresets ;
        uint64_t until ;
        uint64_t presets[Gterms-1];
    } ;
    struct wJob * pRankJobs = vRankJobs ;
    struct wJob * pR = pRankJobs + rank ;
    struct wJob * pScratch = vScratchJob ;
    int nps = pR->nPresets ;
    
    if ( nps > 0 && pR->until != pR->presets[nps-1] ) {
        // This level splittable
        //printf("Root from rank %d, Split THIS level %d\n",rank,nps-1);
        memcpy( pScratch, pR, sizeof( struct wJob ) ) ;
        pScratch->until = pScratch->presets[nps-1] ;
        pR->presets[nps-1] += Gsum ;
        R_SendRank( rank ) ;
    } else {
        // Split next level
        //printf("Root from rank %d, Split NEXT level %d\n",rank,nps);
        ++ pR->nPresets ;
        memcpy( pScratch, pR, sizeof( struct wJob ) ) ;
        pR->presets[nps] = xResponse.from + Gsum ;
        pR->until = xResponse.to ;
        R_SendRank( rank ) ;
        pScratch->presets[nps] = xResponse.from ;
        pScratch->until = xResponse.from ;
    }
    /*
    printf("\tSplit: Same: ");
    for ( int i = 0 ; i < pR->nPresets ; ++i ) {
        printf("%" PRIu64 " ", pR->presets[i] ) ;
    }
    printf(" - %" PRIu64 "\n",pR->until ) ;
    printf("\tSplit: Plus: ");
    for ( int i = 0 ; i < pScratch->nPresets ; ++i ) {
        printf("%" PRIu64 " ", pScratch->presets[i] ) ;
    }
    printf(" - %" PRIu64 "\n",pScratch->until ) ;
    */
    
    R_SendOrQueue( pScratch ) ;
}

void R_CloseAll( void ) {
    // ROOT only
    struct wJob {
        int      nPresets ;
        uint64_t until ;
        uint64_t presets[Gterms-1];
    } ;
    struct wJob * pScratch = vScratchJob ;
    // Close
    pScratch->nPresets = Gterms ;
    for ( int rank=0 ; rank < Gworkers ; ++rank ) {
        //printf("Root close worker %d \n",rank);
        R_Load_SendRank( rank, pScratch ) ;
    }
    R_freeFreeWorker();
    R_freeJobSpace();
}

void R_Process( void ) {
    // ROOT only
    // Structure for Job with presets array specified. 
    // Have to do everything in this routine because C has no nested subroutines. 

    struct wJob {
        int      nPresets ;
        uint64_t until ;
        uint64_t presets[Gterms-1];
    } ;
    
    R_allocJobSpace() ;
    R_allocFreeWorker() ;
    struct wJob * pRankJobs = vRankJobs ;
    struct wJob * pScratch = vScratchJob ;

    // Add Basic start:
    Gfrom = 2 ;
    Gto = Gsum * ( (uint64_t) ( 1.1 + (Gterms) / (exp(Gsum)-1) ) ) ;
    for ( int v = Gto ; v >= Gfrom ; --v ) {
        pScratch->nPresets   = 1 ;
        pScratch->presets[0] = v ;
        pScratch->until      = v ;
        R_SendOrQueue( pScratch ) ;
    }
    
    // Loop through waiting for results
    do {
        MPI_Recv( &xResponse, 1, Response_t, MPI_ANY_SOURCE, mResponse, MPI_COMM_WORLD, MPI_STATUS_IGNORE ) ;
        int rank = xResponse.rank ;
        Gtime_total += xResponse.elapsed ;
        switch( xResponse.status ) {
            case eYes:
            case eNo:
            case eError:
                // Successful search
                Gcounter += xResponse.count ;
                Gtime_good += xResponse.elapsed ;
                fprintf(stderr, "Counter %" PRIu64 ", Workers: %d, Pending Jobs: %d, Time efficiency %4.2f%%\n", Gcounter, Gworkers-nFW, nWQ, 100.* Gtime_good / Gtime_total);
                if ( nWQ == 0 ) {
                    // nothing in work queue to assign, so leave worker unused
                    //printf("Root from rank %d, Add Free Worker\n",rank ) ;
                    R_addFreeWorker( rank ) ;
                } else {
                    // pull a task from the work queue
                    //printf("Root from rank %d, Use Queue %d\n",rank,nWQ ) ;
                    R_useQueue( rank ) ;
                }
                break ;
            case eTimeout:
                // Unsuccessful, need to split
                //printf("Root from rank %d, Timeout -> Split\n",rank ) ;
                R_SplitJob( rank ) ;
                break ;
        }
    } while ( nFW < Gworkers ) ;

    R_CloseAll() ;
}

int main( int argc, char * argv[] ) {
    // BOTH ROOT and WORKER
    MPI_Init( &argc, &argv ) ;

    MPI_Comm_rank( MPI_COMM_WORLD, &Grank ) ;
    MPI_Comm_size( MPI_COMM_WORLD, &Gsize ) ;
    Groot    = Gsize - GROOTS ; // last worker "rank" is root
    Gworkers = Gsize - GROOTS ; // rest are workers (index from 0)

    if ( Grank==Groot ) {
        // MASTER
        R_ParseCommandLine( argc, argv ) ;
        fprintf(stderr,"Find sets of %d unique reciprocals that sum to %" PRIu64 "\n", Gterms, Gsum );
        fprintf(stderr,"MPI size=%d, Workers=%d, Root is %d\n",Gsize,Gworkers,Groot);
   }
    
    B_CommunicationSetup() ; // Send command-line derived parameters to all workers, set up message field sizes
        
    if ( Grank == Groot ) {
        // ROOT PROCESS
        R_Process() ;
        fprintf(stderr,"\n");
        fprintf(stderr,"Solution = %" PRIu64 "\n", Gcounter );
        fprintf(stderr,"\n");
    } else {
        // WORKER PROCESS (in a loop)
        do {
        } while ( W_GetJob() ) ;
    }

    MPI_Finalize() ;

    return 0 ;
}

