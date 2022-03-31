// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file.
//
// Original bcm_* functions and variable code by Mike McCauley at https://www.airspayce.com/mikem/bcm2835/index.html.
//
// NOTE: There is inadequate error handling in the application. Take care when copying.
//
// References:
//  1. https://www.airspayce.com/mikem/bcm2835/index.html - BCM2835 library documentation
//
// Build: make blinkinglednolib
//
#include "bcmfuncs.h"
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#define LEDPIN 17 // BCM GPIO pin 17

/* Gracefully handle interrupts, releases all resources prior to exiting */
void interruptHandler(int);

int main(void)
{
    signal(SIGINT, interruptHandler);

    if (!bcm_init())
    {
        printf("Unable to init GPIO.\n");
        return 1;

    }

    // Set LEDPIN to output mode so it can be written to.
    bcm_gpio_fsel(LEDPIN, BCM_GPIO_FSEL_OUTP);

    // Blink LED twice/second
    while(1) {
        // Setting the GPIO pin to LOW allows current to flow from the power source thru
        // the anode to cathode turning on the LED.
        bcm_gpio_write(LEDPIN,LOW); // LED on
        bcm_delay(500);

        bcm_gpio_write(LEDPIN,HIGH); // LED off
        bcm_delay(500);
    }

    // Release resources
    bcm_close();

    return 0;
}

// interruptHandler catches SIGINT when ctl-C is pressed in order to halt the program gracefully.
void interruptHandler(int sig) {
    bcm_gpio_write(LEDPIN,HIGH); // LED off
    bcm_close(); // Release resources

    printf("\nExiting...\n");
    exit(0);
}

