//Emanuel Contreras
//lct377

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_THREADS 10
#define MAX_COUNT 1000000000

// Global variables
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int counter = 0;

void *increment(void *arg) {
    int i = 0;
    while (i < MAX_COUNT / MAX_THREADS) {
        pthread_mutex_lock(&mutex); //acquire the lock to access the shared counter
        counter++; //increment the counter
        pthread_mutex_unlock(&mutex); //release the lock
        i+=1; //This will increment the loop counter.
    }
    pthread_exit(NULL); //This will exit the thread.
}

void *decrement(void *arg) { //Void function that will decrement the counter
    int i = 0; //initializing i to 0
    while (i < MAX_COUNT / MAX_THREADS) {
        pthread_mutex_lock(&mutex); //acquires the lock
        counter--;//Used to decrement the counter.
        pthread_mutex_unlock(&mutex); //This will release the lock
        i+=1; //Will increment the loop counter
    }
    pthread_exit(NULL);//This will exit the thread.
}

int main() {
    pthread_t threads[MAX_THREADS]; //An array to store the thread IDs

    
    for (int i = 0; i < MAX_THREADS / 2; ++i) { //This will create the threads for incrementing
        pthread_create(&threads[i], NULL, increment, NULL);
    }

    
    for (int i = MAX_THREADS / 2; i < MAX_THREADS; ++i) { //This will now create the threads for decrementation.
        pthread_create(&threads[i], NULL, decrement, NULL);
    }

   
    for (int i = 0; i < MAX_THREADS; ++i) {  // Join threads
        pthread_join(threads[i], NULL);
    }

   
    printf("Incrementing counter from 0 to %d using %d threads\n", MAX_COUNT, MAX_THREADS);
    printf("Final value is %d\n", counter);

   
    counter = 0;  // Will reset the counter to a value of 0.

    
    for (int i = 0; i < MAX_THREADS / 2; ++i) { // Join threads again for decrementing
          pthread_join(threads[i + MAX_THREADS / 2], NULL);
      }
    
    printf("Decrementing counter from %d to 0 using %d threads\n", MAX_COUNT, MAX_THREADS);// Prints the final result after decrementing
    printf("Final value is %d\n", counter);

    return 0;
}
