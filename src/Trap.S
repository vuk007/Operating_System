.extern _ZN6System10handleTrapEv
.align 4
.global _ZN6System14supervisorTrapEv
.type _ZN6System14supervisorTrapEv, @function
_ZN6System14supervisorTrapEv:
    addi sp, sp, -256
    .irp ind,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    sd x\ind, \ind*8(sp)
    .endr

    call _ZN6System10handleTrapEv

    .irp ind,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    ld x\ind, \ind*8(sp)
    .endr
    addi sp, sp, 256

    sret