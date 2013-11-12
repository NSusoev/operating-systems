#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <string.h>
#include <pthread.h>

#define DEBUG
#define PARTBUF_SIZE 1024
#define CLIENT_LIMIT 10
#define PATH "/home/susoev/Документы/Operating-systems/HTTP-server/fork1"

#ifdef DEBUG
#define TRACE printf("%s %d\n", __FILE__, __LINE__);
#else
#define TRACE
#endif

struct http_procotol
{
  char *header;
  char *body;
};

typedef struct c_client
{
    int curcount;
    pthread_mutex_t count_mutex;
} c_client;

struct http_procotol server_answer;
char   msg[99999];
char   path[99999];
char   html[PARTBUF_SIZE];
char   *ROOT, *req_params[2];
int    shmid;
void   *shared_memory = (void *)0;

void close_server(int sig);

int main()
{
  int      server_sockfd, client_sockfd;
  int      server_len, client_len;
  int      res;
  struct   sockaddr_in server_address;
  struct   sockaddr_in client_address;
  struct   sigaction act;
  c_client *shared_limit;

  act.sa_handler = close_server;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  sigaction(SIGINT, &act, 0);

  shmid = shmget((key_t)123,sizeof(c_client),0666 | IPC_CREAT);
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

  shared_limit = (c_client *)shared_memory;

  res = pthread_mutex_init(&shared_limit->count_mutex, NULL);
  if(res != 0)
  {
      perror("Mutex init failed\n");
      exit(EXIT_FAILURE);
  }

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
    int bytes, fd;

    printf("server waititng \n");
    client_len = sizeof(client_address);
    client_sockfd = accept(server_sockfd,(struct sockaddr *)&client_address,&client_len);

    if(shared_limit->curcount >= CLIENT_LIMIT)
    {
        printf("============server clients limit is exceeded! socket will be closed=============\n");
        close(client_sockfd);
    }
    else
        if (fork() == 0)
        {
            pthread_mutex_lock(&shared_limit->count_mutex);
            shared_limit->curcount++;
            printf("clients count = %d\n", shared_limit->curcount);
            pthread_mutex_unlock(&shared_limit->count_mutex);

            printf("connection is init\n");
            ROOT = PATH;
            printf("SERVER_PATH: %s\n", ROOT);

            memset((void *)msg,(int)'\0',99999);
            read(client_sockfd, msg, sizeof(msg));
            printf("server get this:\n%s\n", msg);

            req_params[0] = strtok(msg, " ");
            if (strncmp(req_params[0], "GET", 4) == 0)
            {
                printf("this is GET query\n");
                req_params[1] = strtok(NULL," ");

                if (strncmp(req_params[1],"/", 2) == 0)
                    req_params[1] = "/index.html";

                printf("need file: %s\n", req_params[1]);
                strcpy(path,ROOT);
                strcpy(&path[strlen(ROOT)], req_params[1]);
                printf("FILE_PATH: %s\n", path);

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

                close(client_sockfd);
                pthread_mutex_lock(&shared_limit->count_mutex);
                shared_limit->curcount--;
                printf("clients count = %d\n", shared_limit->curcount);
                pthread_mutex_unlock(&shared_limit->count_mutex);

                exit(EXIT_SUCCESS);
            }
        }
        else
        {
            close(client_sockfd);
        }
  }
} 

void close_server(int sig)
{
    printf("server closing...\n");

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

    exit(EXIT_SUCCESS);
}
