#include "rtc.h"
#include "i8259.h"
#include "lib.h"

#define RTC_IRQ     8
#define RTC_IDX     0x70
#define RTC_CMOS    0x71
#define RTC_REG_A   0x8A
#define RTC_REG_B   0x8B
#define RTC_REG_C   0x0C
#define RATE        15
#define RTC_MAX_FREQ 32768
#define RTC_MAX     1024
#define RTC_MIN     0
#define RTC_MIN_RATE 5
#define RTC_MAX_RATE 15


static int handled = 0; //for read, checks if int has been handled

/* Note: 0xF0 is a bit mask */

/*
 * rtc_init
 *   DESCRIPTION: Initializes the system RTC.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Enables the RTC and sets the frequency
 */
void rtc_init(){
    char prev;
    uint32_t flags;

    // Save the flags
    cli_and_save(flags);

    enable_irq(RTC_IRQ);

    outb(RTC_REG_B, RTC_IDX);	    	    // select register B, and disable NMI
    prev = inb(RTC_CMOS);       	        // read the current value of register B
    outb(RTC_REG_B, RTC_IDX);	    	    // set the index again (a read will reset the index to register D)
    outb(prev | 0x40, RTC_CMOS);	        // write the previous value ORed with 0x40. This turns on bit 6 of register B

    outb(RTC_REG_A, RTC_IDX);	    	    // set index to register A, disable NMI
    prev = inb(RTC_CMOS);       	        // get initial value of register A
    outb(RTC_REG_A, RTC_IDX);	    	    // reset index to A
    outb((prev & 0xF0) | RATE, RTC_CMOS);	// write only our rate to A. Note, rate is the bottom 4 bits and previous is the upper 4 bits

    // Restore the flags
    restore_flags(flags);
    return;
}

/*
 * rtc_handler
 *   DESCRIPTION: Handles the RTC interrupt generated.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Calls the test_interrupts function, which modifies video memory
 */
void rtc_handler(){

    //test_interrupts();
    // putc('1');
    // Read the value of the C register
    outb(RTC_REG_C, RTC_IDX);
    inb(RTC_CMOS);

    send_eoi(RTC_IRQ);

    handled = 1; //exception is handled 
    //changeHandled(1);
}


/*
 * rtc_write
 *   DESCRIPTION: Changed rtc frequency.
 *   INPUTS: int32_t fd, const void* buf, int32_t nbytes
 *   OUTPUTS: 0 for sucsess, -1 for fail
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes rtc frequency
 */
int rtc_write (int32_t fd, const void* buf, int32_t nbytes){
  int rate;
  int hertz;
  char prev;
  uint32_t flags;
  int * getInt;
  getInt = (int *)buf; //turns buffer into int *

if(nbytes != 4 || buf == NULL){
  return -1;
}
hertz = *getInt;

  if (hertz < RTC_MIN || hertz > RTC_MAX){ //makes sure hertz is in right region
    return -1;
  }
  for(rate = RTC_MIN_RATE; rate <=(RTC_MAX_RATE +1); rate++){ //gets rate num for RTC
    if(rate==RTC_MAX_RATE +1){
      return -1;
    }
    if(hertz == (RTC_MAX_FREQ >> (rate-1))){ //chekcs which "rate" number the hertz paramter is represneted by
      break;
    }
  }


  // Save the flags
  cli_and_save(flags);


  outb(RTC_REG_A, RTC_IDX);	    	    // set index to register A, disable NMI
  prev = inb(RTC_CMOS);       	        // get initial value of register A
  outb(RTC_REG_A, RTC_IDX);	    	    // reset index to A
  outb((prev & 0xF0) | rate, RTC_CMOS);	// write only our rate to A. Note, rate is the bottom 4 bits and previous is the upper 4 bits


  restore_flags(flags);

  return 0;
  }

  /*
   * rtc_open
   *   DESCRIPTION: Changed rtc frequency.
   *   INPUTS: int32_t fd, const void* buf, int32_t nbytes
   *   OUTPUTS: 0 for sucsess, -1 for fail
   *   RETURN VALUE: none
   *   SIDE EFFECTS: changes rtc frequency to 2 hertz
   */
int rtc_open (const uint8_t* filename){
  char prev;
  uint32_t flags;
    cli_and_save(flags);


    outb(RTC_REG_A, RTC_IDX);	    	    // set index to register A, disable NMI
    prev = inb(RTC_CMOS);       	        // get initial value of register A
    outb(RTC_REG_A, RTC_IDX);	    	    // reset index to A
    outb((prev & 0xF0) | RATE, RTC_CMOS);	// write only our rate to A. Note, rate is the bottom 4 bits and previous is the upper 4 bits


    restore_flags(flags);
    return 0;

  }

/*
   * rtc_read
   *   DESCRIPTION: Waits for inturupt to come in
   *   INPUTS: int32_t fd, const void* buf, int32_t nbytes
   *   OUTPUTS: 0 for sucsess
   *   RETURN VALUE: none
   *   SIDE EFFECTS: hangs untill rtc interupt is handled
   */
int rtc_read (int32_t fd, void* buf, int32_t nbytes){
  int test = handled;
  while(test == 0){ //waits while no intuprt has been handled
    //putc('w');
  test = handled;
  }
  handled = 0; //resets handled variable 
  return 0;
}

/*
   * rtc_close
   *   DESCRIPTION: Returns 0
   *   INPUTS: int32_t
   *   OUTPUTS: 0 for sucsess
   *   RETURN VALUE: none
   *   SIDE EFFECTS: 0 is returned 
   */
int rtc_close (int32_t fd){
  return 0;
}
