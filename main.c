/* 
 * File:   Water4Project.c
 * Author: Jacqui
 *
 * Created on January 20, 2016, 6:35 PM
 *            Revised 9/6/2016 R.Fish
 * * This is the version shipped in the three repaired units on 9/6/2016
 * Changes in this revision (this is saved in O:/CodeDevSpace/PumpMinder
 *  1 - rather than delay for 0.5 sec, sleep when activity done and wake up every 0.5sec
 *  2 - change ReadWaterSensor function so the time required to detect NoWater is almost the same as Water
 *  3 - remove all configuration bit settings which are defaults not directly necessary for this application
 *  4 - Change the hoursToAsciiDisplay function to properly handle int/float math
 *  5 - Change RESET_THRESHOLD to 16 when the COUNTDOWN_THRESHOLD is 6 so countdown is displayed before screen in blanked
 *  6 - Do not turn the 555 on and off, not worth the power savings
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <xc.h>
#include <string.h>
#include "display.h"
#include "utilities.h"





// ****************************************************************************
// *** PIC24F32KA302 Configuration Bit Settings *******************************
// ****************************************************************************

// FOSCSEL
//#pragma config FNOSC = FRC // Oscillator Select (Fast RC Oscillator (FRC - 8Mhz))
#pragma config FNOSC = LPFRC // Oscillator Select (Low power FRC - 500 khz)
#pragma config SOSCSRC = 0 // SOSC Source Type (Analog Mode for use with crystal)
#pragma config LPRCSEL = LP // LPRC Oscillator Power and Accuracy (High Power, High Accuracy Mode)
#pragma config IESO = OFF // Internal External Switch Over bit (Internal External Switchover mode enabled (Two-speed Start-up enabled))
// FICD
#pragma config ICS = PGx2 // ICD Pin Placement Select bits (EMUC/EMUD share PGC2/PGD2)  NOT ALWAYS THE DEFAULT


static volatile int buttonFlag = 0; // alerts us that the button has been pushed and entered the inerrupt subroutine
static bool isButtonTicking = false;
static volatile int buttonTicks = 0;


void _ISR _T1Interrupt( void){
// The T1 interrupt is used to wake the PIC up from SLEEP.

// Note:  The timer is reset to zero once it matches its Preset value
 
_T1IF = 0;  //clear the interrupt flag
} //T1Interrupt

void ConfigTimerT1WithInt(int num_ms_sleep){
    _T1IP = 4; // confirm interrupt priority is the default value
    TMR1 = 0; // clear the timer

    // configure Timer1 module to
    // use LPRC as its clock so it is still running during sleep
    // set prescale to 1:1
    // continue running in IDLE mode and start running
    T1CON = 0x8202;

    // set the period register for num_ms_sleep milliseconds 
    // Since oscillator = LPRC it is 32khz
    // so one count every 31.25usec
    // 
    PR1 = ((float) num_ms_sleep *1000)/31.25;

// Timer1 Interrupt control bits
    _T1IF = 0; // clear the interrupt flag, before enabling interrupt
    _T1IE = 1; // enable the T1 interrupt source
}
void ConfigTimerT2NoInt(){
// Timer 2 is used to measure the HIGH and LOW times from the WPS
    TMR2 = 0; // clear the timer
    // Configure Timer2 
    //   As a 16bit counter running from Fosc/2 with a 1:1 prescale
    //   shut off in IDLE, Don't enable yet
    T2CON = 0x2000; // Don't prescale
    // Assume the period register PR2 will be set by the function using Timer2

    // init the Timer2 Interrupt control bits
    _T2IF = 0; // clear the interrupt flag, this can be used to see if Timer2 got to PR2
    _T2IE = 0; // disable the T2 interrupt source
}

void initAdc(void); // forward declaration of init adc

/*********************************************************************
 * Function: initialization()
 * Input: None
 * Output: None
 * Overview: configures chip to work in our system (when power is turned on, these are set once)
 * Note: Pic Dependent
 * TestDate: 06-03-14
 ********************************************************************/
void initialization(void) {
    //IO port control
    ANSA = 0; // Make PORTA digital I/O
    TRISA = 0xFFFF; // Make PORTA all inputs
    ANSB = 0; // All port B pins are digital. Individual ADC are set in the readADC function
    TRISB = 0x0DD0; //0x0DD0 no debug and 0x0DC0 if pin11 is output; // Set LCD outputs as outputs
    // DEBUG TRISBbits.TRISB4 = 1;
 //old//   // Timer control (for WPS)
 //old//   T1CONbits.TCS = 0; // Source is Internal Clock (8MHz)
 //old//   T1CONbits.TCKPS = 0b11; // Prescalar to 1:256
 //old//   T1CONbits.TON = 1; // Enable the timer (timer 1 is used for the water sensor)

    //H2O sensor config
    // WPS_ON/OFF pin 2
    TRISAbits.TRISA0 = 0; //makes water presence sensor pin an output.
    PORTAbits.RA0 = 1; //turns on the water presence sensor.

    CNEN2bits.CN24IE = 1; // enable interrupt for pin 15
    IEC1bits.CNIE = 1; // enable change notification interrupt

    initAdc();
    ConfigTimerT2NoInt();    // used by readWaterSensor to time the WPS_OUT pulse
}

/*********************************************************************
 * Function: readWaterSensor
 * Input: None
 * Output: pulseWidth indicates Water present True/False (sqaure wave too slow)
 * Overview: RB8 is water sensor 555 square wave
 * Note: Pic Dependent
 * TestDate: Not tested as of 03-05-2015 ?????????????????
 ********************************************************************/
int readWaterSensor(void) // RB8 is one water sensor
{
    int WaterPresent = true;  // initialize WPS variable
    
    // System clock is 500khz clock  500khz/2 = 250khz => 4us period
    // Choose 2khz as min threshold for water present
    //     so High or Low last no longer than 250us
    // PR2 250us/4us = 62.5
    PR2 = 63;
    TMR2 = 0;  //Clear Timer2
    _T2IF = 0; //Clear Timer2 Interrupt Flag
    T2CONbits.TON = 1;  //turn on Timer2
    
    //Must get through both a HIGH and a LOW of short enough duration
    //to be considered the frequency (2khz min) representing water is present
    while(PORTBbits.RB8 && WaterPresent){
        if(_T2IF){
            WaterPresent=false;
            _T2IF = 0;
        } //high too long, no water
    }
    TMR2 = 0;  //Restart Timer2
    while(!PORTBbits.RB8 && WaterPresent){
        if(_T2IF){
            WaterPresent=false;
            _T2IF = 0;
        } //low too long, no water
    }
    TMR2 = 0;  //Restart Timer2
    while(PORTBbits.RB8 && WaterPresent){
        if(_T2IF){
            WaterPresent=false;
            _T2IF=0;
        } //high too long, no water
    }
    T2CONbits.TON = 0;  //turn off Timer2

    //WaterPresent variable is high if the freq detected is fast enough (PR2 value)
    return (WaterPresent);
    

}

/*********************************************************************
 * Function: initAdc()
 * Input: None
 * Output: None
 * Overview: Initializes Analog to Digital Converter
 * Note: Pic Dependent
 * TestDate: 06-02-2014
 ********************************************************************/
void initAdc(void) 
{
    // ??? The PumpMinder system does not use the ADC????
    // 10bit conversion
    AD1CON1 = 0; // Default to all 0s
    AD1CON1bits.ADON = 0; // Ensure the ADC is turned off before configuration
    AD1CON1bits.FORM = 0; // absolute decimal result, unsigned, right-justified
    AD1CON1bits.SSRC = 0; // The SAMP bit must be cleared by software
    AD1CON1bits.SSRC = 0x7; // The SAMP bit is cleared after SAMC number (see
    // AD3CON) of TAD clocks after SAMP bit being set
    AD1CON1bits.ASAM = 0; // Sampling begins when the SAMP bit is manually set
    AD1CON1bits.SAMP = 0; // Don't Sample yet
    // Leave AD1CON2 at defaults
    // Vref High = Vcc Vref Low = Vss
    // Use AD1CHS (see below) to select which channel to convert, don't
    // scan based upon AD1CSSL
    AD1CON2 = 0;
    // AD3CON
    // This device needs a minimum of Tad = 600ns.
    // If Tcy is actually 1/8Mhz = 125ns, so we are using 3Tcy
    //AD1CON3 = 0x1F02; // Sample time = 31 Tad, Tad = 3Tcy
    AD1CON3bits.SAMC = 0x1F; // Sample time = 31 Tad (11.6us charge time)
    AD1CON3bits.ADCS = 0x2; // Tad = 3Tcy
    // Conversions are routed through MuxA by default in AD1CON2
    AD1CHSbits.CH0NA = 0; // Use Vss as the conversion reference
    AD1CSSL = 0; // No inputs specified since we are not in SCAN mode
    // AD1CON2
}

/*********************************************************************
 * Function: readAdc()
 * Input: channel
 * Output: adcValue
 * Overview: check with accelerometer
 * Note: Pic Dependent
 * TestDate:
 ********************************************************************/
int readAdc(int channel) //check with accelerometer
{
    // ??? The PumpMinder system does not use the ADC????
    switch (channel) 
    {
        case 4:
            ANSBbits.ANSB2 = 1; // AN4 is analog
            TRISBbits.TRISB2 = 1; // AN4 is an input
            AD1CHSbits.CH0SA = 4; // Connect AN4 as the S/H input
            break;
    }
    AD1CON1bits.ADON = 1; // Turn on ADC
    AD1CON1bits.SAMP = 1;
    while (!AD1CON1bits.DONE) 
    {
    }
    // Turn off the ADC, to conserve power
    AD1CON1bits.ADON = 0;
    return ADC1BUF0;
}

void __attribute__((__interrupt__, __auto_psv__)) _DefaultInterrupt() 
{ 
    // We should never be here
}

void __attribute__((interrupt, auto_psv)) _CNInterrupt(void) 
{
    if (IFS1bits.CNIF && PORTBbits.RB6) 
    { // If the button is pushed and we're in the right ISR
        buttonFlag = 1;
    }

    // Always reset the interrupt flag
    IFS1bits.CNIF = 0;
}


void hoursToAsciiDisplay(int hours, int tickCounter, int ticksperhour)
{
    int startLcdView = 0;
    DisplayTurnOff();
    unsigned char aryPtr[] = "H: ";
    DisplayDataAddString(aryPtr, sizeof ("H: "));
 
// First display the number of whole hours
    if (hours == 0)
    {
        DisplayDataAddCharacter(48);  //display a 0
    } 
    else 
    {
        if (startLcdView || (hours / 10000 != 0)) 
        {
            DisplayDataAddCharacter(hours / 10000 + 48);
            startLcdView = 1;
            hours = hours - ((hours / 10000) * 10000); // moving the decimal point - taking advantage of int rounding
        }
        
        if (startLcdView || hours / 1000 != 0) 
        {
            DisplayDataAddCharacter(hours / 1000 + 48);
            startLcdView = 1;
            hours = hours - ((hours / 1000) * 1000);
        }
        
        if (startLcdView || hours / 100 != 0) 
        {
            DisplayDataAddCharacter(hours / 100 + 48);
            startLcdView = 1;
            hours = hours - ((hours / 100) * 100);
        }
        
        if (startLcdView || hours / 10 != 0) 
        {
            DisplayDataAddCharacter(hours / 10 + 48);
            startLcdView = 1;
            hours = hours - ((hours / 10) * 10);
        }
        
        DisplayDataAddCharacter(hours + 48);
    }
    // Now display the fractional hours
    int decimalHour = ((float)tickCounter/ticksperhour)*1000;

    DisplayDataAddCharacter('.');
    startLcdView = 0;
    
    if (decimalHour == 0) 
    {
        DisplayDataAddCharacter(48);
    } 
    else 
    {   
        if (startLcdView || decimalHour / 100 != 0) 
        {
            DisplayDataAddCharacter(decimalHour / 100 + 48);
            startLcdView = 1;
            decimalHour = decimalHour - ((decimalHour / 100) * 100);
        }
        else
        {
            DisplayDataAddCharacter(48);
        }
        
        if (startLcdView || decimalHour / 10 != 0) 
        {
            DisplayDataAddCharacter(decimalHour / 10 + 48);
            startLcdView = 1;
            decimalHour = decimalHour - ((decimalHour / 10) * 10);
        }
        else
        {
            DisplayDataAddCharacter(48);
        }
        
        DisplayDataAddCharacter(decimalHour + 48);
    }

    DisplayLoop(15, true);
}

static int countdownPos = 0;
const unsigned char countdownArray[] = { '5', '5', '4', '4', '3', '3', '2', '2', '1', '1', '0', '0' };
const unsigned char countdownResetArray[] = "Reset In ";
static void DisplayCountdown(void)
{
    DisplayTurnOff();
    DisplayDataAddString((unsigned char *)&countdownResetArray, sizeof(countdownResetArray));    
    DisplayDataAddCharacter(countdownArray[countdownPos++]);
    DisplayLoop(15, true);
}

static void ResetDisplayCountdown(void)
{
    countdownPos = 0;
    buttonTicks = 0;
}

#define delayTime                   500  // this is in ms
#define msHr                        (uint32_t)3600000
#define hourTicks                   (msHr / delayTime)
#define BUTTON_TICK_COUNTDOWN_THRESHOLD          5
#define BUTTON_TICK_RESET_THRESHOLD              16  //shipped May 2016 as 10


int main(void)
{   
    resetCheckRemedy();

    initialization();

    DisplayInit();
    ConfigTimerT1WithInt(delayTime); // used to wake up from SLEEP every delayTime milli-seconds
   
    uint16_t tickCounter = 0;
    uint16_t hourCounter = 0;
    
    while (1) 
    {
      Sleep(); //sleep until Timer1 wakes us up every delayTime ms
   //   PORTBbits.RB4 = 1;  //debug start of tick time
    
       if (readWaterSensor())
        {
            tickCounter++;          //keeps track of fractional hours pumped

            if (tickCounter >= hourTicks) 
            {
                hourCounter++;      //keeps track of integer hours pumped
                tickCounter = 0;
            }
        }
   //    PORTBbits.RB4 = 0;  //debug end of tick time
       if(isButtonTicking)
        {
            if(PORTBbits.RB6) //button is pressed
            {
               buttonTicks++; 
               if(buttonTicks > BUTTON_TICK_COUNTDOWN_THRESHOLD) //Start Reset Countdown
               {
                   DisplayCountdown();
               }
               if(buttonTicks > BUTTON_TICK_RESET_THRESHOLD) //Reset the system
               {
                   tickCounter = 0;
                   hourCounter = 0;
                   isButtonTicking = false;
                   ResetDisplayCountdown();
                   DisplayTurnOff();
               }
            }
            else
            {
                isButtonTicking = false;
                ResetDisplayCountdown();
                DisplayTurnOff();
                DisplayLoop(1, true);
            }
        }
       if (buttonFlag)
        { // If someone pushed the button
           buttonFlag = 0;
           hoursToAsciiDisplay(hourCounter, tickCounter, hourTicks);
           
           isButtonTicking = true;
        }
   
    }

    return -1;
}
