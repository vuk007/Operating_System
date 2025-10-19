#include "../h/print.h"
#include "../h/PCB.h"
#include "../h/syscall_c.h"
#include "../h/System.h"

sem_t sem1, sem2; // globalni semafori

void threadFunc1(void* arg) {
    Syso s;
    s.printString("Thread 1 start\n");
    for (int i = 0; i < 3; i++) {
        sem_wait(sem1);  // čeka svoj red
        s.printString("Thread 1 iteration: ");
        s.printHex(i);
        __putc('\n');
        sem_signal(sem2); // dozvoli drugoj niti
    }
    thread_exit();
}

void threadFunc2(void* arg) {
    Syso s;
    s.printString("Thread 2 start\n");
    for (int i = 10; i < 13; i++) {
        sem_wait(sem2);  // čeka dok joj Thread 1 ne signalizira
        s.printString("Thread 2 iteration: ");
        s.printHex(i);
        __putc('\n');
        sem_signal(sem1); // dozvoli prvoj niti
    }
    thread_exit();
}
void mytest(){
    Syso s;

    // Otvaramo semafore
    sem_open(&sem1, 1); // prva nit ima prednost
    sem_open(&sem2, 0); // druga čeka

    thread_t t1, t2, main;

    int status2 = thread_create(&t2, threadFunc2, nullptr);
    int status1 = thread_create(&t1, threadFunc1, nullptr);
    int status = thread_create(&main, nullptr, nullptr);

    s.printString("Thread create statuses: ");
    s.printHex(status1);
    s.printHex(status2);
    s.printHex(status);
    __putc('\n');

    PCB::running = main;

    // dok obe niti ne završe
    while (t1->getState() != FINISHED || t2->getState() != FINISHED) {
        thread_dispatch();
    }

    // Zatvaramo semafore
    sem_close(sem1);
    sem_close(sem2);

    s.printString("All threads finished\n");
}

extern void userMain();

void userMainWrapper(void* arg) {
    userMain();
}

int main() {
    __asm__ volatile("csrw stvec, %[trap]" :: [trap] "r" (&System::supervisorTrap));
    thread_t mainThread , user;
    thread_create(&mainThread, nullptr, nullptr);
    thread_create(&user, userMainWrapper, nullptr);
    PCB::running = mainThread;

    while (user->getState() != FINISHED)
        thread_dispatch();
    return 0;
}
