#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "lock.h"

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

double parallelSerial (int numPackets,
                       int numSources,
                       long mean,
                       int uniformFlag,
                       int queueDepth,
                       short experimentNumber);

enum strategy_t {LOCK_FREE, HOMEQUEUE, RANDOMQUEUE, AWESOME};
extern enum strategy_t strat;
extern lock_iface (*lock_gen)(void);

int test_main () {
    printf("Test 1: Idle Lock Overhead\n");

    int w1[] = {25, 50, 100, 200, 400, 800};
    int T = (pow(2, 14));
    for (int  i = 0; i < sizeof(w1)/sizeof(int); i++) {
        printf("W=%d\n", w1[i]);
        strat = LOCK_FREE;
        lock_gen = NULL;
        double t1 = parallelFirewall(T, 1, w1[i], 1, 10, 0);

        strat = HOMEQUEUE;
        lock_gen = new_explock;
        double t2 = parallelFirewall(T, 1, w1[i], 1, 10, 0);

        strat = HOMEQUEUE;
        lock_gen = new_taslock;
        double t3 = parallelFirewall(T, 1, w1[i], 1, 10, 0);

        printf("EXP: lock free=%lf, home queue=%lf, speedup=%lf\n", t1, t2, t1/t2);
        printf("TAS: home queue=%lf, speedup=%lf\n", t3, t1/t3);

    }

    return 0;


}