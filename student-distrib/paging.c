#include "paging.h"

// Global PDE and PDE arrays
int pde[ENTRIES] __attribute__((aligned (ALIGN)));
int pte[ENTRIES] __attribute__((aligned (ALIGN)));
int pte_user[ENTRIES] __attribute__((aligned (ALIGN)));
/*
 * paging_init
 *   DESCRIPTION: Initializes paging for use in the terminal.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Fills the PDE and PTE and sets the video memory and kernel entries
 */
void paging_init() {
    int i;
    
    // Fill in the PDE first (4 MB Entries)
    for (i = 0; i < ENTRIES; i++) {
        pde[i] = READ_WRITE;
    }

    // Map the entries of the PTE (4 KB Entries)
    for (i = 0; i < ENTRIES; i++) {
        // pte[i] = (i * ALIGN) | READ_WRITE | PRESENT;
        pte[i] = (i * ALIGN) | READ_WRITE;
    }

    // Map the Video Memory to the correct entry of the Page Table
    pte[VIDEO_ADDRESS/ALIGN] = VIDEO_ADDRESS | READ_WRITE | PRESENT;

    /*Paging for the terminal buffers*/
    pte[(VIDEO_ADDRESS + ALIGN)/ALIGN] = (VIDEO_ADDRESS + ALIGN) | READ_WRITE| PRESENT;
    pte[(VIDEO_ADDRESS + 2*ALIGN)/ALIGN] = (VIDEO_ADDRESS + 2*ALIGN) | READ_WRITE | PRESENT;
    pte[(VIDEO_ADDRESS + 3*ALIGN)/ALIGN] = (VIDEO_ADDRESS + 3*ALIGN) | READ_WRITE | PRESENT;

    // Map the Page Table to the first entry of the Page Directory
    pde[0] = (int) pte | READ_WRITE | PRESENT;

    // Map the KERNEL_ADDR to virtual memory
    pde[1] = KERNEL_ADDRESS | GLOBAL | PAGE_SIZE | READ_WRITE | PRESENT;

    // Call the assembly functions
    loadPageDirectory(pde);
    enablePaging();
}
