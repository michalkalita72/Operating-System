# header in idt.h
.text
.globl pit_handle
pit_handle:
    cli
    pushal
    call    PIT_handler
    popal
    iret


.text
.globl keyboard_handle

# Linkage function

# header in idt.h
# GOD MODE
keyboard_handle: 
    cli
    pushal
    call    keyboard_handler
    popal
    sti
    iret


# header in idt.h
.text
.globl rtc_handle
rtc_handle:
    cli
    pushal
    call    rtc_handler
    popal
    sti
    iret


# used for checking where exception happened 
.text
.globl get_cr2

get_cr2: 
    movl %cr2, %eax
    ret
