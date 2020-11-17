#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#define N 20
int buffer[N];

typedef struct
{
    int value;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Sema;

typedef struct
{
    int num;
} Param;

Sema sema_mutex[N];
Sema full_buffer_sema[N];

void sema_init(Sema *sema, int value)
{
    sema->value = value;
    pthread_mutex_init(&sema->mutex, NULL);
    pthread_cond_init(&sema->cond, NULL);
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
    ++sema->value;
    pthread_cond_signal(&sema->cond);
    pthread_mutex_unlock(&sema->mutex);
}

void *consume_produce(void *arg)
{
    int item, num;
    Param *p;
    p = (Param *)arg;
    num = p->num;

    if (num == 0)  //T1
    {
        sema_wait(&sema_mutex[num + 1]);

        buffer[num + 1] = 1;

        sema_signal(&sema_mutex[num + 1]);
        sema_signal(&full_buffer_sema[num + 1]);

        sema_wait(&full_buffer_sema[num]);
        sema_wait(&sema_mutex[num]);

        item = buffer[num];
        printf("线程 %d received: %d\n", num + 1, item);

        sema_signal(&sema_mutex[num]);
    }
    else if (num == N - 1) //TN
    {
        sema_wait(&full_buffer_sema[num]);
        sema_wait(&sema_mutex[num]);

        item = buffer[num];
        printf("线程 %d received: %d\n", num + 1, item);

        sema_signal(&sema_mutex[num]);
        sema_wait(&sema_mutex[0]);

        buffer[0] = item + 1;

        sema_signal(&sema_mutex[0]);
        sema_signal(&full_buffer_sema[0]);
    }
    else  //其余
    {
        sema_wait(&full_buffer_sema[num]);
        sema_wait(&sema_mutex[num]);

        item = buffer[num];
        printf("线程 %d received: %d\n", num + 1, item);

        sema_signal(&sema_mutex[num]);

        sema_wait(&sema_mutex[num + 1]);

        buffer[num + 1] = item + 1;

        sema_signal(&sema_mutex[num + 1]);
        sema_signal(&full_buffer_sema[num + 1]);
    }
}

int main()
{
    pthread_t tid[N];
    Param param[N];
    for (int i = 0; i < N; i++)
    {
        sema_init(&full_buffer_sema[i], 0);
        sema_init(&sema_mutex[i], 1);
    }

    for (int i = 0; i < N; i++)
    {
        param[i].num = i;
        pthread_create(&tid[i], NULL, &consume_produce, &param[i]);
    }

    for (int i = 0; i < N; i++)
    {
        pthread_join(tid[i], NULL);
    }
    return 0;
}