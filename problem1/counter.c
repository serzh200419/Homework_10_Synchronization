#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#define N 4    
#define M 1000000  

long long counter = 0;

pthread_mutex_t mutex;

pthread_spinlock_t spin;

void* worker_no_sync(void* arg) {
    for (int i = 0; i < M; i++)
        counter++;    
    return NULL;
}

void* worker_mutex(void* arg) {
    for (int i = 0; i < M; i++) {
        pthread_mutex_lock(&mutex);
        counter++;     
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void* worker_spin(void* arg) {
    for (int i = 0; i < M; i++) {
        pthread_spin_lock(&spin);
        counter++;
        pthread_spin_unlock(&spin);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s [nosync | mutex | spin]\n", argv[0]);
        return 1;
    }

    pthread_t threads[N];

    if (strcmp(argv[1], "nosync") == 0) {

        for (int i = 0; i < N; i++)
            pthread_create(&threads[i], NULL, worker_no_sync, NULL);

    } else if (strcmp(argv[1], "mutex") == 0) {

        pthread_mutex_init(&mutex, NULL);
        for (int i = 0; i < N; i++)
            pthread_create(&threads[i], NULL, worker_mutex, NULL);

    } else if (strcmp(argv[1], "spin") == 0) {

        pthread_spin_init(&spin, 0);
        for (int i = 0; i < N; i++)
            pthread_create(&threads[i], NULL, worker_spin, NULL);

    } else {
        printf("Invalid mode\n");
        return 1;
    }

    for (int i = 0; i < N; i++)
        pthread_join(threads[i], NULL);

    long long expected = (long long) N * M;
    printf("Expected: %lld\n", expected);
    printf("Actual:   %lld\n", counter);

    if (strcmp(argv[1], "mutex") == 0)
        pthread_mutex_destroy(&mutex);
    if (strcmp(argv[1], "spin") == 0)
        pthread_spin_destroy(&spin);

    return 0;
}

