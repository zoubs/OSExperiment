#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#define N 100

int array[N];
typedef struct param
{
    int start; //起始下标
    int end;   //结束下标,end无效
} Para;

void SelectSort(Para *para)
{
    int n = para->end - para->start;
    int *a = &array[para->start];
    int min;
    int i, j;
    for (i = 0; i < n; i++)
    {
        min = i;
        for (j = i + 1; j < n; j++)
        {
            if (a[j] < a[min])
            {
                min = j;
            }
        }
        if (min != i)
        {
            int temp = a[i];
            a[i] = a[min];
            a[min] = temp;
        }
    }
}
void *worker(void *arg)
{
    Para* p = (Para*)arg;
    SelectSort(p);
}
void gen_array()
{
    srandom(time(NULL));
    for (int i = 0; i < N; i++)
    {
        array[i] = rand() % 1000;
        //printf("array[%d] = %d", i,array[i]);
    }
}
void print_array(int* arr, int a,int b)
{
    printf("array[] = {");
    for(int i = a; i < b;i++)
    {
        printf("%d", arr[i]);
        if(i != b-1)
        {
            printf(", ");
        }
    }
    printf("}\n");
}
void merge()
{
    int arr[N];
    int i = 0, j = N/2, k = 0;
    while(i < N/2 && j < N)
    {
        if(array[i] < array[j])
        {
            arr[k++] = array[i++];
        }
        else
        {
            arr[k++] = array[j++];
        }
    }
    if(i < N/2)
    {
        while(i < N/2)
        {
            arr[k++] = array[i++];
        }
    }
    else if(j < N)
    {
        while(j < N)
        {
            arr[k++] = array[j++];
        }
    }
    for(i = 0;i < N;i++)
    {
        array[i] = arr[i];
    }
}
int main()
{
    pthread_t worker_tid[2];
    gen_array();
    printf("unsorted:\n");
    print_array(array, 0, N);
    Para pa[2];
    pa[0].start = 0;
    pa[0].end = N/2;
    pa[1].start = N/2;
    pa[1].end = N;
    Para* p[2];
    p[0] = &pa[0];
    p[1] = &pa[1];
    //SelectSort(p[0]);
    pthread_create(&worker_tid[0], NULL, &worker, p[0]);
    pthread_create(&worker_tid[1], NULL, &worker, p[1]);
    pthread_join(worker_tid[0], NULL);
    pthread_join(worker_tid[1], NULL);


    merge();
    printf("\nsorted:\n");
    print_array(array,0,N);
    return 0;
}