#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <sys/shm.h>
#include <signal.h>
#include "headers/answer.h"

#define DEBUG
#define POOL_SIZE 10

#ifdef DEBUG
#define TRACE printf("%s %d\n", __FILE__,__LINE__);
#else
#define TRACE
#endif

typedef struct pool_attr
{
    int proc_pids[POOL_SIZE];
    int proc_busy[POOL_SIZE];

} pool_attr;


void close_server(int sig);

int       shmid;
void      *shared_memory = (void *)0;
pool_attr *shared_attributes;

int main()
{
    int    i, fork_result;
    int    server_sockfd, client_sockfd;
    int    server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    struct sigaction act;

    act.sa_handler = close_server;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    shmid = shmget((key_t)123,sizeof(pool_attr),0666 | IPC_CREAT);
    if (shmid == -1)
    {
        fprintf(stderr,"shmget failed\n");
        exit(EXIT_FAILURE);
    }

    shared_memory = shmat(shmid,(void *)0,0);
    if (shared_memory == (void *)-1)
    {
        fprintf(stderr,"shmat failed\n");
        exit(EXIT_FAILURE);
    }

    shared_attributes = (pool_attr *)shared_memory;

    server_sockfd = socket(AF_INET,SOCK_STREAM,0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(6565);
    server_len = sizeof(server_address);
    bind(server_sockfd,(struct sockaddr *)&server_address, server_len);

    listen(server_sockfd, 5);

    for(i = 0; i < POOL_SIZE; i++)
    {
        if((fork_result = fork()) == 0)
        {
            while(1)
            {
                shared_attributes->proc_busy[i] = 0;
                printf("child %d waiting...\n", getpid());
                client_len = sizeof(client_address);
                client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);

                shared_attributes->proc_busy[i] = 1;
                printf("client connected to child process %d\n", getpid());
                answer(client_sockfd);
                close(client_sockfd);
            }
        }
        else
        {
            shared_attributes->proc_pids[i] = fork_result;
        }
    }

    sigaction(SIGINT, &act, 0);
    wait(NULL);
}

void close_server(int sig)
{
    int i;
    int wait_limit;

    printf("server closing...\n");

    for(i = 0; i < POOL_SIZE; i++)
    {
        while(shared_attributes->proc_busy[i])
        {
            printf("waiting busy process...\n");
            if(wait_limit == 100)
            {
                wait_limit = 0;
                break;
            }
            wait_limit++;
        }
        kill(shared_attributes->proc_pids[i], SIGKILL);
    }

    printf("pool is closed\n");

    if(shmdt(shared_memory) == -1)
    {
        fprintf(stderr,"shmdt failed\n");
        exit(EXIT_FAILURE);
    }

    if(shmctl(shmid,IPC_RMID,0) == -1)
    {
        fprintf(stderr,"shmctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);
    }

    printf("server is closed\n");

    exit(EXIT_SUCCESS);
}


