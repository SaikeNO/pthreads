#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define HAIRCUT_TIME 3 // 3 sekundy

typedef struct
{
	int waitingRoomCapacity;
	int *waitingRoom;
	int nextSeat;
	int rejections;
	pthread_mutex_t mutex;
	sem_t barberReady;
	sem_t clientReady;
	sem_t accessSeats;
} BarberShop;

BarberShop shop;
int infoMode = 0;		// Flaga dla trybu -info
int barberSleeping = 0; // Flaga wskazująca, czy fryzjer śpi

void initializeBarberShop(BarberShop *shop, int capacity)
{
	shop->waitingRoomCapacity = capacity;
	shop->waitingRoom = (int *)malloc(capacity * sizeof(int));
	if (shop->waitingRoom == NULL)
	{
		perror("Nie udało się zaalokować pamięci dla poczekalni");
		exit(EXIT_FAILURE);
	}
	shop->nextSeat = 0;
	shop->rejections = 0;
	if (pthread_mutex_init(&shop->mutex, NULL) != 0)
	{
		perror("Inicjalizacja mutexa nie powiodła się");
		exit(EXIT_FAILURE);
	}
	if (sem_init(&shop->barberReady, 0, 0) != 0)
	{
		perror("Inicjalizacja semafora barberReady nie powiodła się");
		exit(EXIT_FAILURE);
	}
	if (sem_init(&shop->clientReady, 0, 0) != 0)
	{
		perror("Inicjalizacja semafora clientReady nie powiodła się");
		exit(EXIT_FAILURE);
	}
	if (sem_init(&shop->accessSeats, 0, 1) != 0)
	{
		perror("Inicjalizacja semafora accessSeats nie powiodła się");
		exit(EXIT_FAILURE);
	}
}

void cleanupBarberShop(BarberShop *shop)
{
	free(shop->waitingRoom);
	if (pthread_mutex_destroy(&shop->mutex) != 0)
	{
		perror("Niszczenie mutexa nie powiodło się");
	}
	if (sem_destroy(&shop->barberReady) != 0)
	{
		perror("Niszczenie semafora barberReady nie powiodło się");
	}
	if (sem_destroy(&shop->clientReady) != 0)
	{
		perror("Niszczenie semafora clientReady nie powiodło się");
	}
	if (sem_destroy(&shop->accessSeats) != 0)
	{
		perror("Niszczenie semafora accessSeats nie powiodło się");
	}
}

void *client(void *arg)
{
	int id = *(int *)arg;
	sleep(rand() % (HAIRCUT_TIME * 3)); // Losowy czas przybycia

	if (sem_wait(&shop.accessSeats) != 0)
	{
		perror("Błąd przy oczekiwaniu na semafor accessSeats");
		pthread_exit(NULL);
	}

	if (shop.nextSeat < shop.waitingRoomCapacity)
	{
		shop.waitingRoom[shop.nextSeat++] = id;
		if (infoMode)
		{
			printf("Klient %d wchodzi do poczekalni.\n", id);
		}
		if (sem_post(&shop.clientReady) != 0)
		{
			perror("Błąd przy podnoszeniu semafora clientReady");
			pthread_exit(NULL);
		}
		if (sem_post(&shop.accessSeats) != 0)
		{
			perror("Błąd przy podnoszeniu semafora accessSeats");
			pthread_exit(NULL);
		}
		if (sem_wait(&shop.barberReady) != 0)
		{
			perror("Błąd przy oczekiwaniu na semafor barberReady");
			pthread_exit(NULL);
		}

		if (pthread_mutex_lock(&shop.mutex) != 0)
		{
			perror("Błąd przy blokowaniu mutexa");
			pthread_exit(NULL);
		}
		printf("Rezygnacja: %d Poczekalnia: %d/%d [Fotel: %d]\n", shop.rejections, shop.nextSeat, shop.waitingRoomCapacity, id);
		if (pthread_mutex_unlock(&shop.mutex) != 0)
		{
			perror("Błąd przy odblokowywaniu mutexa");
			pthread_exit(NULL);
		}

		sleep(HAIRCUT_TIME); // Czas strzyżenia
	}
	else
	{
		shop.rejections++;
		if (infoMode)
		{
			printf("Klient %d odchodzi, brak miejsc w poczekalni.\n", id);
		}
		if (sem_post(&shop.accessSeats) != 0)
		{
			perror("Błąd przy podnoszeniu semafora accessSeats");
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

		if (shop.nextSeat == 0 && infoMode)
		{
			if (!barberSleeping)
			{
				printf("Fryzjer śpi.\n");
				barberSleeping = 1;
			}
		}

		if (sem_wait(&shop.clientReady) != 0)
		{
			perror("Błąd przy oczekiwaniu na semafor clientReady");
			pthread_exit(NULL);
		}
		if (sem_wait(&shop.accessSeats) != 0)
		{
			perror("Błąd przy oczekiwaniu na semafor accessSeats");
			pthread_exit(NULL);
		}

		if (barberSleeping && infoMode)
		{
			printf("Fryzjer się budzi.\n");
			barberSleeping = 0;
		}
		if (pthread_mutex_unlock(&shop.mutex) != 0)
		{
			perror("Błąd przy odblokowywaniu mutexa");
			pthread_exit(NULL);
		}

		int clientID = shop.waitingRoom[--shop.nextSeat];
		if (sem_post(&shop.barberReady) != 0)
		{
			perror("Błąd przy podnoszeniu semafora barberReady");
			pthread_exit(NULL);
		}
		if (sem_post(&shop.accessSeats) != 0)
		{
			perror("Błąd przy podnoszeniu semafora accessSeats");
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

		sleep(HAIRCUT_TIME); // Czas strzyżenia
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
