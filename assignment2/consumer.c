#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include "buffer.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <consumer_id> <num_items>\n", argv[0]);
        exit(1);
    }

    int consumer_id = atoi(argv[1]);
    int num_items = atoi(argv[2]);

    // Get shared memory (must already exist)
    int shm_id = shmget(SHM_KEY, sizeof(shared_buffer_t), 0666);
    if (shm_id == -1) {
        perror("shmget failed (did you start a producer first?)");
        exit(1);
    }

    // Attach shared memory
    shared_buffer_t *buffer = (shared_buffer_t *) shmat(shm_id, NULL, 0);
    if (buffer == (void *) -1) {
        perror("shmat failed");
        exit(1);
    }

    // Open semaphores
    sem_t *sem_empty = sem_open("/sem_empty", 0);
    sem_t *sem_full  = sem_open("/sem_full",  0);
    sem_t *sem_mutex = sem_open("/sem_mutex", 0);

    if (sem_empty == SEM_FAILED || sem_full == SEM_FAILED || sem_mutex == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }

    for (int i = 0; i < num_items; i++) {
        sem_wait(sem_full);    // wait for item
        sem_wait(sem_mutex);   // enter critical section

        // Remove item from buffer
        item_t item = buffer->buffer[buffer->tail];
        buffer->tail = (buffer->tail + 1) % BUFFER_SIZE;
        buffer->count--;

        printf("Consumer %d: Consumed value %d from Producer %d\n",
               consumer_id, item.value, item.producer_id);

        sem_post(sem_mutex);   // exit critical section
        sem_post(sem_empty);   // signal slot available

        usleep(150000); // small delay to visualize interleaving
    }

    // Cleanup
    shmdt(buffer);
    sem_close(sem_empty);
    sem_close(sem_full);
    sem_close(sem_mutex);

    return 0;
}

