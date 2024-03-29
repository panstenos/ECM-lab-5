#ifndef _SERIAL_H
#define _SERIAL_H

#include <xc.h>

#define _XTAL_FREQ 64000000 //note intrinsic _delay function is 62.5ns at 64,000,000Hz  

#define RX_BUF_SIZE 20
#define TX_BUF_SIZE 300

#define ADC_BUF_SIZE 60
#define COMMAND_BUF_SIZE 34



//basic EUSART funcitons
void initUSART4(void);
char getCharSerial4(void);
void sendCharSerial4(char charToSend);
void sendStringSerial4(char *string);

// circular Rx buffer functions (Ex3+)
char getCharFromRxBuf(void);
void putCharToRxBuf(char byte);
char isDataInRxBuf (void);

void saveADCval(void);
void putValToADCbuf(unsigned int val);
void sendADCBuf(void);

void read_byte(char byte);
void exec_command(void);

// circular Tx buffer functions (Ex3+)
char getCharFromTxBuf(void);
void putCharToTxBuf(char byte);
char isDataInTxBuf (void);
void TxBufferedString(char *string); //Send buffered string with interrupts
void sendTxBuf(void);
#endif
