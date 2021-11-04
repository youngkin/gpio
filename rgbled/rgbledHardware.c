// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file.
//
// Build - gcc -o rgbledHardware rgbledHardware.c  -lwiringPi -lpthread
// 
#include <wiringPi.h>
#include <softPwm.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#define uchar unsigned char
#define LedPinRed    24
#define LedPinGreen  1
#define LedPinBlue   23

static volatile int keepRunning = 1;

void interruptHandler(int);

//
// Hardware PWM pins include WiringPi pins 1, 26, 23, 24 (BCM 18, 12, 13, and 19 respectively).
// For practical purposes only 2 are usable as the other 2 are linked, turn one on they both turn on.
// Pins 24 & 26 (BCM 19 & 12) and Pins 1 & 23 (BCM 13 and 18) are independent. Pins 1 and 26 (BCM 18 & 12)
// are linked. Set one and the other will also be set. Ditto for pins 23 & 24 (BCM 13 & 19). In addition,
// Writing to linked pins and one other results in inconsistent results. The independent pin will always
// be set, but the dependent pins won't always be set correctly.
// 
void ledInit(void) {
    pinMode(LedPinRed, PWM_OUTPUT);
    pinMode(LedPinGreen, PWM_OUTPUT);
    pinMode(LedPinBlue, PWM_OUTPUT);
    pwmSetRange(0xff);
    pwmSetClock(2);

    pwmWrite(LedPinRed, 0xff);
    pwmWrite(LedPinGreen, 0x32);
    pwmWrite(LedPinBlue, 0xff);

    delay(1000);
    printf("Initialization complete\n");
}

void ledColorSet(uchar r_val, uchar g_val, uchar b_val) {
    pwmWrite(LedPinRed, r_val);
    pwmWrite(LedPinGreen, g_val);
    pwmWrite(LedPinBlue, b_val);
}

int main(void) {
    if(wiringPiSetup() == -1){ //when initialize wiring failed, printf messageto screen 
        printf("setup wiringPi failed !");
        return 1;
    }

    signal(SIGINT, interruptHandler);
    printf("Hit ^-c to exit\n");

    ledInit();
    while(keepRunning) {
            printf("Red\n");
            ledColorSet(0xff,0x00,0x00);   //red
            delay(2000);
            printf("Green\n");
            ledColorSet(0x00,0x32,0x00);   //green
            delay(2000);
            printf("Blue\n");
            ledColorSet(0x00,0x00,0xff);   //blue
            delay(2000);
            printf("Yellow\n");
            ledColorSet(0xff,0x32,0x00);   //yellow
            delay(1000);
            printf("Purple\n");
            ledColorSet(0xff,0x00,0xff);   //purple
            delay(1000);
            printf("Cyan\n");
            ledColorSet(0xc0,0x32,0xff);   //cyan
            delay(1000);
    }
    return 0;
}

void interruptHandler(int sig) {
        // Turn off LED
        // Don't understand why, but ledColorSet(0,0,0); doesn't work
        pinMode(LedPinRed, OUTPUT);
        digitalWrite(LedPinRed, LOW);
        pinMode(LedPinGreen, OUTPUT);
        digitalWrite(LedPinGreen, LOW);
        pinMode(LedPinBlue, OUTPUT);
        digitalWrite(LedPinBlue, LOW);
   
        printf("\nExiting...\n");
        
        exit(0);
}
