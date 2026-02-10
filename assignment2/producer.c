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
        fprintf(stderr, "Usage: %s <producer_id> <num_items>\n", argv[0]);
        exit(1);
    }

    int producer_id = atoi(argv[1]);
    int num_items = atoi(argv[2]);

    // Create or get shared memory
    int shm_id = shmget(SHM_KEY, sizeof(shared_buffer_t), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget failed");
        exit(1);
    }

    // Attach shared memory
    shared_buffer_t *buffer = (shared_buffer_t *) shmat(shm_id, NULL, 0);
    if (buffer == (void *) -1) {
        perror("shmat failed");
        exit(1);
    }

    // Initialize buffer if first time (simple check)
    if (buffer->count == 0 && buffer->head == 0 && buffer->tail == 0) {
        buffer->head = 0;
        buffer->tail = 0;
        buffer->count = 0;
    }

    // Open semaphores
    sem_t *sem_empty = sem_open("/sem_empty", O_CREAT, 0644, BUFFER_SIZE);
    sem_t *sem_full  = sem_open("/sem_full",  O_CREAT, 0644, 0);
    sem_t *sem_mutex = sem_open("/sem_mutex", O_CREAT, 0644, 1);

    if (sem_empty == SEM_FAILED || sem_full == SEM_FAILED || sem_mutex == SEM_FAILED) {
        perror("sem_open failed");
        exit(1);
    }

    for (int i = 0; i < num_items; i++) {
        int value = producer_id * 1000 + i;

        sem_wait(sem_empty);   // wait for empty slot
        sem_wait(sem_mutex);   // enter critical section

        // Add item to buffer
        buffer->buffer[buffer->head].value = value;
        buffer->buffer[buffer->head].producer_id = producer_id;
        buffer->head = (buffer->head + 1) % BUFFER_SIZE;
        buffer->count++;

        printf("Producer %d: Produced value %d\n", producer_id, value);

        sem_post(sem_mutex);   // exit critical section
        sem_post(sem_full);    // signal item available

        usleep(100000); // small delay to visualize interleaving
    }

    // Cleaning up
    shmdt(buffer);
    sem_close(sem_empty);
    sem_close(sem_full);
    sem_close(sem_mutex);

    return 0;
}
