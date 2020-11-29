#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define N 32 

struct data {
	int m;
	int n;
    int procNum;
};
pthread_t tid[N];
int a[N];     /* Array to add */
void genvalues(int n); /* Initializes  a[] */
void prnvalues(int n); /* Prints a[] */
void sum_array(int n); /* Displays the sum of the array */
//void addarray(struct data * p); 
void thread_addarray(struct data *p);

void main() {
    genvalues(N);    /* Initialize a[] with random values */
    printf("Initial values:\n");
    prnvalues(N);    /* Display the values */
    sum_array(N);    /* Display the sum value */

    for (int size=1; size<N; size=2*size) {
        printf("*** adding a[i+%d] to a[i]\n",size);
        int procNum = 0;
        for (int i=0; i+size<N; i+=2*size) {
            struct data p;
            p.m = i;
            p.n = i+size;
            p.procNum = procNum;
            thread_addarray(&p);
            procNum++;
        }
        for(int j =0; j<procNum; j++){
            pthread_join(tid[j] ,NULL);
        }
    }
    printf("\nOutput:\n");
    prnvalues(N); /* Display the sorted values */

}
//make this a function pointer to be ran in a thread

void *addarray(void *p){
    printf("creating thread: %d\n", ((struct data*)p)->procNum);
    //pthread_detach(pthread_self());
    a[((struct data*)p)->m] += a[((struct data*)p)->n];
    //exit the thread
    printf("exiting thread: %d\n", ((struct data*)p)->procNum);
    pthread_exit(NULL);
}

void thread_addarray(struct data *p){
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    //create thread to run addarray
    pthread_create(&tid[p->procNum], &attr, &addarray, (void *)p);
    //printf("return of join: %d\n",pthread_join(tid[p->procNum], NULL));
    //printf("thread func side Process ID: %lu, procNum: %d\n",tid[p->procNum], p->procNum);
}

/* Initializes array a[] with random values. */
void genvalues(int n){
    int k = 2*n;
    int current = 0;
    for (int i=0; i<n; i++) {
    current = (current*73 + 19)%k;
    a[i] = current;
    }
}

/* Prints the values in the array a[] */
void prnvalues(int n)
{
for (int i=0; i<n; i++) {
   printf(" %d ", a[i]);
   if ((i+1)%10 == 0) printf("\n");
}
printf("\n");
}

/* Ssum the array */
void sum_array(int n)
{
int sum = 0;
for (int i=0; i<n; i++) {
    sum += a[i];
}
printf("Sum of the array =%d\n", sum);
}

