#include <xc.h>
#include "LCD.h"
#include <stdio.h>
/************************************
 * Function to toggle LCD enable bit on then off
 * when this function is called the LCD screen reads the data lines
************************************/
void LCD_E_TOG(void)
{
    //turn the LCD enable bit on
    LATCbits.LATC2 = 1;
    __delay_us(2); //wait a short delay
    //turn the LCD enable bit off again
    LATCbits.LATC2 = 0;
 
}
 
/************************************
 * Function to set the 4-bit data line levels for the LCD
************************************/
void LCD_sendnibble(unsigned char number)
{
    
    //set the data lines here (think back to LED array output)
    if(number & 0b0001){ LCD_DB4 = 1;}else{LCD_DB4 = 0;}
    if(number & 0b0010){ LCD_DB5 = 1;}else{LCD_DB5 = 0;}
    if(number & 0b0100){ LCD_DB6 = 1;}else{LCD_DB6 = 0;}
    if(number & 0b1000){ LCD_DB7 = 1;}else{LCD_DB7 = 0;}
        
    LCD_E_TOG();            //toggle the enable bit to instruct the LCD to read the data lines
    __delay_us(5);      //Delay 5uS
}
 
 
/************************************
 * Function to send full 8-bit commands/data over the 4-bit interface
 * high nibble (4 most significant bits) are sent first, then low nibble sent
************************************/
void LCD_sendbyte(unsigned char Byte, char type)
{
    
    LCD_RS = type; // set RS pin whether it is a Command (0) or Data/Char (1) using type argument
    LCD_sendnibble(Byte >> 4); // send high bits of Byte using LCDout function
    LCD_sendnibble(Byte);// send low bits of Byte using LCDout function
 
    __delay_us(50);               //delay 50uS (minimum for command to execute)
}
 
/************************************
 * Function to initialise the LCD after power on
************************************/
void LCD_Init(void)
{
    
    //Define LCD Pins as Outputs and
    //set all pins low (might be random values on start up, fixes lots of issues)
    TRISCbits.TRISC2 = 0;
    TRISCbits.TRISC6 = 0;
    TRISBbits.TRISB3 = 0;
    TRISBbits.TRISB2 = 0;
    TRISEbits.TRISE3 = 0;
    TRISEbits.TRISE1 = 0;
    
    // initialise LCD output -> set ports below to 0
    LCD_RS = 0;
    LCD_E = 0;
    LCD_DB4 = 0;
    LCD_DB5 = 0;
    LCD_DB6 = 0;
    LCD_DB7 = 0;
    
    __delay_ms(60);      //Delay 60mS
     
    LCD_sendnibble(0b0011);
    __delay_us(40);      //Delay 40uS
 
    LCD_sendnibble(0b0010);
    __delay_us(40);      //Delay 40uS
 
    LCD_sendbyte(0b00101000,0); //turn on DB5
    __delay_us(40);      //Delay 40uS
 
    LCD_sendbyte(0b00101000,0); //enable DB5
    __delay_us(40);      //Delay 40uS
    
    LCD_sendbyte(0b00000001,0); //Set DB4-7 to 0 -> Clear screen
    __delay_ms(2);      //Delay 2ms
        
    LCD_sendbyte(0b00000110,0);  // 0x06 Auto Increment cursor, shift display off

    LCD_sendbyte(0b00001110,0);  //Cursor on, blinking off
    LCD_sendbyte(0b00001100,0);  //Cursor off, blinking off
 
    //remember to turn the LCD display back on at the end of the initialisation (not in the data sheet)
}
 
void LCD_clear(void)
{
    __delay_us(40);
    LCD_sendbyte(0b00000001,0); //Set DB4-7 to 0 -> Clear screen
    
    __delay_ms(2);      //Delay 2ms    
    LCD_sendbyte(0b00000110,0);  // 0x06 Auto Increment cursor, shift display off
    LCD_sendbyte(0b00001110,0);  //Cursor on, blinking off
    LCD_sendbyte(0b00001100,0);  //Cursor off, blinking off
    
}

/************************************
 * Function to set the cursor to beginning of line 1 or 2
************************************/
void LCD_setline(int line)
{
    if(line == 0){
        LCD_sendbyte(0x80,0);   //Set cursor for day of week
    }else if(line == 1){
        LCD_sendbyte(0x86,0);    //Set cursor for date of month
    }else if(line == 2){
        LCD_sendbyte(0x89,0);    //Set cursor for month
    }else if(line == 3){
        LCD_sendbyte(0x8C,0);    //Set cursor for year
    }else if(line == 4){
        LCD_sendbyte(0xC0,0);    //Set cursor for hour
    }else if(line == 5){
        LCD_sendbyte(0xC3,0);    //Set cursor for minute
    }else if(line == 6){
        LCD_sendbyte(0xC6,0);    //Set cursor for second
    }else if(line == 7){
        LCD_sendbyte(0xCD,0);    //Set cursor for intensity
    }
}
 
/************************************
 * Function to send string to LCD screen
************************************/

void LCD_sendstring(char *string){ //input two strings
        LCD_setline(0); //write first input in the first line
        while(*string !=0){ //write all the characters of each element
                LCD_sendbyte(*string++,1);
        }
            
} 

/************************************
 * Function to send string to LCD screen
************************************/
void LCD_scroll(int max) //scrolls back and forth; input maximum length among the two strings
{
    max -= 15; //LCD has 16 bits length 
    if (max > 0){ //only if one of the two lines is more than 16 characters in length
        int i; //declare the integer
        for (i=0;i<max;i++){ //scroll right max characters; notice max -= 16
        LCD_sendbyte(0b00011000,0); //scroll right function
        //code here to scroll the text on the LCD screen
        __delay_ms(500); // delay 0.5 sec
        }
        for (i=0;i<max;i++){ //scroll back (left) max characters; notice max -= 16
        LCD_sendbyte(0b00011100,0); //scroll left function
        //code here to scroll the text on the LCD screen
        __delay_ms(500); // delay 0.5 sec
        }
    }
}

/************************************
 * Function takes a ADC value and works out the voltage to 2 dp
 * the result is stored in buf as ascii text ready for display on LCD
 * Note result is stored in a buffer using pointers, it is not sent to the LCD
************************************/
void ADC2String(char *buf, unsigned int x, unsigned int leading_zeros){
    
    if(leading_zeros == 0){
    sprintf(buf, "%u",x); //convert integer to float no leading 0s
    }else if(leading_zeros == 1){
    sprintf(buf, "%01u",x); //convert integer to float; 1 leading zero for x < 10 
    }else if(leading_zeros == 2){
    sprintf(buf, "%02u",x); //convert integer to float; 1 leading zero for x < 100, 2 for x < 10
    }else if(leading_zeros == 3){
    sprintf(buf, "%03u",x); //convert integer to float; 1 leading zero for x < 1000, etc.
    }else{sprintf(buf, "%04u",x); //convert integer to float; 1 leading zeros for x < 1000, etc
    }
}

