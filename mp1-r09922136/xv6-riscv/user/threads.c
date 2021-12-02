#include "kernel/types.h"
#include "user/setjmp.h"
#include "user/threads.h"
#include "user/user.h"
#define NULL 0


static struct thread* current_thread = NULL;
static int id = 1;
static jmp_buf env_st;
//static jmp_buf env_tmp;

struct thread *thread_create(void (*f)(void *), void *arg){
    struct thread *t = (struct thread*) malloc(sizeof(struct thread));
    //unsigned long stack_p = 0;
    unsigned long new_stack_p;
    unsigned long new_stack;
    new_stack = (unsigned long) malloc(sizeof(unsigned long)*0x100);
    new_stack_p = new_stack +0x100*8-0x2*8;
    t->fp = f;
    t->arg = arg;
    t->ID  = id;
    t->buf_set = 0;
    t->stack = (void*) new_stack;
    t->stack_p = (void*) new_stack_p;
    id++;
    return t;
}

void thread_add_runqueue(struct thread *t){
    // This function adds an initialized struct thread to the runqueue.
    // next: always points to the next to-be-executed thread 
    // previous: always points to the previously executed thread
    // the newly inserted thread should be current_thread->previous
    //printf("thread_add_runqueue\n", t);

    if(current_thread == NULL){
        // TODO
	t->next = t->previous = t; 
	current_thread = t;
    }
    else{
        // TODO
	t->next = current_thread;
	t->previous = (current_thread->previous);
	(current_thread->previous)->next = t;
	current_thread->previous = t; 
    }
}

void thread_yield(void){
    // TODO 
	/*This function suspends the current thread by saving its
 	context to the jmp_buf in struct thread using setjmp.*/
    if (current_thread->buf_set == 0) {
	current_thread->buf_set = 1;
    }
    if (!setjmp(current_thread->env)) {  //saving the context    		
	schedule(); 
    	dispatch();
    }
}

void dispatch(void){ // 補thread_exit
    // TODO This function will execute a thread, decided by schedule
    //printf("dispatch\n");
    
    if (current_thread->buf_set != 0) {
	longjmp(current_thread->env, 1);
    } else {
    	if (!setjmp(current_thread->env)) {
    	    current_thread->env->sp = ((unsigned long) current_thread->stack_p);
            //current_thread->buf_set = 1;
    	    longjmp(current_thread->env, 1);
    	} else {
    	    current_thread->fp(NULL);
    	}
    	thread_exit();
    }

}

void schedule(void){
    // This function will decide which thread to run next
    //printf("schedule\n");
    current_thread = current_thread->next;
}

void thread_exit(void){
    /* This function removes the calling thread from the runqueue, 
    frees its stack and the struct thread, updates the current_thread variable 
    with the next to-be-executed thread in the runqueue and calls dispatch.*/
    
    //printf("thread_exit\n");

    if(current_thread->next != current_thread){
        // TODO
	struct thread *temp;
	temp = current_thread;
	(current_thread->previous)->next = current_thread->next;
	(current_thread->next)->previous = current_thread->previous;
	current_thread = current_thread->next;
	free(temp->stack);
	free(temp);

	dispatch();
    }
    else{
        // TODO
        // Hint: No more thread to execute
	free(current_thread->stack);
	free(current_thread);
        current_thread = NULL;
	
	longjmp(env_st, 1);
    }
}
void thread_start_threading(void){
    // TODO initialize the threading by calling schedule and dispatch, it should return only if all threads exit

    if (!setjmp(env_st)) {
	dispatch();
    }

}
