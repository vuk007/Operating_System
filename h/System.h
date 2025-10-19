#ifndef OSNOVNI_PROJEKAT_SYSTEM_H
#define OSNOVNI_PROJEKAT_SYSTEM_H
#include "../lib/hw.h"

enum MaskSip {
    SIP_SSIP = (1 << 1),
    SIP_STIP = (1 << 5),
    SIP_SEIP = (1 << 9)
};
enum MaskSstatus {
    SSTATUS_SIE = (1 << 1),
    SSTATUS_SPIE = (1 << 5),
    SSTATUS_SPP = (1 << 8)
};
class System {
public:
    static void popSppSie();
    static void terminate();
    static uint64 sstatus_r();
    static void sstatus_w(uint64 sstatus);
    static void sstatus_s(uint64 mask);
    static void sstatus_c(uint64 mask);
    static uint64 sip_r();
    static void sip_w(uint64 sip);
    static void sip_s(uint64 mask);
    static void sip_c(uint64 mask);
    // ----- CONTROL REGISTRI -----
    static uint64 sepc_r();
    static void sepc_w(uint64 sepc);
    static uint64 stvec_r();
    static void stvec_w(uint64 stvec);
    static uint64 stval_r();
    static void stval_w(uint64 stval);
    static uint64 scause_r();
    static void scause_w(uint64 scause);
    // ----- INTERRUPT -----
    static void supervisorTrap();
private:
    static void handleTrap();
};

inline void System::terminate() {
    uint32* adr = (uint32*)0x100000;
    *adr = 0x5555;
}
inline uint64 System::sstatus_r() {
    uint64 volatile sstatus;
    __asm__ volatile ("csrr %0, sstatus" : "=r" (sstatus));
    return sstatus;
}
inline void System::sstatus_w(uint64 sstatus) {
    __asm__ volatile ("csrw sstatus, %0" : : "r" (sstatus));
}
inline void System::sstatus_s(uint64 mask) {
    __asm__ volatile ("csrs sstatus, %0" : : "r" (mask));
}
inline void System::sstatus_c(uint64 mask) {
    __asm__ volatile ("csrc sstatus, %0" : : "r" (mask));
}
inline uint64 System::sip_r() {
    uint64 volatile sip;
    __asm__ volatile ("csrr %0, sip" : "=r" (sip));
    return sip;
}
inline void System::sip_w(uint64 sip) {
    __asm__ volatile ("csrw sip, %0" : : "r" (sip));
}
inline void System::sip_s(uint64 mask) {
    __asm__ volatile ("csrs sip, %0" : : "r" (mask));
}
inline void System::sip_c(uint64 mask) {
    __asm__ volatile ("csrc sip, %0" : : "r" (mask));
}
inline uint64 System::sepc_r() {
    uint64 volatile sepc;
    __asm__ volatile ("csrr %0, sepc" : "=r" (sepc));
    return sepc;
}
inline void System::sepc_w(uint64 sepc) {
    __asm__ volatile ("csrw sepc, %0" : : "r" (sepc));
}
inline uint64 System::stvec_r() {
    uint64 volatile stvec;
    __asm__ volatile ("csrr %0, stvec" : "=r" (stvec));
    return stvec;
}
inline void System::stvec_w(uint64 stvec) {
    __asm__ volatile ("csrw stvec, %0" : : "r" (stvec));
}
inline uint64 System::stval_r() {
    uint64 volatile stval;
    __asm__ volatile ("csrr %0, stval" : "=r" (stval));
    return stval;
}
inline void System::stval_w(uint64 stval) {
    __asm__ volatile ("csrw stval, %0" : : "r" (stval));
}
inline uint64 System::scause_r() {
    uint64 volatile scause;
    __asm__ volatile ("csrr %0, scause" : "=r" (scause));
    return scause;
}
inline void System::scause_w(uint64 scause) {
    __asm__ volatile ("csrw scause, %0" : : "r" (scause));
}



#endif //OSNOVNI_PROJEKAT_SYSTEM_H
