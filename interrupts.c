#include <xc.h>
#include "interrupts.h"
#include "serial.h"

//extern int seconds;
/************************************
 * Function to turn on interrupts and set if priority is used
 * Note you also need to enable peripheral interrupts in the INTCON register to use CM1IE.
************************************/
void Interrupts_init(void)
{   
    INTCONbits.PEIE = 1;    //turns on all the peripheral interrupts
    PIE4bits.RC4IE=1;	//receive interrupt
    PIE4bits.TX4IE=1;	//transmit interrupt (only turn on when you have more than one byte to send)
    PIE0bits.TMR0IE=1; //Enabling the timer overflow interrupter
    INTCONbits.GIE = 1; 	//turn on interrupts globally (when this is off, all interrupts are deactivated) 
    TRISHbits.TRISH3 = 0;
}

/************************************
 * High priority interrupt service routine
 * Make sure all enabled interrupts are checked and flags cleared
************************************/
void __interrupt(high_priority) HighISR()
{   
    if(PIR4bits.TX4IF){
        if(isDataInTxBuf()){
            TX4REG = getCharFromTxBuf();
        }else{
            PIE4bits.TX4IE=0;
        }
    }
    
    if (PIR4bits.RC4IF){//wait for the data to arrive
        putCharToRxBuf(RC4REG); //return byte in RCREG
    }
    
    if(PIR0bits.TMR0IF){ // Timer overflow flag
        LATHbits.LATH3 = !LATHbits.LATH3;
        PIR0bits.TMR0IF = 0;
        saveADCval();
        TMR0H = 0b1011;
        TMR0L = 0b11011100; //Changing the High and Low default value to better match 1s
    }
}
