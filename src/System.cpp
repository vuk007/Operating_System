//
// Created by os on 10/18/25.
//
 #include "../h/System.h"
 #include "../h/syscall_c.h"
 #include "../h/memoryAllocator.h"
#include "../h/Semaphore.h"
#include "../h/PCB.h"
constexpr uint64  USER_PRIVLAGE     =     0x0000000000000008UL;
constexpr uint64  SYSTEM_PRIVLAGE   =     0x0000000000000009UL;
constexpr uint64  TIMER_INTERUPT    =     0x8000000000000001UL;
constexpr uint64  HARDWARE_INTERUPT =     0x8000000000000009UL;

void System::popSppSie() {}
void System::handleTrap() {
    uint64 scause;
    __asm__ volatile("csrr %[scause] , scause":[scause]"=r"(scause));

    if (scause == USER_PRIVLAGE || scause == SYSTEM_PRIVLAGE) {
        volatile uint64 op, retaddr, savestatus, base;

        __asm__ volatile("csrr %[retaddr], sepc":[retaddr]"=r"(retaddr));
        __asm__ volatile("csrr %[savestatus], sstatus":[savestatus]"=r"(savestatus));
        __asm__ volatile("mv %[where], a0":[where]"=r"(op));
        __asm__ volatile ("mv %0, x8" : "=r" (base));

        switch (op) {
            case SYSCALL_MEM_ALLOC: {
                size_t _size;
                __asm__ volatile ("mv %[where], a1":[where]"=r"(_size));
                void* ptr = memoryAllocator::mem_alloc(_size);
                __asm__ volatile ("sd %0, %1" : : "r" (ptr),"m"(*((uint64*)(base + 80))));
                break;
            }

            case SYSCALL_MEM_FREE: {
                void* ptr;
                __asm__ volatile ("mv %[where], a1":[where]"=r"(ptr));
                memoryAllocator::mem_free(ptr);
                break;
            }
            case SYSCALL_MEM_GET_FREE_SPACE:{
                size_t size = memoryAllocator::mem_get_free_space();
                __asm__ volatile ("sd %0, %1" : : "r" (size),"m"(*((uint64*)(base + 80))));
                break;
            }
            case SYSCALL_THREAD_CREATE: {
                volatile uint64 a0, a1, a2, a3, a4;
                __asm__ volatile ("ld %0, 0(%[base])" : "=r"(a0) : [base] "r"(base + 80));
                __asm__ volatile ("ld %0, 8(%[base])" : "=r"(a1) : [base] "r"(base + 80));
                __asm__ volatile ("ld %0, 16(%[base])" : "=r"(a2) : [base] "r"(base + 80));
                __asm__ volatile ("ld %0, 24(%[base])" : "=r"(a3) : [base] "r"(base + 80));
                __asm__ volatile ("ld %0, 32(%[base])" : "=r"(a4) : [base] "r"(base + 80));

                int status = -1;
                thread_t* handle = (thread_t*)a1;
                if (handle) {
                    *handle = PCB::createPCB((PCB::tBody)a2, (void*)a3, (a2 ? (void*)a4 : 0));
                    status = (*handle ? 0 : -2);
                }
                __asm__ volatile (
                        "sd %0, %1"
                        :
                        : "r" (status), "m" (*((uint64*)(base + 80)))
                        );
                break;
            }
            case SYSCALL_THREAD_EXIT: {
                int status;
                if (PCB::running->getState() != FINISHED) {
                    status = 0;
                    PCB::running->setState(FINISHED);
                    PCB::dispatch();
                } else {
                    status = -1;
                }
                __asm__ volatile ("sd %0, %1" : : "r" (status), "m"(*((uint64*)(base + 80))));
                break;
            }
            case SYSCALL_THREAD_DISPATCH: {
                PCB::dispatch();
                break;
            }

            case SYSCALL_SEM_OPEN: {
                volatile uint64 a1, a2;
                __asm__ volatile ("ld %0, 8(%[base])"  : "=r"(a1) : [base] "r"(base + 80));
                __asm__ volatile ("ld %0, 16(%[base])" : "=r"(a2) : [base] "r"(base + 80));
                sem_t* handle = (sem_t*)a1;
                int flag=0;
                if (handle) {
                    *handle = semaphore::createSemaphore((unsigned)a2);
                    flag = (*handle ? 0 : -1);
                }
                __asm__ volatile ("sd %0, 80(%[base])" : : "r"(flag), [base]"r"(base));
                break;
            }

            case SYSCALL_SEM_CLOSE: {
                int flag = 0;
                volatile uint64 a1;
                __asm__ volatile ("ld %0, 8(%[base])"  : "=r"(a1) : [base] "r"(base + 80));
                sem_t handle = (sem_t)a1;
                if (handle) delete handle;
                else flag = -1;
                __asm__ volatile ("sd %0, 80(%[base])" : : "r"(flag), [base]"r"(base));
                break;
            }

            case SYSCALL_SEM_WAIT: {
                volatile uint64 a1;
                __asm__ volatile ("ld %0, 8(%[base])": "=r"(a1): [base] "r"(base + 80));
                sem_t sem = (sem_t)a1;
                int status = -1;
                if (sem) {
                    status = 0;
                    sem->wait();
                    if (PCB::running->backFromClosedSemaphore)
                        status = -1;
                }
                __asm__ volatile ("sd %0, 80(%[base])":: "r"(status), [base]"r"(base));
                break;
            }
            case SYSCALL_SEM_SIGNAL: {
                volatile uint64 a1;
                __asm__ volatile ("ld %0, 8(%[base])": "=r"(a1): [base] "r"(base + 80));
                sem_t sem = (sem_t)a1;
                int status = 0;
                if (sem) sem->signal();
                else status = -1;
                __asm__ volatile ("sd %0, 80(%[base])":: "r"(status), [base]"r"(base));
                break;
            }

            case SYSCALL_GETC: {
                break;
            }

            case SYSCALL_PUTC: {
                break;
            }
            default: {
                __asm__ volatile("mv a0, %0" :: "r"(-1));
                break;
            }
        }

        __asm__ volatile("csrw sstatus , %[savestatus]"::[savestatus]"r"(savestatus));
        __asm__ volatile("csrw sepc, %[retaddr]"::[retaddr]"r"(retaddr + 4));
    }
    else if (scause == TIMER_INTERUPT) {
    }
    else if (scause == HARDWARE_INTERUPT) {
    }
}