/******************************************************************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <ucontext.h>
#include <stdbool.h>
#include <signal.h>
#include <assert.h>

////////////////////////////////////////////////////

/*
https://www.chiark.greenend.org.uk/~sgtatham/coroutines.html
https://www.geeksforgeeks.org/cpp/coroutines-in-c-cpp/
https://blog.devgenius.io/writing-a-coroutine-in-c-language-6a537275ddc3
https://gist.github.com/lpereira/2154951
https://dev.to/rdentato/coroutines-in-c-1-2-45ig
https://dev.to/rdentato/coroutines-in-c-2-3-h0j
https://dev.to/rdentato/coroutines-in-c-3-3-2ej6
*/

////////////////////////////////////////////////////

typedef struct coro_t_ coro_t;
typedef int (*coro_function_t)(coro_t *coro);

struct coro_t_ {
    coro_function_t function;
    ucontext_t      suspend_context;
    ucontext_t      resume_context;
    int             yield_value;
    bool            is_coro_finished;
};

coro_t *coro_new(coro_function_t function);
int coro_resume(coro_t *coro);    
void coro_yield(coro_t *coro, int value);
void coro_free(coro_t *coro);

////////////////////////////////////////////////////

static void _coro_entry_point(coro_t *coro) {
    int return_value = coro->function(coro);
    coro->is_coro_finished = true;
    coro_yield(coro, return_value);
}

coro_t *coro_new(coro_function_t function) {
    coro_t *coro = calloc(1, sizeof(*coro));
    coro->is_coro_finished = false;
    coro->function = function;
    coro->resume_context.uc_stack.ss_sp = calloc(1, MINSIGSTKSZ);
    coro->resume_context.uc_stack.ss_size = MINSIGSTKSZ;
    coro->resume_context.uc_link = 0;
    getcontext(&coro->resume_context);
    makecontext(&coro->resume_context, (void (*)())_coro_entry_point, 1, coro);
    return coro;
}

int coro_resume(coro_t *coro) {    
    if (coro->is_coro_finished) return -1;
    swapcontext(&coro->suspend_context, &coro->resume_context);
    return coro->yield_value;
}

void coro_yield(coro_t *coro, int value) {
    coro->yield_value = value;
    swapcontext(&coro->resume_context, &coro->suspend_context);
}

void coro_free(coro_t *coro) {
    free(coro->resume_context.uc_stack.ss_sp);
    free(coro);
}

////////////////////////////////////////////////////

int hello_world(coro_t *coro) {    
    puts("A0");
    coro_yield(coro, 1);
    puts("B1");
    return 2;
}

////////////////////////////////////////////////////

int main() {
    coro_t *coro = coro_new(hello_world);
    assert(coro_resume(coro) == 1);
    assert(coro_resume(coro) == 2);
    assert(coro_resume(coro) == -1);
    coro_free(coro);
    return EXIT_SUCCESS;
}
