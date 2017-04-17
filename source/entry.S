
#define ABI_CONVERT(name) \
	.thumb;\
	push {r0-r7, lr};\
	mov r0, r8;\
	mov r1, r9;\
	mov r2, r10;\
	mov r3, r11;\
	mov r4, r12;\
	push {r0-r4};\
	mov r1, sp;\
	sub sp, #0x34;\
	mov r0, sp;\
	bl copyRegs;\
	bl name;\
	mov r1, sp;\
	mov r0, r1;\
	add r0, #0x34;\
	bl restoreRegs;\
	add sp, #0x34;\
	pop {r0-r4};\
	mov r8, r0;\
	mov r9, r1;\
	mov r10, r2;\
	mov r11, r3;\
	mov r12, r4;\
	pop {r0-r7};\
	pop {pc};\
	.align 2;

#define HOOK_VECTOR_ENTRY(name)\
	b name;\
	.align 2;

.section .text.start
	.thumb
	HOOK_VECTOR_ENTRY(ABI_CONVERT_hook_sub3007474)
	HOOK_VECTOR_ENTRY(ABI_CONVERT_hook_sub8035fc8)
	HOOK_VECTOR_ENTRY(ABI_CONVERT_hook_sub8034058)
	HOOK_VECTOR_ENTRY(hook_sub803362C)
	HOOK_VECTOR_ENTRY(hook_sub8033BF0)

ABI_CONVERT_hook_sub3007474:
	ABI_CONVERT(hook_sub3007474)

ABI_CONVERT_hook_sub8035fc8:
	ABI_CONVERT(hook_sub8035fc8)

ABI_CONVERT_hook_sub8034058:
	ABI_CONVERT(hook_sub8034058)

.global callOrignalFunc;
.type callOrignalFunc %function;
.thumb
callOrignalFunc:
	push {r4-r7,lr}
	mov r4,r8
	mov r5,r9
	mov r6,r10
	mov r7,r11
	push {r4-r7}
	push {r0}
	push {r1}
	add r0,r0,#0x20
	ldmia r0!,{r1-r5}
	mov r8,r1
	mov r9,r2
	mov r10,r3
	mov r11,r4
	mov r12,r5
	sub r0,r0,#0x34
	ldmia r0!,{r0-r7}
	mov lr,pc
	pop {r15}
	push {r0}
	ldr r0,[sp,#0x4]
	add r0,r0,#0x4
	stmia r0!,{r1-r7}
	mov r1,r8
	mov r2,r9
	mov r3,r10
	mov r4,r11
	mov r5,r12
	stmia r0!,{r1-r5}
	sub r0,r0,#0x34
	pop {r1}
	str r1,[r0]
	add sp, #4
	pop {r4-r7}
	mov r8,r4
	mov r9,r5
	mov r10,r6
	mov r11,r7
	pop {r4-r7}
	pop {r1}
	bx r1