#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct chef
{
    int id, noOfVessels, p;
    int prepared;

    pthread_t tid;
    pthread_mutex_t lockChef;
} chef;

typedef struct table
{
    int id, noOfSlots, p;
    int found;
    int servedTillNow, slotsCopy;
    // int refilled;

    pthread_t tid;
    pthread_mutex_t lockTable;
} table;

typedef struct student
{
    int id, tableID;
    int found;
    int arrivalTime;

    pthread_t tid;
} student;

int N, M, K;

#define L 100005

chef chefs[L];
table tables[L];
student students[L];

pthread_mutex_t lockStudentsLeft;

void biryani_ready(int id)
{
    // printf("Biryani **** Ready\n");
    
    // pthread_mutex_lock(&(c -> lockChef));

    while(chefs[id-1].noOfVessels > 0 && K > 0)
    {
        // do nothing;
    }

    if(K > 0 && chefs[id-1].noOfVessels == 0)
    {
        printf("All the vessels prepared by Robot Chef %d are emptied. Resuming cooking now.\n", id);
    }

    // pthread_mutex_unlock(&(c -> lockChef));

    // printf("Biryani ***** Ready\n");
}

void *prepareBiryani(void *arg)
{
    chef *c = (chef *)arg;

    // printf("* here *\n");
    while(K > 0)
    {
        srand(time(0));
        int w = 2 + rand()%4;
        int noOfVessels = rand()%10 + 1;
        int serves = 24 + rand()%26 + 1;

        if(K > 0)
        {
            printf("Robot Chef %d is preparing %d vessels of Biryani\n", c -> id, noOfVessels);
            // printf("Chef %d has prepared %d vessels of biryani each having a capacity to serve %d students\n", c -> id, c -> noOfVessels, c -> p);
            // fflush(stdout);
        }

        sleep(w);

        pthread_mutex_lock(&(c -> lockChef));
        
        c -> noOfVessels = noOfVessels;
        
        c -> p = serves;        

        printf("Robot Chef %d has prepared %d vessels of Biryani. Waiting for all the vessels to be emptied to resume cooking\n", c -> id, c -> noOfVessels);
        c -> prepared = 1;

        // if(K > 0)
        // {
        //     printf("Robot Chef %d is preparing %d vessels of Biryani\n", c -> id, c -> prepared);
        //     // printf("Chef %d has prepared %d vessels of biryani each having a capacity to serve %d students\n", c -> id, c -> noOfVessels, c -> p);
        //     // fflush(stdout);
        // }
        pthread_mutex_unlock(&(c -> lockChef));

        
        // pthread_mutex_lock(&(c -> lockChef));
        // printf("Robot Chef %d has prepared %d vessels of Biryani. Waiting for all the vessels to be emptied to resume cooking\n", c -> id, c -> prepared);
        // pthread_mutex_unlock(&(c -> lockChef));
        
        biryani_ready(c -> id);
        
        pthread_mutex_lock(&(c -> lockChef));
        c -> prepared = 0;
        pthread_mutex_unlock(&(c -> lockChef));
    }
    // printf("** here **\n");

    return NULL;
}

void ready_to_serve_table(int id)
{
    // pthread_mutex_lock(&(t -> lockTable));
    // printf("Serving table %d entering Serving Phase\n", id);
    while(tables[id-1].servedTillNow < tables[id-1].slotsCopy && K > 0)
    {
        // printf("*** %d ** %d ***\n", t -> noOfSlots, t -> id);

        // do nothing;
    }

    if(tables[id-1].servedTillNow == tables[id-1].slotsCopy && K > 0)
    {
        printf("Serving Container of Table %d is empty, waiting for refill\n", id);
    }
    // pthread_mutex_unlock(&(t ->lockTable));

    // pthread_mutex_lock(&(t -> lockTable));
    // t -> found = 0;
    // pthread_mutex_unlock(&(t ->lockTable));

    // printf("The table %d is ready to serve biryani with %d slots\n", t -> id, t -> noOfSlots);
}

void *serveBiryani(void *arg)
{
    table *t = (table *)arg;

    while(K > 0)
    {
        // printf("Students left to be served: %d *****\n", K);

        while(t -> found == 0 && K > 0)
        {
            // printf("*\n");
            for(int i = 0; i < N; i++)
            {
                // printf("%d * %d\n", t -> id, i);
                pthread_mutex_lock(&(chefs[i].lockChef));
                pthread_mutex_lock(&(t -> lockTable));
                
                if((chefs[i].prepared) == 1)
                {
                    // if(chefs[i].noOfVessels > 0)
                    // {
                        t -> p = chefs[i].p;   
                        // (t -> refilled)++; 

                        // if(K > 0)
                        // {
                            printf("Robot Chef %d is refilling Serving Container of Serving Table %d\n", chefs[i].id, t -> id);

                            // if(t -> refilled > 1)
                            // {
                                printf("Serving Container of Table %d is refilled by Robot Chef %d; Table %d resuming serving now\n", t -> id, chefs[i].id, t -> id);
                            // }
                         
                            // printf("Serving Table %d is ready to serve with %d slots\n", t -> id, t -> slotsCopy);
                            // printf("Table %d is ready to serve biryani with capacity for %d serves\n", t -> id, t -> p);
                            // fflush(stdout);
                        // }

                        (chefs[i].noOfVessels)--;
                    // }

                    // else
                    // {
                    //     pthread_mutex_unlock(&(chefs[i].lockChef));
                    //     pthread_mutex_unlock(&(t ->lockTable));
                    //     continue;
                    // }

                    t -> found = 1;
                    pthread_mutex_unlock(&(chefs[i].lockChef));
                    pthread_mutex_unlock(&(t ->lockTable));
                    
                    break;
                }
                
                pthread_mutex_unlock(&(chefs[i].lockChef));
                pthread_mutex_unlock(&(t ->lockTable));
            }
        }

        // printf("Serving Table %d is ready to serve with %d slots\n", t -> id, t -> noOfSlots);

        printf("Serving table %d entering Serving Phase\n", t -> id);

        while(t -> p && K > 0)
        {
            pthread_mutex_lock(&(t ->lockTable));

            srand(time(0));
            t -> noOfSlots = 1 + (rand()%10)%(t->p);
            t -> p -= t -> noOfSlots;
            t -> slotsCopy = t -> noOfSlots;
            t -> servedTillNow = 0;

            if(K > 0)
            {
                printf("Serving Table %d is ready to serve with %d slots\n", t -> id, t -> slotsCopy);
                // printf("Table %d can currently serve %d slots\n", t -> id, t -> noOfSlots);
                // fflush(stdout);   
            }

            pthread_mutex_unlock(&(t ->lockTable));

            // if(M > 1)
            // {
            //     sleep(2);
            // }

            ready_to_serve_table(t -> id);
        }

        pthread_mutex_lock(&(t -> lockTable));
        t -> found = 0;
        pthread_mutex_unlock(&(t ->lockTable));

        // if(M > 1)
        // {
        //     sleep(3);
        // }
    }

    return NULL;
}

void student_in_slot(int id)
{
    // printf("Student %d is waiting to be served at table %d\n", id, students[id-1].tableID);
    // fflush(stdout);
    
    // K--;
    
    // printf("Students left to be served: %d *****\n", K);
    // printf("\t\t%d **** %d **** %d\n", students[id-1].tableID, students[id-1].id, tables[students[id-1].tableID - 1].noOfSlots);
    
    while(K > 0 && tables[students[id-1].tableID - 1].noOfSlots > 0)
    {
        // printf("** %d **\n", tables[s -> tableID].noOfSlots);
        // do nothing;
    }

    printf("Student %d on serving table %d has been served.\n", id, students[id - 1].tableID);
    // printf("Student %d is served biryani on table %d\n", id, students[id-1].tableID);
    pthread_mutex_lock(&(tables[students[id-1].tableID - 1].lockTable));
    tables[students[id-1].tableID - 1].servedTillNow++;
    pthread_mutex_unlock(&(tables[students[id-1].tableID - 1].lockTable));

    // fflush(stdout);
}

void *wait_for_slot(void *arg)
{
    student *s = (student *)arg;

    // srand(time(0));    
    // int timeOfArrival = rand()%6;
    sleep(s -> arrivalTime);
    printf("Student %d has arrived\n", s -> id);

    // printf("here * here\n");
    // int found = 0;
    // if(s -> found == 1)
    // {
    //     return NULL;
    // }

    printf("Student %d is waiting to be allocated a slot on the serving table\n", s -> id);
    while(s -> found == 0)
    {
        for(int i = 0; i < M; i++)
        {
            pthread_mutex_lock(&(tables[i].lockTable));
            if(tables[i].noOfSlots > 0)
            {
                
                // if(tables[i].noOfSlots > 0)
                // {
                    s -> tableID = tables[i].id;
                    printf("Student %d assigned a slot on the serving table %d and waiting to be served\n", s -> id, s -> tableID);
                    // printf("Student %d is waiting to be served at table %d\n", s -> id, tables[i].id);

                    (tables[i].noOfSlots)--;
                    // printf("\t\t\t* at student %d * and table %d * slots %d * %d *\n", s -> id, tables[i].id, tables[i].noOfSlots, s -> tableID);
                // }

                // else
                // {
                //     pthread_mutex_unlock(&(tables[i].lockTable));
                //     continue;
                // }

                // // fflush(stdout);

                pthread_mutex_lock(&lockStudentsLeft);
                K--;
                pthread_mutex_unlock(&lockStudentsLeft);
                
                s -> found = 1;
                pthread_mutex_unlock(&(tables[i].lockTable));

                break;
            }
            pthread_mutex_unlock(&(tables[i].lockTable));

        }
        
    }
    // printf("here ** here\n"); 
    
    student_in_slot(s -> id);

    // printf("%d *** %d *** %d\n", s -> tableID, s -> id, tables[s -> tableID].noOfSlots);

    return NULL;
}

int main()
{
    // int N, M, K;

    srand(time(0));

    pthread_mutex_init(&lockStudentsLeft, NULL);

    printf("Enter the number of robot chefs: ");
    scanf("%d", &N);

    printf("Enter the number of serving tables: ");
    scanf("%d", &M);

    printf("Enter the number of students: ");
    scanf("%d", &K);

    printf("\n\n");

    if((N == 0 || M == 0) && K > 0)
    {
        printf("Incompatible Setup\n");
        return 0;
    }

    // chefs = (chef*) malloc(sizeof(chef)*N);
    // tables = (table*) malloc(sizeof(table)*M);
    // students = (student*) malloc(sizeof(student)*K);

    for(int i = 0; i < N; i++)
    {
        chefs[i].id = i + 1;
        // chefs[i].noOfVessels = 0;
        chefs[i].prepared = 0;

        pthread_mutex_init(&(chefs[i].lockChef), NULL);

        int status = pthread_create(&(chefs[i].tid), NULL, prepareBiryani, &chefs[i]);
        // sleep(1);
        
        if(status)
        {
            perror("Cannot Create Thread for Chefs\n");
        }
    }

    for(int i = 0; i < M; i++)
    {
        tables[i].id = i + 1;
        // tables[i].refilled = 0;
        // tables[i].noOfSlots = 0;

        pthread_mutex_init(&(tables[i].lockTable), NULL);

        int status = pthread_create(&(tables[i].tid), NULL, serveBiryani, &tables[i]);
        // sleep(1);
        
        if(status)
        {
            perror("Cannot Create Thread for Tables\n");
        }
    }

    for(int i = 0; i < K; i++)
    {
        students[i].id = i + 1;
        students[i].arrivalTime = rand()%6;
        // students[i].tableID = -1;

        int status = pthread_create(&(students[i].tid), NULL, wait_for_slot, &students[i]);
        // sleep(1);
        
        if(status)
        {
            perror("Cannot Create Thread for Students\n");
        }
    }

    // for(int i = 0; i < N; i++)
    // {
    //     pthread_join(chefs[i].tid, NULL);
    // }

    // for(int i = 0; i < M; i++)
    // {
    //     pthread_join(tables[i].tid, NULL);
    // }

    for(int i = 0; i < K; i++)
    {
        pthread_join(students[i].tid, NULL);
    }

    // free(chefs);
    // free(tables);
    // free(students);

    sleep(2);
    // printf("\n\nAll the students have successfully been served biryani :))\n");
    // fflush(stdout);
    printf("\n\nSimulation Over.\n");

    return 0;
}