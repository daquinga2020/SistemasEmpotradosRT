#define _GNU_SOURCE
#include <sched.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <err.h>


#ifdef DEBUG
    #define DEBUG_PRINTF(...) printf("DEBUG: "__VA_ARGS__)
#else
    #define DEBUG_PRINTF(...)
#endif

#define SECS 60
#define SLEEP 1e8
#define MAX_SIZE 700

struct DataThread
{
    int id;
    int latency_avg;
    int max_latency;
    int latencies[MAX_SIZE];
};

typedef struct DataThread DataThread;


void *
do_work(void * data_th) {
    
    int count = 0;
    double latency, diff;
    struct timespec begin, intermediate, end, remaining, request = {0, 0};
    DataThread * p_dth_th = data_th;
    struct sched_param param;
    cpu_set_t cpuset;

    request.tv_nsec = SLEEP;

    CPU_ZERO(&cpuset);
    CPU_SET(p_dth_th->id, &cpuset);

    param.sched_priority=99;

    //  Running thread on different core
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) != 0) {
        perror("pthread_setaffinity_np");
        return NULL;
    }

    //  Thread is configurated with priority 99
    if (pthread_setschedparam (pthread_self(), SCHED_FIFO, &param) != 0) {
        perror("pthread_setschedparam");
        return NULL;
    }

    if (clock_gettime(CLOCK_MONOTONIC, &begin) < 0) {   //  take the initial time
        err(EXIT_FAILURE, "clock_gettime failed");
    }

    do {

        if (clock_gettime(CLOCK_MONOTONIC, &intermediate) < 0) {
            err(EXIT_FAILURE, "clock_gettime failed");
        }

        if (nanosleep(&request, &remaining) < 0) {
            err(EXIT_FAILURE, "nanosleep failed");
        }

        if (clock_gettime(CLOCK_MONOTONIC, &end) < 0) {
            err(EXIT_FAILURE, "clock_gettime failed");
        }
        
        diff = (end.tv_sec - intermediate.tv_sec)*1e9 + (end.tv_nsec - intermediate.tv_nsec);
        latency = diff-SLEEP;
        if (latency < 0) {
            DEBUG_PRINTF("LATENCIA NEGATIVE: %f TIME AFTER SLEEP: %ld TIME BEFORE SLEEP: %ld\n", latency, end.tv_nsec, intermediate.tv_nsec);
            continue;
        }

        if (p_dth_th->max_latency < latency) {
            p_dth_th->max_latency = latency;
        }
            
        p_dth_th->latency_avg = latency + p_dth_th->latency_avg;
        p_dth_th->latencies[count] =  latency;

        count++;
        
    } while (intermediate.tv_sec-begin.tv_sec <= SECS);
    
    p_dth_th->latency_avg = (p_dth_th->latency_avg/count);
    
    DEBUG_PRINTF("HILO:[%d]\tlatencia media: %09d ns. | max: %09d ns.\n", p_dth_th->id, p_dth_th->latency_avg, p_dth_th->max_latency);
    pthread_exit(NULL);
}


int main() {

    int Ncores = (int) sysconf(_SC_NPROCESSORS_ONLN);
    int i, j, latency_target_fd, fd_csv;
    long int av_lat = 0, max_lat = 0;
    pthread_t threads[Ncores];
    DataThread dths[Ncores];

    DEBUG_PRINTF("CORES: %d\n", Ncores);

    //  Linux configured with minimum latency
    static int32_t latency_target_value = 0;
    latency_target_fd = open("/dev/cpu_dma_latency", O_RDWR);
    if (write(latency_target_fd, &latency_target_value, 4) < 0) {
        err(EXIT_FAILURE, "write failed");
    }
    

    for (i = 0; i < Ncores; i++) {

        dths[i].id = i;
        dths[i].latency_avg = 0;
        dths[i].max_latency = 0;

        if (pthread_create(&threads[i], NULL, &do_work, &dths[i]) < 0) { //  Threads are created
            warnx("error creating thread");
			return 1;
        }
    }

    for (i = 0; i < Ncores; i++) {

        if (pthread_join(threads[i], NULL) != 0) {  //  waiting for termination of threads
			warnx("error joining thread");
			return 1;
		}
    }

    for (i = 0; i < Ncores; i++) {

        printf("[%d]\tlatencia media: %09d ns. | max: %09d ns\n",
                dths[i].id, dths[i].latency_avg, dths[i].max_latency);

        av_lat = av_lat + dths[i].latency_avg;
        max_lat = max_lat + dths[i].max_latency;
    }
    printf("\nTotal latencia media: %09ld ns. | max: %09ld ns\n", av_lat/Ncores, max_lat/Ncores);

    fd_csv = open("cyclictestURJC.csv", O_RDWR|O_CREAT|O_TRUNC, 0660);
    if (fd_csv < 0) {
        err(EXIT_FAILURE, "open failed");
    }

    if (dup2(fd_csv, 1) < 0) {
        err(EXIT_FAILURE, "dup2 failed");
    }
    close(fd_csv);
    
    for (i = 0; i < Ncores; i++) {

        for (j = 0; dths[i].latencies[j] != 0; j++) {

            printf("%d, %d, %d\n", i, j, dths[i].latencies[j]);
        }
    }

    exit(EXIT_SUCCESS);
}