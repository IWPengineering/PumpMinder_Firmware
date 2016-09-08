/*
 * File:   display.c
 * Author: KSK0419
 *
 * Created on March 22, 2016, 10:30 AM
 */


#include "xc.h"
#include "display.h"

static void dspDisplayInit(void);
static void dspDisplaySend(enum _DisplayCommand, unsigned char);
static void dspDisplayDataAddOne(enum _DisplayCommand, unsigned char);
static void dspDisplayDataAddString(unsigned char *, int size);
static void dspDisplayLoop(int);

struct _DisplayData DisplayData[DISPLAY_DATA_POSITION_MAX];
unsigned char DisplayDataPositionRead = 0;
unsigned char DisplayDataPositionWrite = 0;
unsigned char DisplayDataPositionInverse;

/*********************************
 DISPLAY FUNCTIONS
 * first functions only call the internal functions if we are using them
 * second set are entirely excluded if we are not using display features
 * this should help turn on and off the display without much trouble
 *********************************/
void DisplayInit(void) {
    dspDisplayInit();
    return;
}

void DisplayLoop(int count, bool resetCursor) {
    if(resetCursor)
    {
        dspDisplaySend(sendCommand, 0x02);
    }
    int inx;
    for (inx = 0; inx < count; inx++) {
        dspDisplayLoop(1);
    }
    return;
}

void DisplayDataAddString(unsigned char *string, int size) {

    dspDisplayDataAddString(string, size);
    return;
}

void DisplayDataAddCharacter(unsigned char ch) {

    dspDisplayDataAddOne(sendData, ch);
    return;
}

void dspDisplayInit(void) {

    DISPLAY_BIT_REGISTER_SELECT__COMMAND_DATA_DIR = 0b0;
    DISPLAY_BIT_SELECT_DIR__READ_WRITE_DIR = 0b0;
    DISPLAY_BIT_ENABLE__HIGH_LO_DIR = 0b0;

    DISPLAY_BIT_SELECT_DIR__READ_WRITE = 0b0;

    //    DISPLAY_DATA_WRITE = 0b00000000;

    DSP0_PORT_DIR = 0b0;
    DSP1_PORT_DIR = 0b0;
    DSP2_PORT_DIR = 0b0;
    DSP3_PORT_DIR = 0b0;
    DSP4_PORT_DIR = 0b0;
    DSP5_PORT_DIR = 0b0;
    DSP6_PORT_DIR = 0b0;
    DSP7_PORT_DIR = 0b0;
    delayMs(100);

    dspDisplayDataAddOne(sendCommand, DISPLAY_COMMAND_FUNCTION_SET);
    DisplayLoop(1, false);
    dspDisplayDataAddOne(sendCommand, DISPLAY_COMMAND_OFF);
    DisplayLoop(1, false);
    dspDisplayDataAddOne(sendCommand, DISPLAY_COMMAND_CLEAR);
    DisplayLoop(1, false);
    dspDisplayDataAddOne(sendCommand, DISPLAY_COMMAND_ENTRY);
    DisplayLoop(1, false);
    dspDisplayDataAddOne(sendCommand, DISPLAY_COMMAND_HOME);
    DisplayLoop(1, false);
    dspDisplayDataAddOne(sendCommand, DISPLAY_COMMAND_ON);
    DisplayLoop(1, false);
    
    return;
}

void dspDisplaySend(enum _DisplayCommand command, unsigned char data) {

    switch (command) {
        case sendCommand:
            DISPLAY_BIT_REGISTER_SELECT__COMMAND_DATA = 0b0;
            break;
        case sendData:
            DISPLAY_BIT_REGISTER_SELECT__COMMAND_DATA = 0b1;
            break;
        case setPosition:
            DISPLAY_BIT_REGISTER_SELECT__COMMAND_DATA = 0b0;
            // bit 7 must be a 1
            data |= 1 << 7;
            break;
        default:
            break;
    }

    DISPLAY_BIT_ENABLE__HIGH_LO = 0b1;

    DSP0_PORT = (data & 0b00000001) >> 0;
    DSP1_PORT = (data & 0b00000010) >> 1;
    DSP2_PORT = (data & 0b00000100) >> 2;
    DSP3_PORT = (data & 0b00001000) >> 3;
    DSP4_PORT = (data & 0b00010000) >> 4;
    DSP5_PORT = (data & 0b00100000) >> 5;
    DSP6_PORT = (data & 0b01000000) >> 6;
    DSP7_PORT = (data & 0b10000000) >> 7;

    DISPLAY_BIT_ENABLE__HIGH_LO = 0b0;

    return;
}

void dspDisplayDataAddOne(enum _DisplayCommand command, unsigned char data) {

    if (DisplayDataPositionWrite <= DISPLAY_DATA_POSITION_MAX) {
        DisplayData[DisplayDataPositionWrite].command = command;
        DisplayData[DisplayDataPositionWrite].data = data;
        DisplayDataPositionWrite++;
    }

    return;
}

void dspDisplayDataAddString(unsigned char *string, int size) {
    unsigned char i;
    for (i = 0; i < size - 1; i++) {
        dspDisplayDataAddOne(sendData, string[i]);
    }

    return;
}

void dspDisplayLoop(int count) {
    // reading busy flag does not work correctly right now
    // need to check it out
    int i;
    for (i = 0; i < count; i++) {
        //if ( IsDisplayBusy ( ) == 0 )
        {
            if (DisplayDataPositionRead < DisplayDataPositionWrite) {
                dspDisplaySend(DisplayData[DisplayDataPositionRead].command, DisplayData[DisplayDataPositionRead].data);
                DisplayDataPositionRead++;

                if (DisplayDataPositionRead >= DisplayDataPositionWrite) {
                    // we have caught up with all of the write data - so reset
                    DisplayDataPositionRead = 0;
                    DisplayDataPositionWrite = 0;
                }
            }
        }
    }

    return;
}

void DisplayDataSetRow(unsigned char row) 
{
    unsigned char rowValue;
    switch (row) 
    {
        case 0:
            rowValue = 0x00;
            break;
        case 1:
            rowValue = 0x40;
            break;
        case 2:
            rowValue = 0x14;
            break;
        case 3:
            rowValue = 0x54;
            break;
        default:
            rowValue = 0x00;
            break;
    }
    
    dspDisplayDataAddOne(setPosition, rowValue);
    
    return;
}

void DisplayTurnOff(void)
{
    // Reset the cursor, this clears the screen
    dspDisplaySend(sendCommand, 0x01);
    delayMs(5);
    dspDisplaySend(sendCommand, 0x02);
}
