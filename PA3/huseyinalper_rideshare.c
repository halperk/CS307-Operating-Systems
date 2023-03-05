// Huseyin Alper Karadeniz (28029) - CS 307 PA3

#include <stdlib.h>
#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdbool.h>

int waitingFanCountA = 0, waitingFanCountB = 0;  // number of fans waiting in the queue from Club A and B
sem_t semA, semB; // semaphores to wait/signal for the fans of Club A and B
pthread_mutex_t mutex; // mutex to prevent issues in queues
pthread_barrier_t barrier; // barrier to ensure that the car is being full

void* rideFuncA() {
    
    pthread_mutex_lock(&mutex); // locks the shared mutex
    
    printf("Thread ID, %ld, Team A, I am looking for a car\n", pthread_self());
    
    waitingFanCountA++; // increments the number of waiting fans from Club A
    bool captainFan;
    
    if (waitingFanCountA == 4) { // 4 A fans - 0 B fans car band formation
        
        captainFan  = true;
        waitingFanCountA -= 4;
        
        sem_post(&semA);
        sem_post(&semA);
        sem_post(&semA);
        sem_post(&semA);
        
    } else {
        
        if ((waitingFanCountA == 3 || waitingFanCountA == 2) && waitingFanCountB >= 2) { // 2 A fans - 2 B fans car band formation
            
            captainFan = true;
            waitingFanCountA -= 2;
            waitingFanCountB -= 2;
        
            sem_post(&semA);
            sem_post(&semA);
            sem_post(&semB);
            sem_post(&semB);
            
        } else { // no car band formation yet, will wait for other fans
            
            captainFan = false;
            pthread_mutex_unlock(&mutex); // unlocks the shared mutex
            
        }
    }
    
    sem_wait(&semA); // waits for the formation of a car band
    
    printf("Thread ID: %ld, Team: A, I have found a spot in a car\n", pthread_self());
    
    pthread_barrier_wait(&barrier); // ensures all related fan threads printed their messages
    
    if (captainFan) { // the fan is the captain
        
        printf("Thread ID: %ld, Team: A, I am the captain and driving the car\n", pthread_self());
        captainFan = false;
        pthread_mutex_unlock(&mutex); // unlocks the shared mutex
        
    }
    
    return NULL;
}

void* rideFuncB() {
    
    pthread_mutex_lock(&mutex); // locks the shared mutex
    
    printf("Thread ID, %ld, Team B, I am looking for a car\n", pthread_self());
    
    waitingFanCountB++; // increments the number of waiting fans from Club B
    bool captainFan;
    
    if (waitingFanCountB == 4) { // 4 B fans - 0 A fans car band formation
        
        captainFan  = true;
        waitingFanCountB -= 4;
        
        sem_post(&semB);
        sem_post(&semB);
        sem_post(&semB);
        sem_post(&semB);
        
    } else {
        
        if ((waitingFanCountB == 3 || waitingFanCountB == 2) && waitingFanCountA >= 2) { // 2 B fans - 2 A fans car band formation
            
            captainFan = true;
            waitingFanCountB -= 2;
            waitingFanCountA -= 2;
        
            sem_post(&semB);
            sem_post(&semB);
            sem_post(&semA);
            sem_post(&semA);
            
        } else { // no car band formation yet, will wait for other fans
            
            captainFan = false;
            pthread_mutex_unlock(&mutex); // unlocks the shared mutex
            
        }
    }
    
    sem_wait(&semB); // waits for the formation of a car band
    
    printf("Thread ID: %ld, Team: B, I have found a spot in a car\n", pthread_self());
    
    pthread_barrier_wait(&barrier); // ensures all related fan threads printed their messages
    
    if (captainFan) { // the fan is the captain
        
        printf("Thread ID: %ld, Team: B, I am the captain and driving the car\n", pthread_self());
        captainFan = false;
        pthread_mutex_unlock(&mutex); // unlocks the shared mutex
        
    }
    
    return NULL;
}

int main(int argc, char* argv[]) {
    
    // gets the number of fans from each club as a console input
    int fanCountA, fanCountB;
    
    fanCountA = atoi(argv[1]);
    fanCountB = atoi(argv[2]);
    
    // creates the validation conditions
    bool correctTotalNumberAll, correctTotalNumberA, correctTotalNumberB;
    
    correctTotalNumberAll = (((fanCountA + fanCountB) % 4 ) == 0);
    correctTotalNumberA = ((fanCountA % 2) == 0);
    correctTotalNumberB = ((fanCountB % 2) == 0);
    
    // checks the validity of the inputs given by the user
    if (correctTotalNumberAll && correctTotalNumberA && correctTotalNumberA) {
        
        // initializes the semaphores and barrier
        sem_init(&semA, 0, 0);
        sem_init(&semB, 0, 0);
        pthread_barrier_init(&barrier, NULL, 4);
        
        // dynamically allocates space in memory for all threads
        pthread_t* threads = (pthread_t*) malloc((fanCountA + fanCountB) * sizeof(pthread_t));
        
        int i;

        // creates the threads for the fans of Club A
        for (i = 0; i < fanCountA; i++) {

            pthread_create(&threads[i], NULL, rideFuncA, NULL);

        }

        // creates the threads for the fans of Club B
        for (i = fanCountA; i < (fanCountA + fanCountB); i++) {
            
            pthread_create(&threads[i], NULL, rideFuncB, NULL);
            
        }

        // joins all threads with each other, so that the main thread waits for all
        for (i = 0; i < (fanCountA + fanCountB); i++) {
            
            pthread_join(threads[i], NULL);
            
        }
        
        free(threads); // frees up allocated space for the threads before
        
        printf("The main terminates\n"); // terminates successfully after execution of threads
        
    } else {
        
        printf("The main terminates\n"); // terminates successfully since the inputs are invalid
        
    }
    
    return 0;
    
}
