#include "idt.h"
#include "x86_desc.h"
#include "lib.h"
#include "keyboard.h"
#include "rtc.h"
#include "system_call.h"
#include "system_handler.h"
#include "schedule.h"


// typedef union idt_desc_t {
//     uint32_t val[2];
//     struct {
//         uint16_t offset_15_00;
//         uint16_t seg_selector;
//         uint8_t  reserved4;
//         uint32_t reserved3 : 1;
//         uint32_t reserved2 : 1;
//         uint32_t reserved1 : 1;
//         uint32_t size      : 1;
//         uint32_t reserved0 : 1;
//         uint32_t dpl       : 2;
//         uint32_t present   : 1;
//         uint16_t offset_31_16;
//     } __attribute__ ((packed));
// } idt_desc_t;


// blueprint for defining funtions that get called on exceptions
#define ERROR(number, name)                                         \
    void PRINT_##number() {                                         \
        printf("Exception: " #name " at %x \n", get_cr2());                           \
        if (anythingRunning == 0){                                  \
          cli();                                                    \
          while(1){} }                                               \
        sys_halt(0);                                                \
    }

/*
 * gen_int
 *   DESCRIPTION: Default interrupt handler
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Prints a message and freezes the kernel
 */
void gen_int() {
    printf("Interupt \n");
    while(1);
}

/*
 * sys_call
 *   DESCRIPTION: System call interrupt handler, jumps
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Prints the System Call message

1. int32 t halt (uint8 t status);
2. int32 t execute (const uint8 t* command);
3. int32 t read (int32 t fd, void* buf, int32 t nbytes);
4. int32 t write (int32 t fd, const void* buf, int32 t nbytes);
5. int32 t open (const uint8 t* filename);
6. int32 t close (int32 t fd);
7. int32 t getargs (uint8 t* buf, int32 t nbytes);
8. int32 t vidmap (uint8 t** screen start);
9. int32 t set handler (int32 t signum, void* handler address);
10. int32 t sigreturn (void);
 */
// int32_t sys_call(uint32_t eax, uint32_t ebx, uint32_t ecx,  uint32_t edx) {

//     if(eax < 1 | eax > 10){
//         return -1;
//     }
//     switch(eax){
//     case 1:
//     return sys_halt((uint8_t ) (ebx && 0x00FF));
//     case 2:
//     return sys_execute(const uint8_t* command);
//     case 3:
//     return sys_read(int32_t fd, void* buf, int32_t nbytes);
//     case 4:
//     return sys_write (int32_t fd, const void* buf, int32_t nbytes);
//     case 5:
//     return sys_open(const uint8_t* filename);
//     case 6:
//     return sys_close(int32_t fd);
//     case 7:
//     return sys_getargs(uint8_t* buf, int32_t nbytes);
//     case 8:
//     return sys_vidmap(uint8_t** screen start);
//     case 9:
//     return sys_set_handler (int32_t signum, void* handler address)
//     case 10:
//     return  sys_sigreturn();
//     }
//     printf("System Call \n");
// }


// generating execptions functions
ERROR(0, Divide_Error);
ERROR(1, Debug_Exception);
ERROR(2, NMI_Interrupt);
ERROR(3, Breakpoint);
ERROR(4, Overflow);
ERROR(5, BOUND_Range_Exceeded);
ERROR(6, Invalid_Opcode);
ERROR(7, Device_Not_Available);
ERROR(8, Double_Fault);
ERROR(9, Coprocessor_Segment_Overrun);
ERROR(10, Invalid_TSS);
ERROR(11, Segment_Not_Present);
ERROR(12, Stack-Segment_Fault);
ERROR(13, General_Protection);
ERROR(14, Page_Fault);
ERROR(15, Intel_Reserved);
ERROR(16, x87_FPU_Floating_Point_Error);
ERROR(17, Alignment_Check);
ERROR(18, Machine_Check);
ERROR(19, SIMD_Floating_Point_Exception);

/*
 * idt_init
 *   DESCRIPTION: Initializes the IDT and fills all 256 entries.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: populates the idt array
 */
void idt_init() {
	int curVec;
    int i;

    // Fill each entry with the corresponding fields
	for(curVec = 0; curVec < NUM_VEC; curVec++) {
        idt[curVec].present = 1;
        idt[curVec].dpl = 0;
        idt[curVec].reserved0 = 0;
        idt[curVec].size = 1;
        idt[curVec].reserved1 = 1;
        idt[curVec].reserved2 = 1;
        idt[curVec].reserved3 = 0;
        idt[curVec].reserved4 = 0;
        idt[curVec].seg_selector = KERNEL_CS;

        if(curVec < 32){
          idt[curVec].reserved3 = 1;
        }
        // 0x80 (i.e system call vector), initialize entry, 3 is user level
        if(curVec == 0x80){
            idt[curVec].dpl = 3;
            idt[curVec].reserved3 = 0;
        }
	}


    // Setting offset values of the exectpions in the IDT table
    SET_IDT_ENTRY(idt[0], PRINT_0);
    SET_IDT_ENTRY(idt[1], PRINT_1);
    SET_IDT_ENTRY(idt[2], PRINT_2);
    SET_IDT_ENTRY(idt[3], PRINT_3);
    SET_IDT_ENTRY(idt[4], PRINT_4);
    SET_IDT_ENTRY(idt[5], PRINT_5);
    SET_IDT_ENTRY(idt[6], PRINT_6);
    SET_IDT_ENTRY(idt[7], PRINT_7);
    SET_IDT_ENTRY(idt[8], PRINT_8);
    SET_IDT_ENTRY(idt[9], PRINT_9);
    SET_IDT_ENTRY(idt[10], PRINT_10);
    SET_IDT_ENTRY(idt[11], PRINT_11);
    SET_IDT_ENTRY(idt[12], PRINT_12);
    SET_IDT_ENTRY(idt[13], PRINT_13);
    SET_IDT_ENTRY(idt[14], PRINT_14);
    SET_IDT_ENTRY(idt[15], PRINT_15);
    SET_IDT_ENTRY(idt[16], PRINT_16);
    SET_IDT_ENTRY(idt[17], PRINT_17);
    SET_IDT_ENTRY(idt[18], PRINT_18);
    SET_IDT_ENTRY(idt[19], PRINT_19);

    // User defined
    for(i = 32; i < NUM_VEC; i++) {
      if(i != 0x80){

        SET_IDT_ENTRY(idt[i], gen_int);
      }
    }

    //0x20 id pit, 0x21 is the Keyboard entry, 0x28 is the RTC entry, 0x80 is a System Call
    SET_IDT_ENTRY(idt[0x20], pit_handle);
    SET_IDT_ENTRY(idt[0x21], keyboard_handle);
    SET_IDT_ENTRY(idt[0x28], rtc_handle);
    SET_IDT_ENTRY(idt[0x80], sys_call_handler);
    lidt(idt_desc_ptr);
}
