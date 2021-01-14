#ifndef SYSTEM_HANDLER_H
#define SYSTEM_HANDLER_H

#include "types.h"
#include "lib.h"
#include "keyboard.h"
#include "rtc.h"
#include "filesystem.h"

int32_t getProcessNum();

int32_t sys_halt(uint8_t status);

int32_t sys_execute(const uint8_t* command);

int32_t sys_read(int32_t fd, void* buf, int32_t nbytes);

int32_t sys_write (int32_t fd, const void* buf, int32_t nbytes);

int32_t sys_open(const uint8_t* filename);

int32_t sys_close(int32_t fd);

int32_t sys_getargs(uint8_t* buf, int32_t nbytes);

int32_t sys_vidmap(uint8_t** screen_start);

int32_t sys_set_handler (int32_t signum, void* handler_address);

int32_t sys_sigreturn();

int32_t error();

void save_PCB(uint32_t esp, uint32_t ebp);

void boot();

int32_t boot_execute(const int num);

void tswitch(int num);

#define VIDEO_ADDR_FOURKB   0x000B9000
#define VIRTUAL_ADDRESS     0x08000000
#define PHYSICAL_ADDRESS    0x00800000
#define ZEROMB              0x00000000
#define FOURMB              0x00400000
#define EIGHTKB             0x00002000
#define FD_MAX              8
#define FD_MIN              2
#define ADDR_OFFSET         0x00048000
#define RTC_TYPE            0
#define DIR_TYPE            1
#define FILE_TYPE           2
#define BUFFER_SIZE         32
#define VIDMAP              33
#define OFFSET22            22
#define OFFSET12            12

extern int32_t anythingRunning;
extern volatile int pnum;

typedef struct FOT {
    int32_t (*open)(const uint8_t* filename);
    int32_t (*close)(int32_t fd);
    int32_t (*read)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*write)(int32_t fd, const void* buf, int32_t nbytes);
} fot_t;

typedef struct FDT {
    fot_t* fotp;               //file operation table pointer
    int32_t inode;
    int32_t file_position;
    int32_t flags;
} fd_t;

typedef struct PCB{
    uint32_t esp;
    uint32_t ebp;
    struct PCB* parent;
    struct PCB* child;
    struct PCB* next;
    uint32_t sesp;       // save 
    uint32_t sebp;
    int terminal;
    int32_t process_num; // 0-5
    fd_t fdt[8];
    uint8_t args[128];
}  pcb_t;

/*
 * flushTLB
 *   DESCRIPTION: flushes the tlb
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   SIDE EFFECTS: flushes tlb
 */
extern void flushTLB();

/*
 * context_switch
 *   DESCRIPTION: switches context to user spaces
 *   INPUTS: uint32_t entry
 *   OUTPUTS: NONE
 */
extern uint32_t context_switch(uint32_t entry);

/*
 * assembly_halt
 *   DESCRIPTION: assembly function used in halt 
 *   INPUTS: status: the status number of the process being halted
 *   OUTPUTS: NONE
 *   RETURN VALUE: returns value to execute 
 *   SIDE EFFECTS: restores parent ESP and EBP and returns to execute
 */
extern uint32_t assembly_halt(uint32_t status);

/*
 * get_PCB
 *   DESCRIPTION: gets pointer of PCB currenly in use
 *   RETURN VALUE: pcb_t* pointer of the current pcb
 */
extern pcb_t* get_PCB();

// extern pcb_t* assembly_get_PCB();
/*
 * getProcessNum
 *   DESCRIPTION: Gives us the next open process and marks it as active
 *   INPUTS: N/A
 *   OUTPUTS: 
 *   RETURN VALUE: The process num (0-5) or -1 if everything is active
 *   SIDE EFFECTS: sets next available process as active
 */
int32_t getProcessNum();


/*
 * update_stack
 *   DESCRIPTION: updates the esp and ebp
 *   RETURN VALUE: none
 */
extern void update_stack(uint32_t esp, uint32_t ebp);

extern int parray[3];

/*For the scheduling that didnt work*/
/*
 * save_ESP
 *   DESCRIPTION: saves the esp
 *   RETURN VALUE: none
 */
extern uint32_t save_ESP();
/*
 * save_EBP
 *   DESCRIPTION: saves the ebp
 *   RETURN VALUE: none
 */
extern uint32_t save_EBP();
/*Stores the root shell pcbs*/
extern pcb_t *tpcb[3]; // pointers for base shell PCB in each terminal
/*
 * get_PCBs
 *   DESCRIPTION: gets pcb with respect to the process number
 *   RETURN VALUE: pcb_t* pointer of the pcb
 */
extern pcb_t* get_PCBs(int pnum);

#endif
