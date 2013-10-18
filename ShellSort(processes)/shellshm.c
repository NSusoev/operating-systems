#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <semaphore.h>
#include <sys/shm.h>

#define PART_SIZE 80000
#define INTERMEDIATE_SIZE 160000
#define GLOBAL_SIZE 320000

typedef struct shared_use
{
    sem_t sem;
    int array[INTERMEDIATE_SIZE];

} shared_use;

void write_to_inputfile(char *filename);
void read_from_file(char *filename,int *array1,int *array2,int *array3,int *array4);
void write_to_outputfile(char *filename,int *array);
void shell_sort(int *array,int n);
void merge_sort(int *array1,int *array2,int *result,int partsize);
void time_start();
void time_stop();

int arr1[PART_SIZE],arr2[PART_SIZE],arr3[PART_SIZE],arr4[PART_SIZE];
int in_arr1[INTERMEDIATE_SIZE],in_arr2[INTERMEDIATE_SIZE];
int final_array[GLOBAL_SIZE];

pid_t fork_result;

sem_t sm;
int res;

struct timeval tim;
double time_1,time_2;

void *shared_memory = (void *)0;
shared_use *shared_stuff;
int shmid;

int main(int argc,char *argv[])
{

    if (argc != 3)
    {
        perror("program format: program <input file> <output file>");
        exit(EXIT_FAILURE);
    }

    write_to_inputfile(argv[1]);
    read_from_file(argv[1],arr1,arr2,arr3,arr4);

    time_start();

    fork_result = fork();
    if(fork_result == -1)
    {
        fprintf(stderr,"Fork failure");
        exit(EXIT_FAILURE);
    }

    if(fork_result == 0)
    {
        shmid = shmget((key_t)123,sizeof(shared_use),0666 | IPC_CREAT);
        if(shmid == -1)
        {
            fprintf(stderr,"shmget failed\n");
            exit(EXIT_FAILURE);
        }

        shared_memory = shmat(shmid,(void *)0,0);
        if(shared_memory == (void *)-1)
        {
            fprintf(stderr,"shmat failed\n");
            exit(EXIT_FAILURE);
        }

        shared_stuff = (shared_use *)shared_memory;

        shell_sort(arr3,PART_SIZE);
        shell_sort(arr4,PART_SIZE);
        merge_sort(arr3,arr4,in_arr2,PART_SIZE);

        memcpy(shared_stuff->array,in_arr2,sizeof(in_arr2));

        sem_post(&shared_stuff->sem);

        if(shmdt(shared_memory) == -1)
        {
            fprintf(stderr,"shmdt failed\n");
            exit(EXIT_FAILURE);
        }

        exit(EXIT_SUCCESS);
    }
    else
    {

        shmid = shmget((key_t)123,sizeof(shared_use),0666 | IPC_CREAT);
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

        shared_stuff = (shared_use *)shared_memory;
        res = sem_init(&shared_stuff->sem,1,0);
        if(res != 0)
        {
            perror("Semaphore init failed\n");
            exit(EXIT_FAILURE);
        }

        shell_sort(arr1,PART_SIZE);
        shell_sort(arr2,PART_SIZE);

        merge_sort(arr1,arr2,in_arr1,PART_SIZE);
        sem_wait(&shared_stuff->sem);

        merge_sort(in_arr1,shared_stuff->array,final_array,INTERMEDIATE_SIZE);
        time_stop();

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
    }
    printf("%.6lf seconds\n",time_2-time_1);
    write_to_outputfile(argv[2],final_array);
    return EXIT_SUCCESS;
}

void write_to_inputfile(char *filename)
{
    FILE *fo;
    fo = fopen(filename,"w");
    int size = GLOBAL_SIZE;

    if(fo == 0)
    {
        perror(filename);
        exit(EXIT_FAILURE);
    }

    while(size--)
        fprintf(fo,"%d ",1 + rand() % 9);
    fclose(fo);
}

void read_from_file(char *filename,int *arr1,int *arr2,int *arr3,int *arr4)
{
    FILE *fo;
    fo = fopen(filename,"r");
    int size = PART_SIZE;

    if(fo == 0)
    {
        perror(filename);
        exit(EXIT_FAILURE);
    }

    while(size--)
        fscanf(fo,"%d",arr1++);
    size = PART_SIZE;
    while(size--)
        fscanf(fo,"%d",arr2++);
    size = PART_SIZE;
    while(size--)
        fscanf(fo,"%d",arr3++);
    size = PART_SIZE;
    while(size--)
        fscanf(fo,"%d",arr4++);

    fclose(fo);
}

void write_to_outputfile(char *filename,int *array)
{
    FILE *fo;
    fo = fopen(filename,"w");
    int size = GLOBAL_SIZE;

    if(fo == 0)
    {
        perror(filename);
        exit(EXIT_FAILURE);
    }

    while(size--)
        fprintf(fo,"%d ",*array++);

    fclose(fo);
}

void shell_sort(int* a, int n)
{
    int step = n / 2;
    int i,j;

    while (step > 0)
    {
        for (i = 0; i < n - step; i++)
        {
            j = i;
            while ((j >= 0) && (a[j] > a[j + step]))
            {
                int buf = a[j];
                a[j] = a[j + step];
                a[j + step] = buf;
                j--;
            }
        }
        step /= 2;
    }
}

void merge_sort(int *array1,int *array2,int *result,int partsize)
{
    int i = 0,j = 0,index = 0;

    while(i < partsize && j < partsize)
    {
        if(array1[i] > array2[j])
            result[index++] = array2[j++];
        else
            result[index++] = array1[i++];
    }

    while(i < partsize)
        result[index++] = array1[i++];

    while(j < partsize)
        result[index++] = array2[j++];
}

void time_start()
{
    gettimeofday(&tim, NULL);
    time_1 = tim.tv_sec + (tim.tv_usec/1000000.0);
}

void time_stop()
{
    gettimeofday(&tim, NULL);
    time_2 = tim.tv_sec + (tim.tv_usec/1000000.0);
}

