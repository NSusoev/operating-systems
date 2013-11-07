#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/shm.h>
#include "headers/processpool.h"
#include "headers/debug.h"

#define DEBUG

typedef struct shared_use
{
    sem_t sems[10];
    proc_pool_t pool;

} shared_use;

void queue_test();

entry_t proc;
pid_t fork_result;
entry_t *centry;

int main()
{
    void *shared_memory = (void *)0;
    shared_use *shared_stuff;
    int shmid, res , i;
    TRACE

    shmid = shmget((key_t)123, sizeof(shared_use), 0666 | IPC_CREAT);
    if(shmid == -1)
    {
        fprintf(stderr,"shmget failed\n");
        exit(EXIT_FAILURE);
    }

    shared_memory = shmat(shmid, (void *)0, 0);
    if(shared_memory == (void *)-1)
    {
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }

    shared_stuff = (shared_use *)shared_memory;

    for(i; i < 10; i++)
    {
        res = sem_init(&shared_stuff->sems[i],1,0);
        if(res != 0)
        {
            perror("Semaphore init failed");
            exit(EXIT_FAILURE);
        }
    }
    printf("sems init ok\n");
    fflush(stdout);
    shared_stuff->pool = get_pool();
    printf("59\n");
    fflush(stdout);

    i = 0;
    for(i; i < 1; i++);
    {
        fork_result = fork();
        if(fork_result == -1)
        {
            fprintf(stderr,"Fork failure\n");
            exit(EXIT_FAILURE);
        }

        if(fork_result == 0)
        {
            shmid = shmget((key_t)123, sizeof(shared_use), 0666 | IPC_CREAT);
            if(shmid == -1)
            {
                fprintf(stderr,"shmget failed\n");
                exit(EXIT_FAILURE);
            }

            shared_memory = shmat(shmid, (void *)0, 0);
            if(shared_memory == (void *)-1)
            {
                fprintf(stderr, "shmat failed\n");
                exit(EXIT_FAILURE);
            }

            shared_stuff = (shared_use *)shared_memory;
            printf("child add shm ok\n");
            fflush(stdout);

            proc.entry_semaphore = &shared_stuff->sems[i];
            proc.proc_pid = getpid();

            printf("child proc init ok\n");
            fflush(stdout);

            add_in_tail(shared_stuff->pool.proc_queue, &proc);
            shared_stuff->pool.proc_queue->head = &proc;
            printf("child proc add in queue ok\n");
            sem_post(&shared_stuff->sems[0]);

            while(1)
            {
                ;
            }
        }
        else
        {
            printf("child created!\n");
        }

    }

    printf("116\n");
    fflush(stdout);
    sem_wait(&shared_stuff->sems[0]);

    centry = shared_stuff->pool.proc_queue->head;

    while(centry != NULL)
    {
        printf("free proc pid = %d\n", centry->proc_pid);
        centry = centry->next;
    }

    if(centry == NULL)
        printf("curentry == NULL\n");

    printf("119\n");
    fflush(stdout);
    if(shmdt(shared_memory) == -1)
    {
        fprintf(stderr,"shmdt failed\n");
        exit(EXIT_FAILURE);
    }

    printf("127\n");
    fflush(stdout);
    if(shmctl(shmid,IPC_RMID,0) == -1)
    {
        fprintf(stderr,"shmctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}

void queue_test()
{
    int i = 0;
    proc_pool_t mypool = get_pool();
    entry_t *pop_entry1, *pop_entry2;
    entry_t *curentry;

    for(i; i < 10; i++)
    {
        entry_t *entry = (entry_t *)malloc(sizeof(entry_t));
        entry->proc_pid = (pid_t)i;
        add_in_tail(mypool.proc_queue, entry);
    }
    TRACE
    pop_entry1 = pop_free_proc(mypool.proc_queue);
    pop_entry2 = pop_free_proc(mypool.proc_queue);
    TRACE
    printf("work pid = %d\n",pop_entry1->proc_pid);
    printf("work pid = %d\n",pop_entry2->proc_pid);
    printf("\n");

    curentry = mypool.proc_queue->head;
    while(curentry != NULL)
    {
        printf("free pid = %d\n", curentry->proc_pid);
        curentry = curentry->next;
    }

    printf("\n");
    TRACE
    add_in_tail(mypool.proc_queue,pop_entry1);
    add_in_tail(mypool.proc_queue,pop_entry2);
    TRACE
    curentry = mypool.proc_queue->head;

    while(curentry != NULL)
    {
        printf("free pid = %d\n", curentry->proc_pid);
        curentry = curentry->next;
    }

    printf("\n");
    printf("count = %d\n",mypool.proc_queue->count);

}

