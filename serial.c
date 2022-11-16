#include <xc.h>
#include "serial.h"
#include "ADC.h"
#include "LCD.h"

#define BYTE_START 0x01
#define BYTE_END 0x04
#define BYTE_INSTRUCTION 0xFF
#define BYTE_SEND_LDR 0xF0
#define BYTE_SENS_CURR_LDR 0xF1

#define BYTE_START_TEXT 0x02
#define BYTE_END_TEXT 0x03

//variables for a software RX/TX buffer
volatile char EUSART4RXbuf[RX_BUF_SIZE];
volatile char RxBufWriteCnt=0;
volatile char RxBufReadCnt=0;

char EUSART4TXbuf[TX_BUF_SIZE];
volatile char TxBufWriteCnt=0;
volatile char TxBufReadCnt=0;

volatile int ADCbuf[ADC_BUF_SIZE];
volatile char ADCbufCnt = 0;

volatile char Commandbuf[COMMAND_BUF_SIZE];
volatile signed char CommandbufCnt = -1;

void initUSART4(void) {
    RC0PPS = 0x12; // Map EUSART4 TX to RC0
    RX4PPS = 0x11; // RX is RC1
	//code to set up USART4 for Reception and Transmission =
    BAUD4CONbits.BRG16 = 0; 	//set baud rate scaling
    TX4STAbits.BRGH = 0; 		//high baud rate select bit
    SP4BRGL = 51; 			//set baud rate to 51 = 19200bps
    SP4BRGH = 0;			//not used

    RC4STAbits.CREN = 1; 		//enable continuos reception
    TX4STAbits.TXEN = 1; 		//enable transmitter
    RC4STAbits.SPEN = 1; 		//enable serial port	
    
    for(int i = 0; i < ADC_BUF_SIZE;i++){
        ADCbuf[i] = -1;
    }
}

//function to check the TX reg is free and send a byte
/*void sendCharSerial4(char charToSend) {
    while (!PIR4bits.TX4IF); // wait for flag to be set
    TX4REG = charToSend; //transfer char to transmitter
}*/


//function to send a string over the serial interface
void sendStringSerial4(char *string){
	//code to calculate the inegeter and fractions part of a ADC value
    int int_part;
    int frac_part;
    float num = 255/3.3;
    int_part = ADC_getval()/num;
    frac_part = (ADC_getval()*100)/num - int_part*100;
    sprintf(string, "V = %u.%02u, ",int_part, frac_part);
    
    TxBufferedString(string);
    sprintf(string, "V = %u.%02u, ",int_part, frac_part);
    sendTxBuf();
    __delay_ms(100);
}

void saveADCval(){
    unsigned int adcval = ADC_getval();

    char string[4];
    LCD_clear();
    sprintf(string,"%03u",adcval);
    LCD_sendstring(string);
    
    putValToADCbuf(adcval);
}


//functions below are for Ex3 and 4 (optional)

// circular buffer functions for RX
// retrieve a byte from the buffer
char getCharFromRxBuf(void){
    if (RxBufReadCnt>=RX_BUF_SIZE) {RxBufReadCnt=0;} 
    return EUSART4RXbuf[RxBufReadCnt++];
}

// add a byte to the buffer
void putCharToRxBuf(char byte){
    if (RxBufWriteCnt>=RX_BUF_SIZE) {RxBufWriteCnt=0;}
    EUSART4RXbuf[RxBufWriteCnt++]=byte;
}

// function to check if there is data in the RX buffer
// 1: there is data in the buffer
// 0: nothing in the buffer
char isDataInRxBuf (void){
    return (RxBufWriteCnt!=RxBufReadCnt);
}

// circular buffer functions for TX
// retrieve a byte from the buffer
char getCharFromTxBuf(void){
    return EUSART4TXbuf[TxBufReadCnt++];
    if (TxBufReadCnt>=TX_BUF_SIZE) {TxBufReadCnt=0;} 
}

// add a byte to the buffer
void putCharToTxBuf(char byte){
    if (TxBufWriteCnt>=TX_BUF_SIZE) {TxBufWriteCnt=0;}
    EUSART4TXbuf[TxBufWriteCnt++]=byte;
}

// function to check if there is data in the TX buffer
// 1: there is data in the buffer
// 0: nothing in the buffer
char isDataInTxBuf (void){
    return (TxBufWriteCnt!=TxBufReadCnt);
}

//add a string to the buffer
void TxBufferedString(char *string){
	while(*string !=0)
    {
        putCharToTxBuf(*string++);
    }
}

//initialise interrupt driven transmission of the Tx buf
//your ISR needs to be setup to turn this interrupt off once the buffer is empty
void sendTxBuf(void){
    if (isDataInTxBuf()) {PIE4bits.TX4IE=1;} //enable the TX interrupt to send data
}

void putValToADCbuf(unsigned int val){
    if(ADCbufCnt == ADC_BUF_SIZE){ADCbufCnt = 0;}
    ADCbuf[ADCbufCnt++] = val;
}

void sendADCBuf(){
    char string = "000,";
    unsigned int stop_index = ADCbufCnt;
    unsigned int index = stop_index;
    do{
        if(index == ADC_BUF_SIZE){
            index = 0;
        }
        if(ADCbuf[index] == -1){
            index++;
            continue;
        }
        sprintf(string,"%03u,",ADCbuf[index]);
        TxBufferedString(string);
        index++;
    }while(index != stop_index);
    sendTxBuf();
}

void read_byte(char byte){
    if(CommandbufCnt == -1){
        if(byte == BYTE_START){
            CommandbufCnt = 0;
            Commandbuf[CommandbufCnt] = BYTE_START;
            CommandbufCnt++;

        }
    }else{
        if(byte == BYTE_END || CommandbufCnt == COMMAND_BUF_SIZE - 1){
            Commandbuf[CommandbufCnt] = BYTE_END;
            exec_command();
            CommandbufCnt = -1;
        }
        else if(CommandbufCnt != -1){
            Commandbuf[CommandbufCnt] = byte;
            CommandbufCnt++;

        }
    }
}

void exec_command(){
    unsigned int index = 0;
    if(Commandbuf[index] == BYTE_START){
        index++;
        if(Commandbuf[1] == BYTE_INSTRUCTION){
            if(Commandbuf[2] == BYTE_SEND_LDR){
                if(Commandbuf[3] == BYTE_END){
                    sendADCBuf();
                }
            }else if(Commandbuf[2] == BYTE_SENS_CURR_LDR){
                char string = "000,";
                unsigned int index = ADCbufCnt - 1 >= 0 ? ADCbufCnt - 1 : ADC_BUF_SIZE - 1; 
                if(ADCbuf[index] != -1){
                    sprintf(string,"%03u,",ADCbuf[index]);
                    TxBufferedString(string);
                }else{
                    TxBufferedString("ERRO");
                }
                sendTxBuf();
            }
        }
    }
}