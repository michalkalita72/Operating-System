#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "paging.h"
#include "rtc.h"
#include "keyboard.h"
#include "filesystem.h"


#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test() {
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 20; ++i) {
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)) {
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/*Exception testings*/

/* Divide by zero
 *
 * Checks if the divide by zero exception works
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Divide by zero exception
 * Files: None
 */
int divide_by_zero() {
	int a = 0;
	int b = 2;
	b = b/a;
	return b;
}

/* Page fault
 *
 * Checks if dereferencing NULL causes a page fault
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Page fault
 * Coverage: Page fault exception, requires paging to be setup
 * Files: None
 */
int derefNULL() {
	int * ptr;
	ptr = NULL;
	int i = *(ptr);
	return i;
}

/* Paging tests
 *
 * Checks if accessing memory triggers a page fault
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Page fault exception, requires paging to be setup
 * Files: None
 */
int pagetabletest() {
	int * x = 0x00000;

	return 	*x;
}





/* Checkpoint 2 tests */

/*
 * rtcChangeTest
 *   DESCRIPTION: changed rtc freq to prove it can
 *   INPUTS:
 *   OUTPUTS: PASS for sucsess
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes frequency of rtc several times to show it is in fact changing
 */
int rtcChangeTest(){
	int i;
	int x;
	int hertz;
	hertz = 1;
	for(x = 0; x < 10; x++){
			clear();
			hertz = hertz * 2; //increase hertz by power of 2
		//putc('s');
			if(rtc_write(NULL, &hertz ,4) == 0){ //changes freq
				//putc('k');
			}
			for(i = 0; i < (x * 30); i++){ //waits for interupts to happen
				rtc_read(NULL, NULL ,4);
				//putc('d');
			}
		}
	clear();
	rtc_open(NULL); //proves that rtc_open sets freq to what it should be
	return PASS;

}


/*Terminal Test*/
/*
 * terminal_rw
 *   DESCRIPTION: test terminal read and write from keyboard buffer
 *   INPUTS:
 *   OUTPUTS: PASS for sucsess
 *   RETURN VALUE: none
 *   SIDE EFFECTS:
 */
int terminal_rw(){
	char temp [128];
	while(1){
		terminal_read (0 , temp, 128);
		terminal_write(0 , temp, 128);
	}
	return PASS;
}

/*
 * fileindextest
 *   DESCRIPTION: Pritns the name and type of all the dentries
 *   INPUTS: NONE
 *   OUTPUTS: PASS for sucsess
 *   RETURN VALUE: none
 *   SIDE EFFECTS: Prints the name and types of dentries onto the screen
 */
int fileindextest(){
	dentry_t dentry;
    int i;
	int j;
    int test;
	/*Checking all the possible dentries*/
    for(i =0 ; i < 63; i++){
        test = read_dentry_by_index(i, &dentry);
		/*Check if dentry exsist*/
        if( test != -1 && dentry.filename != NULL){
			printf("File name: ");
			/*Prints the 32 character in the file name*/
			for(j = 0; j <32; j++){
				if(dentry.filename[j] == NULL){
					break;
				}
				putc(dentry.filename[j]);
			}
			printf("     File_type:%d" , dentry.file_type);
			printf("     File Size: %d", inodes[dentry.inode].length);
			putc('\n');
        }
    }
	return PASS;
}

/*
 * testReaddata
 *   DESCRIPTION: reads data from a given file
 *   INPUTS:
 *   OUTPUTS: PASS for sucsess
 *   RETURN VALUE: none
 *   SIDE EFFECTS: prints out the data for a file specified
 */
int testReaddata(){
	// int32_t length;
	// int realSize;
	// int i;
	// if(file_open("frame0.txt") == -1){ //opens file with a certain name, change this to test other files
	// 	return FAIL;
	// }
	// length = sizeOfOpen; //sets length to size of the opened file
	// int8_t buf[length];
	// realSize = file_read(NULL, buf, length); //fills buf with data to be printed
	// //printf("FILE READ");
	// for(i = 0; i < length ; i++){ //prints data minus null characters
	// 	if(buf[i] == '\0'){
	// 		continue;
	// 	}
	// 	// if(buf[i] < 32 || buf[i] > 127){
	// 	// 	continue;
	// 	// }
	// 	putc(buf[i]);
	// }
	// putc('\n');
	return PASS;
}

/*
 * testExecutable
 *   DESCRIPTION: reads data from a given file
 *   INPUTS:
 *   OUTPUTS: PASS for sucsess
 *   RETURN VALUE: none
 *   SIDE EFFECTS: prints out the data for a file specified
 */
int testExecutable(){
	// int32_t length;
	// int realSize;
	// int i;
	// if(file_open("ls") == -1){ //opens file with a certain name, change this to test other files
	// 	return FAIL;
	// }
	// length = sizeOfOpen; //sets length to size of the opened file
	// int8_t buf[length];
	// realSize = file_read(NULL, buf, length); //fills buf with data to be printed
	// //printf("FILE READ");
	// for(i = 0; i < length ; i++){ //prints data minus null characters
	// 	if(buf[i] == '\0'){
	// 		continue;
	// 	}
	// 	putc(buf[i]);
	// }
	// putc('\n');
	return PASS;
}

/*
 * testReaddataLong
 *   DESCRIPTION: reads data from a given file
 *   INPUTS:
 *   OUTPUTS: PASS for sucsess
 *   RETURN VALUE: none
 *   SIDE EFFECTS: prints out the data for a file specified
 */
int testReaddataLong(){
	// int32_t length;
	// int realSize;
	// int i;
	// if(file_open("fish") == -1){ //opens file with a certain name, change this to test other files
	// 	return FAIL;
	// }
	// length = sizeOfOpen; //sets length to size of the opened file
	// int8_t buf[length];
	// realSize = file_read(NULL, buf, length); //fills buf with data to be printed
	// //printf("FILE READ");
	// for(i = 0; i < length ; i++){ //prints data minus null characters
	// 	if(buf[i] == '\0'){
	// 		continue;
	// 	}
	// 	if(buf[i] < 32){
	// 		continue;
	// 	}
	// 	putc(buf[i]);
	// }
	// putc('\n');
	return PASS;
}

/*
 * dirread
 *   DESCRIPTION: tests dir_read function to read file or directory names
 *   INPUTS:
 *   OUTPUTS: PASS for sucsess
 *   RETURN VALUE: none
 *   SIDE EFFECTS: puts anme of files or dir onto screen
 */
int dirread(){
	char buf[33];
	int j;
	while(dir_read(0, buf, 32) == 0){
        for(j = 0; j <32; j++){
			if(buf[j] == NULL){
				break;
			}
			putc(buf[j]);
		}
		putc('\n');
	}
	return PASS;
}







/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests() {
	clear();
	// TEST_OUTPUT("idt_test", idt_test());
	// TEST_OUTPUT("Divide by zero", divide_by_zero());
	// TEST_OUTPUT("Dereference NULL", derefNULL());
	// TEST_OUTPUT("Paging Table", pagetabletest());
	// TEST_OUTPUT("RTC test", rtcChangeTest());
	// TEST_OUTPUT("Terminal", terminal_rw());
	// TEST_OUTPUT("Fileindextest", fileindextest());
	// TEST_OUTPUT("test .txt", testReaddata());
	// TEST_OUTPUT("test long file", testReaddataLong());
	// TEST_OUTPUT("test executable", testExecutable());
	// TEST_OUTPUT("Test Dir", dirread());
	// TEST_OUTPUT("Test Dir", pagemem());

}
