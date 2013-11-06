#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <semaphore.h>
#include "headers/processpool.h"
#include "headers/debug.h"

//#define DEBUG

entry_t *pop_entry1, *pop_entry2;
entry_t *curentry;

int main()
{
    int i = 0;
    proc_pool_t mypool = get_pool();

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

    return EXIT_SUCCESS;
}
