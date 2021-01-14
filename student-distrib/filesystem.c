#include "filesystem.h"
#include "lib.h"
#include "system_handler.h"

/*
 * file_init
 *   DESCRIPTION: initilizes file system with propper address
 *   INPUTS:
 *   OUTPUTS:
 *   RETURN VALUE:
 *   SIDE EFFECTS: changes all the memory address to proper location
 */
void file_init() {
    // printf("Init \n");
    boot_block = (boot_t *)FILESYS_ADDR;
    inodes = (inode_t *) (FILESYS_ADDR+INODE_OFFSET);
    blocks = (block_t *) (FILESYS_ADDR + ((boot_block->num_inodes + 1) * INODE_OFFSET));
    // printf ("Init end \n");
}

// file functions

/*
 * file_write
 *   DESCRIPTION: does nothing
 *   INPUTS: int32_t fd, const void* buf, int32_t nbytes
 *   OUTPUTS: -1
 *   RETURN VALUE:
 *   SIDE EFFECTS: returns -1
 */
int32_t file_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}


/*
 * file_open
 *   DESCRIPTION: opens file to be read
 *   INPUTS: const int8_t* filename
 *   OUTPUTS:
 *   RETURN VALUE: 0
 *   SIDE EFFECTS:
 */
int32_t file_open(const uint8_t* filename) {
    return 0;
}

/*
 * file_read
 *   DESCRIPTION: gets dated from open file and puts in buffer
 *   INPUTS: int32_t fd, void* buf, int32_t nbytes
 *   OUTPUTS: buf filled with proper data
 *   RETURN VALUE: -1 if fail, num of bits written on sucsess
 *   SIDE EFFECTS:
 */
int32_t file_read(int32_t fd, void* buf, int32_t nbytes) {
    if(buf == NULL || nbytes < 0){
        return -1;
    }
    pcb_t* pcb = get_PCB();
    if (pcb->fdt[fd].flags == 0) {
        return -1;
    }
    int32_t bytes = read_data(pcb->fdt[fd].inode, pcb->fdt[fd].file_position, (int8_t *)buf, nbytes); //offset to be changed later maybe
    pcb->fdt[fd].file_position += bytes;
    return bytes;
}


/*
 * file_close
 *   DESCRIPTION: returns 0
 *   INPUTS: int32_t fd
 *   OUTPUTS:
 *   RETURN VALUE: 0
 *   SIDE EFFECTS:
 */
int32_t file_close(int32_t fd) {
    return 0;
}

// directory functions

/*
 * dir_write
 *   DESCRIPTION: does nothing
 *   INPUTS: int32_t fd, const void* buf, int32_t nbytes
 *   OUTPUTS: Nothing
 *   RETURN VALUE: -1
 *   SIDE EFFECTS: returns -1
 */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}
/*
 * dir_open
 *   DESCRIPTION: opens directory
 *   INPUTS: const int8_t* filename
 *   OUTPUTS: Nothing
 *   RETURN VALUE: 0 for success, -1 for failure
 *   SIDE EFFECTS: returns -1 or 0
 */

int32_t dir_open(const uint8_t* filename) {
    return 0;
}

/*
 * dir_read
 *   DESCRIPTION: reads the name of each file
 *   INPUTS: int32_t fd, void* buf, int32_t nbytes
 *   OUTPUTS: Name of file, one for each function call
 *   RETURN VALUE: 0 for success, -1 for failure
 *   SIDE EFFECTS: returns -1 or 0
 */

int32_t dir_read(int32_t fd, void* buf, int32_t nbytes) {
    if(buf == NULL || nbytes < 0){
        return -1;
    }
    pcb_t* pcb = get_PCB();
    if (pcb->fdt[fd].flags == 0) {
        return -1;
    }
    dentry_t dentry;
    if(read_dentry_by_index(pcb->fdt[fd].file_position, &dentry) == -1){
        return -1;
    }
    pcb->fdt[fd].file_position += 1;
    memcpy((uint8_t*)buf, (uint8_t*)dentry.filename, 32);
    int bytes = strlen(dentry.filename);
    if (bytes > 32) {
        bytes = 32;
    }
    // buf = dentry.filename;
    return bytes;
}

/*
 * dir_close
 *   DESCRIPTION: returns 0
 *   INPUTS: int32_t fd
 *   OUTPUTS: Nothing
 *   RETURN VALUE: 0
 *   SIDE EFFECTS:
 */
int32_t dir_close(int32_t fd) {
    return 0;
}


//other things

/*
 * read_dentry_by_name
 *   DESCRIPTION: gets dentry based on string name
 *   INPUTS: const int8_t* fname, dentry_t* dentry
 *   OUTPUTS: -1 on fail, 0 on sucsess
 *   RETURN VALUE:
 *   SIDE EFFECTS: fills the dentry paramter with data of dentry found
 */
int32_t read_dentry_by_name(const int8_t* fname, dentry_t* dentry){
    int maxDentries;
    int i;
    int8_t* filename2;
    //int size;
    maxDentries = boot_block->num_dir_entries;
    if( fname == NULL || strlen(fname) > 32){
        return -1;
    }

    for(i = 0; i < maxDentries; i++){ //loops through all filled dentreies
        filename2 = (int8_t *) boot_block->dir_entries[i].filename;
        if(strncmp(filename2, fname, 32) == 0 ){ //checks if the names line up, 32 is the number of bytes to compare
            *dentry = boot_block->dir_entries[i]; //fills given dentry
        //     size =  sizeof((uint8_t *) boot_block->dir_entries[i].filename);
        // if(size > 32){
        //     size = 32;
        //     }
        //     strncpy((int8_t* ) dentry->filename, (int8_t *) boot_block->dir_entries[i].filename, size);
            // strncpy((int8_t* ) dentry->filename, (int8_t *) boot_block->dir_entries[i].filename, sizeof((uint8_t *) boot_block->dir_entries[i].filename));
            // //(uint8_t *) dentry->filename = (uint8_t *) boot_block->dir_entries[i].filename;
            // dentry->file_type = boot_block->dir_entries[i].file_type;
            // dentry->inode = boot_block->dir_entries[i].inode;
            // strncpy(dentry->reserved, (uint8_t *) boot_block->dir_entries[i].reserved, sizeof((uint8_t *) boot_block->dir_entries[i].reserved));
            //(uint8_t *) dentry->reserved = (uint8_t *) boot_block->dir_entries[i].reserved;
            // check dentry here?
            return 0; // found file
        }
    }
    return -1;
}

/*
 * read_dentry_by_index
 *   DESCRIPTION: gets dentry based on index in dentry list
 *   INPUTS: int32_t index, dentry_t* dentry
 *   OUTPUTS: -1 on fail, 0 on sucsess
 *   RETURN VALUE:
 *   SIDE EFFECTS: fills the dentry paramter with data of dentry found
 */
int32_t read_dentry_by_index(int32_t index, dentry_t* dentry){
    int maxDentries;
    int size;
    maxDentries = boot_block->num_dir_entries;
    if(index < 0 || index > maxDentries){ //checks if index is valid
        return -1;
    }
    *dentry = boot_block->dir_entries[index]; //fills dentry
    size =  sizeof((uint8_t *) boot_block->dir_entries[index].filename); //fills name (might not be needed))
    strncpy((int8_t* ) dentry->filename, (int8_t *) boot_block->dir_entries[index].filename, size);
    return 0;
}


/*
 * read_data
 *   DESCRIPTION: gets data from data blocks given an inode number
 *   INPUTS: int32_t inode, int32_t offset, int8_t* buf, int32_t length
 *   OUTPUTS: -1 on fail, 0 on sucsess
 *   RETURN VALUE:
 *   SIDE EFFECTS: fills buffer with the data the function asked for
 */
int32_t read_data(int32_t inode, int32_t offset, int8_t* buf, int32_t length){
    inode_t *curInode;
    int inodeLength;
    int i;
    int curByte;
    int bytesWritten;
    int x;
    bytesWritten = 0;
    curByte = 0;
    curInode = &(inodes[inode]); //gets inode
    //checks if parameters are valid
    if(offset > curInode->length){
        return 0;
    }
    if(offset < 0 || length < 0 || inode < 0 ||
        inode > boot_block->num_inodes || buf == NULL){
        return -1; //fails
    }

    inodeLength = (curInode->length / BLOCK_SIZE); //gets number of data blocks used

    for(i = 0; i <= inodeLength; i++){ //cycles through blocks
        for(x = 0; x < BLOCK_SIZE; x++){ //cycles through blocks data
            if(curByte >= offset){ //make sure its offset
                buf[bytesWritten] = blocks[ curInode->data_blocks[i] ].data[x];
                //putc(blocks[curInode.data_blocks[i]].data[x]);
                bytesWritten++; //incremints the number of bytes weve written
            }
            if(bytesWritten >= length || curByte >= curInode->length){ //makes sure we arnt dont
                return bytesWritten;
            }
            curByte++; //increments the number of blocks weve checked
        }
    }
    return 0; // end of file
}
