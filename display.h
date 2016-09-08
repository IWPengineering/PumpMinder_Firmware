/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef DISPLAY_H__
#define	DISPLAY_H__

#include <xc.h> // include processor files - each processor file is guarded.  
#include <stdbool.h>
#include "utilities.h"

#define DISPLAY_DATA_POSITION_MAX 75

#define DSP0_PORT           LATBbits.LATB14 
#define DSP1_PORT           LATBbits.LATB2 
#define DSP2_PORT           LATBbits.LATB13 
#define DSP3_PORT           LATBbits.LATB3
//#define DSP4_PORT           LATAbits.LATA2
#define DSP4_PORT           LATBbits.LATB12
//#define DSP5_PORT           LATBbits.LATB12
#define DSP5_PORT           LATAbits.LATA2
#define DSP6_PORT           LATBbits.LATB9
#define DSP7_PORT           LATBbits.LATB5

#define DSP0_PORT_DIR       TRISBbits.TRISB14 
#define DSP1_PORT_DIR       TRISBbits.TRISB2 
#define DSP2_PORT_DIR       TRISBbits.TRISB13 
#define DSP3_PORT_DIR       TRISBbits.TRISB3 
#define DSP4_PORT_DIR       TRISAbits.TRISA2 
#define DSP5_PORT_DIR       TRISBbits.TRISB12 
#define DSP6_PORT_DIR       TRISBbits.TRISB9 
#define DSP7_PORT_DIR       TRISBbits.TRISB5 

#define DISPLAY_COMMAND_FUNCTION_SET    0b00111000  
#define DISPLAY_COMMAND_OFF             0b00001000
#define DISPLAY_COMMAND_CLEAR           0b00000001
#define DISPLAY_COMMAND_ENTRY           0b00000110
#define DISPLAY_COMMAND_HOME            0b00000010
#define DISPLAY_COMMAND_ON              0b00001100

#define DISPLAY_BIT_REGISTER_SELECT__COMMAND_DATA  LATBbits.LATB0
#define DISPLAY_BIT_SELECT_DIR__READ_WRITE LATBbits.LATB15
#define DISPLAY_BIT_ENABLE__HIGH_LO LATBbits.LATB1

#define DISPLAY_BIT_REGISTER_SELECT__COMMAND_DATA_DIR TRISBbits.TRISB0 
#define DISPLAY_BIT_SELECT_DIR__READ_WRITE_DIR TRISBbits.TRISB15 
#define DISPLAY_BIT_ENABLE__HIGH_LO_DIR TRISBbits.TRISB1 

typedef enum _DisplayCommand {
    sendCommand = 1,
    sendData = 2,
    setPosition = 3
} DisplayCommand_t;

typedef struct _DisplayData {
    enum _DisplayCommand command;
    unsigned char data;
} DisplayData_t;

void DisplayInit(void);
void DisplayLoop(int count, bool resetCursor);
void DisplayDataAddString(unsigned char *string, int size);
void DisplayDataAddCharacter(unsigned char ch);
void DisplayDataSetRow(unsigned char row);
void DisplayTurnOff(void);
#endif	/* DISPLAY_H__ */

