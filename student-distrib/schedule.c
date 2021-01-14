#include "schedule.h"
#include "i8259.h"
#include "lib.h"
#include "system_handler.h"
#include "paging.h"
#include "x86_desc.h"
#include "keyboard.h"

#define PIT_IRQ     0
#define PIT_PORT    0x43
#define CMDB        0x36
#define PIT_CPORT   0x40
#define DIVISOR     11932

/*Flags*/
int t1 = 0;
int t2 = 0;
int t3 = 0;

/*
 * PIT_init
 *   DESCRIPTION: intializes the pit
 *   INPUTS:NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: NONE
 *   SIDE EFFECTS:intializes the pit
 */
void PIT_init(){
    outb(CMDB, PIT_PORT);               /* Set our command byte 0x34 */
    outb(DIVISOR & 0xFF , PIT_CPORT);   /* Set low byte of divisor */
    outb(DIVISOR >> 8 , PIT_CPORT);     /* Set high byte of divisor */
    enable_irq(PIT_IRQ);
    return;
}


/*
 * PIT_handler
 *   DESCRIPTION: handles the pit
 *   INPUTS:NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: NONE
 *   SIDE EFFECTS:handles the pit and boots the terminals
 */
void PIT_handler() {
    send_eoi(PIT_IRQ);
    /*Booting code for the three terminals*/
    if(t3 == 0) {
        t3 = 1;

        /* GETTING CURRENT VIDEO MEMORY*/
        memcpy((int32_t *)VIDEO_ADDRESS, (int32_t *) (VIDEO_ADDRESS + ALIGN * 3), ALIGN);
        update_c(tcurserx[2],tcursery[2]); // updates show_x, show_y
        update_cursor(tcurserx[2],tcursery[2]); // updates position
        boot_execute(2);
    }

    if(t2 == 0) {
        t2 = 1;
        /* SAVING PREVIOUS VIDEO MEMORY*/
        memcpy((uint8_t*) (VIDEO_ADDRESS + ALIGN * (3)), (uint8_t*) VIDEO_ADDRESS , ALIGN); 
        tcurserx[2] = getx(); 
        tcursery[2] = gety();
        
        /* GETTING CURRENT VIDEO MEMORY*/
        memcpy((int32_t *)VIDEO_ADDRESS, (int32_t *) (VIDEO_ADDRESS + ALIGN * 2), ALIGN);
        update_c(tcurserx[1],tcursery[1]); // updates show_x, show_y
        update_cursor(tcurserx[1],tcursery[1]); // updates position
        boot_execute(1);
    }

    if(t1 == 0) {
        t1 = 1;
        pnum = tnum;
        /* SAVING PREVIOUS VIDEO MEMORY*/
        memcpy((uint8_t*) (VIDEO_ADDRESS + ALIGN * (2)), (uint8_t*) VIDEO_ADDRESS, ALIGN); 
        tcurserx[1] = getx(); //save getx()
        tcursery[1] = gety(); //save gety()
        
        /* GETTING CURRENT VIDEO MEMORY*/
        memcpy((int32_t *)VIDEO_ADDRESS, (int32_t *) (VIDEO_ADDRESS + ALIGN * 1), ALIGN);
        update_c(tcurserx[0],tcursery[0]); // updates show_x, show_y
        update_cursor(tcurserx[0],tcursery[0]); // updates position
        boot_execute(0);
    }

    /*Scheduling Code that doesnt work*/

    /*Gets current prcoess PCB*/
    // pcb_t* pcb = get_PCBs(parray[pnum]);
    // if (t1 == 1 || t2 == 1 || t3 == 1) {
    //     pcb->sesp = save_ESP();
    //     pcb->sebp = save_EBP();
    // }
    // //memcpy( (uint8_t*) VIDEO_ADDRESS, (uint8_t*)(VIDEO_ADDR_FOURKB + ALIGN*((tnum))), ALIGN);
    //     // pte[VIDEO_ADDRESS/ALIGN] = (VIDEO_ADDRESS + ALIGN * (tnum + 1)) | READ_WRITE | PRESENT;
    // // pde[0] = (int) pte | READ_WRITE | PRESENT;


    // // update_c(tcurserx[tnum],tcursery[tnum]); // updates show_x, show_y
    // // update_cursor(tcurserx[tnum],tcursery[tnum]); // updates position
    
    // pcb_t* next = pcb->next; // get the
    // while (next->child) { // find base child of pcb for this process
    //     next = next->child;
    // }

    // // if (next->process_num > 2) {
    //     pnum = next->terminal;
    // // }

    // if(pnum != tnum){ // VIDEO MEMORY PAGING
    //     pte[VIDEO_ADDRESS/ALIGN] = (VIDEO_ADDRESS + ALIGN * (pnum + 1)) | READ_WRITE | PRESENT;
    // }
    // else{
    //     pte[VIDEO_ADDRESS/ALIGN] = VIDEO_ADDRESS | READ_WRITE | PRESENT;
    // }
    // // flushTLB();

    // // page for process USER PROGRAM PAGING
    // pde[VIRTUAL_ADDRESS / FOURMB] = (PHYSICAL_ADDRESS + (next->process_num) * FOURMB) | PAGE_SIZE | READ_WRITE | PRESENT | USER;
    // flushTLB();

    // // save kernal stack for process
    // tss.ss0 = KERNEL_DS;
    // tss.esp0 = PHYSICAL_ADDRESS - (EIGHTKB * (next->process_num + 1)) - 4; // location of stack for a process, 4 is an address offset
    // if (next->sesp && next->sebp){                  
    //     update_stack(next->sesp, next->sebp); //update stack for next terminal's process stack
    // }
    return;
}
