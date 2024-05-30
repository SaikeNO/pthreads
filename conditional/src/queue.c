#include <stdio.h>
#include <stdlib.h>
#include "../headers/queue.h"

void initializeQueue(Queue *q, int capacity)
{
	q->buffer = (int *)malloc(capacity * sizeof(int));
	q->capacity = capacity;
	q->size = 0;
	q->front = 0;
	q->rear = -1;
}

void cleanupQueue(Queue *q)
{
	free(q->buffer);
}

void enqueue(Queue *q, int item)
{
	if (q->size == q->capacity)
	{
		fprintf(stderr, "Kolejka jest pelna.\n");
		return;
	}
	q->rear = (q->rear + 1) % q->capacity;
	q->buffer[q->rear] = item;
	q->size++;
}

int dequeue(Queue *q)
{
	if (q->size == 0)
	{
		fprintf(stderr, "Kolejka jest pusta.\n");
		return -1;
	}
	int item = q->buffer[q->front];
	q->front = (q->front + 1) % q->capacity;
	q->size--;
	return item;
}

void displayQueue(Queue *q)
{
	if (q->size == 0)
	{
		printf("Kolejka jest pusta.\n");
		return;
	}

	int index = q->rear;
	printf("Czekajcy klienci: ");
	for (int i = 0; i < q->size; ++i)
	{
		printf("%d ", q->buffer[index]);
		index = (index - 1 + q->capacity) % q->capacity;
	}
	printf("\n");
}