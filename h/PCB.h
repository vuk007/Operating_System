#ifndef OSNOVNI_PROJEKAT_PCB_H
#define OSNOVNI_PROJEKAT_PCB_H

#include "syscall_cpp.h"
#include "../lib/hw.h"

enum State {
    INITALIZED,
    READY,
    RUNNING,
    BLOCKED,
    SLEEPING,
    FINISHED
};

class PCB {
public:
    using tBody = void (*)(void*);
    static PCB* createPCB(tBody tbody, void* arg, void* stack_space);
    ~PCB() { delete stack; }
    State getState() const { return state; }
    void setState(State value) { state = value; }
    uint64 getTimeSlice() const { return timeSlice; }
    static PCB* running;

private:
    explicit PCB(tBody tbody, void* arg, void* stack_space);

    struct Context {
        uint64 ra;
        uint64 sp;
    };

    tBody tbody;
    void* args;
    uint8* stack;
    Context context;
    uint64 timeSlice;
    State state;
    bool backFromClosedSemaphore;

    static uint64 timeSliceCnt;
    static void threadWrapper();
    static void dispatch();
    static void contextSwitch(Context* oldContext, Context* newContext);
    friend class semaphore;
    friend class System;
    friend class Scheduler;
};

#endif // OSNOVNI_PROJEKAT_PCB_H
