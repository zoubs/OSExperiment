#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>

#define CAPACITY 4
#define ITEM_COUNT (2*CAPACITY)

char buffer1[CAPACITY];
char buffer2[CAPACITY];


int in1, in2, out1, out2;


typedef struct pa
{
    char* buffer;
    int* in;
    int* out;
}Param;

typedef struct se
{
    int value;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Sema;

Sema mutex_sema1, mutex_sema2;
Sema empty_buffer_sema1, empty_buffer_sema2;
Sema full_buffer_sema1, full_buffer_sema2;


char get_item(Param param)
{
    char item;
    item = param.buffer[*param.out];
    *param.out = (*param.out + 1) % CAPACITY;
    return item;
}

void put_item(char item, Param param)
{
    param.buffer[*param.in] = item;
    *param.in = (*param.in + 1) % CAPACITY;
}

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
        pthread_cond_wait(&sema->cond, &sema->mutex);
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

void* consume(void* arg)
{
    int i;
    char item;
    Param p;
    p.buffer = buffer2;
    p.in = &in2;
    p.out = &out2;
    for(i = 0; i < ITEM_COUNT;i++)
    {
        sema_wait(&full_buffer_sema2);
        sema_wait(&mutex_sema2);

        item = get_item(p);
        printf("\033[32m consume item: %c\n\033[0m", item);

        sema_signal(&mutex_sema2);
        sema_signal(&empty_buffer_sema2);
    }
}

void* compute(void* arg)
{
    Param param[2];
    param[0].buffer = buffer1;
    param[0].in = &in1;
    param[0].out = &out1;
    param[1].buffer = buffer2;
    param[1].in = &in2;
    param[1].out = &out2;
    int i;
    char item;

    for(i = 0; i < ITEM_COUNT;i++)
    {
        sema_wait(&full_buffer_sema1);
        sema_wait(&mutex_sema1);

        item = get_item(param[0]);
        item -= 32;
        printf("\033[33m compute item: %c\n\033[0m", item);

        sema_signal(&mutex_sema1);
        sema_signal(&empty_buffer_sema1);

        sema_wait(&empty_buffer_sema2);
        sema_wait(&mutex_sema2);

        put_item(item, param[1]);

        sema_signal(&mutex_sema2);
        sema_signal(&full_buffer_sema2);
    }
}

void* produce(void* arg)
{
    Param p;
    p.buffer = buffer1;
    p.in = &in1;
    p.out = &out1;
    int i;
    char item;
    item = 'a';
    for(i=0;i<ITEM_COUNT;i++)
    {
        sema_wait(&empty_buffer_sema1);
        sema_wait(&mutex_sema1);
        printf("\033[31m produce item: %c\n\033[0m", item);
        put_item(item, p);

        sema_signal(&mutex_sema1);
        sema_signal(&full_buffer_sema1);
        item++;
    }
}

int main()
{
    sema_init(&empty_buffer_sema1, CAPACITY);
    sema_init(&mutex_sema1, 1);
    sema_init(&full_buffer_sema1, 0);
    sema_init(&mutex_sema2, 1);
    sema_init(&empty_buffer_sema2, CAPACITY);
    sema_init(&full_buffer_sema2, 0);

    pthread_t producer_tid, consumer_tid, computer_tid;
    pthread_create(&producer_tid, NULL, &produce, NULL);
    pthread_create(&computer_tid, NULL, &compute, NULL);
    pthread_create(&consumer_tid, NULL, &consume, NULL);

    pthread_join(producer_tid,NULL);
    pthread_join(computer_tid, NULL);
    pthread_join(consumer_tid, NULL);
    return 0;
}