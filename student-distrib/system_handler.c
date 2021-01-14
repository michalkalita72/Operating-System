#include "system_handler.h"
#include "filesystem.h"
#include "rtc.h"
#include "keyboard.h"
#include "lib.h"
#include "paging.h"
#include "x86_desc.h"


/*Globals*/
int32_t anythingRunning = 0;
int32_t vidmem = 0;
int32_t process_num[6] = {1, 1, 1, 0, 0, 0};
volatile int pnum = 0;
/*For three terminals pcbs*/
fot_t rtc_fot = {&rtc_open, &rtc_close, &rtc_read, &rtc_write};
fot_t dir_fot = {&dir_open, &dir_close, &dir_read, &dir_write};
fot_t file_fot = {&file_open, &file_close, &file_read, &file_write};
fot_t stdin = {&error, &error, &terminal_read, &error};
fot_t stdout = {&error, &error, &error, &terminal_write};
pcb_t *tpcb[3];
pcb_t *current_pcb[3];
int parray[3]= {0,0,0};






/*
 * getProcessNum
 *   DESCRIPTION: Gives us the next open process and marks it as active
 *   INPUTS: N/A
 *   OUTPUTS: 
 *   RETURN VALUE: The process num (0-5) or -1 if everything is active
 *   SIDE EFFECTS: sets next available process as active
 */
int32_t getProcessNum() {
    int32_t i;
    for(i = 0; i < 6; i++) {
        if(process_num[i] == 0) {
            process_num[i] = 1;
            return i;
        }
    }
    return -1;
}


/*
 * tswitch
 *   DESCRIPTION: Terminal switch
 *   INPUTS: num, num is the terminal to switch to
 *   OUTPUTS: NONE
 *   RETURN VALUE: None
 *   SIDE EFFECTS: Switches the terminal to the correct screen
 */
void tswitch(int num) {
    cli();
    // num is number of the terminal we want to switch to
    pte[VIDEO_ADDRESS/ALIGN] = VIDEO_ADDRESS | READ_WRITE | PRESENT;
    flushTLB();
    memcpy((uint8_t*) (VIDEO_ADDRESS + ALIGN * ((tnum + 1))), (uint8_t*) VIDEO_ADDRESS , ALIGN); 
    tcurserx[tnum] = getx(); 
    tcursery[tnum] = gety();
    // clear screen
    
    tnum = num;
    // restore all things for terminal switching to
    update_c(tcurserx[num],tcursery[num]); // updates show_x, show_y
    update_cursor(tcurserx[num],tcursery[num]); // updates position
    memcpy( (uint8_t*) VIDEO_ADDRESS, (uint8_t*)(VIDEO_ADDRESS + ALIGN*((num + 1))), ALIGN);

    //memcpy((int32_t *)VIDEO_ADDRESS, (int32_t *) (VIDEO_ADDR_FOURKB + ALIGN*num), ALIGN);
    pcb_t *temp = tpcb[num];
    while(temp->child !=NULL){
        temp=temp->child;
    }
    
    pde[VIRTUAL_ADDRESS / FOURMB] = (PHYSICAL_ADDRESS + (temp->process_num) * FOURMB) | PAGE_SIZE | READ_WRITE | PRESENT | USER;
    flushTLB();
    tss.ss0 = KERNEL_DS;
    tss.esp0 = PHYSICAL_ADDRESS - EIGHTKB * (temp->process_num+1) - 4; // location of stack for a process
    sti();
    return;
}

/*
 * boot_execute
 *   DESCRIPTION: execute for the boot shells
 *   INPUTS: num, 0-2 index
 *   OUTPUTS: NONE
 *   RETURN VALUE: ret value from halt
 *   SIDE EFFECTS: executes the shell for one of the three terminals
 */
int32_t boot_execute(const int num) {
    cli();
    anythingRunning++;
    int i;
    int x;
    x = 0;
    i = 0;

    dentry_t dentry;
    if(read_dentry_by_name((const int8_t*)"shell", &dentry) == -1){
        return -1;
    }

    /*Allocating memory*/
    pde[VIRTUAL_ADDRESS / FOURMB] = (PHYSICAL_ADDRESS + (num) * FOURMB) | PAGE_SIZE | READ_WRITE | PRESENT | USER;
    flushTLB();

    /*program image must be copied to the corret offset*/
    int8_t * mem = (int8_t *) VIRTUAL_ADDRESS + ADDR_OFFSET;
    // reads that from data block and puts it in array starting at mem
    read_data(dentry.inode, 0, mem, inodes[dentry.inode].length);


    uint8_t buf2[BUFFER_SIZE];
    read_data(dentry.inode, 0, (int8_t *) buf2, BUFFER_SIZE);
    uint32_t entry;
    // setup PCB
    /*information is stored as a 4-byte unsigned integer in bytes 24-27 of the exeutable*/
    /*Grabbing the correct bytes and shifting them into the correct place*/
    entry = ((uint8_t) buf2[27] << 24) | ((uint8_t) buf2[26] << 16) | ((uint8_t) buf2[25] << 8) | (uint8_t) buf2[24];
    pcb_t* pcb;
    uint32_t * ptr = (uint32_t *) ((PHYSICAL_ADDRESS - (EIGHTKB * (num+1)) - 4) & 0xFFFFE000); //memory location of PCB
    pcb = (pcb_t*) ptr;
    tpcb[num]=pcb;
    parray[num] = num;
    current_pcb[num]=pcb;
    x = 0;
    while (x < 128) {
        pcb->args[x++] = '\0';
    }
    /*Code for scheduling to store the next pcb pointer*/
    uint32_t * addr = (uint32_t *) ((PHYSICAL_ADDRESS - (EIGHTKB * (((num+2)%3)+1)) - 4) & 0xFFFFE000); //memory location of PCB
    /*Sets up the pcb*/
    pcb->next = (pcb_t*) addr;
    pcb->parent = NULL;
    pcb->child = NULL;
    pcb->terminal = num;
    pcb->process_num = num; //PCB process num,
    pcb->fdt[0].flags = 1; //present,
    pcb->fdt[0].fotp = &stdin;
    pcb->fdt[1].flags = 1;
    pcb->fdt[1].fotp = &stdout;
    for (i = FD_MIN; i < FD_MAX; i++) {
        pcb->fdt[i].flags = 0;
    }
    tss.ss0 = KERNEL_DS;
    tss.esp0 = PHYSICAL_ADDRESS - EIGHTKB * (num + 1) - 4; // location of stack for a process
    context_switch(entry);
    return 0;
}





/*
 * save_PCB
 *   DESCRIPTION: Saves the current proccess pcb, ebp is for pcb
 *   INPUTS: uint32_t esp, uint32_t ebp
 *   OUTPUTS: NONE
 *   RETURN VALUE: None
 *   SIDE EFFECTS: saves pcb
 */
void save_PCB(uint32_t esp, uint32_t ebp){
    pcb_t* pcb = get_PCB();
    pcb->esp = esp;
    pcb->ebp = ebp;
    return;
}

/*
 * error
 *   DESCRIPTION: Placeholder function for stdin and stdout
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: -1
 *   SIDE EFFECTS: none
 */
int32_t error(){
    return -1;
}

/*
 * sys_halt
 *   DESCRIPTION: halts a process that is being called, undoes execute,
 *   INPUTS: const uint8_t* status
 *   OUTPUTS: NONE
 *   RETURN VALUE: nothing on sucsess, -1 on fail
 *   SIDE EFFECTS: halt a process, returns terminal control to parent process,
 *   redirects paging,flushes TLB, if proces has no parent, shell executes.
 *
 */
int32_t sys_halt(uint8_t status){
    cli();
    int i;
    pcb_t* pcb = get_PCB(); // gets current process's PCB from tss.esp0
    for (i = FD_MIN; i < FD_MAX; i++) {
        sys_close(i);
    }
    process_num[pcb->process_num] = 0;
    if(pcb->parent){
        parray[pcb->process_num] = pcb->parent->process_num;
    }
    //status is the 8 bit number, go to assembly and grab Parent from current process stack
    if (!(pcb->parent)) { // if its shell, no parent
        process_num[pcb->process_num] = 1;
        boot_execute(pcb->process_num);
        return 0;
    }
    // deallocate page memory for user vidmap, place it as available
    if(vidmem == 1){
        pte_user[(VIDEO_ADDRESS + ALIGN)/ALIGN] = READ_WRITE;
        pde[VIDMAP] = READ_WRITE;
        flushTLB();
        vidmem = 0;
    }
    pcb->parent->child = NULL;
    pde[VIRTUAL_ADDRESS / FOURMB] = (PHYSICAL_ADDRESS + (pcb->parent->process_num) * FOURMB) | PAGE_SIZE | READ_WRITE | PRESENT | USER;
    flushTLB();
    anythingRunning--;
    tss.ss0 = KERNEL_DS;
    tss.esp0 = PHYSICAL_ADDRESS - (EIGHTKB * (pcb->parent->process_num + 1)) - 4; // location of stack for a process, 4 is an address offset
    assembly_halt(status); // pass in status
    return 0;
}

/*
 * sys_execute
 *   DESCRIPTION: starts a process that is being called, command is a char array
 *   of words with first word as file name to be executed, and rest is for getargs syscall
 *   INPUTS: const uint8_t* command
 *   OUTPUTS: NONE
 *   RETURN VALUE: nothing on sucsess there should always be a process running, -1 on fail
 *   SIDE EFFECTS: starts a process, sets up pcb for that process, context switches to go back to user space
 */
int32_t sys_execute(const uint8_t* command){
    cli();
    int index = getProcessNum(); 
    if (index == -1) {
        printf("MAX SHELLS ACTIVE\n");
        return -1;
    }
    uint8_t temp[128];
    strcpy((int8_t*)temp, (int8_t*)command);
    anythingRunning++;
    int i;
    int x;
    int8_t file[BUFFER_SIZE];
    x = 0;
    i = 0;
    while(command[i] == ' ') {
        i++;
    }
    for (; i < strlen((int8_t *)command); i++) {
        if (x >= BUFFER_SIZE || command[i] == ' ') {
            break;
        }
        if (command[i] != '\0') {
            file[x++] = command[i];
        }
    }
    file[x] = '\0';
    dentry_t dentry;
    if(read_dentry_by_name((const int8_t*)file, &dentry) == -1){
        process_num[index] = 0;
        return -1;
    }

    /*Check if executable (4 is length of magic number)*/
    int8_t buf[4];
	read_data(dentry.inode, 0, buf, 4);
    if (!(buf[0] == 0x7F && buf[1] =='E' && buf[2] == 'L' && buf[3] == 'F')){ // executable check
        // printf("NOT EXECUTABLE");
        process_num[index] = 0;
        return -1;
    }

    /*Allocating memory*/
    pde[VIRTUAL_ADDRESS / FOURMB] = (PHYSICAL_ADDRESS + (index) * FOURMB) | PAGE_SIZE | READ_WRITE | PRESENT | USER;
    flushTLB();

    /*program image must be copied to the corret offset*/
    int8_t * mem = (int8_t *) VIRTUAL_ADDRESS + ADDR_OFFSET;
    // reads that from data block and puts it in array starting at mem
    read_data(dentry.inode, 0, mem, inodes[dentry.inode].length);

    uint8_t buf2[BUFFER_SIZE];
    //file_read(NULL,  buf2, 32);
    read_data(dentry.inode, 0, (int8_t *) buf2, BUFFER_SIZE);
    uint32_t entry;
    // setup PCB
    /*information is stored as a 4-byte unsigned integer in bytes 24-27 of the exeutable*/
    /*Grabbing the correct bytes and shifting them into the correct place*/
    entry = ((uint8_t) buf2[27] << 24) | ((uint8_t) buf2[26] << 16) | ((uint8_t) buf2[25] << 8) | (uint8_t) buf2[24];
    pcb_t* pcb;
    uint32_t * ptr = (uint32_t *) ((PHYSICAL_ADDRESS - (EIGHTKB * (index+1)) - 4) & 0xFFFFE000); //memory location of PCB
    pcb = (pcb_t*) ptr;
    parray[tnum] = index;
    if (index > 2) {
        pcb_t* parent = get_PCB();
        while (parent->terminal != tnum) {
            parent = parent->next;
        }
        pcb->parent = parent;
        parent->child = pcb;
    }
    x = 0;
    for (; i < strlen((int8_t *)temp); i++) {
        pcb->args[x++] = temp[i];
    }
    pcb->args[x++] = '\0';

    while (x < 128) {
        pcb->args[x++] = '\0';
    }
    pcb->next = pcb->parent->next;
    pcb->terminal = pcb->parent->terminal;
    pcb->process_num = index;
    pcb->fdt[0].flags = 1; //present,
    pcb->fdt[0].fotp = &stdin;
    pcb->fdt[1].flags = 1;
    pcb->fdt[1].fotp = &stdout;
    for (i = FD_MIN; i < FD_MAX; i++) {
        pcb->fdt[i].flags = 0;
    }
    tss.ss0 = KERNEL_DS;
    tss.esp0 = PHYSICAL_ADDRESS - (EIGHTKB * (index+1)) - 4; // location of stack for a process
    uint32_t ret;
    ret = context_switch(entry);
    return ret;
}

/*
 * sys_read
 *   DESCRIPTION: reads data from a procoss, process can be rc, terminal, or file type
 *   nbytes is the number of bytes to be read, fd is index into file descriptor table, buf
 *   is the buffer we want to fill
 *   INPUTS: int32_t fd, void* buf, int32_t nbytes
 *   OUTPUTS: NONE
 *   RETURN VALUE: number of bytes read, -1 if all fd blocks are being used
 *   SIDE EFFECTS: puts number of bytes read in ret_read
 */
int32_t sys_read(int32_t fd, void* buf, int32_t nbytes){
    int32_t ret_read;
    sti();
    pcb_t* pcb = get_PCB();
    if (fd < 0 || fd >= FD_MAX || !buf || pcb->fdt[fd].flags == 0) {
        return -1;
    }

    ret_read = pcb->fdt[fd].fotp->read(fd, (uint8_t *) buf, nbytes);

    return ret_read;
}

/*
 * sys_write
 *   DESCRIPTION: Writes data from a given process,  process can be rc, terminal, or file type
 *   nbytes is the number of bytes to be read, fd is index into file descriptor table, buf
 *   is the buffer we to write from
 *   INPUTS: int32_t fd, const void* buf, int32_t nbytes
 *   OUTPUTS: Writes to screen
 *   RETURN VALUE: how many bits are wrote or 0
 *   SIDE EFFECTS: Calls the correct write function given the fd
 */
int32_t sys_write (int32_t fd, const void* buf, int32_t nbytes){
    pcb_t* pcb = get_PCB();
    if (fd < 0 || fd >= FD_MAX || !buf || pcb->fdt[fd].flags == 0 ) {
        return -1;
    }
    return pcb->fdt[fd].fotp->write(fd, (uint8_t *) buf, nbytes);
}

/*
 * sys_open
 *   DESCRIPTION: opens process
 *   INPUTS: const uint8_t* filename
 *   OUTPUTS: NONE
 *   RETURN VALUE: fdt index on sucsess, -1 on fail
 *   SIDE EFFECTS: sets fdt values on process being opened
 */
int32_t sys_open(const uint8_t* filename){
    if(!filename){
        return -1;
    }
    dentry_t dentry;
    pcb_t* pcb;
    int x;
    if ((int32_t)read_dentry_by_name((int8_t *)filename, &dentry) == -1){
        return -1; // didn't find a file with that name
    }
    // Find PCB
    pcb = get_PCB(); // gets PCB block
    for (x = FD_MIN; x < FD_MAX; x++) {
        if (pcb->fdt[x].flags == 0) {
            break;  // file descriptor found, flag = 0 means available
        }
    }
    if (x == FD_MAX) {
        return -1; // all file descriptors are being used
    }
    if (dentry.file_type == RTC_TYPE) {
        pcb->fdt[x].fotp = &rtc_fot; // if file type is rtc
    } else if (dentry.file_type == DIR_TYPE) {
        pcb->fdt[x].fotp = &dir_fot; // if file type is directory
    }else if (dentry.file_type == FILE_TYPE) {
        pcb->fdt[x].fotp = &file_fot; // if file typr
    }
    pcb->fdt[x].inode = dentry.inode;
    pcb->fdt[x].flags = 1; //allocating file descriptor at x
    pcb->fdt[x].file_position = 0;
    pcb->fdt[x].fotp->open((uint8_t*)filename);
    return x;
}

/*
 * sys_close
 *   DESCRIPTION: closes process
 *   INPUTS: file descriptor index
 *   OUTPUTS: NONE
 *   RETURN VALUE: 0 on sucsess, -1 on fail
 *   SIDE EFFECTS: resets all entries in fdt for that file position
 */
int32_t sys_close(int32_t fd){
    if (fd < FD_MIN || fd >= FD_MAX ) {
        return -1;
    }
    pcb_t* pcb = get_PCB();
    if (pcb->fdt[fd].flags == 0) {
        return -1; //already closed
    }
    // closes process by setting pcb entries to 0
    pcb->fdt[fd].fotp->close(fd);
    pcb->fdt[fd].fotp = NULL;
    pcb->fdt[fd].inode = 0;
    pcb->fdt[fd].flags = 0;
    pcb->fdt[fd].file_position = 0;
    return 0;
}

/*Scheduling code that didn work*/
// /
// pcb_t* get_PCBs(int processnum){
//     return (pcb_t*)(uint32_t *) ((PHYSICAL_ADDRESS - (EIGHTKB * (processnum+1)) - 4) & 0xFFFFE000); 
// }

/*
 * sys_getargs
 *   DESCRIPTION: parses the buffer entered in the terminal and gets the arguments passed in
 *   to a process(eg. cat). Gets terminal buffer from current processes PCB, gets args, puts
 *   args to into buffer passed in as input which is terminated with a null character
 *   INPUTS: uint8_t* buf, buffer for arguments for a process, int32_t nbytes, max num
 *   bytes allowed
 *   OUTPUTS: buffer input is filled with args
 *   RETURN VALUE: 0 on success, -1 on fail
 *   SIDE EFFECTS: fills buffer input
 */

int32_t sys_getargs(uint8_t* buf, int32_t nbytes){
    if( buf == NULL || nbytes < 0){
        return -1;
    }
    uint8_t temp[128];
    strcpy((int8_t*)buf, (int8_t*)temp);
    uint8_t* command;
    int i;
    int x;
    pcb_t* pcb = get_PCB();
    x = 0;
    i = 0;
    command = pcb->args;
    // skip process name and spaces
    while (command[i] != ' '){
        i++;
    }
    while (command[i] == ' '){
        i++;
    }
    // gets the arguments user passed into pcb->args and puts into buf
    for (; i < strlen((int8_t *)command); i++){
        if (x >= nbytes) {
            return -1;
        }
        if (command[i] != '\0') {
            buf[x++] = command[i];
        } else {
            break;
        }
    }
    if (x > 0) {
        buf[x++] = '\0'; // terminates string with null
        return 0;
    } else {
        return -1;
    }
}

/*
 * sys_vidmap
 *   DESCRIPTION: Allows user space to access the physical video memory through virtual
 *   memory. A new virtual page is allocated for the user space of 4KB, screen_start
 *   has all the neccessary offsets for vidmap physical memory. Flush TLB since paging structure 
 *   has been changed
 *   INPUTS: uint8_t** screen_start, address of the address for the offsets
 *   OUTPUTS: fills screen_start with paging offsets for pde, pte, and physical
 *   RETURN VALUE: 0 on success, -1 on fail
 *   SIDE EFFECTS: fills screen_start
 */
int32_t sys_vidmap(uint8_t** screen_start){
    if(screen_start == NULL || (*screen_start < (uint8_t *) VIRTUAL_ADDRESS && *screen_start >= (uint8_t *) VIRTUAL_ADDRESS + FOURMB) 
        || (screen_start >= (uint8_t **) FOURMB && screen_start < (uint8_t **) PHYSICAL_ADDRESS)){
        return -1;
    }

    // Map the Video Memory to the correct entry of the Page Table
    pte_user[(VIDEO_ADDRESS + ALIGN)/ALIGN] = (VIDEO_ADDRESS) | READ_WRITE | PRESENT | USER;

    // // Map the Page Table to the first entry of the Page Directory, pte is base address of page table
    // // with offset from *screen_start
    pde[VIDMAP] = (int) pte_user | READ_WRITE | PRESENT | USER;

    flushTLB();
    vidmem = 1;
    // rightmost bits is for pde offset, middle bits are for pte offset, leftmost offset is for physical memory offset
    *screen_start = (uint8_t*) ((VIDMAP << OFFSET22) | ((VIDEO_ADDRESS+ALIGN)/ALIGN << OFFSET12) | ZEROMB);
    
    return 0;
}

/*
 * sys_set_handler
 *   DESCRIPTION: Placeholder
 *   INPUTS: int32_t signum, void* handler_address
 *   OUTPUTS: 
 *   RETURN VALUE: returns -1
 *   SIDE EFFECTS: 
 */
int32_t sys_set_handler (int32_t signum, void* handler_address){
    return -1;
}

/*
 * sys_sigreturn
 *   DESCRIPTION: Placeholder
 *   INPUTS: 
 *   OUTPUTS: 
 *   RETURN VALUE: returns -1
 *   SIDE EFFECTS: 
 */
int32_t sys_sigreturn(){
    return -1;
}
