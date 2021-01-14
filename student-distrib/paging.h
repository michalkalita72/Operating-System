#ifndef _PAGING_H
#define _PAGING_H

// Constants used in the paging initialization
#define KERNEL_ADDRESS      0x00400000
#define VIDEO_ADDRESS       0x000B8000
#define ENTRIES             1024
#define ALIGN               4096
#define GLOBAL              0x00000100
#define PAGE_SIZE           0x00000080
#define DIRTY               0x00000040              
#define ACCESSED            0x00000020
#define CACHE               0x00000010
#define WRITE_THROUGH       0x00000008
#define USER                0x00000004
#define READ_WRITE          0x00000002
#define PRESENT             0x00000001

/* Initialize paging */
void paging_init();

/* Functions found in paging.S */
/*Loads Page Directy in CR3 with the given pde address*/
/*
 * loadPageDirectory
 *   DESCRIPTION:Loads page directory
 *   INPUTS: pde address
 *   OUTPUTS: NONE
 *   RETURN VALUE: NONE
 *   SIDE EFFECTS: Loads page directory 
 */
extern void loadPageDirectory(int*);

/*
 * enablePaging
 *   DESCRIPTION: Enables paging
 *   INPUTS: NONE
 *   OUTPUTS: NONE
 *   RETURN VALUE: NONE
 *   SIDE EFFECTS: enables paging 
 */
extern void enablePaging();

/* Arrays holding the PDE and PTE */
extern int pde[ENTRIES] __attribute__((aligned (ALIGN)));
extern int pte[ENTRIES] __attribute__((aligned (ALIGN)));
extern int pte_user[ENTRIES] __attribute__((aligned (ALIGN)));

#endif /* _PAGING_H */
