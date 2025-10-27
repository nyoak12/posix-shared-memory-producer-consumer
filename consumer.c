#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <pthread.h>
#include "common.h"

//shared memory pointer to table buffer of 2 defined in common.h
static ring_t *ringp;

//declaring pointers to semaphores for sync
static sem_t *empty_sem, *full_sem, *mutex_sem;

//enter consumer thread
void* run(void* arg) {
    // (void)arg;
    while (1) { 
        sem_wait(full_sem); //wait until there is at least 1 full slot to consume item

        sem_wait(mutex_sem); //enter critical section. lock shared buffer

        //critical section - read from buffer
        int item = ringp->buf[ ringp->out ];
        printf("Consumed %d (slot %d)\n", item, ringp->out);

        //move consumer index forward in circular/ring buffer
        ringp->out = (ringp->out + 1) % SIZE;

        //leave critical section -> unlock
        sem_post(mutex_sem);

        //tell producer there is one more empty slot
        sem_post(empty_sem);
    }
    return NULL;
}

int main(void) {
    // Open existing shared memory (producer must have created it
    int fd = -1;
    for (int tries = 0; tries < 50 && fd < 0; ++tries) {
        fd = shm_open(SHM_NAME, O_RDWR, 0666);
        if (fd < 0) {
            sleep(1); // producer not ready yet -> wait
            continue; // retry
        }
    }
    if (fd < 0) {
        //in case producer never created shared memory
        perror("shm_open (consumer)");
        return 1;
    }

    //map the shared memory into this process for read/write from table
    ringp = mmap(NULL, sizeof(ring_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // Open semaphores (names shared system-wide)
    empty_sem = sem_open(SEM_EMPTY, 0);
    if (!empty_sem) empty_sem = sem_open(SEM_EMPTY, O_CREAT, 0666, SIZE);
    full_sem  = sem_open(SEM_FULL,  0);
    if (!full_sem)  full_sem  = sem_open(SEM_FULL,  O_CREAT, 0666, 0);
    mutex_sem = sem_open(SEM_MUTEX, 0);
    if (!mutex_sem) mutex_sem = sem_open(SEM_MUTEX, O_CREAT, 0666, 1);

    //start the consumer thread
    pthread_t th;
    pthread_create(&th, NULL, run, NULL);
    pthread_join(th, NULL);
    return 0;
}