#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>

#define PART_SIZE 80000 /* size of 1/4 */
#define INTERMEDIATE_SIZE 160000 /* size of 1/2 */
#define GLOBAL_SIZE 320000 /* size of the initial array */

typedef struct
{
    int mass[PART_SIZE];
    int count;
} workpart;

typedef struct
{
    int part1[PART_SIZE];
    int part2[PART_SIZE];
    int resarray[INTERMEDIATE_SIZE];
    int curpartcount;
} mergepart;

typedef struct
{
    int part1[INTERMEDIATE_SIZE];
    int part2[INTERMEDIATE_SIZE];
    int resarray[GLOBAL_SIZE];
    int curpartcount;
} mergefinal;

void *shell_sort(void *args);
void *merge_sort(void *args);
void read_from_file(const char* filename,int *array1,int *array2,int *array3,int *array4);
void write_to_file(const char* filename, int* array, size_t size);
void work_part_init(workpart *part,int *array);
void merge_part_init(mergepart *part,int *array1,int *array2,int *resultarray,size_t size);
void merge_final_part_init(mergefinal *part,int *array1,int *array2,int *resultarray,size_t size);
void time_start();
void time_stop();
void write_to_inputfile(const char* filename);
void join_threads(pthread_t *threads,void *thread_res,size_t size);

int type_id = 0;
struct timeval tim; 
double time_1,time_2;

int arr1[PART_SIZE],arr2[PART_SIZE],arr3[PART_SIZE],arr4[PART_SIZE];
int mergeres1[INTERMEDIATE_SIZE],mergeres2[INTERMEDIATE_SIZE],mergeres[GLOBAL_SIZE];

pthread_t th[4];
workpart parts[4];
mergepart m_parts[2];
mergefinal mergepart_final;

int main(int argc, const char* argv[])
{
    int res;
    void *thread_result;
    int index = 0;

    if (argc != 3)
    {
        printf("Udage: program <INPUT FILE> <OUTPUT FILE>\n");
        exit(EXIT_FAILURE);
    }

 //   printf("write_to_inputfile");
   // fflush(stdout);
    write_to_inputfile(argv[1]);


    //printf("read_from_file");
    //fflush(stdout);
    read_from_file(argv[1],arr1,arr2,arr3,arr4);
    work_part_init(&parts[0],arr1);
    work_part_init(&parts[1],arr2);
    work_part_init(&parts[2],arr3);
    work_part_init(&parts[3],arr4);

    //printf("time_start");
    //fflush(stdout);
    time_start();

    /* run 4 threads to sort each subarray by Shell Sort  */
    for(index;index < 4;index++)
    {
        res = pthread_create(&th[index],NULL,shell_sort,(void *)&parts[index]);
        if(res != 0)
        {
            perror("thread creation failed");
            exit(EXIT_FAILURE);
        }
    }

    printf("Program: All threads were created\n");
    //fflush(stdout);
    printf("Wait threads...\n");
    //fflush(stdout);
    join_threads(th,&thread_result,4);

    printf("=======================Merge sort begins======================\n");
    //fflush(stdout);

    
    merge_part_init(&m_parts[0],parts[0].mass,parts[1].mass,mergeres1,PART_SIZE);
    merge_part_init(&m_parts[1],parts[2].mass,parts[3].mass,mergeres2,PART_SIZE);
    printf("New threads creating...\n");
    //fflush(stdout);

    /* Pairwise merge sub-arrays */
    index = 0;
    for(index; index < 2;index++)
    {
        res = pthread_create(&th[index],NULL,merge_sort,(void *)&m_parts[index]);
        if(res != 0)
        {
            perror("1st merge thread creation failed");
            exit(EXIT_FAILURE);
        }
    }

    join_threads(th,&thread_result,2);
    printf("======================FINAL SORT============================\n");
    //fflush(stdout);

    merge_final_part_init(&mergepart_final,m_parts[0].resarray,m_parts[1].resarray,mergeres,INTERMEDIATE_SIZE);
    type_id = 1;

    printf("Final merge starts\n");

    /* run final merge  */
    res = pthread_create(&th[0],NULL,merge_sort,(void *)&mergepart_final);
    if(res != 0)
    {
        perror("Final thread creation failed");
        exit(EXIT_FAILURE);
    }

    printf("Wait final thread...\n");
    //fflush(stdout);
    join_threads(th,&thread_result,1);
    time_stop();

    write_to_file(argv[2], mergepart_final.resarray, GLOBAL_SIZE);

    printf("Program: Sort complete! Check your output file\n");
    printf("%.6lf seconds\n",time_2-time_1);

    return EXIT_SUCCESS;
}

void join_threads(pthread_t *threads, void *thread_res,size_t size)
{
    int index = 0,res;

    for(index;index < size;index++)
    {
        res = pthread_join(threads[index],&thread_res);
        if(res != 0)
        {
            perror("thread join failed");
            exit(EXIT_FAILURE);
        }
        printf("Program: %s\n",(char *)thread_res);
    }
}

void work_part_init(workpart *curpart,int *array)
{
    //memcpy(curpart.mass, array, sizeof(curpart.mass));
    int i = 0;
    for(i;i < PART_SIZE;i++)
        curpart->mass[i] = array[i];
    curpart->count = PART_SIZE;
}

void merge_part_init(mergepart *part, int *array1,int *array2,int *resultarray,size_t size)
{
    int i = 0;
    for(i;i < PART_SIZE;i++)
    {
        part->part1[i] = array1[i];
        part->part2[i] = array2[i];
    }

    i = 0;
    for(i; i < INTERMEDIATE_SIZE;i++)
        part->resarray[i] = resultarray[i];

    part->curpartcount = size;
}

void merge_final_part_init(mergefinal *part, int *array1, int *array2, int *resultarray, size_t size)
{
    int i = 0;
    for(i;i < INTERMEDIATE_SIZE;i++)
    {
        part->part1[i] = array1[i];
        part->part2[i] = array2[i];
    }

    i = 0;
    for(i; i < GLOBAL_SIZE;i++)
        part->resarray[i] = resultarray[i];

    part->curpartcount = size;
}

void *shell_sort(void *curworkpart)
{
    int n = ((workpart *)curworkpart)->count;
    int *a = ((workpart *)curworkpart)->mass;

    int step = n / 2;
    int j;

    while (step > 0)
    {
        int i = step;
        for (; i < n - step; i++)
        {
            j = i;
            while ((j >= 0) && (a[j]) > a[j + step])
            {
                int buf = a[j];
                a[j] = a[j + step];
                a[j + step] = buf;
                j--;
            }
        }
        step /= 2;
    }
    pthread_exit("Thread exit");
}

void *merge_sort(void *curmergepart)
{
   // sem_wait(&bin_sem);
    int n;
    int *a,*b,*c;

    if(!type_id)
    {
        n = ((mergepart *)curmergepart)->curpartcount;
        a = ((mergepart *)curmergepart)->part1;
        b = ((mergepart *)curmergepart)->part2;
        c = ((mergepart *)curmergepart)->resarray;
    }
    else
    {
        n = ((mergefinal *)curmergepart)->curpartcount;
        a = ((mergefinal *)curmergepart)->part1;
        b = ((mergefinal *)curmergepart)->part2;
        c = ((mergefinal *)curmergepart)->resarray;
    }

    int i = 0,j = 0,index = 0;

    while(i < n && j < n)
    {
        if(a[i] < b[j])
        {
            c[index] = a[i++];
        }
        else
        {
            c[index] = b[j++];
        }
        index++;
    }

    while(i < n)
        c[index++] = a[i++];

    while(j < n)
        c[index++] = b[j++];

    pthread_exit("Merge sort is Ok");
}

void time_start()
{
    gettimeofday(&tim, NULL);
    time_1 = tim.tv_sec + (tim.tv_usec/1000000.0);
}

void time_stop()
{
    gettimeofday(&tim, NULL);
    time_2 = tim.tv_sec+(tim.tv_usec/1000000.0);
}

void read_from_file(const char* filename,int *array1,int *array2,int *array3,int *array4)
{
    FILE* fo;
    fo = fopen(filename, "r");
    int size = PART_SIZE;

    if (fo == 0)
    {
        perror(filename);
        exit(EXIT_FAILURE);
    }

    while(size-- && fscanf(fo, "%d", array1++));
    size = PART_SIZE;
    while(size-- && fscanf(fo, "%d", array2++));
    size = PART_SIZE;
    while(size-- && fscanf(fo, "%d", array3++));
    size = PART_SIZE;
    while(size-- && fscanf(fo, "%d", array4++));

    fclose(fo);
}

void write_to_file(const char* filename, int* array, size_t size)
{
    FILE* fo;
    fo = fopen(filename, "w");

    if (fo == 0)
    {
        perror(filename);
        exit(EXIT_FAILURE);
    }

    while (size--)
    {
        fprintf(fo, "%d ", *array++);
    }

    fclose(fo);
}

void write_to_inputfile(const char *filename)
{
    FILE *fo;
    fo = fopen(filename,"w");
    int size = GLOBAL_SIZE;
    int randnumber;

    if(fo == 0)
    {
        perror(filename);
        exit(EXIT_FAILURE);
    }

    while(size--)
    {
        randnumber = 1 + rand() % 9;
        fprintf(fo, "%d ", randnumber);
    }
    fclose(fo);
}
