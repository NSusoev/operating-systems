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

static char            msg[99999];
static char            html[PARTBUF_SIZE];
static http_protocol_t server_answer;
static char            *req_params[2];

extern void answer(int fd)
{
    int bytes, file_fd;

    memset((void *)msg,(int)'\0',99999);
    printf("descr %d is readable\n", fd);
    read(fd, msg, sizeof(msg));

    req_params[0] = strtok(msg, " ");
    if (strncmp(req_params[0], "GET", 4) == 0)
    {
        printf("this is GET query\n");
        req_params[1] = strtok(NULL," ");

        if (strncmp(req_params[1],"/", 2) == 0)
            req_params[1] = "/index.html";

        if (strstr(req_params[1], "..") != NULL)
            req_params[1] = "/index.html";

        printf("NEED FILE: %s\n", req_params[1]);

        if ((file_fd = open(req_params[1] + 1, O_RDONLY)) != -1)    
        {
            server_answer.header = "HTTP/1.1 200 OK\n\n";
            send(fd, server_answer.header, strlen(server_answer.header), 0);
                    
            while((bytes = read(file_fd, html, PARTBUF_SIZE)) > 0)
            {
                write(fd, html, bytes);
            }
            close(file_fd);
        }
        else
        {
            server_answer.header = "HTTP/1.1 404 Not Found\n\n";
            server_answer.body = "<html><body><h1>404 Not Found</h1></body></html>";
            send(fd, server_answer.header, strlen(server_answer.header), 0);
            send(fd, server_answer.body, strlen(server_answer.body), 0);
        }
    }
}
