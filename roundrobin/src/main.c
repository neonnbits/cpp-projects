#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define n 10

int getRandom(ssize_t limit) {
    FILE *random = fopen("/dev/urandom", "rb");
    unsigned int seed;

    if (random) {
        fread(&seed, sizeof(seed), 1, random);
        fclose(random);
        srand(seed);
    } else {
        srand(1);
    }

    return rand() % limit;
}

int compare_arrival_time(const void *a, const void *b) {
    Process *processA = (Process *)a;
    Process *processB = (Process *)b;

    return processA->arrival_time - processB->arrival_time;
}

int main() {

    Process processes[n];
    int time_quantum = 3;

    for (int i = 0; i < n; i++) {
        processes[i].pid = i;
        processes[i].arrival_time = getRandom(31);
        processes[i].burst_time = getRandom(20);
        processes[i].completion_time = 0;
        processes[i].remaining_time = processes[i].burst_time;
        processes[i].start_time = 0;
    }

    qsort(processes, n, sizeof(Process), compare_arrival_time);
    processes[0].arrival_time = 0;
    processes[0].burst_time = 6;

    printf("Generated Processes:\n");
    printf("PID\tArrival\tBurst\n");
    for (int i = 0; i < n; i++) {
        printf("%d\t%d\t%d\n", processes[i].pid, processes[i].arrival_time,
               processes[i].burst_time);
    }

    printf("\n=== Round Robin Scheduling (Time Quantum: %d) ===\n",
           time_quantum);
    round_robin_schedule(processes, n, time_quantum);

    // After "All processes completed!"
    printf("\n=== Summary ===\n");
    for (int i = 0; i < n; i++) {
        int turnaround =
            processes[i].completion_time - processes[i].arrival_time;
        int waiting = turnaround - processes[i].burst_time;
        printf("P%d: Arrival=%d, Burst=%d, Completion=%d, Turnaround=%d, "
               "Waiting=%d\n",
               processes[i].pid, processes[i].arrival_time,
               processes[i].burst_time, processes[i].completion_time,
               turnaround, waiting);
    }

    return 0;
}