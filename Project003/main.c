/*! \file */
/******************************************************************************
 * File Name: ece230_2122s_project3MalipeddiR.c
 * Author : Rohan Malipeddi
 * Last Modified: 03/22/24
 *
 * Description: Using timers and compare/capture modules to play a song. Device configuration
 *               to enable operation at MCLK=48MHz using HFXT source.
 *
 * An external HF crystal between HFXIN & HFXOUT is required for MCLK,SMCLK
 * An external LF crystal between
 *
 *                MSP432P411x
 *             -----------------
 *         /|\|                 |
 *          | |                 |
 *          --|RST         P1.4 |<-- S2
 *            |            P2.4 |--> CCR1 - speaker
 *            |                 |
 *            |            PJ.2 |------
 *            |                 |     |
 *            |                 |    HFXT @ 6MHz
 *            |                 |     |
 *            |            PJ.3 |------
 *            |            PJ.0 |------
 *            |                 |     |
 *            |                 |    LFXT @ 32 kHz
 *            |                 |     |
 *            |            PJ.1 |------
 *
*******************************************************************************/
#include "msp.h"

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include "csHFXT.h"
#include "csLFXt.h"
#include "switches.h"
#include "rgbLED.h"

/* TODO add note defines */
#define MaxNoteIndex 47
// Note A4 - 440 Hz, B4 - 493.88 Hz, C5 - 523.26 Hz, G3 - 196 Hz, G4 - 392 Hz
#define REST    0
#define NOTEG3  30612
#define NOTEC4  22933
#define NOTED4  20432
#define NOTEE4  18202
#define NOTEF4  17180
#define NOTEG4  15307

/* same index for the noteArray and noteDurationArray */
int noteIndex = 0;

/* Actual song in note Array */
// I am playing itsy bitsy spider first
const uint8_t noteArray[48] = {0, 1, 1, 1, 2, 3, 3, 3, 2, 1, 2, 3, 2, 3, 3, 4, 5, 5, 4, 3, 4, 5, 3, 1,
                              1, 2, 3, 3, 2, 1, 2, 3, 1, 0, 0, 1, 1, 1, 2, 3, 3, 3, 2, 1, 2, 3, 1, 6};

/* Contains the actual frequencies of notes, ex. A4, B4, C5 */
const uint16_t notePeriodArray[7] = {NOTEG3, NOTEC4, NOTED4, NOTEE4, NOTEF4, NOTEG4, REST};

/* Contains the numbers of times the current note should play, same length of the noteArray*/
const uint8_t noteDurationArray[48] = {1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 4, 2, 1, 1, 3, 1, 1, 1, 1, 1, 4, 2,
                                      1, 1, 3, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 7, 1} ;

// function declaration
void delay5(void);

/**
 * main.c
 */
void main(void)
{
    /* Stop Watchdog timer */
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

    /* Sets MCLK and SMCLK to 48MHz HFXT */
    /* Sets ACLK to 32 kHz LFXT */
    configLFXT();
    configHFXT();
    SWITCHES_init();
    RGBLED_init();

    /* Configure GPIO for speaker */
    P2->DIR |= BIT4;            // set P2.4 as output
    P2->SEL0 |= BIT4;           // P2.4 set to TA0.1
    P2->SEL1 &= ~BIT4;

    /* Timer_A0 will be used to play individual notes in the song */
    /* Timer_A0 CCR0 controls the speaker */

    // Configure Timer_A0 in Up Mode with source SMCLK prescale 8:1
    //      Tick rate will be (48MHz/8) with rollover at value in CCR0
    TIMER_A0->CTL = 0x2D4;

    // Configure CCR0 for Compare mode with output mode 0
    TIMER_A0->CCTL[0] = 0x0;

    /* Configure Timer_A0 and CCRs */
    // Set Period in CCR0 register
    TIMER_A0->CCR[0] = notePeriodArray[0];

    // Configure CCR0 for Compare mode with output mode 0
    TIMER_A0->CCTL[1] = 0x0;

    // Set high pulse-width in CCR1 register (determines duty cycle)
    TIMER_A0->CCR[1] = notePeriodArray[0]/2;

    /* Timer_A1 will be used to interrupt Timer_A0 to stop playing the note
     *  after the note duration which will be in CCR1 trigger a flag.
     *  Then there will be a pause for at least 100ms and not more than
     *  duration of the shortest note. --> Rest is constant
     */
    // Configure Timer_A1 in Stop Mode with source ACLK prescale 1:1 and
    //  interrupt enabled
    //      Tick rate will be 32kHz with rollover at 0xFFFF
    TIMER_A1->CTL = 0x106; // up mode: 0x116 stop mode: 0x106

    // Configure CCR1 for Compare mode with interrupt enabled (no output mode - 0)
    TIMER_A1->CCTL[1] = 0x10;

    /* Timer_A1 CCR1 will be used for resting for at least 100ms */
    TIMER_A1->CCR[1] = 0xD00;

    // Configure CCR0 for Compare mode with interrupt enabled (no output mode - 0)
    TIMER_A1->CCTL[0] = 0x10;

    /* Timer_A1 CCR0 will be used for running the note some time */
    TIMER_A1->CCR[0] = noteDurationArray[0]*0x1FFF + 0xD00; // add the 100ms, because the note is still playing, but its not making

    /* Configure global interrupts and NVIC */
    // Enable TA1 overflow and TA1CCR1 compare interrupt
    //NVIC->ISER[0] = 1 << (TA1_0_IRQn);
    NVIC->ISER[0] = 1 << (TA1_N_IRQn);   // which interrupt do we enable

    __enable_irq(); //enabling the global interrupt

    /* TA0State to 0 first, because its in stop mode initially*/
    int TA0_State = 0x0;

    while(1)
    {
        if(!(SWITCH_PORT->IN & SWITCH_2)) { //while switch 2 is not pressed
            delay5();
            while(!(SWITCH_PORT->IN & SWITCH_2)); //if s2 is released
            delay5();
            TIMER_A1->CTL |= TIMER_A_CTL_MC_1;  // set the Timer_A1 to up mode, starts counting
            TIMER_A0->CCTL[1] |= TA0_State; // setting outmode for timer A0 which is sometimes off and sometimes set/reset


            while(SWITCH_PORT->IN & SWITCH_2); //if s2 is pressed again
            TA0_State = TIMER_A0->CCTL[1] & TIMER_A_CCTLN_OUTMOD_MASK;  // anded with mask to remember what mode its in, set/rest or outmode 0
            TIMER_A0->CCTL[1] &= ~TIMER_A_CCTLN_OUTMOD_MASK; //turning the speaker off, by setting outmode 0
            TIMER_A1->CTL &= ~0x30;  //stopping timer_A1, so it starts where it left off

            while(!(SWITCH_PORT->IN & SWITCH_2)); // checks if s2 is released or not
            delay5();
        }
    }

}

/*!
 * \brief This function just delays for 5 milli seconds
 *
 * \return none
 */
void delay5(void) {
    SysTick->CTRL |= 0x5;
    SysTick->LOAD |= 0x3A981;
    while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
}

/*
 * \brief turns all the LEDs off
 *
 * \return none
 */
void toggleLEDoff() {
    RGB_PORT->OUT &= 0xFFFFFFF8;    // clear the last 3 bits
    //isOn = false;
}

/*
 * \brief turns all the LEDs off
 *
 * \return none
 */
void toggleCurrentLED() {
    RGB_PORT->OUT ^= 1;
}


/**
 * Interrupt handler for Timer_A1 which resets the flag and increments the note index
 */
void TA1_N_IRQHandler() {
    if(TIMER_A1->CCTL[1] & TIMER_A_CCTLN_CCIFG) {
        //for testing purposes
        toggleCurrentLED(); //shows my song is working
        TIMER_A0->CCTL[1] |= TIMER_A_CCTLN_OUTMOD_3;  //setting the Timer_A0 mode to set and reset
        TIMER_A1->CCTL[1] &= ~TIMER_A_CCTLN_CCIFG;  //clearing the flag

    }
    if(TIMER_A1->CTL & TIMER_A_CTL_IFG) {
        //for testing purposes
        toggleLEDoff(); //shows my song is working
        TIMER_A0->CCTL[1] &= ~TIMER_A_CCTLN_OUTMOD_MASK;    //setting to outmode 0 so it doesn't play any output

        noteIndex = (noteIndex + 1) % MaxNoteIndex; //incrementing the noteIndex
        uint8_t note = noteArray[noteIndex]; //getting the note that it should play
        TIMER_A1->CCR[0] = noteDurationArray[noteIndex]*0x1FFF + 0xD00; //each beat is multiplied by 250ms and add 100ms for rest period
        TIMER_A0->CCR[0] = notePeriodArray[note];  //getting the value for note selected for setting the output to high
        TIMER_A0->CCR[1] = notePeriodArray[note]/2; //getting the value for note selected/2 for reseting the output

        TIMER_A1->CTL &= ~TIMER_A_CTL_IFG; //clearing the flag of CTL register

    }
}

