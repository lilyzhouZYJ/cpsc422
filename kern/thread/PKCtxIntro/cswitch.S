/*
 * void cswitch(struct kctx *from, struct kctx *to);
 * From PKCtxIntro.c: cswitch(&kctx_pool[from_pid], &kctx_pool[to_pid])
 */
	.globl cswitch
cswitch:
	movl	4(%esp), %eax	/* %eax <- from */
	movl	8(%esp), %edx	/* %edx <- to */

	/* TODO: save the old kernel context */
	movl	%esp, (%eax)		/* kctx_pool[from_pid].esp <- %esp */
	movl	%edi, 0x4(%eax)		/* kctx_pool[from_pid].edi <- %edi */
	movl	%esi, 0x8(%eax)		/* kctx_pool[from_pid].esi <- %esi */
	movl	%ebx, 0xc(%eax)		/* kctx_pool[from_pid].ebx <- %ebx */
	movl	%ebp, 0x10(%eax)	/* kctx_pool[from_pid].ebp <- %ebp */
	movl	(%esp), %esi		/* %esi <- return address on stack */
	movl	%esi, 0x14(%eax)	/* kctx_pool[from_pid].eip <- return address on stack */

	/* TODO: load the new kernel context */
	movl	(%eax), %esp		/* %esp <- kctx_pool[to_pid].esp */
	movl	0x4(%eax), %edi		/* %edi <- kctx_pool[to_pid].edi */
	movl	0x8(%eax), %esi		/* %esi <- kctx_pool[to_pid].esi */
	movl	0xc(%eax), %ebx		/* %ebx <- kctx_pool[to_pid].ebx */
	movl	0x10(%eax), %ebp 	/* %ebp <- kctx_pool[to_pid].ebp */
	jmp		*0x14(%eax)

	xor	%eax, %eax
	ret
