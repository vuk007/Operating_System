
#ifndef PROJEKAT_PRINT_H
#define PROJEKAT_PRINT_H
#include "../lib/console.h"
#include "../lib/hw.h"
class Syso{
public:
    void printString(const char* s) {
        while (*s) {
            __putc(*s);   // ili console_putc(*s);
            s++;
        }
    }

    void printHex(uint64 num) {
        static const char* hex = "0123456789ABCDEF";
        char buf[17];
        buf[16] = '\0';
        for (int i = 15; i >= 0; i--) {
            buf[i] = hex[num & 0xF];
            num >>= 4;
        }
        printString("0x");
        printString(buf);
    }
};

#endif //PROJEKAT_PRINT_H
