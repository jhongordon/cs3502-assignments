/* Jhon Gordon producer.c*/
#include <stdio.h>      // printf, fprintf
#include <stdlib.h>     // atoi, exit
#include <unistd.h>     // usleep
#include <sys/ipc.h>    // IPC keys
#include <sys/shm.h>    // shared memory
#include <semaphore.h> // POSIX semaphores
#include <fcntl.h>     // O_CREAT
#include "buffer.h"    // shared buffer definitions

int main(int argc, char *argv[]) {
    // Expect: ./producer <producer_id> <num_items>
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <producer_id> <num_items>\n", argv[0]);
        exit(1);
    }

    int producer_id = atoi(argv[1]);
    int num_items   = atoi(argv[2]);

    // Create the shared memory segment for the circular buffer
    int shm_id = shmget(SHM_KEY, sizeof(shared_buffer_t), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget failed");
        exit(1);
    }

    // Attaches the shared memory
    shared_buffer_t *sharedBuf = (shared_buffer_t *) shmat(shm_id, NULL, 0);
    if (sharedBuf == (void *) -1) {
        perror("shmat failed");
        exit(1);
    }

    // Initialize the buffer fields if this is the first process (simple guard)
    if (sharedBuf->count == 0 && sharedBuf->head == 0 && sharedBuf->tail == 0) {
        sharedBuf->head  = 0;
        sharedBuf->tail  = 0;
        sharedBuf->count = 0;
    }

    // Open (or create) named semaphores:
    // sem_empty -> number of empty slots
    // sem_full  -> number of filled slots
    // sem_mutex -> mutual exclusion for buffer access
    sem_t *sem_empty = sem_open("/sem_empty", O_CREAT, 0644, BUFFER_SIZE);
    sem_t *sem_full  = sem_open("/sem_full",  O_CREAT, 0644, 0);
    sem_t *sem_mutex = sem_open("/sem_mutex", O_CREAT, 0644, 1);

    if (sem_empty == SEM_FAILED || sem_full == SEM_FAILED || sem_mutex == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }

    // Produces the requested number of items
    for (int i = 0; i < num_items; i++) {
        int value = producer_id * 1000 + i;

        // Waits for an empty slot, then locks the buffer
        sem_wait(sem_empty);
        sem_wait(sem_mutex);

        // Insert item into the circular buffer
        sharedBuf->buffer[sharedBuf->head].value = value;
        sharedBuf->buffer[sharedBuf->head].producer_id = producer_id;
        sharedBuf->head = (sharedBuf->head + 1) % BUFFER_SIZE;
        sharedBuf->count++;

        printf("Producer %d: Produced value %d\n", producer_id, value);

        // Unlock and signal that an item is available
        sem_post(sem_mutex);
        sem_post(sem_full);

        // Small delay to help visualize interleaving during tests
        usleep(100000);
    }

    // Detach from shared memory and close semaphores
    shmdt(sharedBuf);
    sem_close(sem_empty);
    sem_close(sem_full);
    sem_close(sem_mutex);

    return 0;
}

