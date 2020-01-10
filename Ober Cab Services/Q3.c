#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

int N, M, K;

typedef struct cab
{
    int id, type;
    int state;
} cab;

typedef struct rider
{
    int id, type, cabID, RideTime, maxWaitTime;
    int waiting;
    int found;
    int arrivalTime;
    int completed;
    int paid;

    pthread_t tid;
    pthread_mutex_t lockRider;
    pthread_cond_t condLockRider;
} rider;

typedef struct server
{
    int id;
    int status;
    pthread_t tid;
} paymentServer;

#define L 100005

int paymentDone = 0;
pthread_mutex_t lockPayment;

cab cabss[L];
rider riders[L];
paymentServer servers[L];

sem_t payment;
pthread_mutex_t lockCab;

void rideEnded(int id)
{
    printf("Rider %d has got a ride in Cab %d\n", id, riders[id-1].cabID);
    sleep(riders[id-1].RideTime);
    printf("Rider %d has completed his ride in Cab %d\n", id, riders[id-1].cabID);

    pthread_mutex_lock(&riders[id-1].lockRider);
    printf("Rider %d is ready to make payment\n", id);
    riders[id-1].completed = 1;
    pthread_mutex_unlock(&riders[id-1].lockRider);
    
    pthread_mutex_lock(&lockCab);

    if(cabss[riders[id-1].cabID - 1].state == 3)
    {
        cabss[riders[id-1].cabID - 1].state = 0;

        for(int i = 0; i < M; i++)
        {
            if(riders[i].waiting == 1)
            {
                riders[i].waiting = 0;
                
                if(riders[i].type == 1)
                {
                    cabss[riders[id-1].cabID - 1].state = 1;   
                    riders[i].cabID = riders[id-1].cabID;
                    riders[i].found = 1;
                }

                else
                {
                    cabss[riders[id-1].cabID - 1].state = 3;
                    riders[i].cabID = riders[id-1].cabID;
                    riders[i].found = 1;
                }
            }
                
            if(cabss[riders[id-1].cabID - 1].state > 0)
            {
                pthread_cond_signal(&(riders[i].condLockRider));
                break;
            }            
        }
    }

    else if(cabss[riders[id-1].cabID - 1].state == 1)
    {
        cabss[riders[id-1].cabID - 1].state = 0;

        for(int i = 0; i < M; i++)
        {
            if(riders[i].waiting == 1)
            {
                riders[i].waiting = 0;
                
                if(riders[i].type == 1)
                {
                    cabss[riders[id-1].cabID - 1].state = 1;   
                    riders[i].cabID = riders[id-1].cabID;
                    riders[i].found = 1;
                }

                else
                {
                    cabss[riders[id-1].cabID - 1].state = 3;
                    riders[i].cabID = riders[id-1].cabID;
                    riders[i].found = 1;
                }
            }
                
            if(cabss[riders[id-1].cabID - 1].state > 0)
            {
                pthread_cond_signal(&(riders[i].condLockRider));
                break;
            }            
        }
    }

    else
    {   
        cabss[riders[id-1].cabID - 1].state = 1;

        for(int i = 0; i < M; i++)
        {
            if(riders[i].waiting == 1 && riders[i].type == 1)
            {
                riders[i].waiting = 0;
                cabss[riders[id-1].cabID - 1].state = 2;
                riders[i].cabID = riders[id-1].cabID;
                riders[i].found = 1;
            }

            if(cabss[riders[id-1].cabID - 1].state > 0)
            {
                pthread_cond_signal(&(riders[i].condLockRider));
                break;
            }
        }
    }

    pthread_mutex_unlock(&lockCab);   
}

void *BookCab(void *arg)
{
    rider *r = (rider *)arg;
    sleep(r -> arrivalTime);

    // srand(time(0));
    r -> RideTime = 1 + rand()%10;
    r -> maxWaitTime = 1 + rand()%6;    
    r -> type = 1 + rand()%2;

    if(r -> type == 1)
    {
        printf("Rider %d requires a Pool Cab, and will wait for a maximum of %d seconds\n", r -> id, r -> maxWaitTime);
    }

    else
    {
        printf("Rider %d requires a Premier Cab, and will wait for a maximum of %d seconds\n", r -> id, r -> maxWaitTime);
    }

    pthread_mutex_lock(&lockCab);

    if(r -> type == 1)
    {
        for(int i = 0; i < N; i++)
        {
            if(cabss[i].state == 1)
            {
                cabss[i].state = 2;
                r -> cabID = cabss[i].id;
                r -> found = 1;
                break;
            }
        }

        if(r -> found == 0)
        {
            for(int i = 0; i < N; i++)
            {
                if(cabss[i].state == 0)
                {
                    cabss[i].state = 1;
                    r -> cabID = cabss[i].id;
                    r -> found = 1;
                    break;
                }
            }
        }
    }

    else
    {
        // while(r -> found == 0)
        // {
            for(int i = 0; i < N; i++)
            {
                if(cabss[i].state == 0)
                {
                    cabss[i].state = 3;
                    r -> cabID = cabss[i].id;
                    r -> found = 1;
                    break;
                }
            } 
        // }
    }

    if(r -> found == 0)
    {
        r -> waiting = 1;

        struct timespec t;
        clock_gettime(CLOCK_REALTIME, &t);
        // int waitTill = t.tv_sec + r -> maxWaitTime;
        t.tv_sec += r -> maxWaitTime;
        pthread_cond_timedwait(&(r -> condLockRider), &lockCab, &t);

        r -> waiting = 0;
    }

    pthread_mutex_unlock(&lockCab);

    if(r -> found == 0)
    {
        pthread_mutex_lock(&lockPayment);
        paymentDone++;
        pthread_mutex_unlock(&lockPayment);
        
        printf("Rider %d exited due to timeout\n", r -> id);
    }

    else
    {
        rideEnded(r -> id);
    }

    return NULL;
}

void *makePayment(void *arg)
{
    paymentServer *s = (paymentServer*) arg;

    while(1)
    {
        for(int i = 0; i < M; i++)
        {
            pthread_mutex_lock(&riders[i].lockRider);

            if(riders[i].completed == 1 && riders[i].paid == 0)
            {
                sem_wait(&payment);

                riders[i].paid = 1;

                printf("Server %d has accepted payment on Rider %d\n", s -> id, riders[i].id);

                pthread_mutex_unlock(&riders[i].lockRider);

                sleep(2);

                pthread_mutex_lock(&lockPayment);
                paymentDone++;
                pthread_mutex_unlock(&lockPayment);
                
                sem_post(&payment);

                // pthread_mutex_lock(&riders[i].lockRider);
            }

            else
            {
                pthread_mutex_unlock(&riders[i].lockRider);
            }
        }
    }

    return NULL;
}

int main()
{
    srand(time(0));

    printf("Enter the number of cabs: ");
    scanf("%d", &N);

    printf("Enter the number of riders: ");
    scanf("%d", &M);

    printf("Enter the number of payment servers: ");
    scanf("%d", &K);

    printf("\n\n");

    if((N == 0 || K == 0) && (M > 0))
    {
        printf("Incompatible Setup\n");
        return 0;
    }
    
    sem_init(&payment, 0, K);
    pthread_mutex_init(&lockPayment, NULL);
    pthread_mutex_init(&lockCab, NULL);

    for(int i = 0; i < N; i++)
    {
        cabss[i].id = i + 1;
        cabss[i].type = 0;
        cabss[i].state = 0;
    }

    for(int i = 0; i < M; i++)
    {
        riders[i].id = i + 1;
        riders[i].waiting = 0;
        riders[i].found = 0;
        riders[i].arrivalTime = rand()%7;

        pthread_mutex_init(&(riders[i].lockRider), NULL);
        pthread_cond_init(&(riders[i].condLockRider), NULL);
        
        if(pthread_create(&(riders[i].tid), NULL, BookCab, &riders[i]))
        {
            perror("Cannot Create Thread for Riders\n");
        }

        // sleep(1);
    }

    for(int i = 0; i < K; i++)
    {
        servers[i].id = i + 1;
        servers[i].status = 0;
        
        if(pthread_create(&(servers[i].tid), NULL, makePayment, &servers[i]))
        {
            perror("Cannot Create Thread for Payment Servers\n");
        }
    }

    for(int i = 0; i < M; i++)
    {
        pthread_join(riders[i].tid, NULL);
    }

    while(paymentDone < M)
    {
        // printf("** %d **\n", paymentDone);
        // do nothing;
    }

    printf("\n\nSimulation Over.\n");

    sem_destroy(&payment);

    // for(int i = 0; i < M; i++)
    // {
    //     pthread_join(servers[i].tid, NULL);
    // }

    return 0;
}