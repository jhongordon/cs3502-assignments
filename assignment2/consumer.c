/* Jhon Gordon Section W03 consumer.c*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include "buffer.h"

int main(int argc, char *argv[]) {

    // Checks if the user provided the correct number of arguments
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <consumer_id> <num_items>\n", argv[0]);
        exit(1);
    }

    // Converts arguments into integers
    int consumer_id = atoi(argv[1]);
    int num_items = atoi(argv[2]);

    // Accesses the shared memory segment created by the producer
    int shm_id = shmget(SHM_KEY, sizeof(shared_buffer_t), 0666);
    if (shm_id == -1) {
        perror("shmget failed (shared memory may not exist yet)");
        exit(1);
    }

    // Attaches the shared memory
    shared_buffer_t *sharedBuf = (shared_buffer_t *) shmat(shm_id, NULL, 0);
    if (sharedBuf == (void *) -1) {
        perror("shmat failed");
        exit(1);
    }

    // Open the existing named semaphores created by the producer
    sem_t *semEmpty = sem_open("/sem_empty", 0);  // counts empty slots
    sem_t *semFull  = sem_open("/sem_full",  0);  // counts filled slots
    sem_t *semMutex = sem_open("/sem_mutex", 0);  // mutual exclusion lock

    if (semEmpty == SEM_FAILED || semFull == SEM_FAILED || semMutex == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }

    // Consume the requested number of items
    for (int i = 0; i < num_items; i++) {

        // Wait until there is at least one item available
        sem_wait(semFull);

        // Lock the buffer so only one process can modify it
        sem_wait(semMutex);

        // Remove the next item from the circular buffer
        item_t item = sharedBuf->buffer[sharedBuf->tail];
        sharedBuf->tail = (sharedBuf->tail + 1) % BUFFER_SIZE;
        sharedBuf->count--;

        // Print what was consumed
        printf("Consumer %d: Consumed value %d from Producer %d\n",
               consumer_id, item.value, item.producer_id);

        // Unlock the buffer
        sem_post(semMutex);

        // Shows it's free now
        sem_post(semEmpty);

        // Slowing down output slightly so interleaving is visible
        usleep(150000);
    }

    // Detach from shared memory and close semaphores
    shmdt(sharedBuf);
    sem_close(semEmpty);
    sem_close(semFull);
    sem_close(semMutex);

    return 0;
}

