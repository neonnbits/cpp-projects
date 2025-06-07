#ifndef SCHEDULER_H
#define SCHEDULER_H

// Process structure
typedef struct {
    int pid;             // Process ID
    int arrival_time;    // When process arrives
    int burst_time;      // Total time needed to complete
    int remaining_time;  // Time left to complete
    int start_time;      // When process first starts executing
    int completion_time; // When process finishes
} Process;

// Queue node for ready queue
typedef struct QueueNode {
    Process *process;
    struct QueueNode *next;
} QueueNode;

// Queue structure
typedef struct {
    QueueNode *front;
    QueueNode *rear;
    int size;
} Queue;

// Function declarations
void round_robin_schedule(Process processes[], int n, int time_quantum);
Queue *create_queue(void);
void enqueue(Queue *q, Process *process);
Process *dequeue(Queue *q);
int is_empty(Queue *q);
void free_queue(Queue *q);
void print_gantt_chart_step(int current_time, int pid, int time_slice);

#endif