#ifndef PROCPOOL
#define PROCPOOL

typedef struct entry_t
{
  sem_t *entry_semaphore;
  pid_t proc_pid;
  struct entry_t *next;
} entry_t;

typedef struct queue_t
{
  entry_t *head;
  entry_t *tail;
  int count;
} queue_t;

typedef struct proc_pool_t
{
  queue_t *proc_queue;
} proc_pool_t;

void add_in_tail(queue_t *proc_queue, entry_t *newproc);
entry_t *pop_free_proc(queue_t *proc_queue);
proc_pool_t get_pool();



#endif
