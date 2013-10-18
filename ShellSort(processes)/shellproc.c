#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>

#define PART_SIZE 80000
#define INTERMEDIATE_SIZE 160000
#define GLOBAL_SIZE 320000

void write_to_inputfile(char *filename);
void read_from_file(char *filename,int *array1,int *array2,int *array3,int *array4);
void write_to_outputfile(char *filename,int *array);
void shell_sort(int *array,int n);
void merge_sort(int *array1,int *array2,int *result,int partsize);
void time_start();
void time_stop();
void ding(int sig);

int arr1[PART_SIZE],arr2[PART_SIZE],arr3[PART_SIZE],arr4[PART_SIZE];
int in_arr1[INTERMEDIATE_SIZE],in_arr2[INTERMEDIATE_SIZE];
int final_array[GLOBAL_SIZE];

pid_t child_pid;
pid_t fork_result;
int file_pipes[2];
int alarm_fired = 0;

int stat_val;
int data;

struct timeval tim;
double time_1,time_2;

int main(int argc,char *argv[])
{

    if (argc != 3)
    {
        perror("program format: program <input file> <output file>");
        exit(EXIT_FAILURE);
    }

    write_to_inputfile(argv[1]);
    read_from_file(argv[1],arr1,arr2,arr3,arr4); /* filling of subarrays */

    time_start();
    if(pipe(file_pipes) == 0)
        printf("pipes create\n");
    else
        exit(EXIT_FAILURE);

    fork_result = fork();
    if(fork_result == -1)
    {
        fprintf(stderr,"Fork failure ");
        exit(EXIT_FAILURE);
    }

    if(fork_result == 0)
    {
        close(file_pipes[0]);
        shell_sort(arr3,PART_SIZE);
        shell_sort(arr4,PART_SIZE);

        merge_sort(arr3,arr4,in_arr2,PART_SIZE);
        /* transmission sorted part */
        data = write(file_pipes[1],in_arr2,INTERMEDIATE_SIZE*sizeof(int));
        close(file_pipes[1]);
        exit(EXIT_SUCCESS);
    }
    else
    {
        close(file_pipes[1]);
        shell_sort(arr1,PART_SIZE);
        shell_sort(arr2,PART_SIZE);

        merge_sort(arr1,arr2,in_arr1,PART_SIZE);
        /* reading sorted part from child process */
        data = read(file_pipes[0],in_arr2,sizeof(int)*INTERMEDIATE_SIZE);
            close(file_pipes[0]);
       
        merge_sort(in_arr1,in_arr2,final_array,INTERMEDIATE_SIZE);
        time_stop();
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

void ding(int sig)
{
    alarm_fired = 1;
}
