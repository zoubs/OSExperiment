#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define CAPACITY 4
#define item_count CAPACITY*2
char buffer1[CAPACITY];
char buffer2[CAPACITY];
typedef struct pa
{
    char* buffer;   //代表要操作的缓冲区
    int* in;        //该缓冲区的写指针位置
    int* out;       //该缓冲区的读指针位置
}Param;
int in1, in2, out1, out2;

pthread_cond_t wait_buffer1_full,wait_buffer1_empty,wait_buffer2_full,wait_buffer2_empty;
pthread_mutex_t mutex1, mutex2;
int buffer_is_full(Param param)
{
    return (*param.in + 1) % CAPACITY == *param.out;
}
int buffer_is_empty(Param param)
{
    return (*param.in == *param.out);
}
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
void* consume(void* arg)
{
    Param p;
    p.buffer = buffer2;
    p.in = &in2;
    p.out = &out2;
    char item;
    int i;
    for(i=0;i<item_count;i++)
    {
        pthread_mutex_lock(&mutex2);
        while(buffer_is_empty(p))
        {
            pthread_cond_wait(&wait_buffer2_full,&mutex2);
        }
        item = get_item(p);
        printf("\033[32m consume item: %c\n\033[0m", item);
        pthread_cond_signal(&wait_buffer2_empty);
        pthread_mutex_unlock(&mutex2);
    }
}
void* produce(void* arg)
{
    Param p;
    p.buffer = buffer1;
    p.in = &in1;
    p.out = &out1;
    char item = 'a';
    int i;
    for(i=0; i < item_count;i++)
    {
        pthread_mutex_lock(&mutex1);
        while(buffer_is_full(p))
        {
            pthread_cond_wait(&wait_buffer1_empty,&mutex1);
        }
        put_item(item, p);
        printf("\033[31m produce item: %c\n\033[0m", item);
        pthread_cond_signal(&wait_buffer1_full);
        pthread_mutex_unlock(&mutex1);
        item++;
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
    for(i=0;i<item_count;i++)
    {
        pthread_mutex_lock(&mutex1);
        while(buffer_is_empty(param[0]))
        {
            pthread_cond_wait(&wait_buffer1_full, &mutex1);
        }
        
        item = get_item(param[0]);
        item -= 32;
        pthread_cond_signal(&wait_buffer1_empty);
        pthread_mutex_unlock(&mutex1);
        
        pthread_mutex_lock(&mutex2);
        while(buffer_is_full(param[1]))
        {
            pthread_cond_wait(&wait_buffer2_empty, &mutex2);
        }
        printf("\033[33m compute item: %c\n\033[0m", item);
        put_item(item, param[1]);
        pthread_cond_signal(&wait_buffer2_full);
        pthread_mutex_unlock(&mutex2);
    }
}
int main()
{
    pthread_mutex_init(&mutex1, NULL);
    pthread_mutex_init(&mutex2, NULL);
    pthread_cond_init(&wait_buffer1_empty, NULL);
    pthread_cond_init(&wait_buffer1_full,NULL);
    pthread_cond_init(&wait_buffer2_empty, NULL);
    pthread_cond_init(&wait_buffer2_full, NULL);

    pthread_t producer_tid, consumer_tid, computer_tid;
    pthread_create(&producer_tid, NULL, &produce, NULL);
    pthread_create(&computer_tid, NULL, &compute, NULL);
    pthread_create(&consumer_tid, NULL, &consume, NULL);

    pthread_join(producer_tid,NULL);
    pthread_join(computer_tid, NULL);
    pthread_join(consumer_tid, NULL);
    return 0;
}