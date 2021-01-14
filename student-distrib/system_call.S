#define ASM     1
#define CALL_MIN    1
#define CALL_MAX    10
#define KERNEL_DS   0x0018
#define USER_DS     0x002B
#define USER_CS     0x0023
#define MEM         0x083FFFFC
#define PCB_MASK    0xFFFFE000
#define FLAGMASK    0x200

# header in system_handler.h

.text
.globl  assembly_halt
assembly_halt: # halts process running
    pushl   %ebp
    movl    %esp, %ebp      
    xorl    %ebx, %ebx    # clear EBX
    movl    8(%ebp), %ebx # move 8 bit status parameter to ebx
    call    get_PCB       # Get PCB of current process
    movl    8(%eax), %eax # Get Parent's PCB
    movl    4(%eax), %ebp # get parent's ebp pointer
    movl    (%eax), %esp    # get parent's esp pointer
    movl    %ebx, %eax    # The return value "32 bit status" to eax
    leave
    ret

.text
.globl  get_PCB
get_PCB: # gets top PCB
    pushl   %ebp
    movl    %esp, %ebp
    movl    %esp, %eax
    andl    $PCB_MASK, %eax
    leave
    ret

.text
.globl  save_EBP
save_EBP:
    movl    %ebp, %eax
    ret

.text
.globl  save_ESP
save_ESP:
    movl    %esp, %eax
    ret

.text
.globl  update_stack

update_stack:
    pushl   %ebp
    movl    %esp, %ebp
    xorl    %eax, %eax
    movl    12(%esp), %eax  # get ebp from parameter list
    movl    %eax , %ebp
    movl    8(%esp), %eax # get esp from parameter list
    movl    %eax , %esp
    leave
    ret

.text
.globl  context_switch

context_switch: # switch context to user level
    cli
    pushl   %ebp
    movl    %esp, %ebp    
    movl    8(%ebp), %eax
    xorl    %ebx, %ebx
    movw    $USER_DS, %bx
    movw    %bx, %ds
    pushl   $USER_DS
    pushl   $MEM

    pushfl
    popl    %ebx
    orl     $FLAGMASK, %ebx
    pushl   %ebx
    pushl   $USER_CS
    pushl   %eax
    
    pushl   %ebp
    pushl   %esp
    call    save_PCB
    addl    $8, %esp # only pop 2
    sti
    iret

sys_call_jump_table:
.long 0x0, sys_halt, sys_execute, sys_read, sys_write, sys_open, sys_close, sys_getargs, sys_vidmap, sys_set_handler, sys_sigreturn

.text
.globl sys_call_handler

sys_call_handler: # assembly linkage for systen call handler 
    # pushl %esp
    # pushl %ebp
    pushl %esi
    pushl %edi
    pushl %edx
    pushl %ecx
    pushl %ebx
    # pushfl

    cmpl    $CALL_MIN, %eax
    jl      invalid
    cmpl    $CALL_MAX, %eax
    jg      invalid

    call    *sys_call_jump_table(, %eax, 4)

call_done:
    # popfl
    popl %ebx
    popl %ecx
    popl %edx
    popl %edi
    popl %esi
    # popl %ebp
    # popl %esp
halt_jump:    
    iret  # changes back to original privilege level

invalid:
    movl $-1, %eax
    # popfl
    popl %ebx
    popl %ecx
    popl %edx
    popl %edi
    popl %esi
    # popl %ebp
    # popl %esp
    iret
