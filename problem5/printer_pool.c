#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define K 3
#define N 10  

sem_t printers;
pthread_mutex_t counter_mutex;

int active_printers = 0;

void* print_job(void* arg) {
    int id = *(int*)arg;

    sem_wait(&printers);

    pthread_mutex_lock(&counter_mutex);
    active_printers++;
    int current = active_printers;
    pthread_mutex_unlock(&counter_mutex);

    printf("Thread %d is printing... (active printers = %d)\n", id, current);

    usleep(200000); 

    pthread_mutex_lock(&counter_mutex);
    active_printers--;
    pthread_mutex_unlock(&counter_mutex);

    sem_post(&printers);

    printf("Thread %d finished printing.\n", id);
    return NULL;
}

int main() {
    pthread_t threads[N];
    int ids[N];

    sem_init(&printers, 0, K);
    pthread_mutex_init(&counter_mutex, NULL);

    for (int i = 0; i < N; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, print_job, &ids[i]);
    }

    for (int i = 0; i < N; i++)
        pthread_join(threads[i], NULL);

    sem_destroy(&printers);
    pthread_mutex_destroy(&counter_mutex);

    printf("\nAll print jobs completed.\n");

    return 0;
}

