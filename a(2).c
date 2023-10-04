#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include "a2_helper.h"
#include <semaphore.h>
#include <pthread.h>

pthread_barrier_t barrier_p3;

void *thread_function_P3(void *arg)
{
    int thread_id = *((int *)arg);

    info(BEGIN, 3, thread_id);
    pthread_barrier_wait(&barrier_p3);
    info(END, 3, thread_id);

    return NULL;
}

void create_threads_P3()
{
    pthread_t threads[5];
    int thread_ids[5];

    pthread_barrier_init(&barrier_p3, NULL, 5);
    for (int i = 0; i < 5; i++)
    {
        thread_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, thread_function_P3, &thread_ids[i]);
    }

    for (int i = 0; i < 5; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_barrier_destroy(&barrier_p3);
}
void create_P4_P5(int p3)
{
    if (fork() == 0)
    {

        info(BEGIN, 4, 0);
        info(END, 4, 0);
        exit(0);
    }
    else
    {
        if (fork() == 0)
        {

            info(BEGIN, 5, 0);
            info(END, 5, 0);
            exit(0);
        }
        else
        {

            wait(NULL);
            wait(NULL);
            info(END, p3, 0);
            exit(0);
        }
    }
}

void create_P7(int p6)
{
    if (fork() == 0)
    {

        info(BEGIN, 7, 0);
        info(END, 7, 0);
        exit(0);
    }
    else
    {

        wait(NULL);
        info(END, p6, 0);
        exit(0);
    }
}

void *thread_T8_2(void *arg)
{
    info(BEGIN, 8, 2);
    info(END, 8, 2);
    return NULL;
}

void *thread_T8_1(void *arg)
{
    info(BEGIN, 8, 1);
    pthread_t t8_2;
    pthread_create(&t8_2, NULL, thread_T8_2, NULL);
    pthread_join(t8_2, NULL);
    info(END, 8, 1);
    return NULL;
}

void *thread_function_8_3(void *arg)
{
    info(BEGIN, 8, 3);
    info(END, 8, 3);
    return NULL;
}

void *thread_function_8_4(void *arg)
{
    info(BEGIN, 8, 4);
    info(END, 8, 4);
    return NULL;
}

void create_threads_P8()
{
    pthread_t t8_1, t8_3, t8_4;

    pthread_create(&t8_1, NULL, thread_T8_1, NULL);
    pthread_create(&t8_3, NULL, thread_function_8_3, NULL);
    pthread_create(&t8_4, NULL, thread_function_8_4, NULL);

    pthread_join(t8_1, NULL);
    pthread_join(t8_3, NULL);
    pthread_join(t8_4, NULL);
}

sem_t semaphore;
sem_t semaphore_t14;

void *thread_function_P2(void *arg)
{
    int thread_id = *((int *)arg);

    if (thread_id == 14)
    {
        sem_wait(&semaphore_t14);
    }
    else
    {
        sem_wait(&semaphore);
    }

    info(BEGIN, 2, thread_id);
    info(END, 2, thread_id);

    if (thread_id == 14)
    {
        sem_post(&semaphore_t14);
    }
    else
    {
        sem_post(&semaphore);
    }

    return NULL;
}

void create_threads_P2()
{
    pthread_t threads[35];
    int thread_ids[35];

    sem_init(&semaphore, 0, 5);
    sem_init(&semaphore_t14, 0, 1);
    for (int i = 0; i < 35; i++)
    {
        thread_ids[i] = i + 1;
        pthread_create(&threads[i], NULL, thread_function_P2, &thread_ids[i]);
    }

    for (int i = 0; i < 35; i++)
    {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&semaphore);
    sem_destroy(&semaphore_t14);
}

int main()
{
    init();

    info(BEGIN, 1, 0);

    if (fork() == 0)
    {

        info(BEGIN, 2, 0);
        create_threads_P2();

        if (fork() == 0)
        {
            info(BEGIN, 3, 0);
            create_threads_P3();
            create_P4_P5(3);
        }
        else
        {
            if (fork() == 0)
            {

                info(BEGIN, 6, 0);
                create_P7(6);
            }
            else
            {
                if (fork() == 0)
                {

                    info(BEGIN, 8, 0);
                    create_threads_P8();
                    info(END, 8, 0);
                    exit(0);
                }
                else
                {

                    wait(NULL);
                    wait(NULL);
                    wait(NULL);
                    info(END, 2, 0);
                    exit(0);
                }
            }
        }
    }
    else
    {

        wait(NULL);
        info(END, 1, 0);
    }
    return 0;
}
