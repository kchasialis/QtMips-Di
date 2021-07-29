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

// t1 = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + ... t2

_start:
    li t0, 0 // i = 0
    li t1, 0 // s = 0
    li t2, 10 // # of loops

loop:
    addi t0, t0, 1
    add t1, t1, t0
    j check

end:
    break
	beq  zero, zero, loop
	nop

check:
    bne t0, t2, loop
    j end    

.end _start
