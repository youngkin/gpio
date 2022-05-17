// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by the GPL 3.0
// license that can be found in the LICENSE file.
//
// Desription   : Reset RGB pins back to OUTPUT from PWM
// Build: gcc -I/home/pi/WiringPi/wiringPi -Wall -g -o resetrgb resetrgb.c -L/usr/local/lib -lwiringPi -lwiringPiDev -lpthread -lm -lcrypt -lrt
//

#include <wiringPi.h>
#include <stdio.h>

#define RED_PIN     24
#define GREEN_PIN   1
#define BLUE_PIN    23

int main(void) {
    if (wiringPiSetup() == -1) {
        printf("setup wiringPi failed!");
        return 1;
    }

    // Set pin to output mode, i.e., so we can "write" to it/change its state 
    pinMode(RED_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);

    digitalWrite(RED_PIN, LOW);
    digitalWrite(GREEN_PIN, LOW);
    digitalWrite(BLUE_PIN, LOW);

    return 0;
}
