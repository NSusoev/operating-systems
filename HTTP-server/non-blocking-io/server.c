#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#define PARTBUF_SIZE 1024
#define BEBUG
#define PATH "/home/susoev/Документы/Operating-systems/HTTP-server/non-blocking-io"
#define TRUE  1
#define FALSE 0

#ifdef DEBUG
#define TRACE printf("%s %d\n",__FILE__,__LINE__);
#else
#define TRACE
#endif

typedef struct http_procotol
{
  char *header;
  char *body;
} http_procotol;

struct http_procotol server_answer;
char   msg[99999];
char   path[99999];
char   html[PARTBUF_SIZE];
char   *SERVER_ROOT, *req_params[2];

int main(int argc, char *argv[])
{
    int    fd, file_fd, max_fd, result, on = 1;
    int    server_sockfd, client_sockfd;
    int    desc_ready;
    int    server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    struct timeval     timeout;
    fd_set master_set, working_set;

    memset((void *)msg,(int)'\0',99999);

    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sockfd < 0)
    {
        perror("socket failed\n");
        exit(EXIT_FAILURE);
    }

    result = setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR,(char *)&on, sizeof(on));
    if(result < 0)
    {
        perror("setsockopt failed\n");
        close(server_sockfd);
        exit(EXIT_FAILURE);
    }

    result = ioctl(server_sockfd, FIONBIO, (char *)&on);
    if(result < 0)
    {
        perror("ioctl failed\n");
        close(server_sockfd);
        exit(EXIT_FAILURE);
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_address.sin_port = htons(6565);
    server_len = sizeof(server_address);

    result = bind(server_sockfd,(struct sockaddr *) &server_address, server_len);
    if(result < 0)
    {
        perror("bind failed\n");
        close(server_sockfd);
        exit(EXIT_FAILURE);
    }

    listen(server_sockfd, 5);

    FD_ZERO(&master_set);
    max_fd = server_sockfd;
    FD_SET(server_sockfd, &master_set);

    timeout.tv_sec = 3 * 60;
    timeout.tv_usec = 0;

    while(TRUE)
    {
        int bytes;
        working_set = master_set;

        printf("waiting on select ...\n");
        result = select(max_fd + 1, &working_set, (fd_set *)0, (fd_set *)0, &timeout);
        if(result < 0)
        {
            perror("select failed\n");
            break;
        }

        if(result == 0)
        {
            printf("select time out.");
            break;
        }

        desc_ready = result;

        for(fd = 0; fd <= max_fd && desc_ready > 0; fd++)
        {
            if(FD_ISSET(fd, &working_set))
            {
                desc_ready -= 1;

                if(fd == server_sockfd)
                {
                    printf("listening socket is readable\n");

                    do
                    {
                        client_len = sizeof(client_address);
                        client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address,&client_len);

                        if(client_sockfd < 0)
                        {
                            if(errno != EWOULDBLOCK)
                            {
                                perror("accept failed\n");
                                break;
                            }
                            break;
                        } 

                        printf("adding new client on fd %d\n", client_sockfd);
                        FD_SET(client_sockfd, &master_set);
                        if(client_sockfd > max_fd)
                            max_fd = client_sockfd;

                    } while(client_sockfd != -1);
                }
                else
                {
                    printf("descr %d is readable\n", fd);

                    SERVER_ROOT = PATH;
                    read(fd, msg, sizeof(msg));

                    req_params[0] = strtok(msg, " ");
                    if (strncmp(req_params[0], "GET", 4) == 0)
                    {
                        printf("this is GET query\n");
                        req_params[1] = strtok(NULL," ");

                        if (strncmp(req_params[1],"/", 2) == 0)
                            req_params[1] = "/index.html";

                        printf("need file: %s\n", req_params[1]);
                        strcpy(path,SERVER_ROOT);
                        strcpy(&path[strlen(SERVER_ROOT)], req_params[1]);
                        printf("FILE_PATH: %s\n", path);

                        if ((file_fd = open(path, O_RDONLY)) != -1)
                        {
                            server_answer.header = "HTTP/1.1 200 OK\n\n";
                            send(client_sockfd, server_answer.header, strlen(server_answer.header), 0);

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

                        close(fd);
                    }

                    FD_CLR(fd, &master_set);
                    if(fd == max_fd)
                        while(FD_ISSET(max_fd, &master_set) == FALSE)
                            max_fd -= 1;
                }
            }
        }

    }

    for(fd = 0; fd <= max_fd; fd++)
    {
        if(FD_ISSET(fd, &master_set))
            close(fd);
    }
}
