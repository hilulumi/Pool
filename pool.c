#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#define NAMELEN 100

struct Job
{ 
   int id, head, end;
};
struct JOBQ
{
   void* (*fun)(struct Job);
   struct Job job;
   struct JOBQ *next;   
}*JQ, *back;


void* runner(void* index);
void* partition(struct Job A);
void* insert_sort(struct Job A);
void Add(struct JOBQ* N);
void swap(int *a, int *b);

int running=1;
pthread_t *thread;
int thread_num;
int *array;
int arr_size;
sem_t Sq;
sem_t Sw;
sem_t S[8];

int main(int argc, char* argv[])
{
	FILE *input, *output;
    char name[NAMELEN];  
	int i;
	
	printf("Input file name : ");
    fgets(name, NAMELEN, stdin);
    strtok(name, "\n");
    input=fopen(name, "r");
    output=fopen("output.txt", "w");
    
    printf("thread number : ");
    scanf("%d", &thread_num);       
 
    fscanf(input, "%d", &arr_size);
    array=malloc(sizeof(int)*arr_size);
    for(i=0; i<arr_size; i++)
      fscanf(input, "%d", &array[i]);
	  
    thread=malloc(thread_num*sizeof(pthread_t));  
	sem_init(&Sq, 0, 1);  
	sem_init(&Sw, 0, 0);  
	for(i=0; i<8; i++)
	    sem_init(&S[i], 0, 0);
    		
	for(i=0; i<thread_num; i++)
        pthread_creat(&thread[i], NULL, runner, (void*)&i);
    
    struct JOBQ *first=malloc(sizeof(struct JOBQ));
    first->job.id = 1;
    first->job.head = 0;
    first->job.end = arr_size-1;	
    first->fun = partition;
    Add(first);

    for(i=0; i<8; i++)
        sem_wait(&S[i]);

    runner=0;
	for(i=0; i<arr_size; i++)
        fprintf(output, "%d ", array[i]);

    sem_destroy(&Sq);
    sem_destroy(&Sw);
    for(i=0; i<8; i++)
        sem_destroy(&S[i]);
    fclose(input);		
	fclose(output);
	exit(0);
}

void* runner(void* index)
{
    while(running)
    {
	  if(running)
	  {
	    sem_wait(&Sw);
		void* (*func)(struct Job arg);
		struct Job arg;
		struct JOBQ* A;
		sem_wait(&Sq);
		A=JQ;
		JQ=JQ->next;
		A->next=NULL;
		func=A->fun;
		arg.id = A->job.id;
		arg.head = A->job.head;
		arg.end = A->job.end;
		sem_post(&Sq);
		func(arg);
		free(A);
	   }
       else
        break;  	   
	}
    pthread_exit(0);    
}

void* partition(struct Job A)
{
   int pivot = A.end;
   int less=A.head-1, more=A.head-1;
   int i;
   
   if(A.head >= A.end)
      pivot=A.end;
    else
	{
      for(i=A.head; i<pivot; i++)
      {
          if(a[i]<=a[pivot])
          {
             more++; 
             less++;
             swap(&a[less], &a[i]); 
          }   
          else 
             more++;
      }
      swap(&a[pivot], &a[less+1]);
      pivot = less+1;
	}
    
    struct JOBQ* njob1, njob2;
    njob1=malloc(sizeof(struct JOBQ));
    njob2=malloc(sizeof(struct JOBQ));
    
    njob1->job.id =	A.id*2;
	njob1->job.head = A.head;
	njob1->job.end = pivot-1;
	njob1->next=NULL;
	if(njob1->job.id < 8)
	   njob1->fun = partition;
	else 
       njob1->fun = insert_sort;

    njob2->job.id =	A.id*2+1;
	njob2->job.head = pivot+1;
	njob2->job.end = A.end;
	njob2->next=NULL;
	if(njob2->job.id < 8)
	   njob2->fun = partition;
	else 
       njob2->fun = insert_sort;

    sem_wait(&Sq);
    Add(njob1);	
	Add(njob2);
	sem_post(&Sq);
}

void* insert_sort(struct Job A)
{
   if(A.head >= A.end)
     return;

   int i, j;
   for(i=A.head; i<A.end; i++)
      for(j=i+1; j<=A.end; j++)
          if(a[i]>a[j])
             swap(&a[i], &a[j]);
			 
	sem_post(&S[A.id-8]);		 
}

void Add(struct JOBQ* N)
{
    if(JQ==NULL)
	{
	   JQ=N;
	   back=N;
	}
	else
	{
	   back->next=N;
	   back=back->next;
	}
    
	sem_post(&Sw);
}

void swap(int *a, int *b)
{
    int temp=*a;
    *a = *b; 
    *b = temp; 
}