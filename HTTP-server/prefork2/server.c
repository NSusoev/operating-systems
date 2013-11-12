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

void close_server(int sig);

int proc_pids[POOL_SIZE];

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
                printf("child %d waiting...\n", getpid());
                client_len = sizeof(client_address);
                client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);

                printf("client connected to child process %d\n", getpid());
                answer(client_sockfd);
                close(client_sockfd);
            }
        }
        else
        {
            proc_pids[i] = fork_result;
        }
    }

    sigaction(SIGINT, &act, 0);
    wait(NULL);
}

void close_server(int sig)
{
    int i;

    printf("server closing...\n");

    for(i = 0; i < POOL_SIZE; i++)
    {
        kill(proc_pids[i], SIGKILL);
    }

    printf("pool is closed\n");
    printf("server is closed\n");

    exit(EXIT_SUCCESS);
}


