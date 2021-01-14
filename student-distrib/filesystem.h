#ifndef FILESYSTEM_H
#define FILESYSTEM_H
#include "types.h"

#define BYTE1       1
#define BYTE2       2
#define BYTE16      16
#define BYTE64      64
#define FOUR_KB     1024
#define INODE_OFFSET 4096/4
#define BLOCK_SIZE   4096

typedef struct dentry {
    /*File names max length of 32 bytes*/
    int8_t filename[32];
    int32_t file_type;
    int32_t inode;
    /*24 byte reserved section*/
    int8_t reserved[24];
} dentry_t;

/*Setting up the boot block of data*/
typedef struct boot {
    int32_t num_dir_entries;
    int32_t num_inodes;
    int32_t num_data_blocks;
    /*52 byte reserved section*/
    int8_t reserved[52];
    /*There are 63 dentries*/
    dentry_t dir_entries[63];
} boot_t;

typedef struct inode {
    int32_t length;
    /*Amount of data blocks*/
    int32_t data_blocks[1023];
} inode_t;

typedef struct block{
    int8_t data[BLOCK_SIZE];
} block_t;


/* Initialize the filesystem */

void file_init();

int32_t file_write(int32_t fd, const void* buf, int32_t nbytes);

int32_t file_open(const uint8_t* filename);

int32_t file_read(int32_t fd, void* buf, int32_t nbytes);

int32_t file_close(int32_t fd);

int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);

int32_t dir_open(const uint8_t* filename);

int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);

int32_t dir_close(int32_t fd);

int32_t read_dentry_by_name(const int8_t* fname, dentry_t* dentry);

int32_t read_dentry_by_index(int32_t index, dentry_t* dentry);

int32_t read_data(int32_t inode, int32_t offset, int8_t* buf, int32_t length);

volatile int32_t* FILESYS_ADDR;
dentry_t dentries[62]; // 62 is number of possible files

boot_t * boot_block;
inode_t * inodes;
block_t * blocks;

#endif
