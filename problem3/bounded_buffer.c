#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 8
int buffer[BUFFER_SIZE];
int in_pos = 0;
int out_pos = 0;

sem_t empty_slots;
sem_t full_slots;
pthread_mutex_t buffer_mutex;

#define PRODUCERS 3
#define CONSUMERS 3
#define ITEMS_PER_PRODUCER 20

#define POISON_PILL -1

void* producer(void* arg) {
    int id = *(int*)arg;

    for (int i = 0; i < ITEMS_PER_PRODUCER; i++) {
        int item = id * 1000 + i;

        sem_wait(&empty_slots);
        pthread_mutex_lock(&buffer_mutex);

        buffer[in_pos] = item;
        in_pos = (in_pos + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&buffer_mutex);
        sem_post(&full_slots);
    }

    return NULL;
}

void* consumer(void* arg) {
    int id = *(int*)arg;

    while (1) {
        sem_wait(&full_slots);
        pthread_mutex_lock(&buffer_mutex);

        int item = buffer[out_pos];
        out_pos = (out_pos + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&buffer_mutex);
        sem_post(&empty_slots);

        if (item == POISON_PILL) {
            break;
        }

        printf("Consumer %d consumed item %d\n", id, item);
    }

    return NULL;
}

int main() {
    pthread_t prod[PRODUCERS], cons[CONSUMERS];
    int prod_id[PRODUCERS], cons_id[CONSUMERS];

    sem_init(&empty_slots, 0, BUFFER_SIZE);
    sem_init(&full_slots, 0, 0);
    pthread_mutex_init(&buffer_mutex, NULL);

    for (int i = 0; i < CONSUMERS; i++) {
        cons_id[i] = i;
        pthread_create(&cons[i], NULL, consumer, &cons_id[i]);
    }

    for (int i = 0; i < PRODUCERS; i++) {
        prod_id[i] = i;
        pthread_create(&prod[i], NULL, producer, &prod_id[i]);
    }

    for (int i = 0; i < PRODUCERS; i++)
        pthread_join(prod[i], NULL);

    for (int i = 0; i < CONSUMERS; i++) {
        sem_wait(&empty_slots);
        pthread_mutex_lock(&buffer_mutex);

        buffer[in_pos] = POISON_PILL;
        in_pos = (in_pos + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&buffer_mutex);
        sem_post(&full_slots);
    }

    for (int i = 0; i < CONSUMERS; i++)
        pthread_join(cons[i], NULL);

    printf("\nAll items produced and consumed.\n");

    sem_destroy(&empty_slots);
    sem_destroy(&full_slots);
    pthread_mutex_destroy(&buffer_mutex);

    return 0;
}

