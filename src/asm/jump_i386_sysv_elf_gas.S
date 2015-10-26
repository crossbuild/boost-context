/*
            Copyright Oliver Kowalke 2009.
   Distributed under the Boost Software License, Version 1.0.
      (See accompanying file LICENSE_1_0.txt or copy at
          http://www.boost.org/LICENSE_1_0.txt)
*/

/************************************************************************************************************
 *                                                                                                          *
 *  ------------------------------------------------------------------------------------------------------  *
 *  |    0    |    1    |    2    |    3    |    4     |    5    |    6    |    7    |    8    |    9    |  *
 *  ------------------------------------------------------------------------------------------------------  *
 *  |   0x0   |   0x4   |   0x8   |   0xc   |   0x10   |   0x14  |   0x18  |   0x1c  |   0x20  |   0x24  |  *
 *  ------------------------------------------------------------------------------------------------------  *
 *  | fc_mxcsr|fc_x87_cw|   EDI   |   ESI   |   EBX    |   EBP   |   EIP   |    VA   |    CTX  |   DATA  |  *
 *  ------------------------------------------------------------------------------------------------------  *
 *                                                                                                          *
 ***********************************************************************************************************/

.text
.globl jump_fcontext
.align 2
.type jump_fcontext,@function
jump_fcontext:
    /* third arg of jump_fcontext() == flag indicating preserving FPU */
    movl  0x10(%esp), %ecx

    pushl  %ebp  /* save EBP */
    pushl  %ebx  /* save EBX */
    pushl  %esi  /* save ESI */
    pushl  %edi  /* save EDI */

    /* prepare stack for FPU */
    leal  -0x8(%esp), %esp

    /* test for flag preserve_fpu */
    test  %ecx, %ecx
    je  1f

    /* save MMX control- and status-word */
    stmxcsr  (%esp)
    /* save x87 control-word */
    fnstcw  0x4(%esp)

1:
    /* hidden arg of jump_fcontext() == address of returned transfer_t */
    movl  0x1c(%esp), %edx

    /* temp. store stack address of prev. context */
    movl  %esp, (%edx)

    /* second arg of jump_fcontext() == intptr_t data */
    movl  0x24(%esp), %eax
    /* temp. store intptr_t data of prev. context */
    movl  %eax, 0x4(%edx)

    /* first arg of jump_fcontext() == context jumping to */
    movl  0x20(%esp), %eax

    /* restore ESP (pointing to context-data) from EDX */
    movl  %eax, %esp

    /* test for flag preserve_fpu */
    test  %ecx, %ecx
    je  2f

    /* restore MMX control- and status-word */
    ldmxcsr  (%esp)
    /* restore x87 control-word */
    fldcw  0x4(%esp)
2:
    /* prepare stack for FPU */
    leal  0x8(%esp), %esp

    /* restore EDI */
    popl  %edi
    /* restore ESI */
    popl  %esi
    /* restore EBX */
    popl  %ebx
    /* restore EBP */
    popl  %ebp

    /* get address of transfer_t */
    movl 0x4(%esp), %eax

    /* load stack address of prev. context */
    movl  (%edx), %ecx
    /* store stack address of prev. context in returned transfer_t */
    movl  %ecx, (%eax)
    /* store stack address of prev. context as first arg */
    movl  %ecx, 0x4(%esp)

    /* load intptr_t data of prev. context */
    movl  0x4(%edx), %ecx
    /* store intptr_t data of prev. context in returned transfer_t */
    movl  %ecx, 0x4(%eax)
    /* store intpr_t data of prev. context as first arg */
    movl  %ecx, 0x8(%esp)
 
    /* restore return-address */
    popl  %edx

    /* indirect jump to context */
    jmp  *%edx
.size jump_fcontext,.-jump_fcontext

/* Mark that we don't need executable stack.  */
.section .note.GNU-stack,"",%progbits
