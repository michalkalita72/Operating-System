#include "keyboard.h"
#include "i8259.h"
#include "lib.h"
#include "rtc.h"
#include "paging.h"
#include "system_handler.h"

#define KEYBOARD_IRQ    1
#define RTC_IRQ         8
#define DATA_PORT       0x60
#define COMMAND_PORT    0x64

/*Flags*/
int CapL = 0;
int shift = 0;
int ctrl = 0;
int alt = 0;

/*Terminal arrays*/
unsigned char tbuf[TMAX][BUFSIZE];
volatile unsigned int tindex[TMAX] = {0,0,0};
int tcurserx[TMAX] = {0,0,0};
int tcursery[TMAX] = {0,0,0};

/*Keeps track of current terminal*/
volatile int tnum = 0; // 0-2

volatile int enter[TMAX] = {0,0,0};

/*Regular no caps or shift*/
static unsigned char characters[BUFSIZE] = { 
0, 0, '1', '2', 
'3', '4', '5', '6', 
'7', '8', '9', '0', 
'-', '=', '\b', 0,                 /*Backspace 0x0E,    Tab 0x0F*/
'q', 'w', 'e', 'r', 
't', 'y', 'u', 'i',
'o', 'p', '[', ']', 
'\n', 0, 'a', 's',              /*Left control 0x1D*/
'd', 'f', 'g', 'h', 
'j', 'k', 'l', ';', 
'\'', '`', 0, '\\',             /*Left shift 0x2A*/
'z', 'x', 'c', 'v', 
'b', 'n', 'm', ',', 
'.', '/', 0, '*',               /*Right shift 0x36*/
0, ' ', 0, 0, 0, 0,             /*Left alt 0x38, Caps lock 0x3A, start of F keys 0x3B*/
0, 0, 0, 0, 0, 0, 0,
0, 0, '7', '8', '9',
'-', '4', '5', '6',
'+', '1', '2', '3',
'0', '.', 0, 0, 0,
0, 0
};                              /*0xE0, 0x1D	right control pressed*/

/*Array for just shift press*/
static unsigned char characters2[BUFSIZE] = { 
0, 0, '!', '@', 
'#', '$', '%', '^', 
'&', '*', '(', ')', 
'_', '+', '\b', 0,                 /*Backspace 0x0E,    Tab 0x0F*/
'Q', 'W', 'E', 'R', 
'T', 'Y', 'U', 'I',
'O', 'P', '{', '}', 
'\n', 0, 'A', 'S',              /*Left control 0x1D*/
'D', 'F', 'G', 'H', 
'J', 'K', 'L', ':', 
'"', '~', 0, '|',               /*Left shift 0x2A*/
'Z', 'X', 'C', 'V', 
'B', 'N', 'M', '<', 
'>', '?', 0, '*',               /*Right shift 0x36*/
0, ' ', 0, 0, 0, 0,             /*Left alt 0x38, Caps lock 0x3A, start of F keys 0x3B*/
0, 0, 0, 0, 0, 0, 0,
0, 0, '7', '8', '9',
'-', '4', '5', '6',
'+', '1', '2', '3',
'0', '.', 0, 0, 0,
0, 0
};  

/*Capslock only*/
static unsigned char characters3[BUFSIZE] = { 
0, 0, '1', '2', 
'3', '4', '5', '6', 
'7', '8', '9', '0', 
'-', '=', '\b', 0,                 /*Backspace 0x0E,    Tab 0x0F*/
'Q', 'W', 'E', 'R', 
'T', 'Y', 'U', 'I',
'O', 'P', '[', ']', 
'\n', 0, 'A', 'S',              /*Left control 0x1D*/
'D', 'F', 'G', 'H', 
'J', 'K', 'L', ';', 
'\'', '`', 0, '\\',             /*Left shift 0x2A*/
'Z', 'X', 'C', 'V', 
'B', 'N', 'M', ',', 
'.', '/', 0, '*',               /*Right shift 0x36*/
0, ' ', 0, 0, 0, 0,             /*Left alt 0x38, Caps lock 0x3A, start of F keys 0x3B*/
0, 0, 0, 0, 0, 0, 0,
0, 0, '7', '8', '9',
'-', '4', '5', '6',
'+', '1', '2', '3',
'0', '.', 0, 0, 0,
0, 0
};  

/*
 * get_tnum
 *   DESCRIPTION:Get terminal number
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: tnum
 *   SIDE EFFECTS: None 
 */   

int32_t get_tnum(){
    return tnum;
}

/*
 * terminal_open
 *   DESCRIPTION:Does nothing
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: None 
 */    
int32_t terminal_open(const uint8_t* filename){
    return 0;
}

/*
 * terminal_close
 *   DESCRIPTION:Does nothing
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: 0
 *   SIDE EFFECTS: None 
 */    
int32_t terminal_close(int32_t fd){
    return 0;
}

/*
 * terminal_write
 *   DESCRIPTION:Takes in a string and prints it onto the screnn
 *   INPUTS: int32_t fd -> nothing, const void* buf - > char buffer, int32_t nbytes -> number of bytes need to write
 *   OUTPUTS: Prints whatever string is passed in
 *   RETURN VALUE: 0 if success -1 if failed
 *   SIDE EFFECTS: None 
 */    
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes){
    if(buf == NULL || nbytes < 0){
        return -1;
    }    
    int i;
    cli();
    for(i = 0; i < nbytes ; i++ ){
        putc(((char *)buf) [i]);  
    }
    sti();
    return 0;
}
/*
 * terminal_read
 *   DESCRIPTION:Takes in a char buf and loads it with whats in the keybuffer
 *   INPUTS: int32_t fd -> nothing, const void* buf - > char buffer, int32_t nbytes -> number of bytes need to read
 *   OUTPUTS: stores whatever string is passed in to the buf
 *   RETURN VALUE: 0 if success -1 if failed and nbytes depending on how many char where copied
 *   SIDE EFFECTS: None 
 */   
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){
    if(buf == NULL || nbytes < 0){
        return -1;
    }
    /*Max Buf Size*/
    if(nbytes > BUFSIZE){
        nbytes = BUFSIZE;
    }
    int i;
    int pt = 0;
    while (tindex[tnum] < nbytes){
        if(enter[tnum] == 1){
            enter[tnum] = 0; 
            break;
        }
    }
    for(i = 0; i < nbytes; i++){
        if(tbuf[tnum][i] != 0 && tbuf[tnum][i] != '\n'){
            pt++;
        }
        if(tbuf[tnum][i] == '\n'){
            pt++;
            break;
        }
    }
    for(i = 0; i <= pt; i++){
        if (tbuf[tnum][i] == '\t') {
            ((char*)buf)[i] = ' ';
            continue;
        }
        ((char*)buf)[i] = tbuf[tnum][i];
    }
    // if(keybuf[pt] != '\n'){
    //     printf("Set\n");
    //     ((char*)buf)[pt-1] = '\n';
    // }
    ((char*)buf)[nbytes-1] = '\n';
    for(i = 0; i < sizeof(tbuf[tnum]); i++){
        tbuf[tnum][i] = NULL;
    }
    tindex[tnum] = 0;
    return pt;
}

/*
 * keyboard_init
 *   DESCRIPTION:inits the keyboard
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: NONE
 *   SIDE EFFECTS: enables the keyboard IRQ 
 */ 
void keyboard_init() {
    enable_irq(KEYBOARD_IRQ);
    return;
}
/*
 * keyboard_handler
 *   DESCRIPTION:Handles keyboard presses echoing and putting it on the keybuffer as long as keybuffer has less than 128 chars in it.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Handles keyboard presses echoing and putting it on the keybuffer as long as keybuffer has less than 128 chars in it. 
 */   
void keyboard_handler() {
    int idx, tab;
    idx = inb(DATA_PORT); //reads from data port

    /*Switching Termianl index*/
    /*F1*/
    if(idx == F1_PRESSED && alt){
        /*Updates the current screen with the assocaited terminal memory*/
        // memcpy((uint8_t*) (VIDEO_ADDRESS + ALIGN * ((tnum + 1)%3)), (uint8_t*) VIDEO_ADDRESS , ALIGN); 
        // tcurserx[tnum] = getx(); 
        // tcursery[tnum] = gety();
        //tnum = 0;
        send_eoi(KEYBOARD_IRQ); 
        tswitch(0); // pass in number of terminal to switch to
        return;
    /*F2*/
    } else if (idx == F2_PRESSED && alt){
        /*Updates the current screen with the assocaited terminal memory*/
        // memcpy((uint8_t*) (VIDEO_ADDRESS + ALIGN * ((tnum + 1)%3)),(uint8_t*) VIDEO_ADDRESS , ALIGN);
        // tcurserx[tnum] = getx(); 
        // tcursery[tnum] = gety();        
        // tnum = 1;
        send_eoi(KEYBOARD_IRQ); 
        tswitch(1); // pass in number of terminal to switch to
        return;
    /*F3*/    
    } else if (idx == F3_PRESSED && alt){
        /*Updates the current screen with the assocaited terminal memory*/
        // memcpy((uint8_t*) (VIDEO_ADDRESS + ALIGN * ((tnum + 1)%3)), (uint8_t*) VIDEO_ADDRESS , ALIGN);              
        // tcurserx[tnum] = getx(); 
        // tcursery[tnum] = gety();
        // tnum = 2;
        send_eoi(KEYBOARD_IRQ);
        tswitch(2); // pass in number of terminal to switch to
        return;
    }

    /* Check for backspace */
    if (idx == 0x0E) {      
        if (tindex[tnum] > 0) {
            /*Tab specfic backspace removing the 4 spaces*/
            if (tbuf[tnum][tindex[tnum]-1] == '\t') {
                for(tab = 0; tab < 4; tab ++){
                    if (tindex[tnum] > 0) {
                        if(tbuf[tnum][tindex[tnum]-1] == '\t'){
                            tbuf[tnum][tindex[tnum]-1]= NULL;
                            putc('\b');
                            tindex[tnum]--;
                        }
                    }
                }
            } else {
                /*Normal backspace with regular characters*/
                tbuf[tnum][tindex[tnum]-1] = NULL;
                putc('\b');
                tindex[tnum]--;
            }
            send_eoi(KEYBOARD_IRQ);
            return;
        } else if (tindex[tnum] == 0){
            send_eoi(KEYBOARD_IRQ);
            return;
        }  
    }
    
    /*Wait for \n once array is indexed to 126*/
    if(tindex[tnum] > 126){
        /*Max index for keybuffer*/
        if(characters[idx] == '\n' && tindex[tnum] == 127){
            tbuf[tnum][tindex[tnum]] = characters[idx];
            putc('\n');
            enter[tnum] = 1;
        }
        send_eoi(KEYBOARD_IRQ);
        return;
    }

    /*If L and ctrl are pressed*/
    if(idx == 0x26 && ctrl){
        clear();
        send_eoi(KEYBOARD_IRQ);
        return;
    }

    switch(idx){
        /*Left and Right shift press and depress (sets the shift)*/
        case 0x2A:       shift = 1;
                         send_eoi(KEYBOARD_IRQ);
                         return;
        case 0x36:       shift = 1;
                         send_eoi(KEYBOARD_IRQ);
                         return;         
        case 0xAA:       shift = 0;
                         send_eoi(KEYBOARD_IRQ);
                         return;        
        case 0xB6:       shift = 0;
                         send_eoi(KEYBOARD_IRQ);
                         return;

        /*Left and Right Ctrl press and depress*/
        case 0x1D:       ctrl = 1;
                         send_eoi(KEYBOARD_IRQ);
                         return;        
        case 0x9D:       ctrl = 0;
                         send_eoi(KEYBOARD_IRQ);
                         return;

        /*Left and Right Alt press and depress*/
        case 0x38:       alt = 1;
                         send_eoi(KEYBOARD_IRQ);
                         return;        
        case 0xB8:       alt = 0;
                         send_eoi(KEYBOARD_IRQ);
                         return;

    // /*Sets/Clears caps lock if pressed depending on previous caps state*/
        case 0x3A:      if(CapL == 0)
                            CapL = 1;
                        else
                            CapL = 0;
                         send_eoi(KEYBOARD_IRQ);
                         return;                          
    // /*Returns so it doesnt affect the buffers*/
    //     send_eoi(KEYBOARD_IRQ);
    //     return;
    }

    /*Tab stuff*/
    if(idx == KEYBOARD_TAB){
        /*Tab has 4 spaces and only prints them as ln=ong as index is less than 127*/
        for(tab = 0 ; tab < 4; tab ++){
            if(tindex[tnum] < 127){
                tbuf[tnum][tindex[tnum]] = '\t';
                tindex[tnum]++;
                putc(' ');
            }
        }
        send_eoi(KEYBOARD_IRQ);
        return;
    }

    if(idx < F3_PRESSED){ //ignore depress and things after the characters
        /*Checks if only caps or shift are pressed*/
        if(CapL ^ shift){
            if(characters2[idx] != 0 && shift){
                /*Outputs shift only char array*/
                tbuf[tnum][tindex[tnum]] = characters2[idx];
                putc(tbuf[tnum][tindex[tnum]]);
                tindex[tnum]++;
            }
            else if (characters2[idx] != 0 && CapL){
                /*Outputs caps only char array*/
                tbuf[tnum][tindex[tnum]] = characters3[idx];
                putc(tbuf[tnum][tindex[tnum]]);
                tindex[tnum]++;
            }
        }else{
            if( characters[idx] != 0){
                if(CapL && shift){
                    /*If statement for checking the ascii character position*/
                    if(characters[idx]<=122 && characters[idx] >= 97){
                        /*Out puts regular char when shift and caps are pressed*/
                        tbuf[tnum][tindex[tnum]] = characters[idx];
                        putc(tbuf[tnum][tindex[tnum]]);
                        tindex[tnum]++;
                    }
                    else{
                        if( characters[idx] != 0){
                            /*outputs the correct char when shift is pressed*/
                            tbuf[tnum][tindex[tnum]] = characters2[idx];
                            putc(tbuf[tnum][tindex[tnum]]);
                            tindex[tnum]++;
                        }                
                    }
                }
                else{
                    /*Out puts regular lowercase array*/
                    tbuf[tnum][tindex[tnum]] = characters[idx];
                    putc(tbuf[tnum][tindex[tnum]]);
                    tindex[tnum]++;
                }
            }
        }
    }
    if(tbuf[tnum][tindex[tnum]-1] == '\n'){
        enter[tnum] = 1;
    }   
    send_eoi(KEYBOARD_IRQ); 
    return;
}
