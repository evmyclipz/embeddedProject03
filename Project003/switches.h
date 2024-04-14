/*
 * switched.h
 *
 *  Created on: Mar 22, 2024
 *      Author: Rohan Malipeddi
 *
 *  Description:
 *  A header file that declare the initialization of switched used in Project 2
 */

#ifndef SWITCHES_H_
#define SWITCHES_H_

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

#include "msp.h"

#define SWITCH_PORT                 P1
#define SWITCH_2                    (0x0010) // Original Value:0x0010 test value:0x0040
#define SWITCH_ALL_PINS             (0x0010) // Original Value:0x0010 test value:0x0040

/*!
 * \brief This function configures Switches 1 and 2 pins as output pins
 *
 * This function configures P1.1 and P1.4 as input pins with internal pull-up enabled
 *
 * This function configures P1.5 and P1.6 as input pins with internal pull-up enabled for testing
 *
 * Modified bits 1 and 4 of \b P1DIR register, \b P1SEL registers,
 *  \b P1OUT register and \b P1REN register
 *
 * \return None
 */
extern void SWITCHES_init(void);

#endif /* SWITCHES_H_ */
