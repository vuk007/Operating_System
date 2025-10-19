//
// Created by os on 10/13/25.
//

#ifndef OSNOVNI_PROJEKAT_SCHEDULER_H
#define OSNOVNI_PROJEKAT_SCHEDULER_H
#include "../h/list.h"
#include "../h/PCB.h"

class Scheduler{
private:
    static List<PCB> ready;
    static List<PCB> sleep;
    Scheduler() {}
    Scheduler(const Scheduler&) = delete;
    Scheduler& operator=(const Scheduler&) = delete;
    Scheduler(Scheduler&&) = delete;
    Scheduler& operator=(Scheduler&&) = delete;
public:
    static Scheduler& instance() {
        static Scheduler instance;
        return instance;
    }
    static PCB* get(){
        return Scheduler::ready.remove_first();
    }
    static void put(PCB* pcb){
        pcb->setState(READY);
        Scheduler::ready.addLast(pcb);
    }
    static void putsleep(PCB* pcb){
        pcb->setState(SLEEPING);
        Scheduler::sleep.addLast(pcb);
    }
    static PCB* getsleep(){
        return Scheduler::sleep.remove_first();
    }

};

#endif //OSNOVNI_PROJEKAT_SCHEDULER_H