#define CR0_MASK    $0x80000001
#define CR4_MASK    $0x00000010

# Code adapted from osdev @https://wiki.osdev.org/Setting_Up_Paging


# header in paging.h
.text
.globl loadPageDirectory
loadPageDirectory:
    pushl    %ebp
    movl     %esp, %ebp
    movl     8(%esp), %eax
    movl     %eax, %cr3
    movl     %ebp, %esp
    popl     %ebp
    ret

# header in sys_handler.h

.text
.globl flushTLB
flushTLB:
    pushl    %ebp
    movl     %esp, %ebp
    movl     %cr3, %eax
    movl     %eax, %cr3
    movl     %ebp, %esp
    popl     %ebp
    ret

# header in paging.h

.text
.globl enablePaging
enablePaging:
    pushl   %ebp
    movl    %esp, %ebp
    movl    %cr4, %eax
    orl     CR4_MASK, %eax
    movl    %eax, %cr4
    movl    %cr0, %eax
    orl     CR0_MASK, %eax
    movl    %eax, %cr0
    movl    %ebp, %esp
    popl    %ebp
    ret
