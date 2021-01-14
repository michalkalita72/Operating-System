#ifndef RTC_H
#define RTC_H
#include "types.h"
/* Initialize the RTC */
void rtc_init();

/* Handle the RTC Interrupt */
void rtc_handler();

int rtc_write(int32_t fd, const void* buf, int32_t nbytes);

int rtc_open(const uint8_t* filename);

int rtc_read(int32_t fd, void* buf, int32_t nbytes);

int rtc_close(int32_t fd);
#endif /* RTC_H */
