//
// Created by os on 10/18/25.
//
#include "../h/PCB.h"
#include "../h/syscall_cpp.h"
void* operator new(uint64 n) {
    return mem_alloc(n);
}

void* operator new[](uint64 n) {
    return mem_alloc(n);
}

void operator delete(void* ptr) {
    mem_free(ptr);
}

void operator delete[](void* ptr) {
    mem_free(ptr);
}
Thread::Thread(void (*body)(void *), void *arg) : body(body), arg(arg) {}

int Thread::start() {
    thread_create(&myHandle, body, arg);
    return 0;
}

Thread::~Thread() {
    if (myHandle) delete myHandle;
}

void Thread::dispatch() {
    thread_dispatch();
}

int Thread::sleep (time_t){return 0;}

Thread::Thread() : body(threadWrapper), arg(this) {}

void Thread::threadWrapper(void* arg) {
    ((Thread*)arg)->run();
}

Semaphore::Semaphore(unsigned int init) {
    sem_open(&myHandle,init);
}
Semaphore::~Semaphore() {
    sem_close(myHandle);
}
int Semaphore::wait() {
    return sem_wait(myHandle);
}
int Semaphore::signal() {
    return sem_signal(myHandle);
}

char Console::getc() {return ::getc();}

void Console::putc(char c) {::putc(c);}