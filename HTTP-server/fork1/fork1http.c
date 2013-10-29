#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

#define DEBUG
#define PARTBUF_SIZE 1024

#ifdef DEBUG
#define TRACE printf("%s %d\n", __FILE__, __LINE__);
#else
#define TRACE
#endif

struct http_procotol
{
  char *header;
  char *mime_type;
  char *connection_stat;
  char *data_length;
  char *server;
  char *body;
};

char *getfile_html_code(char *filepath);
int check_file_on_exist(char *filepath);

struct http_procotol server_answer;
char msg[99999];
char path[99999];

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
    char *ppath , *pmsg;
    int bytes;
    int written_bytes;
    ppath = path;
    pmsg = msg;

    printf("server waititng \n");
  
    client_len = sizeof(client_address);
    client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address,&client_len);
    if (fork() == 0) {
        printf("connection is init\n");
        memset((void *)msg,(int)'\0',99999);
        TRACE
        read(client_sockfd, msg, sizeof(msg));
        TRACE
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
            printf("file exists\n");
            TRACE
            server_answer.header = "HTTP/1.1 200 OK\n";
            server_answer.server = "Host: localhost:6565\n";
            server_answer.mime_type = "Content-Type: text/html; charset=utf-8\n";
            server_answer.connection_stat = "Connection: close\n\n";
            TRACE
            server_answer.body = getfile_html_code(path);
            printf("html: %s\n",server_answer.body);

            send(client_sockfd, server_answer.header, strlen(server_answer.header), 0);
            send(client_sockfd, server_answer.server, strlen(server_answer.server), 0);
            send(client_sockfd, server_answer.mime_type, strlen(server_answer.mime_type), 0);
            send(client_sockfd, server_answer.connection_stat, strlen(server_answer.connection_stat), 0);

            written_bytes = 0;
            while(written_bytes < strlen(server_answer.body))
            {
              bytes = write(client_sockfd, server_answer.body, strlen(server_answer.body));
              written_bytes += bytes;
            }
          }
          else
          {
            printf("file not exists\n");
            server_answer.header = "HTTP/1.1 404 Not Found\n";
            server_answer.mime_type = "Content-Type: text/html\n";
            server_answer.connection_stat = "Connection: close\n\n";
            server_answer.body = "<html><body><h1>404 Not Found</h1></body></html>";

            write(client_sockfd, server_answer.header, sizeof(char) * strlen(server_answer.header));
            write(client_sockfd, server_answer.mime_type, sizeof(char) * strlen(server_answer.mime_type));
            write(client_sockfd, server_answer.connection_stat, sizeof(char) * strlen(server_answer.connection_stat));
            write(client_sockfd, server_answer.body, sizeof(char) * strlen(server_answer.body));
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
  fclose(fo);
  return 0;
} 

char *getfile_html_code(char *filepath)
{
  FILE *fo;
  char *body;

  fo = fopen(filepath,"r");
  fread(body,PARTBUF_SIZE,1,fo);
       
  fclose(fo);
  return body;
} 
