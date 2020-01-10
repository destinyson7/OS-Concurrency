#define _POSIX_C_SOURCE 199309L

#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

typedef struct qSort
{
    int l, r;
    int *a;
} qSort;

void swap(int *arr, int i, int j)
{
    int temp = arr[i];
    arr[i] = arr[j];
    arr[j] = temp;
}

void concQuickSort(int *ff, int l, int r)
{
    if(l >= r)
    {
        return;
    }

    if(r - l + 1 < 5)
    {
        for(int i = l + 1; i <= r; i++)
        {
            int temp = i - 1;
            int key = ff[i];
            
            while(temp >= l && ff[temp] > key)
            {
                ff[temp + 1] = ff[temp];
                temp--;
            }

            ff[temp + 1] = key;
        }

        return;
    }

    int len = r - l + 1;
    int where = l + rand()%len;

    swap(ff, where, r);

    int pivot = ff[r];
    int temp = l - 1;

    for(int i = l; i < r; i++)
    {
        if(ff[i] <= pivot)
        {
            temp++;
            swap(ff, i, temp);
        }
    }

    swap(ff, temp + 1, r);

    int lpid = fork();

    if(lpid == 0)
    {
        concQuickSort(ff, l, temp);
        _exit(0);
    }

    else if(lpid > 0)
    {
        int rpid = fork();

        if(rpid == 0)
        {
            concQuickSort(ff, temp + 2, r);
            _exit(0);
        }

        else if(rpid > 0)
        {
            int status;
            waitpid(lpid, &status, 0);
            waitpid(rpid, &status, 0);
        }
    }
}

void quickSort(int *ss, int l, int r)
{
    if(l >= r)
    {
        return;
    }

    if(r - l + 1 < 5)
    {
        for(int i = l + 1; i <= r; i++)
        {
            int temp = i - 1;
            int key = ss[i];
            
            while(temp >= l && ss[temp] > key)
            {
                ss[temp + 1] = ss[temp];
                temp--;
            }

            ss[temp + 1] = key;
        }

        return;
    }

    int len = r - l + 1;
    int where = l + rand()%len;

    swap(ss, where, r);

    int pivot = ss[r];
    int temp = l - 1;

    for(int i = l; i < r; i++)
    {
        if(ss[i] <= pivot)
        {
            temp++;
            swap(ss, i, temp);
        }
    }

    swap(ss, temp + 1, r);

    quickSort(ss, l, temp);
    quickSort(ss, temp + 2, r);
}

void *threadQuickSort(void *arg)
{
    struct qSort *cur = (struct qSort*) arg;

    int l = cur -> l;
    int r = cur -> r;
    int *tt = cur -> a;

    if(l >= r)
    {
        return NULL;
    }

    if(r - l + 1 < 5)
    {
        for(int i = l + 1; i <= r; i++)
        {
            int temp = i - 1;
            int key = tt[i];
            
            while(temp >= l && tt[temp] > key)
            {
                tt[temp + 1] = tt[temp];
                temp--;
            }

            tt[temp + 1] = key;
        }

        return NULL;
    }

    int len = r - l + 1;
    int where = l + rand()%len;

    swap(tt, where, r);

    int pivot = tt[r];
    int temp = l - 1;

    for(int i = l; i < r; i++)
    {
        if(tt[i] <= pivot)
        {
            temp++;
            swap(tt, i, temp);
        }
    }

    swap(tt, temp + 1, r);

    pthread_t ltid;
    struct qSort left;
    left.l = l;
    left.r = temp;
    left.a = tt;
    pthread_create(&ltid, NULL, threadQuickSort, &left);

    pthread_t rtid;
    struct qSort right;
    right.l = temp + 2;
    right.r = r;
    right.a = tt;
    pthread_create(&rtid, NULL, threadQuickSort, &right);

    pthread_join(ltid, NULL);
    pthread_join(rtid, NULL);

    return NULL;
}

int main()
{
    int n;
    printf("Enter the number of elements in the array: ");
    scanf("%d", &n);

    key_t key = IPC_PRIVATE;
    int id = shmget(key, sizeof(int)*(n+1), IPC_CREAT | 0666);
    int *ff = (int*)shmat(id, NULL, 0);

    printf("Enter the elements\n");
    for(int i = 0; i < n; i++)
    {
        scanf("%d", &ff[i]);
    }
    printf("\n\n");

    int *ss = (int *) malloc(sizeof(int)*(n+1));
    int *tt = (int *) malloc(sizeof(int)*(n+1));

    for(int i = 0; i < n; i++)
    {
        ss[i] = ff[i];
        tt[i] = ff[i];
    }

    struct timespec t;

    printf("Concurrent Quick Sort:\n");
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    long double s = t.tv_nsec/(1e9) + t.tv_sec;
    
    concQuickSort(ff, 0, n-1);

    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    long double e = t.tv_nsec/(1e9) + t.tv_sec;

    long double tconc = e - s;

    for(int i = 0; i < n; i++)
    {
        printf("%d ", ff[i]);
    }
    printf("\n");

    printf("Time Required: %Lf\n\n\n", tconc);

    shmdt(ff);

    printf("Normal Quick Sort:\n");
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    s = t.tv_nsec/(1e9) + t.tv_sec;
    
    quickSort(ss, 0, n-1);

    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    e = t.tv_nsec/(1e9) + t.tv_sec;

    long double tnorm = e - s;

    for(int i = 0; i < n; i++)
    {
        printf("%d ", ss[i]);
    }
    printf("\n");

    printf("Time Required: %Lf\n\n\n", tnorm);

    free(ss);

    struct qSort cur;
    cur.l = 0;
    cur.r = n - 1;
    cur.a = tt;

    pthread_t tid;

    printf("Threaded Quick Sort:\n");
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    s = t.tv_nsec/(1e9) + t.tv_sec;
    
    pthread_create(&tid, NULL, threadQuickSort, &cur);
    pthread_join(tid, NULL);

    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    e = t.tv_nsec/(1e9) + t.tv_sec;

    long double tthread = e - s;

    for(int i = 0; i < n; i++)
    {
        printf("%d ", tt[i]);
    }
    printf("\n");

    printf("Time Required: %Lf\n\n\n", tthread);

    free(tt);

    printf("Normal Quick Sort Runs %Lf times faster than Concurrent Quick Sort\n", tconc/tnorm);
    printf("Normal Quick Sort Runs %Lf times faster than Threaded Quick Sort\n", tthread/tnorm);

    return 0;
}