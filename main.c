// CONFIG1L
#pragma config FEXTOSC = HS     // External Oscillator mode Selection bits (HS (crystal oscillator) above 8 MHz; PFM set to high power)
#pragma config RSTOSC = EXTOSC_4PLL// Power-up default value for COSC bits (EXTOSC with 4x PLL, with EXTOSC operating per FEXTOSC bits)

// CONFIG3L
#pragma config WDTE = OFF        // WDT operating mode (WDT enabled regardless of sleep)

#include <xc.h>
//#include "LEDarray.h"
#include "interrupts.h"
#include "timers.h"
#include "ADC.h"
#include "LCD.h"
//#include "light_manager.h"
#include "serial.h"

#define _XTAL_FREQ 64000000 //note intrinsic _delay function is 62.5ns at 64,000,000Hz

void main(void) {
    LCD_Init();
    initUSART4(); 

    while (1) {
        LCD_sendbyte(getCharSerial4(),1);
    }
}
