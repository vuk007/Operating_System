
# Operating System Kernel

A small educational operating system kernel implemented for the **RISC-V** architecture as part of the **Operating Systems 1** course at the Faculty of Electrical Engineering, University of Belgrade.

The project implements the core mechanisms required for a simple multithreaded operating system: dynamic memory allocation, threads, context switching, scheduling, semaphores, system calls, and C/C++ interfaces between user code and the kernel.

> **Implementation status:** Tasks 1–3 are implemented in the current version of the project. Task 4, which covers asynchronous context switching, timer-based preemption, sleeping threads, and the corresponding console functionality, is not implemented in the current source code.

---

## 1. Project Overview

The goal of the project is to build a small kernel that runs on a virtual **RISC-V** computer and provides basic operating-system services to a user application.

Unlike a conventional desktop operating system, the kernel and user application share the same address space and are statically linked into one executable program. The project therefore resembles the architecture of a small embedded system.

The kernel provides:

- dynamic memory allocation and deallocation
- user-level threads
- synchronous context switching
- FIFO thread scheduling
- semaphores and thread synchronization
- system calls through the RISC-V `ecall` mechanism
- C API for kernel services
- C++ object-oriented API for threads and semaphores
- transition between user and privileged execution modes
- low-level context switching written in RISC-V assembly

The project runs inside the educational RISC-V environment provided for the course.

---

## 2. System Architecture

The project follows a layered architecture in which the user program communicates with the kernel through several interfaces.

```text
+--------------------------------------------------+
|                 User Application                 |
|                 userMain()                       |
+--------------------------------------------------+
|                   C++ API                        |
|       Thread / Semaphore / Console               |
+--------------------------------------------------+
|                    C API                         |
| mem_alloc / thread_create / sem_wait / ...       |
+--------------------------------------------------+
|                     ABI                          |
|          RISC-V registers + ecall                |
+--------------------------------------------------+
|                    Kernel                        |
|                                                  |
|  Memory Allocator   PCB   Scheduler   Semaphore  |
|  Trap Handler       Context Switching            |
+--------------------------------------------------+
|              RISC-V Hardware Layer              |
|                  hw.lib / console                |
+--------------------------------------------------+
```

The important design principle is that the upper layers do not directly manipulate kernel internals. Instead, a user-level operation is translated into a system call, transferred through the ABI, and handled by the kernel.

---

## 3. Execution Model

The kernel and user application are linked together into one executable image.

At startup:

1. The kernel installs the supervisor trap handler.
2. A PCB representing the initial/main thread is created.
3. A separate thread is created for `userMain()`.
4. The kernel starts dispatching between runnable threads.
5. The user application executes through the provided kernel API.
6. When the user thread finishes, control returns to the kernel and the program terminates.

The entry point responsible for the user application is:

```cpp
void userMain();
```

The kernel owns `main()` and creates a thread whose entry routine invokes `userMain()`.

---

## 4. Memory Management

The project contains its own kernel-level memory allocator instead of relying on the host operating system.

### Free-list allocator

Free memory is represented by a doubly linked list of free blocks.

Each free block contains a header:

```cpp
struct Header {
    Header* next;
    Header* prev;
    size_t size;
};
```

The allocator starts with one large free block covering the available heap.

### Allocation

`mem_alloc(size)`:

1. Initializes the allocator on first use.
2. Rounds the requested size to `MEM_BLOCK_SIZE`.
3. Searches the free-list for the first block large enough to satisfy the request.
4. Splits the block if enough space remains.
5. Removes the block from the free-list when splitting is not possible.
6. Returns the memory immediately after the block header.

The implementation therefore uses a **first-fit style allocation strategy**.

### Deallocation

`mem_free(ptr)`:

1. Converts the returned user pointer back to its block header.
2. Inserts the block into the free-list in address order.
3. Checks adjacent blocks.
4. Merges contiguous free blocks to reduce fragmentation.

The allocator also provides:

```text
mem_get_free_space()
mem_get_largest_free_block()
```

which allow the current amount of free memory and the largest available contiguous block to be inspected.

---

## 5. Thread Management

Threads are represented by the `PCB` class.

A PCB stores the information required to resume a thread, including:

- thread entry function
- argument passed to the thread
- stack
- saved execution context
- time slice information
- current thread state

Thread states are represented by:

```text
INITIALIZED
READY
RUNNING
BLOCKED
SLEEPING
FINISHED
```

### Thread creation

A new thread is created using:

```cpp
thread_create(&handle, start_routine, arg);
```

For a normal thread, the kernel:

1. Allocates a stack.
2. Creates a PCB.
3. Initializes the thread's first execution context.
4. Places the thread into the ready queue.

A special PCB without a body is used to represent the already-running main thread.

### Thread termination

A running thread can terminate through:

```cpp
thread_exit();
```

The kernel changes its state to `FINISHED` and dispatches another ready thread.

---

## 6. Scheduling

The scheduler is implemented through the `Scheduler` class.

Two lists are maintained:

```text
ready
sleep
```

The current implementation uses the ready list as a FIFO queue.

When a thread becomes ready:

```text
Thread -> Scheduler::put()
       -> READY
       -> end of ready queue
```

When the scheduler needs another thread:

```text
Scheduler::get()
    ↓
remove first ready thread
    ↓
set state to RUNNING
    ↓
context switch
```

The scheduling mechanism is therefore intentionally simple and deterministic.

---

## 7. Context Switching

Context switching is implemented using a combination of C++ and RISC-V assembly.

The PCB contains a minimal saved context:

```cpp
struct Context {
    uint64 ra;
    uint64 sp;
};
```

The low-level context switch is implemented in `ContextSwitch.S`.

The old thread's:

- return address (`ra`)
- stack pointer (`sp`)

are saved, and the corresponding values of the next thread are restored.

```text
Current thread
     |
     | save ra, sp
     v
  Old Context

  Scheduler
     |
     v
 Next Thread
     |
     | restore ra, sp
     v
Continue execution
```

This is a **synchronous context switch**, meaning the current implementation switches threads when the kernel explicitly performs a dispatch rather than because of a timer interrupt.

---

## 8. Thread Startup

A newly created thread does not begin by directly calling its user function.

Instead, its initial context points to a kernel wrapper:

```text
New Thread
    ↓
PCB::threadWrapper()
    ↓
switch to user execution mode
    ↓
thread body
    ↓
thread_exit()
```

`PCB::threadWrapper()` invokes `System::popSppSie()` before executing the actual thread body, providing the intended transition toward user-mode execution.

---

## 9. Semaphores and Synchronization

Synchronization is implemented through the `semaphore` class.

A semaphore contains:

- an integer value
- a list of blocked threads
- a flag indicating whether the semaphore has been closed

### Wait

When `wait()` is called:

```text
decrement semaphore value
        |
        +---- value >= 0 ---> continue
        |
        +---- value < 0 ----> block current thread
```

A blocked thread is removed from active execution and placed into the semaphore's blocked list.

### Signal

When `signal()` is called:

```text
increment semaphore value
        |
        +---- value > 0 ---> continue
        |
        +---- value <= 0 --> unblock a waiting thread
```

The unblocked thread is returned to the scheduler's ready queue.

### Closing a semaphore

When a semaphore is closed, all threads waiting on it are released and marked so that their `sem_wait()` operation can return an error.

This provides a controlled way of waking threads that are blocked on a semaphore which is no longer valid.

---

## 10. System Calls and ABI

Communication between user code and the kernel is implemented through RISC-V system calls.

System call identifiers are defined in `syscall_c.h`.

Examples include:

```text
0x01  mem_alloc
0x02  mem_free

0x11  thread_create
0x12  thread_exit
0x13  thread_dispatch

0x21  sem_open
0x22  sem_close
0x23  sem_wait
0x24  sem_signal

0x31  time_sleep
0x41  getc
0x42  putc
```

The C API acts as the user-facing wrapper around these operations.

For example:

```cpp
thread_dispatch();
```

eventually executes a RISC-V `ecall`, transferring control to the supervisor trap handler.

---

## 11. Trap Handling

The trap entry point is implemented in `Trap.S`.

When a trap occurs, the routine:

1. Allocates space on the kernel stack.
2. Saves the processor registers.
3. Calls `System::handleTrap()`.
4. Restores the registers.
5. Returns using the RISC-V `sret` instruction.

Conceptually:

```text
User code
   |
   | ecall
   v
Trap entry
   |
   | save registers
   v
System::handleTrap()
   |
   | identify system call
   v
Kernel operation
   |
   | restore registers
   v
sret
   |
   v
User code
```

`System::handleTrap()` reads the `scause` register and determines whether the event represents a system call or an interrupt.

---

## 12. System Call Processing

For system calls, the operation identifier is passed through register `a0`, while additional arguments are passed through subsequent registers.

The kernel reads the requested operation and dispatches it to the corresponding subsystem.

For example:

```text
mem_alloc()
    ↓
C API
    ↓
ecall
    ↓
System::handleTrap()
    ↓
memoryAllocator::mem_alloc()
    ↓
result returned through register
```

Thread and semaphore system calls follow the same layered approach.

---

## 13. C API

The C API exposes procedural functions to user applications.

### Memory

```cpp
void* mem_alloc(size_t size);
int mem_free(void* ptr);
size_t mem_get_free_space();
size_t mem_get_largest_free_block();
```

### Threads

```cpp
int thread_create(thread_t* handle,
                  void (*start_routine)(void*),
                  void* arg);

int thread_exit();
void thread_dispatch();
```

### Semaphores

```cpp
int sem_open(sem_t* handle, unsigned init);
int sem_close(sem_t handle);
int sem_wait(sem_t id);
int sem_signal(sem_t id);
```

The C API is the lowest user-facing layer before the ABI.

---

## 14. C++ API

The project also provides an object-oriented wrapper around the C API.

The main abstractions are:

### `Thread`

Provides an object-oriented interface for creating and starting threads.

It supports two usage models:

- supplying a function pointer and argument
- subclassing `Thread` and overriding `run()`

Conceptually:

```cpp
class MyThread : public Thread {
protected:
    void run() override {
        // thread code
    }
};
```

### `Semaphore`

Wraps the semaphore C API:

```cpp
Semaphore sem(1);

sem.wait();
sem.signal();
```

### `Console`

Provides:

```cpp
Console::getc();
Console::putc(char);
```

for console access.

---

## 15. Operator `new` and `delete`

The C++ API redirects dynamic allocation to the kernel memory allocator.

The global operators:

```cpp
operator new
operator new[]
operator delete
operator delete[]
```

are implemented using:

```text
new
 ↓
mem_alloc

delete
 ↓
mem_free
```

This allows C++ objects created by the kernel/user-level code to use the project's own heap instead of the host operating system's allocator.

---

## 16. Testing

The repository contains tests for the implemented functionality.

Current tests include:

```text
Threads_C_API_test
Threads_CPP_API_test
System_Mode_test

ConsumerProducer_C_API_test
ConsumerProducer_CPP_Sync_API_test

ThreadSleep_C_API_test
ConsumerProducer_CPP_API_test
```

The test selection is controlled through `test/userMain.cpp`.

The current configuration enables:

```cpp
#define LEVEL_1_IMPLEMENTED 1
#define LEVEL_2_IMPLEMENTED 1
#define LEVEL_3_IMPLEMENTED 1
#define LEVEL_4_IMPLEMENTED 0
```

Therefore the current project declares:

- memory allocation implemented
- thread support implemented
- semaphore support implemented
- asynchronous context switching / time sharing not implemented

The current `userMain()` is configured to run test 3 directly:

```cpp
int test = 3;
```

It can be changed to select another test.

---

## 17. Producer-Consumer Example

One of the main synchronization tests demonstrates producer-consumer communication using semaphores.

The basic execution pattern is:

```text
Producer
   |
   | produce item
   v
Shared buffer
   ^
   |
   | consume item
   |
Consumer
```

Semaphores are used to coordinate access to the shared buffer and prevent invalid concurrent operations.

The project provides both C and C++ API variants of the producer-consumer test.

---

## 18. Project Structure

```text
Operating_System/
│
├── h/
│   ├── PCB.h
│   ├── Scheduler.h
│   ├── Semaphore.h
│   ├── System.h
│   ├── list.h
│   ├── memoryAllocator.h
│   ├── print.h
│   ├── syscall_c.h
│   └── syscall_cpp.h
│
├── src/
│   ├── ContextSwitch.S
│   ├── Trap.S
│   ├── PCB.cpp
│   ├── Scheduler.cpp
│   ├── Semaphore.cpp
│   ├── System.cpp
│   ├── main.cpp
│   ├── memoryAllocator.cpp
│   ├── syscall_c.cpp
│   └── syscall_cpp.cpp
│
├── test/
│   ├── Threads_C_API_test.cpp
│   ├── Threads_CPP_API_test.cpp
│   ├── ConsumerProducer_C_API_test.cpp
│   ├── ConsumerProducer_CPP_Sync_API_test.cpp
│   ├── System_Mode_test.cpp
│   ├── ThreadSleep_C_API_test.cpp
│   ├── ConsumerProducer_CPP_API_test.cpp
│   ├── buffer.hpp
│   ├── printing.cpp
│   ├── printing.hpp
│   ├── userMain.cpp
│   └── uputstvo.txt
│
├── build/
│   └── generated object/dependency/listing files
│
└── Projektni zadatak 2025. v1.0.pdf
```

### Important components

| Component | Responsibility |
|---|---|
| `PCB` | Thread control block and thread state |
| `Scheduler` | Ready/sleep queues and thread selection |
| `Semaphore` | Thread synchronization and blocking |
| `memoryAllocator` | Dynamic memory allocation |
| `System` | Trap handling and system call dispatch |
| `ContextSwitch.S` | Low-level context switching |
| `Trap.S` | Trap entry/exit and register preservation |
| `syscall_c.cpp` | C API and ABI wrappers |
| `syscall_cpp.cpp` | C++ API implementation |
| `main.cpp` | Kernel initialization and user application startup |
| `test/` | Functional tests |

---

## 19. Technologies

- **C++**
- **RISC-V Assembly**
- **RISC-V architecture**
- **RISC-V `ecall` / trap mechanism**
- **xv6-based educational execution environment**
- **C API**
- **C++ object-oriented API**
- **Low-level memory management**
- **Thread scheduling**
- **Semaphores**
- **Context switching**

The project intentionally implements the operating-system mechanisms itself rather than relying on the host operating system for threads, synchronization, or memory management.

---

## 20. Building and Running

The project is designed to be built and executed inside the development environment provided for the **Operating Systems 1** course.

The repository contains the kernel source code and tests, while the complete execution environment depends on the course-provided RISC-V/xv6 support files and hardware library.

After configuring the course environment:

1. Build the kernel and link it with the required support libraries.
2. Start the RISC-V educational emulator/environment.
3. Run the resulting executable.
4. Select or configure the desired test in `test/userMain.cpp`.

The repository does not currently contain a standalone build script, so the exact build command depends on the course environment used to compile and launch the project.

---

## 21. Implementation Status

| Feature | Status |
|---|---|
| Memory allocator | ✅ Implemented |
| `mem_alloc` / `mem_free` | ✅ Implemented |
| Free-space inspection | ✅ Implemented |
| Thread creation | ✅ Implemented |
| Thread termination | ✅ Implemented |
| Synchronous dispatch | ✅ Implemented |
| Context switching | ✅ Implemented |
| FIFO scheduler | ✅ Implemented |
| Semaphores | ✅ Implemented |
| C API | ✅ Implemented for supported features |
| C++ Thread API | ✅ Implemented |
| C++ Semaphore API | ✅ Implemented |
| Kernel/user trap mechanism | ✅ Implemented |
| Timer-based preemption | ❌ Not implemented |
| Asynchronous context switching | ❌ Not implemented |
| `time_sleep` | ❌ Not implemented |
| Full console syscall layer | ❌ Not implemented |

---

## 22. Example Execution

A simple synchronization scenario can be built using two threads and two semaphores:

```text
Thread 1                  Thread 2
   |                         |
   | wait(sem1)              | wait(sem2)
   |                         |
   | print                   | blocked
   |                         |
   | signal(sem2) ---------->|
   |                         | print
   |                         | signal(sem1)
   |<------------------------|
   | print                   |
   |                         |
   ...                       ...
```

This demonstrates how the scheduler, thread control blocks, semaphore queues, system calls, and context switching cooperate to provide synchronized concurrent execution.

---

## 23. Project Context

This project was developed as part of the **Operating Systems 1** course at the Faculty of Electrical Engineering, University of Belgrade.

The project specification is included in the repository as:

```text
Projektni zadatak 2025. v1.0.pdf
```

The implementation follows the architecture and interfaces specified for the RISC-V educational operating-system environment.

---

## Author

**Vuk Dinić**

GitHub: [@vuk007](https://github.com/vuk007)
