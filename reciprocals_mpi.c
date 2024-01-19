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

struct sSetup {
    uint64_t terms ;
    uint32_t sum ;
    uint32_t timeout ;
} xSetup ;
MPI_Datatype Setup_t ;

struct sJob {
    uint32_t nPresets ;
    uint64_t until ;
    uint64_t presets[0] ; // Gterms, actually
} ;
struct sJob * pJob ;
MPI_Datatype Job_t ;

struct sResponse {
    int      rank ;
    uint32_t status ;
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
struct timespec Gtime ;
int        Gtimeout = 0 ;
uint64_t   Gfrom ;
uint64_t   Gto ;
int        Grank ; // MPI rank 
int        Gsize ; // MPI size
int        Groot ;
int        Gworkers ;
sigjmp_buf Gmark_spot ;

#define GROOTS 1

uint64_t normalize_threshold = 1000000000000 ;

struct fraction{
    uint64_t val ; // 1/val
    uint64_t num ; // cumulative diff numerator
    uint64_t den ; // cumulative diff denominator
} G[ MAXTERMS ] ;

typedef enum { eNo = 0, eYes = 1, eError = 2, eTimeout = 3 } search_more ;

typedef enum { mJob, mResponse } mMessage ;

void CommunicationSetupPre(void) {
    // Setup before number of terms is known
    // Create the message struct as a defined MPI Datatype

    // Setup
    int sSetup_count = 3 ;
    int sSetup_blocklen[] = {1,1,1} ;
    MPI_Aint sSetup_offset[] = {
        offsetof(struct sSetup, terms),
        offsetof(struct sSetup, sum),
        offsetof(struct sSetup, timeout)
    } ;
    MPI_Datatype Setup_ts[] = { MPI_UINT64_T, MPI_UINT32_T, MPI_UINT32_T } ;
    MPI_Type_create_struct( sSetup_count, sSetup_blocklen, sSetup_offset, Setup_ts, &Setup_t ) ;
    MPI_Type_commit( &Setup_t ) ;
}

void CommunicationSetupPost(void) {
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
    MPI_Datatype Job_ts[] = { MPI_UINT32_T, MPI_UINT64_T, MPI_UINT64_T } ;
    MPI_Type_create_struct( sJob_count, sJob_blocklen, sJob_offset, Job_ts, &Job_t ) ; 
    MPI_Type_commit( &Job_t ) ;

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
    MPI_Datatype Response_ts[] = { MPI_INT, MPI_UINT32_T, MPI_UINT64_T, MPI_UINT64_T, MPI_UINT64_T, MPI_DOUBLE } ;
    MPI_Type_create_struct( sResponse_count, sResponse_blocklen, sResponse_offset, Response_ts, &Response_t ) ;
    MPI_Type_commit( &Response_t ) ;
}    

search_more Lastterm( struct fraction * pfrac_old ) {
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
    }
    return val != init_val ? eYes : eNo ;
}
        
search_more Lastterm_for_1( struct fraction * pfrac_old ) {
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
    }
    return val != init_val ? eYes : eNo ; 
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

        if ( ((index<Gterms-2)?Midterm( index+1, pfrac_new ):Lastterm(pfrac_new)) == eNo ) {
            return val != init_val ? eYes : eNo ;
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

        if ( ((index<Gterms-2)?Midterm_for_1( index+1, pfrac_new ):Lastterm_for_1(pfrac_new)) == eNo ) {
            return val != init_val ? eYes : eNo ;
        }

        val += 1;
    }
}

search_more Add_preset( uint64_t index, uint64_t val ) {
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
            uint64_t a = gcd( G[index].num, G[index].den ) ;
            G[index].num /= a;
            G[index].den /= a;
        }
        G[index].val = val ;
    }
    //printf("Preset add: index=%" PRIu64 ", num=%" PRIu64 ", den=%" PRIu64 ", val=%" PRIu64 "\n",index,G[index].num,G[index].den,G[index].val);
    return eYes ;
}

void SendResponse( search_more status ) {
    // Clear alarm
    alarm( 0 ) ;

    // calc time
    struct timespec now;
    clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &now ) ;
    
    // Full struct
    xResponse.rank    = Grank ;
    xResponse.count   = Gcounter ;
    xResponse.status  = status ;
    xResponse.from    = Gfrom ;
    xResponse.to      = Gto ;
    xResponse.elapsed = now.tv_sec - Gtime.tv_sec + 10E-9 * ( now.tv_nsec - Gtime.tv_nsec );
    printf("Worker send rank %d, counter %" PRIu64 ", status %d, from %" PRIu64 ", to %" PRIu64 ",elapsed %g\n",
		xResponse.rank,xResponse.count,xResponse.status,xResponse.from,xResponse.to,xResponse.elapsed);
    MPI_Send( &xResponse, 1, Response_t, Groot, mResponse, MPI_COMM_WORLD ) ;
}

search_more Range_search( int index, uint64_t from, uint64_t to ) {
    // now run through laast preset level
    for ( uint64_t v = from; v < to + Gsum ; v += Gsum ) {
        Add_preset( index , v ) ;
        switch ((Gsum==1) ? Midterm_for_1( index+1, &G[index] ) : Midterm( index+1, &G[index] ) ) {
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

void Timeout_handler( int sig ) {
    siglongjmp( Gmark_spot, -1 ) ;
}

void WorkerSetup( void ) {
    // Broadcast parameters to all workers
    CommunicationSetupPre() ;

    // Load struct
    xSetup.terms   = Gterms ;
    xSetup.sum     = Gsum ;
    xSetup.timeout = Gtimeout ;
    
    MPI_Bcast( &xSetup, 1, Setup_t, Groot, MPI_COMM_WORLD ) ;

    // Unload Struct
    Gterms   = xSetup.terms ;
    Gsum     = xSetup.sum ;
    Gtimeout = xSetup.timeout ;

    printf("Broadcast rank %d, terms = %" PRIu64 "\n", Grank, Gterms) ;

    CommunicationSetupPost() ;
}

search_more GetJob( void ) {
    // For Workers only
    printf("Worker %d waiting\n",Grank);
//    MPI_Recv( pJob, 1, Job_t, Groot, mJob, MPI_COMM_WORLD, MPI_STATUS_IGNORE ) ;
    MPI_Recv( pJob, 1, Job_t, MPI_ANY_SOURCE, mJob, MPI_COMM_WORLD, MPI_STATUS_IGNORE ) ;
    printf("Worker %d got job: nPresets %, until %" PRIu64 "\n\t",Grank,pJob->nPresets,pJob->until);
    for ( int i=0 ; i<Gterms-1; ++i ) {
		printf(" %" PRIu64, pJob->presets[i] );
	}
	printf("\n");
    
    
    if ( pJob->nPresets == Gterms ) {
        // Trigger for end of run
        return eNo ;
    }
    
    // total counter
    Gcounter = 0 ;

    // start of elapsed time
    clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &Gtime ) ;
    
    if ( sigsetjmp( Gmark_spot, 1 ) != 0 ) {
        SendResponse( eTimeout ) ;
        return eYes ;
    }

    // Timer handler
    signal( SIGALRM, Timeout_handler);
    alarm( Gtimeout ) ;

    
    if ( pJob->nPresets <= 0) { 
        // no presets
        Gfrom = 2 ;
        Gto = Gsum * ( (uint64_t) ( 1.1 + (Gterms) / (exp(Gsum)-1) ) ) ;

        // Solve
        if ( Guntil == 0 ) {
            SendResponse( Range_search( 0, Gfrom, Gto ) ) ;
        } else {
            SendResponse( Range_search( 0, Gfrom, Guntil ) ) ;
        }
        return eYes ;

    } else {
        // Add presets first
        int index = -1 ;
        for ( int i = 0; i < pJob->nPresets; ++i) {
            ++ index ;
            if ( index == Gterms-2 ) {
                --index ;
                fprintf(stderr, "Too many preset values -- will only use first %" PRIu64 "\n",Gterms-2);
                Guntil = 0 ;
            }
            if ( Add_preset( index, pJob->presets[i]     ) == eError ) {
                SendResponse( eError ) ;
                return eYes ;
            }
        }

        // estimage range of next level after presets
        Gfrom = ( G[index].den / ( Gsum * G[index].num)) * Gsum + Gsum ;
        if ( Gfrom < G[index].val + Gsum ) {
            Gfrom = G[index].val + Gsum ;
        }
        Gto = Gsum * ( (uint64_t) ( 1.1 + (Gterms-index-1) / (exp(((double) Gsum * G[index].num)/(G[index].den))-1) ) );

        // Solve
        if ( Guntil < G[index].val ) {
            SendResponse( Range_search( index, G[index].val, G[index].val ) );
        } else {
            SendResponse( Range_search( index, G[index].val, Guntil ) ) ;
        }
        return eYes ;
    }
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
    printf("\t-n%d\t--number\tnumber of terms in the sum (default %d)\n",Gterms,Gterms);
    printf("\t\tAll terms will be of form 1/v and distinct\n");
    printf("\t-s%d\t--sum\ttarget sum (default %d)\n",Gsum,Gsum);
    printf("\t-t\t--timelimit \t stop calculation after specified seconds (default none)\n");
    printf("\t-r\t--range \t increase last preset to this value\n");
    printf("\t-h\t--help\tthis help\n");
    printf("\nSee https://github.com/alfille/reciprocals for full exposition\n\n");
    exit(1);
}

struct option long_options[] =
{
    {"number"   ,   required_argument, 0, 'n'},
    {"sum"      ,   required_argument, 0, 's'},
    {"timelimit",   required_argument, 0, 't'},       
    {"range"    ,   required_argument, 0, 'r'},       
    {"help"     ,   no_argument,       0, 'h'},
    {0          ,   0          ,       0,   0}
};

void ParseCommandLine( int argc, char * argv[] ) {
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
                help();
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
                help() ;
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

void allocFreeWorker(void) {
    nFW = 0 ;
    pFreeWorker = calloc( Gworkers, sizeof( int ) ) ;
    for ( int rank = 0 ; rank < Gworkers ; ++rank ) {
        pFreeWorker[nFW] = rank ;
        ++ nFW ;
    }
}

void freeFreeWorker( void ) {
    free(pFreeWorker) ;
}
    
void * vScratchJob ;    
void * vRankJobs ;  
void * vWorkQueue ;
void * vvWorkQueue ;
#define WORKQUEUE 1000000

void allocJobSpace(void) {
    struct wJob {
        uint64_t nPresets ;
        uint64_t until ;
        uint64_t presets[Gterms-1];
    } ;
    struct wJob * pScratch = ( struct wJob *) calloc( 1 + Gworkers + WORKQUEUE, sizeof( struct wJob ) ) ;
    vScratchJob = pScratch ;
    struct wJob * pRankJobs = pScratch + 1 ;
    vRankJobs = pRankJobs ;
    struct wJob * pWorkQueue = pRankJobs + Gworkers ;
    vWorkQueue = pWorkQueue ;
    vvWorkQueue = pWorkQueue ;
}

void freeJobSpace( void ) {
    free(vScratchJob) ;
}

void SendRank( int rank ) {
    struct wJob {
        uint64_t nPresets ;
        uint64_t until ;
        uint64_t presets[Gterms-1];
    } ;
    struct wJob * pRankJobs = vRankJobs ;
    MPI_Send( pRankJobs + rank, 1, Job_t, rank, mJob, MPI_COMM_WORLD ) ;
}

void addFreeWorker( int rank ) {
    pFreeWorker[ nFW ] = rank ;
    ++ nFW ;
}

int isFreeWorker( void ) {
    return nFW > 0 ;
}

int getFreeWorker( void ) {
    --nFW ;
    return pFreeWorker[nFW] ;
}

void LoadRank( int rank, void * vjob ) {
    struct wJob {
        uint64_t nPresets ;
        uint64_t until ;
        uint64_t presets[Gterms-1];
    } ;

    struct wJob * pjob = vjob ;
    struct wJob * pRankJobs = vRankJobs ;
    memcpy( pRankJobs + rank, vjob, sizeof( struct wJob ) ) ;
}

void * addQueue( void * vjob ) {
    struct wJob {
        uint64_t nPresets ;
        uint64_t until ;
        uint64_t presets[Gterms-1];
    } ;
    struct wJob * pjob = vjob ;
    struct wJob * cWorkQueue = vvWorkQueue ;
    memcpy( cWorkQueue, vjob, sizeof( struct wJob ) ) ;
    ++ cWorkQueue ;
    return cWorkQueue - 1 ;
}
    
int isQueueEmpty( void ) {
    return vWorkQueue == vvWorkQueue ;
}

void SendOrQueue( void * vjob ) {
    if ( isFreeWorker() ) {
        int rank = getFreeWorker() ;
        printf("Root send to worker %d\n",rank);
        LoadRank( rank, vjob ) ;
    } else {
        printf("Root send to queue %d\n",nFW);
        addQueue( vjob ) ;
    }
}
    
void useQueue( int rank ) {
    struct wJob {
        uint64_t nPresets ;
        uint64_t until ;
        uint64_t presets[Gterms-1];
    } ;
    struct wJob * cWorkQueue = vvWorkQueue ;
    -- cWorkQueue ;
    LoadRank( rank, cWorkQueue ) ;
    SendRank( rank ) ;
}

void SplitJob( int rank ) {
    //Use xResponse data too
    struct wJob {
        uint64_t nPresets ;
        uint64_t until ;
        uint64_t presets[Gterms-1];
    } ;
    struct wJob * pRankJobs = vRankJobs ;
    struct wJob * pR = pRankJobs + rank ;
    struct wJob * pScratch = vScratchJob ;
    int nps = pR->nPresets ;
    
    if ( nps > 0 && pR->until != pR->presets[nps-1] ) {
        // This level splittable
        printf("Root from rank %d, Split this level %d\n",rank,nps-1);
        memcpy( pScratch, pR, sizeof( struct wJob ) ) ;
        pR->presets[nps-1] += Gsum ;
        SendRank( rank ) ;
        pScratch->until = pScratch->presets[nps-1] ;
    } else {
        // Split next level
        printf("Root from rank %d, Split next level %d\n",rank,nps);
        ++ pR->nPresets ;
        memcpy( pScratch, pR, sizeof( struct wJob ) ) ;
        pR->presets[nps] = xResponse.from + Gsum ;
        pR->until = xResponse.to ;
        SendRank( rank ) ;
        pScratch->presets[nps] = xResponse.from ;
        pScratch->until = xResponse.from ;
    }
    SendOrQueue( pScratch ) ;
}

void RootSetup( void ) {
    // Structure for Job with presets array specified. 
    // Have to do everything in this routine because C has no nested subroutines. 

    struct wJob {
        uint64_t nPresets ;
        uint64_t until ;
        uint64_t presets[Gterms-1];
    } ;
    
    allocJobSpace() ;
    allocFreeWorker() ;
    struct wJob * pRankJobs = vRankJobs ;
    struct wJob * pScratch = vScratchJob ;

    // Add Basic start:
    Gfrom = 2 ;
    Gto = Gsum * ( (uint64_t) ( 1.1 + (Gterms) / (exp(Gsum)-1) ) ) ;
    for ( int v = Gto ; v >= Gfrom ; --v ) {
        pScratch->nPresets   = 1 ;
        pScratch->presets[0] = v ;
        pScratch->until      = v ;
        SendOrQueue( pScratch ) ;
    }
    
    // Loop through waiting for results
    do {
        MPI_Recv( &xResponse, 1, Response_t, MPI_ANY_SOURCE, mResponse, MPI_COMM_WORLD, MPI_STATUS_IGNORE ) ;
        int rank = xResponse.rank ;
        switch( xResponse.status ) {
            case eYes:
            case eNo:
            case eError:
                // Successful search
                Gcounter += xResponse.count ;
                if ( isQueueEmpty() ) {
					printf("Root from rank %d, Add Free Worker\n",rank ) ;
                    addFreeWorker( rank ) ;
                } else {
					printf("Root from rank %d, Use Queue %d\n",rank,nFW-1 ) ;
                    useQueue( rank ) ;
                }
                break ;
            case eTimeout:
                // Unsuccessful, need to split
                SplitJob( rank ) ;
				printf("Root from rank %d, Split\n",rank,nFW-1 ) ;
                break ;
        }
    } while ( nFW < Gworkers ) ;
    
    // Close
    for ( int rank=0 ; rank < Gworkers ; ++rank ) {
		printf("Root close worker %d \n",rank);
        pRankJobs->nPresets = Gterms ; // illegal value -- flag for close worker
        SendRank( rank ) ;
    }
    freeFreeWorker();
    freeJobSpace();
    pJob->nPresets = Gterms ; // signal for end    
}

int main( int argc, char * argv[] ) {
    MPI_Init( &argc, &argv ) ;

    MPI_Comm_rank( MPI_COMM_WORLD, &Grank ) ;
    MPI_Comm_size( MPI_COMM_WORLD, &Gsize ) ;
    Groot    = Gsize - GROOTS ; // last worker "rank" is root
    Gworkers = Gsize - GROOTS ; // rest are workers (index from 0)

    if ( Grank==Groot ) {
        // MASTER
        ParseCommandLine( argc, argv ) ;
        printf("Find sets of %d unique reciprocals that sum to %" PRIu64 "\n", Gterms, Gsum );
		printf("MPI size=%d, Workers=%d, Root is %d\n",Gsize,Gworkers,Groot);
   }
    
    WorkerSetup() ; // Send command-line derived parameters to all workers, set up message field sizes
        
    if ( Grank == Groot ) {
        // root process
        RootSetup() ;
    } else {
        // worker process (in a loop)
        int contnu ;
        do {
			contnu = GetJob() ;
        } while ( contnu ) ;
    }

    MPI_Finalize() ;

    return 0 ;
}

