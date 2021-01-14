#ifndef IDT_H
#define IDT_H

/* Initialize the IDT */
void idt_init();

/*
 * pit_handle
 *   DESCRIPTION: pit_handle
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE:
 *   SIDE EFFECTS: handles pit interrupts
 */
extern void pit_handle();

/*
 * keyboard_handle
 *   DESCRIPTION: keyboard_handle
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE:
 *   SIDE EFFECTS: handles keyboard
 */
extern void keyboard_handle();


/*
 * rtc_handle
 *   DESCRIPTION: rtc handler
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE:
 *   SIDE EFFECTS: handles rtc
 */
extern void rtc_handle();

/*
 * get_cr2
 *   DESCRIPTION: gets the location of a page fault
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE:
 *   SIDE EFFECTS: reads the cr2 register
 */
extern int get_cr2();

#endif /* IDT_H */
