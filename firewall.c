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
#include "lock.h"


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


enum strategy_t strat;
lock_iface (*lock_gen)(void) = NULL;

__thread enum strategy_t _strat;



int test_main ();
int main(int argc, char * argv[]) {
    test_main();
    return 0;
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
        }
        stopTimer(&watch);
    }
    return getElapsedTime(&watch);
}

struct WorkerArgs_t {
    int numPackets;
    struct queue* q;
};

struct queue** qs;
int numQueues;


struct queue* dequeue_select() {
    static int QueueCur = 0;
    if (_strat == AWESOME) {
        int i = __sync_fetch_and_add(&QueueCur, 1);
        return qs[i % numQueues];
    }
    else if (_strat == RANDOMQUEUE) {
        int i = random();
        return qs[i % numQueues];
    }
}

void* parallelWorker(void* arg) {
    _strat = strat;
    struct WorkerArgs_t* a = arg;
    long fingerprint = 0;
    for (int i = 0; i < a->numPackets; i++) {
        Packet_t p;
        if (_strat == HOMEQUEUE) {
            lock(&a->q->lock_inst);
            p = queue_dequeue(a->q);
            unlock(&a->q->lock_inst);
        }
        else if (_strat == RANDOMQUEUE) {
            while (true) {
                struct queue *temp_q = dequeue_select();
                lock(&temp_q->lock_inst);
                if (is_empty(temp_q)) {
                    unlock(&temp_q->lock_inst);
                    continue;
                }
                p = queue_dequeue(temp_q);
                unlock(&temp_q->lock_inst);
                break;
            }
        }
        else if (_strat == AWESOME) {
            struct queue* temp_q = a->q;
            lock(&temp_q->lock_inst);
            while (is_empty(temp_q)) {
                unlock(&temp_q->lock_inst);
                temp_q = dequeue_select();
                lock(&temp_q->lock_inst);
            }
            p = queue_dequeue(temp_q);
            unlock(&temp_q->lock_inst);
        }
        else if (_strat == LOCK_FREE) {
            p = queue_dequeue(a->q);
        }
        fingerprint += getFingerprint(p.iterations, p.seed);
    }


    // pretend it is a pointer and return it
    return (void*) fingerprint;
}

struct queue* enqueue_select() {
    assert(strat == AWESOME);
    static int i = 0;
    return qs[__sync_fetch_and_add(&i, 1) % numQueues];
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

    qs = calloc((size_t) numSources, sizeof(struct queue *));
    numQueues = numSources;
    assert(qs);

    // initializing queues
    for (int i = 0; i < numSources; i++) {
        qs[i] = calloc(1, sizeof(struct queue));
        queue_init(qs[i], (size_t) queueDepth);
    }

    // creating threads
    pthread_t **threads = calloc((size_t )numSources, sizeof(pthread_t*));
    for (int i = 0; i < numSources; i++) {
        threads[i] = calloc(1, sizeof(pthread_t));
        struct WorkerArgs_t* args = calloc(1, sizeof(struct WorkerArgs_t));
        args->q = qs[i];
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
            if (strat == LOCK_FREE)
                queue_enqueue(qs[j], *p);
            else if (strat == HOMEQUEUE || strat == RANDOMQUEUE) {
                lock(&qs[j]->lock_inst);
                queue_enqueue(qs[j], *p);
                unlock(&qs[j]->lock_inst);
            }
            else if (strat == AWESOME) {
                struct queue * temp_q;
                while ((temp_q = enqueue_select())) {
                    lock(&temp_q->lock_inst);
                    if (is_full(temp_q)) {
                        unlock(&temp_q->lock_inst);
                        continue;
                    } else {
                        queue_enqueue(temp_q, *p);
                        unlock(&temp_q->lock_inst);
                        break;
                    }
                }
            }
            free(p);
        }
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
    }


    stopTimer(&watch);
    return getElapsedTime(&watch);
}