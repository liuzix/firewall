#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

#include "Utils/generators.h"
#include "Utils/stopwatch.h"
#include "Utils/fingerprint.h"
#include "Utils/packetsource.h"
#include "myqueue.h"


#define DEFAULT_NUMBER_OF_ARGS 6

double runtime = 0.0;
double speedup = 0.0;

double serialFirewall(const int,
                     const int,
                     const long,
                     const int,
                     const short);

double parallelFirewall(int numPackets,
                       int numSources,
                       long mean,
                       int uniformFlag,
                       int queueDepth,
                       short experimentNumber);



int test_main ();
int main(int argc, char * argv[]) {
    test_main();
    return 0;
}

inline void print_progress(long long total, long long now) {
    if (now % 1000000 == 0 && now != 0) {
    //    printf("finished %lf percent\n", 100 * (double)now / (double)total);
    }
}

double serialFirewall(const int numPackets,
                     const int numSources,
                     const long mean,
                     const int uniformFlag,
                     const short experimentNumber)
{

    runtime = 0.0;
    PacketSource_t * packetSource = createPacketSource(mean, numSources, experimentNumber);
    StopWatch_t watch;
    long fingerprint = 0;

    if(uniformFlag == 1) {
        startTimer(&watch);
        for( int j = 0; j < numPackets; j++ ) {
            for( int i = 0; i < numSources; i++ ) {

                volatile Packet_t * tmp = getUniformPacket(packetSource,i);
                fingerprint += getFingerprint(tmp->iterations, tmp->seed);
                free(tmp);
            }
            print_progress(numPackets, j);
        }
        stopTimer(&watch);
    }
    else if (uniformFlag == 0) {
        startTimer(&watch);
        for( int j = 0; j < numPackets; j++ ) {
            for( int i = 0; i < numSources; i++ ) {

                volatile Packet_t * tmp = getExponentialPacket(packetSource,i);
                fingerprint += getFingerprint(tmp->iterations, tmp->seed);
                free(tmp);
            }
            print_progress(numPackets, j);
        }
        stopTimer(&watch);
    } else if (uniformFlag == 3) { // constant packets
        startTimer(&watch);
        for( int j = 0; j < numPackets; j++ ) {
            for( int i = 0; i < numSources; i++ ) {

                volatile Packet_t * tmp = getConstantPacket(mean);
                fingerprint += getFingerprint(tmp->iterations, tmp->seed);
                free(tmp);
            }
            print_progress(numPackets, j);
        }
        stopTimer(&watch);
    }
    return getElapsedTime(&watch);
}

struct WorkerArgs_t {
    struct queue* q;
    int numPackets;
};


void* parallelWorker(void* arg) {
    struct WorkerArgs_t* a = arg;

    long fingerprint = 0;

    for (int i = 0; i < a->numPackets; i++) {
        Packet_t p = queue_dequeue(a->q);
        fingerprint += getFingerprint(p.iterations, p.seed);
    }


    // pretend it is a pointer and return it
    return (void*) fingerprint;
}


double parallelFirewall(int numPackets,
                       int numSources,
                       long mean,
                       int uniformFlag,
                       int queueDepth,
                       short experimentNumber)
{
    PacketSource_t * packetSource = createPacketSource(mean, numSources, experimentNumber);
    StopWatch_t watch;
    long fingerprint = 0;
    startTimer(&watch);

    struct queue **queues = calloc((size_t) numSources, sizeof(struct queue *));
    assert(queues);

    // initializing queues
    for (int i = 0; i < numSources; i++) {
        queues[i] = calloc(1, sizeof(struct queue));
        queue_init(queues[i], (size_t) queueDepth);
    }

    // creating threads
    pthread_t **threads = calloc((size_t )numSources, sizeof(pthread_t*));
    for (int i = 0; i < numSources; i++) {
        threads[i] = calloc(1, sizeof(pthread_t));
        struct WorkerArgs_t* args = calloc(1, sizeof(struct WorkerArgs_t));
        args->q = queues[i];
        args->numPackets = numPackets;
        pthread_create(threads[i], NULL, parallelWorker, args);
    }


    for (int i = 0; i < numPackets; i++) {
        for (int j = 0; j < numSources; j++) {
            Packet_t *p = NULL;
            if (uniformFlag == 3)
                p = getConstantPacket(mean);
            else
                p = uniformFlag ? getUniformPacket(packetSource, j) : getExponentialPacket(packetSource, j);
            queue_enqueue(queues[j], *p);
            free(p);
        }
        print_progress(numPackets, i);
    }

    for (int i = 0; i < numSources; i++) {
        long res = 0;
        pthread_join(*threads[i], (void**)&res);
        fingerprint += res;
    }

    stopTimer(&watch);
    //printf("%f\n",getElapsedTime(&watch));
    //speedup = runtime / getElapsedTime(&watch);
    return getElapsedTime(&watch);
}

double parallelSerial (int numPackets,
                       int numSources,
                       long mean,
                       int uniformFlag,
                       int queueDepth,
                       short experimentNumber)
{
    PacketSource_t * packetSource = createPacketSource(mean, numSources, experimentNumber);
    StopWatch_t watch;
    long fingerprint = 0;
    startTimer(&watch);

    struct queue **queues = calloc((size_t) numSources, sizeof(struct queue *));
    assert(queues);

    // initializing queues
    for (int i = 0; i < numSources; i++) {
        queues[i] = calloc(1, sizeof(struct queue));
        queue_init(queues[i], (size_t) queueDepth);
    }


    for (int i = 0; i < numPackets; i++) {
        for (int j = 0; j < numSources; j++) {
            Packet_t *p = NULL;
            if (uniformFlag == 3)
                p = getConstantPacket(mean);
            else
                p = uniformFlag ? getUniformPacket(packetSource, j) : getExponentialPacket(packetSource, j);
            queue_enqueue(queues[j], *p);
            free(p);
            Packet_t temp = queue_dequeue(queues[j]);
            fingerprint += getFingerprint(temp.iterations, temp.seed);
        }
        print_progress(numPackets, i);
    }


    stopTimer(&watch);
    return getElapsedTime(&watch);
}