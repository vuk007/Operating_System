//
// Created by os on 10/18/25.
//

#ifndef PROJEKAT_SEMAPHORE_H
#define PROJEKAT_SEMAPHORE_H
#include "list.h"
#include "PCB.h"
class semaphore {
public:
    ~semaphore(){closeSemaphore();}
    void wait();
    void signal();
    void closeSemaphore();
    static semaphore* createSemaphore(unsigned init = 1);
private:
    explicit semaphore(unsigned init = 1) : val(init),closed(false) {}
    int val;
    List<PCB> blocked;
    bool closed;
protected:
    void block();
    void unblock();
};


#endif //PROJEKAT_SEMAPHORE_H
