#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>

#define N 20 //N个线程

typedef struct se
{
    int value;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
} Sema;

typedef struct pa
{
    int num;
} Param;

Sema empty_buffer_sema[N], full_buffer_sema[N];
Sema sema_mutex[N];
int buffer[N];

void put_item(int n, int item)
{
    buffer[n] = item;
}

int get_item(int n)
{
    return buffer[n];
}
void sema_init(Sema *sema, int value)
{
    sema->value = value;
    pthread_cond_init(&sema->cond, NULL);
    pthread_mutex_init(&sema->mutex, NULL);
}
void sema_wait(Sema *sema)
{
    pthread_mutex_lock(&sema->mutex);
    while (sema->value <= 0)
    {
        pthread_cond_wait(&sema->cond, &sema->mutex);
    }
    sema->value--;
    pthread_mutex_unlock(&sema->mutex);
}

void sema_signal(Sema *sema)
{
    pthread_mutex_lock(&sema->mutex);
    sema->value++;
    pthread_cond_signal(&sema->cond);
    pthread_mutex_unlock(&sema->mutex);
}
void* consume_produce(void *arg)
{
    Param *p;
    int item;
    p = (Param *)arg;
    int last, next;
    last = (p->num - 1 + N) % N;
    next = (p->num + 1 + N) % N;
    sema_wait(&full_buffer_sema[last]);
    sema_wait(&sema_mutex[last]);

    item = get_item(last);
    printf("线程%d received: %d\n", p->num + 1, item);

    sema_signal(&sema_mutex[last]);
    sema_signal(&empty_buffer_sema[last]);

    item++;

    sema_wait(&empty_buffer_sema[next]);
    sema_wait(&sema_mutex[next]);

    put_item(next, item);
    //printf("线程%d sent: %d\n", p->num, item);
    item++;
    sema_signal(&sema_mutex[next]);
    sema_signal(&full_buffer_sema[next]);
}

int main()
{
    printf("i = /n");
    int i;
    Param param[N];
    
    sema_init(&full_buffer_sema[N - 1], 1);
    sema_init(&empty_buffer_sema[0], 0);
    sema_init(&sema_mutex[0], 1);
    for (i = 0; i < N - 1; i++)
    {
        sema_init(&full_buffer_sema[i], 0);
        sema_init(&empty_buffer_sema[i], 1);
        sema_init(&sema_mutex[i], 1);
    }

    pthread_t tid[N];
    for (i = 0; i < N; i++)
    {
        param[i].num = i;
        printf("i = %d/n", i);
        pthread_create(&tid[i], NULL, &consume_produce, &param[i]);
    }
    for (i = 0; i < N; i++)
    {
        pthread_join(tid[i], NULL);
    }
    return 0;
}
