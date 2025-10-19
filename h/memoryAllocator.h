//
// Created by os on 10/18/25.
//

#ifndef PROJEKAT_MEMORYALLOCATOR_H
#define PROJEKAT_MEMORYALLOCATOR_H

#include "../lib/hw.h"
struct  Header {
    Header* next;
    Header* prev;
    size_t size;
};
class memoryAllocator {
    static bool initialized;
    static Header* freeHead;
    static void init();
    memoryAllocator() = default;
    ~memoryAllocator() = default;
public:
    static size_t mem_get_largest_free_block();
    static size_t mem_get_free_space();
    static int mem_free(void* ptr);
    static void* mem_alloc(size_t size);
    static void tryToJoin(Header* block);
};


#endif //PROJEKAT_MEMORYALLOCATOR_H
