
// false_sharing_mt.c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

#define CACHE_LINE_SIZE 64

int get_valid_iterations(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <iterations>\n", argv[0]);
        fprintf(stderr, "Example: %s 1000\n", argv[0]);
        exit(1);
    }

    // Check if parameter is a pure number
    int valid = 1;
    for (int i = 0; argv[1][i] != '\0'; i++) {
        if (argv[1][i] < '0' || argv[1][i] > '9') {
            valid = 0;
            break;
        }
    }

    if (!valid) {
        fprintf(stderr, "Error: Iterations must be a positive integer.\n");
        exit(1);
    }

    int iterations = atoi(argv[1]);

    if (iterations <= 0) {
        fprintf(stderr, "Error: Iterations must be greater than 0.\n");
        exit(1);
    } else if (iterations > 10000000) {
        fprintf(stderr, "Error: Iterations too large, please enter a number not exceeding 10000000.\n");
        exit(1);
    }

    // printf("Iterations: %d\n", iterations);
    return iterations;
}

/* Put each frequently updated counter on its own cache line. */
struct CounterSlot {
    _Alignas(CACHE_LINE_SIZE) volatile atomic_int value;
};

struct GoodCounter {
    struct CounterSlot counter0;
    struct CounterSlot counter1;
};

struct ThreadData {
    struct GoodCounter *counter;
    int thread_id;
    int iterations;
};

void* increment_counter(void* arg) {
    struct ThreadData* data = (struct ThreadData*)arg;

    if (data->thread_id == 0) {
        // Thread 0 - access counter0
        for (int i = 0; i < data->iterations; i++) {
            atomic_fetch_add(&data->counter->counter0.value, 1);
        }
    } else {
        // Thread 1 - access counter1
        for (int i = 0; i < data->iterations; i++) {
            atomic_fetch_add(&data->counter->counter1.value, 1);
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    int iterations = get_valid_iterations(argc, argv);

    struct GoodCounter *counter = aligned_alloc(CACHE_LINE_SIZE, sizeof(struct GoodCounter));
    if (counter == NULL) {
        fprintf(stderr, "Error: failed to allocate aligned counter storage.\n");
        exit(1);
    }
    atomic_init(&counter->counter0.value, 0);
    atomic_init(&counter->counter1.value, 0);

    pthread_t thread0, thread1;
    struct ThreadData data0, data1;

    data0.counter = counter;
    data0.thread_id = 0;
    data0.iterations = iterations;
    data1.counter = counter;
    data1.thread_id = 1;
    data1.iterations = iterations;

    // Create two threads
    pthread_create(&thread0, NULL, increment_counter, &data0);
    pthread_create(&thread1, NULL, increment_counter, &data1);

    // Wait for threads to complete
    pthread_join(thread0, NULL);
    pthread_join(thread1, NULL);

    printf("counter0: %d, counter1: %d\n", counter->counter0.value, counter->counter1.value);
    free(counter);

    return 0;
}
