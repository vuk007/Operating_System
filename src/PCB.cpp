#include "../h/PCB.h"
#include "../h/System.h"
#include "../h/Scheduler.h"

PCB* PCB::running = nullptr;
uint64 PCB::timeSliceCnt = 0;

PCB* PCB::createPCB(tBody tbody, void* arg, void* stack_space) {
    return new PCB(tbody, arg, stack_space);
}

PCB::PCB(tBody tbody, void* arg, void* stack_space)
        : tbody(tbody),
          args(arg),
          stack(tbody ? (uint8*)stack_space : nullptr),
          context({
                          (uint64)&threadWrapper,
                          stack ? (uint64)&stack[DEFAULT_STACK_SIZE] : 0
                  }),
          timeSlice(DEFAULT_TIME_SLICE),
          state(INITALIZED),
          backFromClosedSemaphore(false)
{
    if (tbody)
        Scheduler::put(this);
    else
        this->state = RUNNING;
}

void PCB::dispatch() {
    PCB* old = running;
    if (old && old->getState() == RUNNING) Scheduler::put(old);

    PCB* next = Scheduler::get();
    if (!next) {
        // nema spremnih niti, ostaje trenutna
        return;
    }

    running = next;
    running->setState(RUNNING);
    PCB::timeSliceCnt = 0;
    PCB::contextSwitch(&old->context, &running->context);
}


void PCB::threadWrapper() {
    System::popSppSie();
    running->tbody(running->args);
    thread_exit();
}
