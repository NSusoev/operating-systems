#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

char *getfile_html_code(char *filepath);
int check_file_on_exist();

int main()
{
  int server_sockfd, client_sockfd;
  int server_len, client_len;
  struct sockaddr_in server_address;
  struct sockaddr_in client_address;

  server_sockfd = socket(AF_INET,SOCK_STREAM,0);
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
  server_address.sin_port = htons(6565);
  server_len = sizeof(server_address);
  bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
  
  listen(server_sockfd, 5);
  signal(SIGCHLD,SIG_IGN);

  while(1)
  {
    char msg[100];
    char path[50], *ppath , *pmsg;
    ppath = path;
    pmsg = msg;

    printf("server waititng \n");
  
    client_len = sizeof(client_address);
    client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address,&client_len);
    if (fork() == 0) {
        printf("connection is init\n");
        read(client_sockfd, msg, sizeof(msg));
        printf("server get this:\n%s\n", msg);

        if (strncmp("GET", msg, 3) == 0)
        {
          printf("this is GET query\n");
          pmsg = pmsg + 5;

          while(*pmsg != ' ')
          {
            *ppath++ = *pmsg++;
          }
          *ppath = 0;
          printf("need file: %s\n", path);

          if(check_file_on_exist(path) == 0)
          {
            printf("file is exists\n");
            write(client_sockfd,"HTTP/1.1 200 OK\n",24);
            write(client_sockfd,"Content-Type: text/html\n",23);
            write(client_sockfd,"Content-Length: 48\n",23);
            write(client_sockfd,"Connection: close\n\n",21);
            write(client_sockfd,"<html><body><h1>HELLO WORLD!</h1></body></html>",48);

          }
          else
          {
            printf("file isn't exists\n");
            write(client_sockfd,"HTTP/1.1 404 Not Found\n",24);
            write(client_sockfd,"Content-Type: text/html\n",23);
            write(client_sockfd,"Content-Length: 48\n",23);
            write(client_sockfd,"Connection: close\n\n",21);
            write(client_sockfd,"<html><body><h1>404 Not Found</h1></body></html>",48);
          }
        }

        close(client_sockfd);
        exit(EXIT_SUCCESS);
    }
    else {
       close(client_sockfd);
    }
  }
}

int check_file_on_exist(char *filepath)
{
  FILE *fo;

  fo = fopen(filepath,"r");
  if (fo == NULL)
  {
    return 1;
  }
  return 0;
}  
