#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <err.h>

#ifdef DEBUG
    #define DEBUG_PRINTF(...) printf("DEBUG: "__VA_ARGS__)
#else
    #define DEBUG_PRINTF(...)
#endif

#define NTHREADS 4
#define NITERATIONS 5
#define PERIOD 0.9
#define TXT 512 

void *
do_work (void *id) {

    char inf[TXT];
    char fail[] = "(fallo temporal)";
    volatile unsigned long long j;
    int i, *p_id = id, bytes;
    double t_cost, dif = 0.0;
    struct timespec begin, end, remaining, request = {0, 0};

    for (i = 0; i < NITERATIONS; i++) {

        if (clock_gettime(CLOCK_MONOTONIC, &begin) < 0) {   //  take the initial time
            err(EXIT_FAILURE, "clock_gettime failed");
        }

        for (j = 0; j < 390000000ULL; j++) { };

        if (clock_gettime(CLOCK_MONOTONIC, &end) < 0) { //  take the final time
            err(EXIT_FAILURE, "clock_gettime failed");
        }

        // calculation of cost time
        t_cost = (end.tv_sec - begin.tv_sec) * 1e9; 
        t_cost = (t_cost + (end.tv_nsec - begin.tv_nsec)) * 1e-9;

        dif = (PERIOD - t_cost) * 1e9;  //  difference calculation between cost time and period

        bytes = sprintf(inf, "[%ld.%ld] Thread %d - Iteracion %d: Coste=%.2f s.",
             begin.tv_sec, begin.tv_nsec, (*p_id)+1, i+1, t_cost);

        if (bytes < 0) {
            err(EXIT_FAILURE, "snprintf failed");
        }

        DEBUG_PRINTF("\nDebo dormirme para cumplir el periodo de 0.9s : %f s. \n", dif*1e-9);

        if (dif < 0) {
            strncat(inf, fail, strlen(fail));
            printf("%s\n", inf);
            continue;
        }

        printf("%s\n", inf);
        request.tv_nsec = dif;
        if (nanosleep(&request, &remaining) < 0) {  //  function nanosleep to comply the period
            err(EXIT_FAILURE, "nanosleep failed");
        }
    }

    return NULL;
}

int main() {

    int i, ids[NTHREADS];
    pthread_t threads[NTHREADS];

    for (i = 0; i < NTHREADS; i++) {
        ids[i] = i;
        if (pthread_create(&threads[i], NULL, do_work, &ids[i]) != 0) { //  threads are created
			warnx("error creating thread");
			return 1;
		}
    }

    for (i = 0; i < NTHREADS; i++) {
		if (pthread_join(threads[i], NULL) != 0) {  //  waiting for termination of threads
			warnx("error joining thread");
			return 1;
		}
	}

    return 0;
}