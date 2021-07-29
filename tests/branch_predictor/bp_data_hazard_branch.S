#define zero	$0
#define at	$1
#define v0 	$2
#define v1	$3
#define a0	$4
#define a1	$5
#define a2	$6
#define a3	$7

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
    li t0, 1 // i = 1
    li t1, 0 // s = 0
    li t2, 11 // s = 1 + 2 + 3 ... (t2 - 1)
loop:
    add t1, t1, t0
    addi t0, t0, 1
    bne t0, t2, loop

end:	
    break
	beq  zero, zero, end

.end _start
