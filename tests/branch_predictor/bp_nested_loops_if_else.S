#define zero	$0
#define at	$1
#define v0 	$2
#define v1	$3
#define a0	$4
#define a1	$5
#define a2	$6
#define	a3	$7

#define t0	$8
#define t1	$9
#define t2	$10
#define t3	$11
#define t4	$12
#define t5	$13
#define t6	$14
#define t7	$15
#define t8	$24
#define t9	$25
#define k0	$26
#define k1	$27

#define s0	$16
#define s1	$17
#define s2	$18
#define s3	$19
#define s4	$20
#define s5	$21
#define s6	$22
#define s7	$23

#define gp	$28
#define sp	$29
#define fp	$30
#define ra	$31

.globl _start
.set noat
.set noreorder
.ent _start

_start:
    li t0, 151 // loops = 150
    li t1, 31 // loops2 = 30
    li t2, 0 // s_even = 0
    li t3, 0 // s_odd = 0
    li t4, 2

    li s0, 1 // i = 1
loop1:
    li s1, 1 // j = 1
loop2:
    div zero, s1, t4 // j % 2
    mfhi t5  // j % 2
    add t6, zero, s1 // move j to t6
    addi s1, s1, 1 // j++
    beq t5, zero, is_even // j % 2 == 0
    bne t5, zero, is_odd // j % 2 != 0
ret:
    bne s1, t1, loop2 // j != 31
    addi s0, s0, 1 // i++
    bne s0, t0, loop1 // i != 151

    mul t6, t2, t3 // p(t6) = s_even * s_odd

loop:	
    break
	beq  zero, zero, loop
	nop

is_even:
    add t2, t2, t6
    j ret

is_odd:
    add t3, t3, t6
    j ret

.end _start

