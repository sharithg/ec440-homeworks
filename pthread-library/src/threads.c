#include "threads.h"
#include "ec440threads.h"
#define STACK_SZ 32767
#define MAX_THREAD_SZ 128
#define MAX_TIME_PER_THREAD 50000
////// Global Variables //////////
thread_t *head = NULL;
thread_t *current = NULL;
int num_blocks = 0;
/////////////////////////////////
/////////LL Stuff///////////////
thread_t *create_tcb(int num_blocks)
{
    thread_t *result = malloc(sizeof(thread_t));
    tcb *block = malloc(sizeof(tcb));
    block->id = (pthread_t)num_blocks;
    block->status = READY;
    result->block = *block;
    result->next = NULL;
    result->stack_ptr = NULL;
    return result;
}
//Got algorithm from: https://www.tutorialspoint.com/data_structures_algorithms/doubly_linked_list_algorithm.htm
thread_t *add_to_queue(thread_t *new_node, thread_t *head)
{
    thread_t *temp, *p;
    p = head;
    temp = new_node;

    if (head == NULL)
    {
        head = temp;
    }
    else
    {
        while (p->next != head)
            p = p->next;
        p->next = temp;
    }
    temp->next = head;
    return head;
}
//////////////////////////////////////////////////
int pthread_create(
    pthread_t *thread,
    const pthread_attr_t *attr,
    void *(*start_routine)(void *),
    void *arg)
{
    if (num_blocks == 0)
    {
        setup_threads();
    }
    if (num_blocks > MAX_THREAD_SZ)
    {
        printf("ERROR: Max number of threads reached\n");
        return -1;
    }
    thread_t *new_thread = create_tcb(num_blocks);
    num_blocks++;
    new_thread->stack_ptr = malloc(sizeof(unsigned long) * STACK_SZ);
    printf("malloc %p\n", new_thread->stack_ptr);
    /*
                                             ----------- 0xfffff
     new_thread->stack_ptr + STACK_SZ - 8 -> -----------    |
                                                 .        STACK_SZ
                                                 .          |
                                                 .          |
                    new_thread->stack_ptr -> ----------- 0x00000
    */
    // new_thread->stack_ptr = new_thread->stack_ptr + STACK_SZ - 8;
    int *temp_ptr = new_thread->stack_ptr + STACK_SZ - 8;
    // Given that the specification requires that start_routine invoks pthread_exit upon completion, we can easily achieve this by putting the address of pthread_exit on the top of the stack
    *(unsigned long *)temp_ptr = (unsigned long)pthread_exit;
    //for start_thunk to work we should store the value of *arg in R13 and the address of start_routine in R12
    new_thread->block.buff[0].__jmpbuf[JB_R13] = (unsigned long)arg;
    //If the start_routine returns, the effect shall be as if there was an implicit call to pthread_exit() using the return value of start_routine as the exit status
    new_thread->block.buff[0].__jmpbuf[JB_R12] = (unsigned long)start_routine;
    //start_thunk is a helper function that moves the value stored in R13 to RDI
    new_thread->block.buff[0].__jmpbuf[JB_PC] = ptr_mangle((unsigned long)start_thunk);
    new_thread->block.buff[0].__jmpbuf[JB_RSP] = ptr_mangle((unsigned long)temp_ptr);
    new_thread->block.status = READY;
    head = add_to_queue(new_thread, head);
    return 0;
}
void pthread_exit(void *value_ptr)
{
    current->block.status = EXITED;
    printf("free %p\n", current->stack_ptr);
    // free(current->stack_ptr);
    num_blocks--;
    schedule(0);
    __builtin_unreachable();
}
void schedule(int sig)
{
    if (current == NULL)
        return;
    if (setjmp(current->block.buff) == 0)
    {
        do
        {
            current = current->next;

        } while (current->block.status == EXITED);
        current->block.status = RUNNING;
        longjmp(current->block.buff, 1);
    }
}
void setup_threads()
{
    thread_t *new_thread = create_tcb(num_blocks);
    new_thread->block.status = READY;
    num_blocks++;
    new_thread->stack_ptr = NULL;
    head = add_to_queue(new_thread, head);
    current = head;
    setjmp(new_thread->block.buff);
    //from https://stackoverflow.com/questions/31919247/c-signal-handler
    struct sigaction sa;
    sa.sa_handler = &schedule;
    sa.sa_flags = SA_NODEFER;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);
    ualarm(MAX_TIME_PER_THREAD, MAX_TIME_PER_THREAD);
}
pthread_t pthread_self()
{
    return current->block.id;
}