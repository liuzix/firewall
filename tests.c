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

    printf("========\n\nTest 2: Uniform Speedup\n");
    int w2[] = {1000, 2000, 4000, 8000};
    int n2[] = {1,2,3,7,13,27};
    //T = 2*3*7*13*27 << 2;
    T = (pow(2, 25));
    for (int i = 0; i < sizeof(w2)/sizeof(int); i++) {
        for (int j = 0; j < sizeof(n2)/sizeof(int); j++) {
            strat = LOCK_FREE;
            lock_gen = NULL;
            double t0 = serialFirewall(T/(n2[j]*w2[i]), n2[j], w2[i], 1, 0);

            printf("W=%d\n", w2[i]);
            strat = LOCK_FREE;
            lock_gen = NULL;
            double t1 = parallelFirewall(T/(n2[j]*w2[i]), n2[j], w2[i], 1, 32, 0);
            printf("lockfree, n = %d, speedup = %lf\n", n2[j], t0 / t1);

            strat = HOMEQUEUE;
            lock_gen = new_explock;
            double t2 = parallelFirewall(T/(n2[j]*w2[i]), n2[j], w2[i], 1, 32, 0);
            printf("homequeue, exp, n = %d, speedup = %lf\n", n2[j], t0 / t2);

            strat = HOMEQUEUE;
            lock_gen = new_taslock;
            double t3 = parallelFirewall(T/(n2[j]*w2[i]), n2[j], w2[i], 1, 32, 0);
            printf("homequeue, tas, n = %d, speedup = %lf\n", n2[j], t0 / t3);

            strat = RANDOMQUEUE;
            lock_gen = new_explock;
            double t4 = parallelFirewall(T/(n2[j]*w2[i]), n2[j], w2[i], 1, 32, 0);
            printf("randomqueue, exp, n = %d, speedup = %lf\n", n2[j], t0 / t4);

            strat = RANDOMQUEUE;
            lock_gen = new_taslock;
            double t5 = parallelFirewall(T/(n2[j]*w2[i]), n2[j], w2[i], 1, 32, 0);
            printf("randomqueue, tas, n = %d, speedup = %lf\n", n2[j], t0 / t5);

            strat = AWESOME;
            lock_gen = new_explock;
            double t6 = parallelFirewall(T/(n2[j]*w2[i]), n2[j], w2[i], 1, 32, 0);
            printf("awesome, exp, n = %d, speedup = %lf\n", n2[j], t0 / t6);
        }
    }

    printf("========\n\nTest 2: Exponential Speedup\n");
    //int w2[] = {1000, 2000, 4000, 8000};
    //int n2[] = {1,2,3,7,13,27};
    //T = 2*3*7*13*27 << 2;
    for (int i = 0; i < sizeof(w2)/sizeof(int); i++) {
        for (int j = 0; j < sizeof(n2)/sizeof(int); j++) {
            strat = LOCK_FREE;
            lock_gen = NULL;
            double t0 = serialFirewall(T/(n2[j]*w2[i]), n2[j], w2[i], 0, 0);

            printf("W=%d\n", w2[i]);
            strat = LOCK_FREE;
            lock_gen = NULL;
            double t1 = parallelFirewall(T/(n2[j]*w2[i]), n2[j], w2[i], 0, 32, 0);
            printf("lockfree, n = %d, speedup = %lf\n", n2[j], t0 / t1);

            strat = HOMEQUEUE;
            lock_gen = new_explock;
            double t2 = parallelFirewall(T/(n2[j]*w2[i]), n2[j], w2[i], 0, 32, 0);
            printf("homequeue, exp, n = %d, speedup = %lf\n", n2[j], t0 / t2);

            strat = HOMEQUEUE;
            lock_gen = new_taslock;
            double t3 = parallelFirewall(T/(n2[j]*w2[i]), n2[j], w2[i], 0, 32, 0);
            printf("homequeue, tas, n = %d, speedup = %lf\n", n2[j], t0 / t3);

            strat = RANDOMQUEUE;
            lock_gen = new_explock;
            double t4 = parallelFirewall(T/(n2[j]*w2[i]), n2[j], w2[i], 0, 32, 0);
            printf("randomqueue, exp, n = %d, speedup = %lf\n", n2[j], t0 / t4);

            strat = RANDOMQUEUE;
            lock_gen = new_taslock;
            double t5 = parallelFirewall(T/(n2[j]*w2[i]), n2[j], w2[i], 0, 32, 0);
            printf("randomqueue, tas, n = %d, speedup = %lf\n", n2[j], t0 / t5);

            strat = AWESOME;
            lock_gen = new_explock;
            double t6 = parallelFirewall(T/(n2[j]*w2[i]), n2[j], w2[i], 0, 32, 0);
            printf("awesome, exp, n = %d, speedup = %lf\n", n2[j], t0 / t6);
        }
    }

    return 0;


}