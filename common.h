#pragma once

//name of the shared memory object in os
//producer creates and consumer opens the same name to access
#define SHM_NAME   "/pc_shm_min"

//name of the semaphores used to do sync between two processes
#define SEM_EMPTY  "/pc_sem_empty_min"
#define SEM_FULL   "/pc_sem_full_min"
#define SEM_MUTEX  "/pc_sem_mutex_min"

//shared table that can only hold two items
#define SIZE 2

//shared structure (circular buffer)
typedef struct {
    int buf[SIZE];
    int in;
    int out;
} ring_t;