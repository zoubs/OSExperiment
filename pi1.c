#include<stdio.h>
#include<unistd.h>
#include<pthread.h>
#define N 10000000

double sum_head = 0, sum_rear = 0;
void *worker(void* arg)
{
    int sign, i;
    for(i=N;i>N/2;i--)
    {
        sign = (i % 2)? -1 : 1;
        sum_rear += (sign)* 1/(double)(2*i+1);
    }
}
void master(int n)
{
    int sign, i;
    for(i=0;i<=n;i++)
    {
        sign = (i % 2)? -1 : 1;
        sum_head += (sign)* 1/(double)(2*i+1);
    }
}
int main()
{
    pthread_t worker_tid;
    double total;
    pthread_create(&worker_tid, NULL, &worker, NULL);
    master(N/2);
    pthread_join(worker_tid, NULL);
    total = sum_head + sum_rear;
    printf("PI = %f\n",4*total);
    return 0;
}