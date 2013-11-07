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
#include "headers/answer.h"

#define PARTBUF_SIZE 1024
#define PATH "/home/susoev/Документы/Operating-systems/HTTP-server/prefork2"

static char msg[99999];
static char path[99999];
static char html[PARTBUF_SIZE];
static http_protocol_t server_answer;
static char *ROOT, *req_params[2];

extern void answer(int client_sockfd)
{
    int bytes, fd;

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
    }

}
