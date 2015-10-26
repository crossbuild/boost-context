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
.globl _make_fcontext
.align 2
_make_fcontext:
    /* first arg of make_fcontext() == top of context-stack */
    movl  0x4(%esp), %eax

    /* reserve space for first argument of context-function
       rax might already point to a 16byte border */
    leal  -0x8(%eax), %eax

    /* shift address in EAX to lower 16 byte boundary */
    andl  $-16, %eax

    /* reserve space for context-data on context-stack */
    /* size for fc_mxcsr .. EIP + return-address for context-function */
    /* on context-function entry: (ESP -0x4) % 8 == 0 */
    leal  -0x28(%eax), %eax

    movl  $-1, %ecx
    /* set member ctx of transfer_t to -1 */
    movl  %ecx, 0x20(%eax) 
    /* set member data of transfer_t to -1 */
    movl  %ecx, 0x24(%eax)
    /* store address of transfer_t into (hidden) VA */
    leal  0x20(%eax), %ecx
    movl  %ecx, 0x1c(%eax)

    /* third arg of make_fcontext() == address of context-function */
    movl  0xc(%esp), %edx
    /* store it in EBP */
    movl  %edx, 0x14(%eax)

    /* save MMX control- and status-word */
    stmxcsr  (%eax)
    /* save x87 control-word */
    fnstcw  0x4(%eax)

    /* compute abs address of label trampoline */
    call  1f
    /* address of label 1 */
1:  popl  %ecx
    /* compute abs address of label trampoline */
    addl  $trampoline-1b, %ecx
    /* save address of trampoline as return-address */
    /* store it in EIP */
    movl  %ecx, 0x18(%eax)

    /* compute abs address of label finish */
    call  2f
    /* address of label 2 */
2:  popl  %ecx
    /* compute abs address of label finish */
    addl  $finish-2b, %ecx
    /* save address of finish as return-address for context-function */
    /* will be entered after context-function returns */
    /* store it in EBX */
    movl  %ecx, 0x10(%eax)

    ret /* return pointer to context-data */

trampoline:
    /* first time context was called */
    /* push return-function (exit()) to context-stack */
    pushl  %ebx
    /* indirect jump to context-fn */
    jmp  *%ebp

finish:
    call  3f
    /* address of label 3 */
3:  popl  %ebx
    /* compute address of GOT and store it in EBX */
    addl  $_GLOBAL_OFFSET_TABLE_+[.-3b], %ebx
    /* exit code is zero */
    xorl  %eax, %eax
    pushl  %eax
    /* exit application */
    call  _exit@PLT
    hlt
