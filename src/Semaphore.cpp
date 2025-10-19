//
// Created by os on 10/18/25.
//

#include "../h/Semaphore.h"
#include "../h/Scheduler.h"
semaphore* semaphore::createSemaphore(unsigned int init ) {
    return new semaphore(init);
}

void semaphore::wait() {
    if(this->closed)return;
    if(--this->val<0){
        block();
    }
    return;
}

void semaphore::signal(){
    if(this->closed) return;
    if(++this->val <=0){
        unblock();
    }
    return ;
}

void semaphore::block() {
    this->blocked.add(PCB::running);
    PCB::running->setState(BLOCKED);
    PCB::timeSliceCnt = 0;
    thread_dispatch();
}

void semaphore::unblock() {
    PCB* thread = this->blocked.remove_first();
    if (thread) {
        thread->setState(READY);
        Scheduler::put(thread);
    }

}

void semaphore::closeSemaphore() {
    closed=true;
    PCB* pcb = blocked.remove_first();
    while (pcb) {
        pcb->backFromClosedSemaphore = true;
        Scheduler::put(pcb);
        pcb = blocked.remove_first();
    }
}