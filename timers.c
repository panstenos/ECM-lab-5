#include <xc.h>
#include "timers.h"

/************************************
 * Function to set up timer 0
************************************/

int test_mode = 1; //1
int seconds = 0; //0
int minutes = 0; //0
int hours = 0; //0
int extra_hours = 0; //0
int adjust_hours = 0; //0
// when 1, it adjusts the hours at 6:00 January 1st 
//to accont for timer error stacked at the end of the year
int adjust_hours_once = 0; 
unsigned int LDR_hours = 0; //0

int day = 26; //1 //26
int week_day = 3; //2 //3
int month = 2; //0 //2
int year = 2020; //2020
int month_days[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
int sunrise_records[31] = {0}; // list that contains the sunrise times of December
int sunset_records[31] = {0}; // list that contains the sunset times of December
void Timer0_init(void)
{
    T0CON1bits.T0CS=0b010; // Fosc/4
    T0CON1bits.T0ASYNC=1; // see datasheet errata - needed to ensure correct operation when Fosc/4 used as clock source
    if(test_mode == 0){
        T0CON1bits.T0CKPS=0b1000; // 1:256 -> required: 1:244.14
        TMR0H=0b00001011;            
        TMR0L=0b11011011;
    }else{
        T0CON1bits.T0CKPS=0; // 1:256 -> required: 1:244.14
        TMR0H=0b00000000;            
        TMR0L=0b00000000;
    }
    T0CON0bits.T016BIT=1;	//8bit mode	
    
    // initialise the time registers to 3035 for the LED to toggle every one second
    
    T0CON0bits.T0EN=1;	//start the timer
}

/************************************
 * Function to return the full 16bit timer value
 * Note TMR0L and TMR0H must be read in the correct order, or TMR0H will not contain the correct value
************************************/
unsigned int get16bitTMR0val(void)
{
	return TMR0L | (TMR0H << 8); //use bitwise operator to combine the H and L bits
}
/*
void increment_hours(int value){
    hours += value;
}
*/
void increment_seconds(){ //main counter function
    if(test_mode == 0){
    seconds += 1 ; // increment by the second
    }else{
        seconds += 30;
    }
    if (seconds == 60){ // if you reach 60 sec
        seconds = 0; // reset seconds to 0
        minutes ++ ; // increse minutes by 1 
    }
    if (minutes == 60){ 
        minutes = 0; //reset minutes to 0
        hours ++ ; // increase hours by 1
        if (LDR_hours <= 23) // increment only when LDR_hours are less than 23
        {
            LDR_hours ++ ; // increase the hour count for the LDR
        }
    }
    if (hours + extra_hours == 24){
        hours = 0 - extra_hours; //reset hours to 0 - extra_hours
        day ++ ; // increase day count by 1
        week_day ++; // increase day of week by 1
    }
    if (week_day == 7){ // if the day is Sunday
        week_day = 0; //reset count to Monday
    }
    if (month == 1 && year%4 == 0){ // check for leap year
        if (day == 30) // check if its after the 29th of February
        {
            day = 1; // reset the days
            month += 1; // add a month
        }
    }else if(day == month_days[month+1]+1) //if the day is one more than the number of days of each month:
    {
        day = 1; // set days to 1
        month += 1; // increment the month
    }
    
    if (month == 12) // when the month reaches 12
    {
        month = 0; // reset the months
        year += 1; // increment the years
    }
    if (month == 2 && day > 24 && week_day == 6 && hours == 2 && extra_hours == 0)
    { // If its March AND its the last week AND the time is 02:00 and extra_hours is 0 (to do it only once)
        extra_hours = 1; // set the extra hours counter to 1
    }
    if (month == 9 && day > 24 && week_day == 6 && hours == 2 && extra_hours == 1)
    { // If its October AND its the last week AND the time is 02:00 and extra_hours is 1 (to do it only once)
        extra_hours = 0; // set the extra hours counter to 0
    }   
}

// functions that return the values to main.c
unsigned int get_seconds(){
    return seconds;
}
unsigned int get_minutes(){
    return minutes;
}
unsigned int get_hours(){
    return hours + extra_hours;
}
unsigned int get_day(){
    return day;
}
const char * get_week_day(){
    if(week_day == 0){
        return("MON");
    }else if(week_day == 1){
        return("TUE");
    }else if(week_day == 2){
        return("WED");
    }else if(week_day == 3){
        return("THU");
    }else if(week_day == 4){
        return("FRI");
    }else if(week_day == 5){
        return("SAT");
    }else if(week_day == 6){
        return("SUN");
    }
}
unsigned int get_month(){
    return month;
}
unsigned int get_year(){
    return year;
}

unsigned int LDR_issue_hours(int LDR_val)
{
// if the LDR can read values higher than 10 then reset the hour count to 0
        if(LDR_val > 10){
            LDR_hours = 0;
        }
        
        if(LDR_hours >= 24){ // if the LDR hour counter is higher than 24 
            return 1; // error state
        }else{
            return 0; // non error state
        }

}

void adjust_time(int LDR_val){
    if (LDR_val>10 && month ==11) // if its day in December
    {
        if (sunrise_records[day] == 0) // if its the first time you update the list
        {
            sunrise_records[day] == hours; // keep only the lowest value
        }
        sunset_records[day] == hours; // keep the highest value
    }
    if (month == 0 && day == 1 && hours == 0 && minutes == 0 & seconds == 0) // calculations for adjust_hour made at midnight
    {
        int j;
        for (j=0; j<31;j++) // add all the measurements from December
        {
            adjust_hours += sunrise_records[j] + sunset_records[j] - 24; 
        }
        adjust_hours /= 62; // calculate the average adjustment needed once!
        adjust_hours_once = 1; // turn flag on
    }
    if (month == 0 && day == 1 && hours == 6 && minutes == 0 & seconds == 0 && adjust_hours_once == 1) // employ the changes at 6 am 
    {
        hours -= adjust_hours; //adjust the value of the current hours
        adjust_hours_once = 0; // turn flag off
    }
}