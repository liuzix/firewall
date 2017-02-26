#include <stdio.h>
#include <math.h>
#include <assert.h>

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

int test_main () {
    printf("Test 1: Parallel Overhead\n");
    int w1[] = {25, 50, 100, 200, 400, 800};
    int n1[] = {1, 8, 14};
    for (int i = 0; i < sizeof(n1) / sizeof(int); i++) {
        for (int j = 0; j < sizeof(w1) / sizeof(int); j++) {
            printf("W=%d, n=%d\n", w1[j], n1[i]);
            int T = (pow(2, 24)) / (w1[j] * n1[i]);
            assert(T > 0);

            double t1 = serialFirewall(T, n1[i], w1[j], 1, 1);
            double t2 = parallelSerial(T, n1[i], w1[j], 1, 32, 1);
            printf("Speedup=%lf\n", t2/t1);
            printf("Worker Rate=%lf\n\n", T / t2);
        }
    }

    printf("======\n");
    printf("Test 2: Dispatcher Rate\n");
    int n2[] = {1, 8, 14};

    for (int j = 0; j < sizeof(n2) / sizeof(int); j++) {
        printf("W=%d, n=%d\n", 1, n2[j]);
        int T = (pow(2, 20)) / (n2[j]);
        assert(T > 0);

        double t1 = parallelFirewall(T, n2[j], 1, 1, 32, 2);
        printf("Rate=%lf\n\n", T/t1);
    }

    printf("======\n");
    printf("Test 3: Speedup with Constant Load\n");
    int w3[] = {1000, 2000, 4000, 8000};
    int n3[] = {1, 2, 4, 8, 14, 28};
    for (int i = 0; i < sizeof(n3) / sizeof(int); i++) {
        for (int j = 0; j < sizeof(w3) / sizeof(int); j++) {
            printf("W=%d, n=%d\n", w3[j], n3[i]);
            int T = pow(2, 17);
            assert(T > 0);

            double t1 = serialFirewall(T, n3[i], w3[j], 3, 3);
            double t2 = parallelFirewall(T, n3[i], w3[j], 3, 32, 3);
            printf("Speedup=%lf\n\n", t1/t2);
        }
    }

    printf("======\n");
    printf("Test 4: Speedup with Uniform Load\n");
    int w4[] = {1000, 2000, 4000, 8000};
    int n4[] = {1, 2, 4, 8, 14, 28};
    for (int i = 0; i < sizeof(n4) / sizeof(int); i++) {
        for (int j = 0; j < sizeof(w4) / sizeof(int); j++) {
            printf("W=%d, n=%d\n", w4[j], n4[i]);
            int T = (pow(2, 17));
            assert(T > 0);

            double t1 = serialFirewall(T, n4[i], w4[j], 1, 4);
            double t2 = parallelFirewall(T, n4[i], w4[j], 1, 32, 4);
            printf("Speedup=%lf\n\n", t1/t2);
        }
    }

    printf("======\n");
    printf("Test 5: Speedup with Exponential Load\n");
    int w5[] = {1000, 2000, 4000, 8000};
    int n5[] = {1, 2, 4, 8, 14, 28};
    for (int i = 0; i < sizeof(n5) / sizeof(int); i++) {
        for (int j = 0; j < sizeof(w5) / sizeof(int); j++) {
            printf("W=%d, n=%d\n", w5[j], n5[i]);
            int T = (pow(2, 17));
            assert(T > 0);

            double t1 = serialFirewall(T, n5[i], w5[j], 0, 5);
            double t2 = parallelFirewall(T, n5[i], w5[j], 0, 32, 5);
            printf("Speedup=%lf\n\n", t1/t2);
        }
    }
    return 0;


}