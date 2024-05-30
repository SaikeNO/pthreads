#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "../headers/queue.h"

#define HAIRCUT_TIME 3 * 1000000 // 3 seconds

typedef struct
{
	int waitingRoomCapacity;
	int nextSeat;
	int rejections;
	pthread_mutex_t mutex;
	pthread_cond_t barberReady;
	pthread_cond_t clientReady;
} BarberShop;

BarberShop shop;
Queue queue;
int infoMode = 0;		// Flag for -info mode
int barberSleeping = 0; // Flag to indicate if barber is sleeping

void initializeBarberShop(BarberShop *shop, int capacity)
{
	shop->waitingRoomCapacity = capacity;
	shop->nextSeat = 0;
	shop->rejections = 0;
	pthread_mutex_init(&shop->mutex, NULL);
	pthread_cond_init(&shop->barberReady, NULL);
	pthread_cond_init(&shop->clientReady, NULL);
	initializeQueue(&queue, capacity);
}

void cleanupBarberShop(BarberShop *shop)
{
	pthread_mutex_destroy(&shop->mutex);
	pthread_cond_destroy(&shop->barberReady);
	pthread_cond_destroy(&shop->clientReady);
	cleanupQueue(&queue);
}

void *client(void *arg)
{
	int id = *(int *)arg;
	usleep(rand() % (HAIRCUT_TIME * 3)); // Random arrival time

	pthread_mutex_lock(&shop.mutex);

	if (queue.size < shop.waitingRoomCapacity)
	{
		enqueue(&queue, id);
		if (infoMode)
		{
			printf("Klient %d wchodzi do poczekalni.\n", id);
		}
		pthread_cond_signal(&shop.clientReady);
		pthread_cond_wait(&shop.barberReady, &shop.mutex);

		pthread_mutex_unlock(&shop.mutex);

		pthread_mutex_lock(&shop.mutex);
		printf("Rezygnacja:%d Poczekalnia: %d/%d [Fotel: %d]\n", shop.rejections, queue.size, shop.waitingRoomCapacity, id);
		if (infoMode)
		{
			displayQueue(&queue);
		}

		pthread_mutex_unlock(&shop.mutex);

		usleep(HAIRCUT_TIME); // Haircut time
	}
	else
	{
		shop.rejections++;
		if (infoMode)
		{
			printf("Klient %d odchodzi, brak miejsc w poczekalni.\n", id);
		}
		pthread_mutex_unlock(&shop.mutex);
	}
	return NULL;
}

void *barber(void *arg)
{
	while (1)
	{
		pthread_mutex_lock(&shop.mutex);

		while (queue.size == 0)
		{
			if (!barberSleeping)
			{
				printf("Fryzjer śpi.\n");
				barberSleeping = 1;
			}
			pthread_cond_wait(&shop.clientReady, &shop.mutex);
		}

		if (barberSleeping)
		{
			printf("Fryzjer się budzi.\n");
			barberSleeping = 0;
		}
		int clientID = dequeue(&queue);
		pthread_cond_signal(&shop.barberReady);
		pthread_mutex_unlock(&shop.mutex);

		pthread_mutex_lock(&shop.mutex);
		printf("Fryzjer obsługuje klienta %d\n", clientID);
		pthread_mutex_unlock(&shop.mutex);

		usleep(HAIRCUT_TIME); // Haircut time
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		fprintf(stderr, "Usage: %s <num_clients> <num_seats> [-info]\n", argv[0]);
		return 1;
	}

	int numClients = atoi(argv[1]);
	int numSeats = atoi(argv[2]);
	if (argc == 4 && strcmp(argv[3], "-info") == 0)
	{
		infoMode = 1;
	}

	initializeBarberShop(&shop, numSeats);

	pthread_t barberThread;
	pthread_t clientThreads[numClients];
	int clientIDs[numClients];

	srand(time(NULL));

	pthread_create(&barberThread, NULL, barber, NULL);

	for (int i = 0; i < numClients; ++i)
	{
		clientIDs[i] = i + 1;
		pthread_create(&clientThreads[i], NULL, client, &clientIDs[i]);
	}

	for (int i = 0; i < numClients; ++i)
	{
		pthread_join(clientThreads[i], NULL);
	}

	pthread_cancel(barberThread);
	pthread_join(barberThread, NULL);

	cleanupBarberShop(&shop);

	return 0;
}
