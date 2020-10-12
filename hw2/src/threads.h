#ifndef THREADS_H
#define THREADS_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <setjmp.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#define RUNNING 101
#define READY 100
#define EXITED 99

#define JB_RBX 0
#define JB_RBP 1
#define JB_R12 2
#define JB_R13 3
#define JB_R14 4
#define JB_R15 5
#define JB_RSP 6
#define JB_PC 7

typedef struct
{
    pthread_t id;
    jmp_buf buff;
    int status;
} tcb;

//Circular LL of blocks
typedef struct thread_t
{
    tcb block;
    void *stack_ptr;
    struct thread_t *next;
} thread_t;

thread_t *create_tcb();
thread_t *add_to_queue(thread_t *new_node, thread_t *head);
void print_blocks();
int pthread_create(
    pthread_t *thread,
    const pthread_attr_t *attr,
    void *(*start_routine)(void *),
    void *arg);

void pthread_exit(void *value_ptr);

pthread_t pthread_self(void);

void setup_threads();

void schedule(int sig);
// void test_ll(void);

#endif