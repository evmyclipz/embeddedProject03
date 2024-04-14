/*! \file */
/*!
 * rgbLED.c
 *
 * Description: RGB driver for LED2 on MSP432P4111 Launchpad
 *
 *  Created on: 03/26/24
 *      Author: Rohan Malipeddi
 */

#include "msp.h"
#include "rgbLED.h"
#include <stdio.h>


void RGBLED_init(void) {
    // TODO set LED2 pins as GPIO outputs
    RGB_PORT->SEL0 &= ~(RGB_ALL_PINS);
    RGB_PORT->SEL1 &= ~(RGB_ALL_PINS);
    RGB_PORT->DIR  |= RGB_ALL_PINS;

    // TODO set LED2 outputs to LOW
    RGB_PORT->OUT &= ~(RGB_ALL_PINS);
}

void RGBLED_toggleRed(void) {
    // XOR bit 0 of Port 2 for red LED
    RGB_PORT->OUT ^= RGB_RED_PIN;
}

void RGBLED_toggleGreen(void) {
    // TODO toggle bit 1 of Port 2 OUT register
    RGB_PORT->OUT ^= RGB_GREEN_PIN;
}

void RGBLED_toggleBlue(void) {
    // TODO toggle bit 2 of Port 2 OUT register
    RGB_PORT->OUT ^= RGB_BLUE_PIN;
}

void RGBLED_setColor(Colors color) {
    // TODO set the R, G, and B output values based on color value
    if(color > 7) {
        RGB_PORT->OUT &= ~(RGB_ALL_PINS);
        printf("Invalid color");
        return;
    }
    RGB_PORT->OUT &= ~(RGB_ALL_PINS);
    RGB_PORT->OUT |= color;
}
