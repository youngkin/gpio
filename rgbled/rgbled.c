// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file.
//
// Credit for the majority of this code goes to Sunfounder,
// https://docs.sunfounder.com/projects/raphael-kit/en/latest/1.1.2_rgb_led_c.html
//
// Build - gcc -o rgbled rgbled.c  -lwiringPi -lpthread
// Run - ./rgbled
// 
#include <wiringPi.h>
#include <softPwm.h>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#define uchar unsigned char
#define LedPinRed    0
#define LedPinGreen  1
#define LedPinBlue   2

static volatile int keepRunning = 1;

void interruptHandler(int);

void ledInit(void){
    softPwmCreate(LedPinRed,  0, 0xff);
    softPwmCreate(LedPinGreen,0, 0xff);
    softPwmCreate(LedPinBlue, 0, 0xff);
}

void ledColorSet(uchar r_val, uchar g_val, uchar b_val) {
    softPwmWrite(LedPinRed,   r_val);
    softPwmWrite(LedPinGreen, g_val);
    softPwmWrite(LedPinBlue,  b_val);
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
            ledColorSet(0xff,0,0);   
            delay(1000);

            printf("Green\n");
            ledColorSet(0,0x32,0);   
            delay(1000);

            printf("Blue\n");
            ledColorSet(0x00,0x00,0xff);   
            delay(1000);

            printf("Yellow\n");
            ledColorSet(0xff,0x32,0x00);  
            delay(1000);

            printf("Purple\n");
            ledColorSet(0xff,0x00,0xff); 
            delay(1000);

            printf("Cyan\n");
            ledColorSet(0xc0,0x32,0xff);
            delay(1000);

            printf("Off\n");
            ledColorSet(0,0,0);
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
