//
// Created by os on 10/18/25.
//

#include "../h/memoryAllocator.h"

bool memoryAllocator::initialized = false;
Header* memoryAllocator::freeHead = nullptr;


void *memoryAllocator::mem_alloc(size_t size) {

    if(!initialized)memoryAllocator::init();
    if(size<=0)return nullptr;
    size_t size_blocks = ((size + MEM_BLOCK_SIZE - 1)/MEM_BLOCK_SIZE)*MEM_BLOCK_SIZE;
    Header* cur = freeHead;
    for (; cur; cur = cur->next)
        if (cur->size >= size_blocks) break;
    if (!cur) return nullptr;
    size_t remaining = cur->size - size_blocks;
    if (remaining > sizeof(Header)) {
        Header* newStart = (Header*)((size_t)cur + sizeof(Header) + size_blocks);
        newStart->next = cur->next;
        newStart->prev = cur->prev;
        newStart->size = remaining - sizeof(Header);
        if (cur->prev)
            cur->prev->next = newStart;
        else
            freeHead = newStart;
        if (cur->next)
            cur->next->prev = newStart;
        cur->size = size_blocks;
    } else {
        if (cur->prev)
            cur->prev->next = cur->next;
        else
            freeHead = cur->next;
        if (cur->next)
            cur->next->prev = cur->prev;
    }
    return (void*)((size_t)cur + sizeof(Header));
}
int memoryAllocator::mem_free(void* address) {
    if (!initialized)
        memoryAllocator::init();
    if (!address) return -1; // sigurnosna provera
    Header* block = (Header*)((size_t)address - sizeof(Header));
    Header* cur = freeHead;
    Header* prev = nullptr;

    while (cur && (size_t)cur < (size_t)block) {
        prev = cur;
        cur = cur->next;
    }

    block->prev = prev;
    block->next = cur;
    if (cur) cur->prev = block;
    if (prev) prev->next = block;
    else freeHead = block;
    if (block->next && ((size_t)block + sizeof(Header) + block->size == (size_t)block->next)) {
        block->size += sizeof(Header) + block->next->size;
        block->next = block->next->next;
        if (block->next) block->next->prev = block;
    }
    if (block->prev && ((size_t)block->prev + sizeof(Header) + block->prev->size == (size_t)block)) {
        block->prev->size += sizeof(Header) + block->size;
        block->prev->next = block->next;
        if (block->next) block->next->prev = block->prev;
        block = block->prev;
    }
    tryToJoin(block);
    return 0;
}

size_t memoryAllocator::mem_get_largest_free_block() {
    if (!initialized)
        memoryAllocator::init();
    size_t max_size = 0;
    Header* cur = freeHead;
    while (cur) {
        if (cur->size > max_size)
            max_size = cur->size;
        cur = cur->next;
    }
    return max_size;
}
void memoryAllocator::tryToJoin(Header* block) {
    if (!block) return;
    if (block->next &&
        (size_t)block + sizeof(Header) + block->size == (size_t)block->next) {
        block->size += sizeof(Header) + block->next->size;
        block->next = block->next->next;
        if (block->next)
            block->next->prev = block;
    }
    if (block->prev &&
        (size_t)block->prev + sizeof(Header) + block->prev->size == (size_t)block) {
        block->prev->size += sizeof(Header) + block->size;
        block->prev->next = block->next;
        if (block->next)
            block->next->prev = block->prev;
    }
}
size_t memoryAllocator::mem_get_free_space() {
    if (!initialized)memoryAllocator::init();
    size_t total_free = 0;
    Header* cur = freeHead;
    while (cur) {
        total_free += cur->size;
        cur = cur->next;
    }
    return total_free;
}
void memoryAllocator::init() {
    freeHead = (Header*)HEAP_START_ADDR;
    freeHead->next = nullptr;
    freeHead->prev = nullptr;
    freeHead->size = (size_t)HEAP_END_ADDR - (size_t)HEAP_START_ADDR - sizeof(Header);
    initialized = true;
}