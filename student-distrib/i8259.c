/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/*
 * i8259_init
 *   DESCRIPTION: Initializes the Master and Slave PIC
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Sends 4 control words to both PICs
 */
void i8259_init(void) {
    uint32_t flags;
    master_mask = 0xFF;
    slave_mask = 0xFF;

    // Save the flags
    cli_and_save(flags);

    // Mask the master and slave
    outb(master_mask, MASTER_8259_DATA);
    outb(slave_mask, SLAVE_8259_DATA);

    // ICW1-4 for the Master
    outb(ICW1, MASTER_8259_PORT);
    outb(ICW2_MASTER, MASTER_8259_DATA);
    outb(ICW3_MASTER, MASTER_8259_DATA);
    outb(ICW4, MASTER_8259_DATA);

    // ICW1-4 for the Slave
    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_8259_DATA);
    outb(ICW3_SLAVE, SLAVE_8259_DATA);
    outb(ICW4, SLAVE_8259_DATA);
    
    // Unmask the master and slave
    outb(master_mask, MASTER_8259_DATA);
    outb(slave_mask, SLAVE_8259_DATA);
    
    // Restore the flags
    restore_flags(flags);

    // Enables slave port
    enable_irq(SLAVE_IRQ); 
}

/*
 * enable_irq
 *   DESCRIPTION: Enables the specified IRQ
 *   INPUTS: irq_num -- the IRQ number to be enabled
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Unmasks the IRQ and sends data to the correct PIC
 */
void enable_irq(uint32_t irq_num) {
    uint8_t mask;
    
    // IRQ0-7 is Master and IRQ8-15 is Slave
    if(0 <= irq_num && irq_num < 8) {
        mask = ~(1 << irq_num);
        master_mask &= mask;
        outb(master_mask, MASTER_8259_DATA);
    } else if (8 <= irq_num && irq_num < 16) {
        mask = ~(1 << (irq_num-8));
        slave_mask &= mask;
        outb(slave_mask, SLAVE_8259_DATA);
    }
}

/*
 * disable_irq
 *   DESCRIPTION: Disables the specified IRQ
 *   INPUTS: irq_num -- the IRQ number to be disabled
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Masks the IRQ and sends data to the correct PIC
 */
void disable_irq(uint32_t irq_num) {
    unsigned int mask;
    
    // IRQ0-7 is Master and IRQ8-15 is Slave
    if(0 <= irq_num && irq_num < 8) {
        mask = (1 << irq_num);
        master_mask |= mask; 
        outb(master_mask, MASTER_8259_DATA);
    } else if (8 <= irq_num && irq_num < 16) {
        mask = (1 << (irq_num-8));
        slave_mask |= mask;
        outb(slave_mask, SLAVE_8259_DATA);
    }
}

/*
 * send_eoi
 *   DESCRIPTION: Sends the EIO signal for the specified IRQ
 *   INPUTS: irq_num -- the IRQ number that is sending the EOI
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Sends the EOI signal to the PIC(s)
 */
void send_eoi(uint32_t irq_num) {
    unsigned int eoi;

    // 0x20 is EOI | irq_num;
    eoi =  0x20;

    // IRQ0-7 is Master, IRQ8-15 is Slave
    if(irq_num < 8) {
        outb(eoi, MASTER_8259_PORT);
    } else {
        outb(eoi, SLAVE_8259_PORT);
        outb(eoi, MASTER_8259_PORT);
    }
}
