//
// Created by os on 10/18/25.
//

#include "../h/syscall_c.h"
#include "../lib/console.h"
#define SYSCALL_WITH_RET(opcode, retval) \
    __asm__ volatile (                   \
        "mv a0, %[op]\n"                 \
        "ecall\n"                        \
        "mv %[ret], a0\n"                \
        : [ret] "=r"(retval)             \
        : [op] "r"(opcode)               \
        : "a0"                           \
    )

#define SYSCALL_NO_RET(opcode) \
    __asm__ volatile (          \
        "mv a0, %[op]\n"        \
        "ecall\n"               \
        :                       \
        : [op] "r"(opcode)      \
        : "a0"                  \
    )
void* mem_alloc (size_t size){
    uint64 op = SYSCALL_MEM_ALLOC;
    void* returnValue;
    __asm__ volatile(
            "mv a0, %[op]\n"
            "mv a1, %[size]\n"
            "ecall\n"
            "mv %[ret], a0\n"
            : [ret]"=r"(returnValue)
    : [op]"r"(op), [size]"r"(size)
    : "a0", "a1"
    );
    return returnValue;
}


int mem_free(void* ptr) {
    uint64 op = SYSCALL_MEM_FREE;
    int retvalue;
    __asm__ volatile(
            "mv a0,%[op]\n"
            "mv a1,%[ptr]\n"
            "ecall\n"
            "mv %[retvalue],a0\n"
            : [retvalue]"=r"(retvalue)
    : [op]"r"(op), [ptr]"r"(ptr)
    : "a0", "a1"
    );
    return retvalue;
}


size_t mem_get_free_space() {
    size_t free_space;

    __asm__ volatile (
            "mv a0, %1\n"     // a0 = SYSCALL_MEM_GET_FREE_SPACE
            "ecall\n"         // izvrši sistemski poziv
            "mv %0, a0\n"     // preuzmi rezultat iz a0 u free_space
            : "=r"(free_space)
            : "r"(SYSCALL_MEM_GET_FREE_SPACE)
            : "a0"
            );

    return free_space;
}

size_t mem_get_largest_free_block() {
    size_t largest_block;
    SYSCALL_WITH_RET(SYSCALL_MEM_GET_LARGEST_BLOCK, largest_block);
    return largest_block;
}


// ---------------------
// THREAD FUNKCIJE
// ---------------------

void thread_dispatch() {
    SYSCALL_NO_RET(SYSCALL_THREAD_DISPATCH);
}
int thread_exit() {
    SYSCALL_NO_RET(SYSCALL_THREAD_EXIT);
    int flag;
    __asm__ volatile ("mv %0, a0" : "=r" (flag));
    return flag;
}
int thread_create(thread_t* handle, void (*start_routine)(void*), void* arg) {
    void* stack_space = 0;
    if (start_routine) {
        stack_space = mem_alloc(DEFAULT_STACK_SIZE);
        if (!stack_space) return -1; // stack allocation failed
    }
    __asm__ volatile ("mv a0, %[code]"     : : [code] "r" (SYSCALL_THREAD_CREATE) : "a0");
    __asm__ volatile ("mv a1, %[handle]"   : : [handle] "r" (handle)              : "a1");
    __asm__ volatile ("mv a2, %[routine]"  : : [routine] "r" (start_routine)     : "a2");
    __asm__ volatile ("mv a3, %[argument]" : : [argument] "r" (arg)              : "a3");
    __asm__ volatile ("mv a4, %[stack]"    : : [stack] "r" (stack_space)         : "a4");
    __asm__ volatile ("ecall");
    int flag;
    __asm__ volatile ("mv %0, a0" : "=r" (flag));
    return flag;
}

// ---------------------
// SEMAFORI
// ---------------------

int sem_open(sem_t* handle, unsigned init){
    int r;
    __asm__ volatile("mv a0, %0" :: "r"(SYSCALL_SEM_OPEN) : "a0");
    __asm__ volatile("mv a1, %0" :: "r"(handle) : "a1");
    __asm__ volatile("mv a2, %0" :: "r"(init) : "a2");
    __asm__ volatile("ecall");
    __asm__ volatile("mv %0, a0" : "=r"(r));
    return r;
}
int sem_close(sem_t handle) {
    __asm__ volatile ("mv a1, %0" : : "r" (handle));
    __asm__ volatile ("mv a0, %0" : : "r" (SYSCALL_SEM_CLOSE));
    __asm__ volatile ("ecall");
    int flag;
    __asm__ volatile ("mv %0, a0" : "=r" (flag));
    return flag;
}

int sem_wait(sem_t id) {
    __asm__ volatile ("mv a1, %0" : : "r" (id));
    __asm__ volatile ("mv a0, %0" : : "r" (SYSCALL_SEM_WAIT));
    __asm__ volatile ("ecall");

    int flag;
    __asm__ volatile ("mv %0, a0" : "=r" (flag));
    return flag;
}

int sem_signal(sem_t id) {
    __asm__ volatile ("mv a1, %0" : : "r" (id));
    __asm__ volatile ("mv a0, %0" : : "r" (SYSCALL_SEM_SIGNAL));
    __asm__ volatile ("ecall");
    int flag;
    __asm__ volatile ("mv %0, a0" : "=r" (flag));
    return flag;
}

char getc (){return __getc();}

void putc (char c ){ __putc(c);}

int time_sleep (time_t) { return 0; }