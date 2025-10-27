#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <pthread.h>
#include "common.h"

//pointer to shared memory with consumer
static ring_t *ringp;

//named semaphores used accross the processes
static sem_t *empty_sem, *full_sem, *mutex_sem;

//generate items and insert them into the buffer
void* run(void* arg) {
    int item = 1;
    while (item <= 25) {       // stop producer producing after 25 items

        //wait until empty slot in buffer
        sem_wait(empty_sem);

        //lock before going into critical section
        sem_wait(mutex_sem);

        //critical section -> place item in buffer
        ringp->buf[ ringp->in ] = item;
        printf("Produced %d (slot %d)\n", item, ringp->in);

        //circular buffer of 2 - move producer index by 1
        ringp->in = (ringp->in + 1) % SIZE;
        item++;

        //release lock
        sem_post(mutex_sem);

        //increase available count by one
        sem_post(full_sem);

    }

    printf("\nProducer finished producing 50 items.\n");
    printf(" -> Run 'make stop' to end both programs after consumer finishes.\n");
    return NULL;
}

int main(void) {

    //create shared memory object due to running first
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(ring_t));

    //map to shared memory
    ringp = mmap(NULL, sizeof(ring_t),
         PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    //create named semaphores system wide
    empty_sem = sem_open(SEM_EMPTY, O_CREAT, 0666, SIZE);
    full_sem  = sem_open(SEM_FULL,  O_CREAT, 0666, 0);
    mutex_sem = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);

    //initialize buffer indices
    ringp->in = ringp->out = 0;

    //start producer thread
    pthread_t th;
    pthread_create(&th, NULL, run, NULL);
    pthread_join(th, NULL);
    return 0;
}