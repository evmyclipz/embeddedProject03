/*
 * switches.c
 *
 * Description: switch driver for switch on MSP432P4111 Launchpad
 *
 *  Created on: Mar 22, 2024
 *      Author: Rohan Malipeddi
 */

#include "msp.h"
#include "switches.h"
#include <stdio.h>

/**
 * \brief function that initializing the switches used
 *
 * \return none
 */
void SWITCHES_init(void) {
    // sets s2 switch as GPIO input
    SWITCH_PORT->SEL0 &= ~(SWITCH_ALL_PINS);
    SWITCH_PORT->SEL1 &= ~(SWITCH_ALL_PINS);
    SWITCH_PORT->DIR  &= ~(SWITCH_ALL_PINS);

    // enable internal pull up resister in switch
    SWITCH_PORT->OUT |= SWITCH_ALL_PINS;
    SWITCH_PORT->REN |= SWITCH_ALL_PINS;
}

