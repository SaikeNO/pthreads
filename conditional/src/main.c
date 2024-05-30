#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "../headers/queue.h"

#define HAIRCUT_TIME 3 * 1000000 // 3 sekundy

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
int infoMode = 0;		// Flaga dla trybu -info
int barberSleeping = 0; // Flaga wskazująca, czy fryzjer śpi

void initializeBarberShop(BarberShop *shop, int capacity)
{
	shop->waitingRoomCapacity = capacity;
	shop->nextSeat = 0;
	shop->rejections = 0;
	if (pthread_mutex_init(&shop->mutex, NULL) != 0)
	{
		perror("Inicjalizacja mutexa nie powiodła się");
		exit(EXIT_FAILURE);
	}
	if (pthread_cond_init(&shop->barberReady, NULL) != 0)
	{
		perror("Inicjalizacja zmiennej warunkowej barberReady nie powiodła się");
		exit(EXIT_FAILURE);
	}
	if (pthread_cond_init(&shop->clientReady, NULL) != 0)
	{
		perror("Inicjalizacja zmiennej warunkowej clientReady nie powiodła się");
		exit(EXIT_FAILURE);
	}
	initializeQueue(&queue, capacity);
}

void cleanupBarberShop(BarberShop *shop)
{
	if (pthread_mutex_destroy(&shop->mutex) != 0)
	{
		perror("Niszczenie mutexa nie powiodło się");
	}
	if (pthread_cond_destroy(&shop->barberReady) != 0)
	{
		perror("Niszczenie zmiennej warunkowej barberReady nie powiodło się");
	}
	if (pthread_cond_destroy(&shop->clientReady) != 0)
	{
		perror("Niszczenie zmiennej warunkowej clientReady nie powiodło się");
	}

	cleanupQueue(&queue);
}

void *client(void *arg)
{
	int id = *(int *)arg;
	usleep(rand() % (HAIRCUT_TIME * 3)); // Losowy czas przybycia

	if (pthread_mutex_lock(&shop.mutex) != 0)
	{
		perror("Błąd przy blokowaniu mutexa");
		pthread_exit(NULL);
	}

	if (queue.size < shop.waitingRoomCapacity)
	{
		enqueue(&queue, id);
		if (infoMode)
		{
			printf("Klient %d wchodzi do poczekalni.\n", id);
		}
		if (pthread_cond_signal(&shop.clientReady) != 0)
		{
			perror("Błąd przy wysyłaniu sygnału clientReady");
			pthread_mutex_unlock(&shop.mutex);
			pthread_exit(NULL);
		}
		if (pthread_cond_wait(&shop.barberReady, &shop.mutex) != 0)
		{
			perror("Błąd przy oczekiwaniu na sygnał barberReady");
			pthread_mutex_unlock(&shop.mutex);
			pthread_exit(NULL);
		}

		if (pthread_mutex_unlock(&shop.mutex) != 0)
		{
			perror("Błąd przy odblokowywaniu mutexa");
			pthread_exit(NULL);
		}

		if (pthread_mutex_lock(&shop.mutex) != 0)
		{
			perror("Błąd przy blokowaniu mutexa");
			pthread_exit(NULL);
		}
		printf("Rezygnacja:%d Poczekalnia: %d/%d [Fotel: %d]\n", shop.rejections, queue.size, shop.waitingRoomCapacity, id);
		if (infoMode)
		{
			displayQueue(&queue);
		}

		if (pthread_mutex_unlock(&shop.mutex) != 0)
		{
			perror("Błąd przy odblokowywaniu mutexa");
			pthread_exit(NULL);
		}

		usleep(HAIRCUT_TIME); // Czas strzyżenia
	}
	else
	{
		shop.rejections++;
		if (infoMode)
		{
			printf("Klient %d odchodzi, brak miejsc w poczekalni.\n", id);
		}
		if (pthread_mutex_unlock(&shop.mutex) != 0)
		{
			perror("Błąd przy odblokowywaniu mutexa");
			pthread_exit(NULL);
		}
	}
	return NULL;
}

void *barber(void *arg)
{
	while (1)
	{
		if (pthread_mutex_lock(&shop.mutex) != 0)
		{
			perror("Błąd przy blokowaniu mutexa");
			pthread_exit(NULL);
		}

		while (queue.size == 0)
		{
			if (!barberSleeping)
			{
				printf("Fryzjer śpi.\n");
				barberSleeping = 1;
			}
			if (pthread_cond_wait(&shop.clientReady, &shop.mutex) != 0)
			{
				perror("Błąd przy oczekiwaniu na sygnał clientReady");
				pthread_mutex_unlock(&shop.mutex);
				pthread_exit(NULL);
			}
		}

		if (barberSleeping)
		{
			printf("Fryzjer się budzi.\n");
			barberSleeping = 0;
		}
		int clientID = dequeue(&queue);
		if (clientID == -1)
		{
			perror("Błąd przy zdejmowaniu klienta z kolejki");
			pthread_mutex_unlock(&shop.mutex);
			pthread_exit(NULL);
		}
		if (pthread_cond_signal(&shop.barberReady) != 0)
		{
			perror("Błąd przy wysyłaniu sygnału barberReady");
			pthread_mutex_unlock(&shop.mutex);
			pthread_exit(NULL);
		}
		if (pthread_mutex_unlock(&shop.mutex) != 0)
		{
			perror("Błąd przy odblokowywaniu mutexa");
			pthread_exit(NULL);
		}

		if (pthread_mutex_lock(&shop.mutex) != 0)
		{
			perror("Błąd przy blokowaniu mutexa");
			pthread_exit(NULL);
		}
		printf("Fryzjer obsługuje klienta %d\n", clientID);
		if (pthread_mutex_unlock(&shop.mutex) != 0)
		{
			perror("Błąd przy odblokowywaniu mutexa");
			pthread_exit(NULL);
		}

		usleep(HAIRCUT_TIME); // Czas strzyżenia
	}
	return NULL;
}

int main(int argc, char *argv[])
{
	if (argc < 3)
	{
		fprintf(stderr, "Użycie: %s <liczba_klientów> <liczba_miejsc> [-info]\n", argv[0]);
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

	if (pthread_create(&barberThread, NULL, barber, NULL) != 0)
	{
		perror("Błąd przy tworzeniu wątku fryzjera");
		cleanupBarberShop(&shop);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < numClients; ++i)
	{
		clientIDs[i] = i + 1;
		if (pthread_create(&clientThreads[i], NULL, client, &clientIDs[i]) != 0)
		{
			perror("Błąd przy tworzeniu wątku klienta");
			cleanupBarberShop(&shop);
			exit(EXIT_FAILURE);
		}
	}

	for (int i = 0; i < numClients; ++i)
	{
		if (pthread_join(clientThreads[i], NULL) != 0)
		{
			perror("Błąd przy oczekiwaniu na zakończenie wątku klienta");
		}
	}

	if (pthread_cancel(barberThread) != 0)
	{
		perror("Błąd przy anulowaniu wątku fryzjera");
	}
	if (pthread_join(barberThread, NULL) != 0)
	{
		perror("Błąd przy oczekiwaniu na zakończenie wątku fryzjera");
	}

	cleanupBarberShop(&shop);

	return 0;
}
