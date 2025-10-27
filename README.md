✅ Program Description:

This project implements the Producer–Consumer Problem using two separate processes that communicate through shared memory and synchronize using POSIX named semaphores.
The producer process generates integer items and places them into a shared 2-slot ring buffer.
The consumer process removes items from the same buffer and prints them.

Because the buffer can only hold two items, the producer must wait when the buffer is full.
Likewise, the consumer must wait when there are no items available.

This demonstrates synchronization through semaphores, mutual exclusion, and inter-process communication (IPC).

Program flow:
- The producer process will generate 25 items to place in shared memory as the consumer picks them off.
- A loop condition will stop the producer from producing more than 25 items to keep terminal clean.
- The consumer process will consume the last item/items and then wait for user to enter ‘make stop’ in terminal.

note: Makefile commands in terminal to run program:

       make all  // compile and run -> uses "./producer & ./consumer &" to run two processes for communication
       make stop // to kill running processes
       make clean // clean up executeables
       
----------------------------------------------------------------------------------------------------------

Components:

✅ Shared Memory
Created by the **producer** and opened by the **consumer** so both processes read/write the **same region**:

```c
int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
ftruncate(fd, sizeof(ring_t));
ringp = mmap(NULL, sizeof(ring_t),
             PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
```

✅ POSIX Named Semaphores (Synchronization)

SEM_EMPTY -> Counts empty slots in the buffer -> initial value = 2

SEM_FULL -> Counts ready-to-consume items -> initial value = 0

SEM_MUTEX -> Mutual exclusion (critical section lock) -> initial value -> 1

------producer pattern
```c
sem_wait(empty_sem); //wait for empty slot
sem_wait(mutex_sem); // lock before going into critical section

// critical section
// write item into ringp->buf[ringp->in]

sem_post(mutex_sem); //release lock
sem_post(full_sem); // increase available count to consume by 1
```
------consumer pattern
```c
sem_wait(full_sem); // wait for one at least one full slot 
sem_wait(mutex_sem); // lock before going into critical section

// critical section
// read item from ringp->buf[ringp->out]

sem_post(mutex_sem); // release lock
sem_post(empty_sem); // increase empty slots to produce by 1
```

✅ Ring Buffer (Circular)
```c
typedef struct {
    int buf[2];  // only 2 items allowed at once
    int in;      // next write index (producer advances)
    int out;     // next read index  (consumer advances)
} ring_t;
```
------circular movement buffer arithmetic
```c
ringp->in  = (ringp->in  + 1) % SIZE;
ringp->out = (ringp->out + 1) % SIZE;
```

✅ Thread Creator Inside Each Program
```c
pthread_create(&th, NULL, run, NULL);
```
- Each process starts one worker thread.
- Threads will execute individual logic inside each program
- Threads access shared memory when conditons are met (critical section)
