#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"

extern int32_t terminal_read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t terminal_write (int32_t fd, const void* buf, int32_t nbytes);
extern int32_t terminal_open (const uint8_t* filename);
extern int32_t terminal_close (int32_t fd);

extern int tcurserx[3];
extern int tcursery[3];
extern volatile int tnum;

/* Initialize the KEYBOARD */
void keyboard_init();

/* Handle a keypress */
void keyboard_handler();

# define KEYBOARD_TAB 0x0F
# define F1_PRESSED 0x3B
# define F2_PRESSED 0x3C
# define F3_PRESSED 0x3D

#define BUFSIZE     128
#define TMAX        3
#endif /* KEYBOARD_H */
