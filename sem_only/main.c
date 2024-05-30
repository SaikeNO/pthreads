#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define HAIRCUT_TIME 3 * 1000000 // 3 seconds

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
int infoMode = 0;       // Flag for -info mode
int barberSleeping = 0; // Flag to indicate if barber is sleeping

void initializeBarberShop(BarberShop *shop, int capacity)
{
    shop->waitingRoomCapacity = capacity;
    shop->waitingRoom = (int *)malloc(capacity * sizeof(int));
    shop->nextSeat = 0;
    shop->rejections = 0;
    pthread_mutex_init(&shop->mutex, NULL);
    sem_init(&shop->barberReady, 0, 0);
    sem_init(&shop->clientReady, 0, 0);
    sem_init(&shop->accessSeats, 0, 1);
}

void cleanupBarberShop(BarberShop *shop)
{
    free(shop->waitingRoom);
    pthread_mutex_destroy(&shop->mutex);
    sem_destroy(&shop->barberReady);
    sem_destroy(&shop->clientReady);
    sem_destroy(&shop->accessSeats);
}

void *client(void *arg)
{
    int id = *(int *)arg;
    usleep(rand() % (HAIRCUT_TIME * 3)); // Random arrival time

    sem_wait(&shop.accessSeats);

    if (shop.nextSeat < shop.waitingRoomCapacity)
    {
        shop.waitingRoom[shop.nextSeat++] = id;
        if (infoMode)
        {
            printf("Klient %d wchodzi do poczekalni.\n", id);
        }
        sem_post(&shop.clientReady);
        sem_post(&shop.accessSeats);
        sem_wait(&shop.barberReady);

        pthread_mutex_lock(&shop.mutex);
        printf("Rezygnacja:%d Poczekalnia: %d/%d [Fotel: %d]\n", shop.rejections, shop.nextSeat, shop.waitingRoomCapacity, id);
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
        sem_post(&shop.accessSeats);
    }
    return NULL;
}

void *barber(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&shop.mutex);

        if (shop.nextSeat == 0 && infoMode)
        {
            if (!barberSleeping)
            {
                printf("Fryzjer śpi.\n");
                barberSleeping = 1;
            }
        }

        sem_wait(&shop.clientReady);
        sem_wait(&shop.accessSeats);

        if (barberSleeping && infoMode)
        {
            printf("Fryzjer się budzi.\n");
            barberSleeping = 0;
        }
        pthread_mutex_unlock(&shop.mutex);

        int clientID = shop.waitingRoom[--shop.nextSeat];
        sem_post(&shop.barberReady);
        sem_post(&shop.accessSeats); // Release the mutex

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
