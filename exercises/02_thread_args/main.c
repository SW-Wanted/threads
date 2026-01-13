#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

// I want to you to create an example of threads with args as a structure
typedef struct {
    int thread_id;
    const char* message;
} thread_data_t;
void* thread_function(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    printf("Thread ID: %d, Message: %s\n", data->thread_id, data->message);
    return NULL;
}
int main() {
    const int NUM_THREADS = 3;
    pthread_t threads[NUM_THREADS];
    thread_data_t thread_data[NUM_THREADS];

    // Initialize thread data and create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].message = "Hello from thread!";
        pthread_create(&threads[i], NULL, thread_function, (void*)&thread_data[i]);
    }

    // Wait for all threads to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
