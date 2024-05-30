#ifndef QUEUE_H
#define QUEUE_H

typedef struct
{
    int *buffer;
    int capacity;
    int size;
    int front;
    int rear;
} Queue;

void initializeQueue(Queue *q, int capacity);
void cleanupQueue(Queue *q);
void enqueue(Queue *q, int item);
int dequeue(Queue *q);
void displayQueue(Queue *q);

#endif /* QUEUE_H */