/*
 * Filename     : led.c
 * Desription   : Make an led light blink
 * Author       : Robot
 * Updates by   : Richard Youngkin
 * E-mail       : support@sunfounder.com
 * website      : www.sunfounder.com
 * Date         : 2014/08/27
 * Updated      : 2021/09/12
 * Build command: gcc blinkingled.c -o blinkingled -lwiringPi
 *
 */

#include <wiringPi.h>
#include <stdio.h>

#define LEDPIN  0

int main(void) {
    // Setup the WiringPi library. With no arguments, it initializes to use 
    // the WiringPi pin numbering scheme.
    if (wiringPiSetup() == -1) {
        printf("setup wiringPi failed!");
        return 1;
    }
    printf("LEDPIN: GPIO %d(wiringPi pin)\n", LEDPIN); // Success!

    // Set pin to output mode, i.e., so we can "write" to it/change its state 
    pinMode(LEDPIN, OUTPUT);

    // Comment for... and uncomment while... to run loop continuously
    for (int i = 0; i < 5; i++) {
    //while(1) {
        // Setting the GPIO pin to LOW allows current to flow from the power source thru 
        // the anode to cathode turning on the LED.
        digitalWrite(LEDPIN, LOW); // LED on
        printf("LED on, Pin Value: %d\n", digitalRead(LEDPIN));

        delay(500);

        digitalWrite(LEDPIN, HIGH); // LED off
        printf("...LED off, Pin Value: %d\n", digitalRead(LEDPIN));

        delay(500);
    }

    return 0;
}
