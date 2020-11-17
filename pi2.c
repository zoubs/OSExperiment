#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

#define N_CPU 4
#define N 10000000
struct param
{
    int start;
    int end;
};
struct result
{
    float sum;
};

void *worker(void *arg)
{
    int sign, i;
    struct param* p;
    struct result* res;
    float sum = 0;
    p = (struct param*)arg;
    for(i = p->start;i < p->end;i++)
    {
        sign = (i % 2) ? -1 : 1;
        sum += (sign)* 1/(float)(2*i+1);
    }
    res = malloc(sizeof(struct param));
    res->sum = 0;
    res->sum = sum;
    return res;
}
int main()
{
    struct param para[N_CPU];
    
    pthread_t worker_pit[N_CPU];
    int part = N / N_CPU;
    float sum = 0;
    for(int i =0; i < N_CPU;i++)
    {
        struct param* p;
        p = &para[i];
        p->start = i * part;
        p->end = (i + 1) * part;
        pthread_create(&worker_pit[i], NULL,&worker, p);
    }
    for(int i = 0; i < N_CPU; i++)
    {
        struct result* res;
        pthread_join(worker_pit[i],(void**) &res);
        sum += res->sum;
        free(res);
    }
    printf("PI = %f\n", 4*sum);
    return 0;
}