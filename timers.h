#ifndef _timers_H
#define _timers_H

#include <xc.h>

#define _XTAL_FREQ 64000000

void Timer0_init(void);
unsigned int get16bitTMR0val(void);
void increment_seconds(void);
unsigned int check_for_hour_shift(void);
unsigned int get_seconds(void);
unsigned int get_minutes(void);
unsigned int get_hours(void);
unsigned int get_day(void);
const char * get_week_day(void);
unsigned int get_month(void);
unsigned int get_year(void);
unsigned int LDR_issue_hours(int);
void adjust_time(int);
int test_mode;
#endif