#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <semaphore.h>
#include "headers/processpool.h"

int main()
{
    proc_pool_t mypool = get_pool();

    entry_t *entry = (entry_t *)malloc(sizeof(entry_t));
    entry->proc_pid = 10;
    add_in_tail(mypool.proc_queue, entry);
    printf("\n");
    printf("count = %d\n",mypool.proc_queue->count);

    return EXIT_SUCCESS;
}
