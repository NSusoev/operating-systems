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
#include <signal.h>

//#define DEBUG
#define PARTBUF_SIZE 1024
#define PATH "/home/susoev/Документы/Operating-systems/HTTP-server/prefork2"
#define POOL_SIZE 10

#ifdef DEBUG
#define TRACE printf("%s %d\n", __FILE__,__LINE__);
#else
#define TRACE
#endif

typedef struct http_procotol_t
{
  char *header;
  char *body;
} http_protocol_t;

char msg[99999];
char path[99999];
char html[PARTBUF_SIZE];
struct http_procotol_t server_answer;

char *ROOT, *req_params[2];

int main()
{
    int i, fork_result, bytes, fd;
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    server_sockfd = socket(AF_INET,SOCK_STREAM,0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(6565);
    server_len = sizeof(server_address);
    bind(server_sockfd,(struct sockaddr *)&server_address, server_len);

    listen(server_sockfd, 5);
    signal(SIGCHLD,SIG_IGN);

    for(i = 0; i < POOL_SIZE; i++)
        if((fork_result = fork()) == 0)
        {
            while(1)
            {
                printf("child %d waiting...\n", getpid());
                client_len = sizeof(client_address);
                client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
                printf("client connected to child process %d\n", getpid());

                ROOT = PATH;
                memset((void *)msg,(int)'\0',99999);
                read(client_sockfd, msg, sizeof(msg));

                req_params[0] = strtok(msg," ");
                if(strncmp(req_params[0], "GET\0", 4) == 0)
                {
                    req_params[1] = strtok(NULL," ");

                    if(strncmp(req_params[1],"/\0",2) == 0)
                        req_params[1] = "/index.html";

                    strcpy(path,ROOT);
                    strcpy(&path[strlen(ROOT)],req_params[1]);

                    if ((fd = open(path, O_RDONLY)) != -1)
                    {
                      server_answer.header = "HTTP/1.1 200 OK\n\n";
                      send(client_sockfd, server_answer.header, strlen(server_answer.header), 0);
                      TRACE
                      while((bytes = read(fd, html, PARTBUF_SIZE)) > 0)
                      {
                        write(client_sockfd, html, bytes);
                      }
                      close(fd);
                    }
                    else
                    {
                      server_answer.header = "HTTP/1.1 404 Not Found\n\n";
                      server_answer.body = "<html><body><h1>404 Not Found</h1></body></html>";
                      send(client_sockfd, server_answer.header, strlen(server_answer.header), 0);
                      send(client_sockfd, server_answer.body, strlen(server_answer.body), 0);
                    }
                    close(client_sockfd);
                }
            }
        }

    wait(NULL);
}


