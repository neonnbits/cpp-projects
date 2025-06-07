#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>

Queue *create_queue() {
    Queue *queue = malloc(sizeof(Queue));
    if (queue == NULL)
        return NULL;

    queue->front = NULL;
    queue->rear = NULL;
    queue->size = 0;

    return queue;
}

void enqueue(Queue *queue, Process *process) {
    QueueNode *node = malloc(sizeof(QueueNode));
    node->process = process;
    node->next = NULL;

    if (queue->rear != NULL) {
        queue->rear->next = node;
    } else {
        queue->front = node;
    }
    queue->rear = node;
    queue->size++;
}

Process *dequeue(Queue *queue) {
    if (queue->front == NULL)
        return NULL;

    QueueNode *node = queue->front;
    Process *process = node->process;

    queue->front = queue->front->next;
    if (queue->front == NULL) {
        queue->rear = NULL;
    }
    free(node);
    queue->size--;
    return process;
}

void free_queue(Queue *queue) {
    while (!is_empty(queue)) {
        dequeue(queue);
    }
    queue->rear = NULL;
    queue->size = 0;
    free(queue);
}

int is_empty(Queue *queue) { return queue->size == 0; }

void print_gantt_chart_step(int current_time, int pid, int time_slice) {
    printf("Time %d-%d: Process P%d\n", current_time, current_time + time_slice,
           pid);
}

void round_robin_schedule(Process processes[], int n, int time_quantum) {
    Queue *queue = create_queue();

    int current_time = 0;
    int completed = 0;

    printf("Starting Round Robin Scheduling...\n");
    printf("Time Quantum: %d\n\n", time_quantum);

    while (completed < n) {
        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time == current_time) {
                enqueue(queue, &processes[i]);
                printf("Time %d: Process P%d arrives\n", current_time,
                       processes[i].pid);
            }
            if (processes[i].arrival_time > current_time) {
                break;
            }
        }
        if (is_empty(queue)) {
            printf("Time %d: CPU idle\n", current_time);
            current_time++;
            continue;
        }

        Process *process = dequeue(queue);
        // set the start time for first time process execution
        if (process->burst_time == process->remaining_time) {
            process->start_time = current_time;
        }

        int execution_time = (process->remaining_time < time_quantum)
                                 ? process->remaining_time
                                 : time_quantum;

        printf("Time %d-%d: Process P%d executes for %d units\n", current_time,
               current_time + execution_time, process->pid, execution_time);

        process->remaining_time -= execution_time;
        current_time += execution_time;

        if (process->remaining_time == 0) {
            process->completion_time = current_time;
            completed++;
            printf("Time %d: Process P%d completed\n", current_time,
                   process->pid);
        } else {
            enqueue(queue, process);
        }

        // add processes that arrived during the execution time
        for (int i = 0; i < n; i++) {
            if (processes[i].arrival_time > current_time - execution_time &&
                processes[i].arrival_time < current_time) {
                enqueue(queue, &processes[i]);
                printf("Time %d: Process P%d arrives during execution\n",
                       current_time, processes[i].pid);
            }
        }
    }
    printf("\nAll processes completed!\n");
    free_queue(queue);
}
