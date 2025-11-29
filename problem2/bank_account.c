#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#define DEPOSIT_THREADS 4
#define WITHDRAW_THREADS 4
#define OPERATIONS 500000  

long long balance = 0;

pthread_mutex_t mutex;
pthread_spinlock_t spin;

int use_mutex = 1;   
int long_cs = 0; 

void lock() {
    if (use_mutex)
        pthread_mutex_lock(&mutex);
    else
        pthread_spin_lock(&spin);
}

void unlock() {
    if (use_mutex)
        pthread_mutex_unlock(&mutex);
    else
        pthread_spin_unlock(&spin);
}

void* deposit_thread(void* arg) {
    for (int i = 0; i < OPERATIONS; i++) {
        lock();

        balance++;

        if (long_cs)
            usleep(100);

        unlock();
    }
    return NULL;
}

void* withdraw_thread(void* arg) {
    for (int i = 0; i < OPERATIONS; i++) {
        lock();

        balance--;

        if (long_cs)
            usleep(100);

        unlock();
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s [mutex|spin] [short|long]\n", argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "mutex") == 0)
        use_mutex = 1;
    else if (strcmp(argv[1], "spin") == 0)
        use_mutex = 0;
    else {
        printf("Invalid lock type.\n");
        return 1;
    }

    if (strcmp(argv[2], "short") == 0)
        long_cs = 0;
    else if (strcmp(argv[2], "long") == 0)
        long_cs = 1;
    else {
        printf("Invalid critical section mode.\n");
        return 1;
    }

    if (use_mutex)
        pthread_mutex_init(&mutex, NULL);
    else
        pthread_spin_init(&spin, 0);

    pthread_t dep[DEPOSIT_THREADS], wdr[WITHDRAW_THREADS];

    struct timeval start, end;
    gettimeofday(&start, NULL);

    for (int i = 0; i < DEPOSIT_THREADS; i++)
        pthread_create(&dep[i], NULL, deposit_thread, NULL);
    for (int i = 0; i < WITHDRAW_THREADS; i++)
        pthread_create(&wdr[i], NULL, withdraw_thread, NULL);

    for (int i = 0; i < DEPOSIT_THREADS; i++)
        pthread_join(dep[i], NULL);
    for (int i = 0; i < WITHDRAW_THREADS; i++)
        pthread_join(wdr[i], NULL);

    gettimeofday(&end, NULL);

    double elapsed =
        (end.tv_sec - start.tv_sec) +
        (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("\nRESULT:\n");
    printf("Final balance: %lld\n", balance);
    printf("Elapsed time: %.4f sec\n", elapsed);

    if (use_mutex)
        pthread_mutex_destroy(&mutex);
    else
        pthread_spin_destroy(&spin);

    return 0;
}

