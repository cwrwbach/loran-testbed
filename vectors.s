;@-------------------------------------------------------------------------

.globl _start
_start:
    ldr pc,reset_handler
    ldr pc,undefined_handler
    ldr pc,swi_handler
    ldr pc,prefetch_handler
    ldr pc,data_handler
    ldr pc,unused_handler
    ldr pc,irq_handler
    ldr pc,fiq_handler
reset_handler:      .word reset
undefined_handler:  .word hang
swi_handler:        .word hang
prefetch_handler:   .word hang
data_handler:       .word hang
unused_handler:     .word hang
irq_handler:        .word irq
fiq_handler:        .word hang

reset:
    mov r0,#0x8000
    mov r1,#0x0000
    ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
    stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}
    ldmia r0!,{r2,r3,r4,r5,r6,r7,r8,r9}
    stmia r1!,{r2,r3,r4,r5,r6,r7,r8,r9}

    ;@ (PSR_IRQ_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
    mov r0,#0xD2
    msr cpsr_c,r0
    mov sp,#0x8000

    ;@ (PSR_FIQ_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
    mov r0,#0xD1
    msr cpsr_c,r0
    mov sp,#0x4000

    ;@ (PSR_SVC_MODE|PSR_FIQ_DIS|PSR_IRQ_DIS)
    mov r0,#0xD3
    msr cpsr_c,r0
    mov sp,#0x8000000

    ;@ SVC MODE, IRQ ENABLED, FIQ DIS
    ;@mov r0,#0x53
    ;@msr cpsr_c, r0

    bl notmain
hang: b hang

;@---

.globl PUT32
PUT32:
    str r1,[r0]
    bx lr
;@---
.globl PUT16
PUT16:
    strh r1,[r0]
    bx lr
;@---
.globl PUT8
PUT8:
    strb r1,[r0]
    bx lr
;@---
.globl GET32
GET32:
    ldr r0,[r0]
    bx lr
;@---
.globl GETPC
GETPC:
    mov r0,lr
    bx lr
;@---

.globl MEMSET
MEMSET:
;@ r0 1st Arg = value to be set
;@ r1 2nd Arg = Adress of destination
;@ r2 3rd Arg = Number of words to store (SIZE ???)

loop_ms:
    str r0,[r1] ;@ destination
    add r1,#0x4
    sub r2,#1 ;@loop counter, r2 has 3rd argument
    cmp r2,#0
    bne loop_ms
    bx lr ;@return to caller (in C)

;@---

.globl MEMCPY
MEMCPY:
   
loop_mc:
    ldr r3,[r0]
    str r3,[r1] ;@ dest
    add r0,#0x4
    add r1,#0x4
    sub r2,#1
    cmp r2,#0
    bne loop_mc
    bx lr

;@---

.globl MEMCPY_B
MEMCPY_B:
   
loop_mc_b:
    ldr r3,[r0]
    str r3,[r1] ;@ dest
    add r0,#0x2
    add r1,#0x1
    sub r2,#1
    cmp r2,#0
    bne loop_mc_b
    bx lr

;@---
.globl RANDOM
RANDOM:
    xnm .req r0
    a .req r1

    mov a,#0xef00
    mul a,xnm
    mul a,xnm
    add a,xnm
.unreq xnm
    add r0,a,#73

.unreq a
    bx lr
;@---

.globl dummy
dummy:
    bx lr
;@---

.globl enable_irq
enable_irq:
    mrs r0,cpsr
    bic r0,r0,#0x80
    msr cpsr_c,r0
    bx lr

irq:
    push {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
    bl c_irq_handler
    pop  {r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,lr}
    subs pc,lr,#4

;@-------------------------------------------------------------------------



