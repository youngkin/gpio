//
// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file.
//
// This program visually demonstrates the association between the wiringPi functions
// 'pwmSetClock()', 'pwmSetRange()' and 'pwmWrite()'. 
//
// 'pwmSetClock' sets the divisor used to scale back the BCM2835 clock input to
// the desired PWM clock frequency. NOTE: The WiringPi library limits the parameter
// to 'pwmSetClock()', called the divisor, to 4095. It uses '&=' to accomplish this,
// which means a value of 4096 equates to a divisor of 0 and 4097 equates to a divisor
// 1, and so on. 'pwmSetRange()' sets the range for the PWM duty cycle. 'pwmWrite()' 
// sets the pulsewidth for the PWM duty cycle. Duty cycle is the ratio between the
// pulsewidth and the range. So a pulsewidth of 5 and a range of 10 would have a duty
// cycle of 0.5 or 50%.
//
// The relationship demonstrated by this program is more specifically the relationship
// between the PWM clock frequency and the range (pwmSetClock() and pwmSetRange()). This
// ratio sets the frequency for the LED.
//
// Given a PWM clock frequency of 100,000 and a range value of 25,000, the LED Hz is 4,
// that is it blinks 4 times per second. If the range value is changed to 400,000, the
// LED Hz is 0.25, or once every 4 seconds. And finally, if the range value is 1,000
// the LED Hz is 100. At 100Hz, the LED will appear to be continuously on, i.e., not
// blinking. Lower values for pulse width will result in a dimmer LED, higher values,
// up to and including the range value, will make the LED brighter.
//
// Build: gcc -o freqtest freqtest.c  -lwiringPi -lpthread
// Run: sudo apps/freqtest --pin=<pwmPinNo> --divisor=<2 to 4095> --range=<n> --pulsewidth=<n> --type=<hardware|software> --mode=<Balanced|Mark/Space>
// E.g., sudo apps/freqtest --pin=1 --divisor=192 --range=1000 --pulsewidth=50 --type=hardware --mode=Balanced
// 
#include <wiringPi.h>
#include <softPwm.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
 
int globalPin = 0;

void interruptHandler(int);
void runPWM(char*, char*, int, int, int, int);

void printUsage() {
    printf("\nUsage:\n");
    printf("\t--help (prints this message)\n");
    printf("\t--type=[software|hardware]\n");
    printf("\t--mode=[Mark/Space|Balanced]\n");
    printf("\t--divisor=[2 to 4095]\n");
    printf("\t--range=[n]\n");
    printf("\t--pulsewidth=[0 to 'range']\n");
    printf("\t--pin=[7|21|22|26|23|27|0|1|24|28|29|3|4|5|6|25|2]\n");
    printf("\t\tHardware PWM pins are 0, 1, 23, and 24\n");
}


int main(int argc, char *argv[]) {
    char *pwmType = "hardware";
    char *pwmMode = "Balanced";
    int divisor = 192;
    int range = 1000;
    int pin = 1;
    int pulsewidth = 50;
                                                                     
    int c;
    int help_flag;

    signal(SIGINT, interruptHandler);

    if (wiringPiSetup() == -1) {
        printf("setup wiringPi failed!");
        return 1;
    }

    static struct option long_options[] =
    {
        {"help",        no_argument,        0,  'h'},
        {"type",        optional_argument,  0,  't'},
        {"mode",        optional_argument,  0,  'm'},
        {"divisor",     optional_argument,  0,  'd'},
        {"range",       optional_argument,  0,  'r'},
        {"pin",         optional_argument,  0,  'p'},
        {"pulsewidth",  optional_argument,  0,  'w'},
         {0, 0, 0, 0}
    };
    /* getopt_long stores the option index here. */
    int option_index = 0;

    while (1)
    {
            c = getopt_long (argc, argv, "h::t::m::d::c::p::w::",
                             long_options, &option_index);
                         
            /* Detect the end of the options. */
            if (c == -1)
                break;
                               
            switch (c)
             {
             case 'h':
                 if (help_flag) {
                     printUsage();
                     exit(0);
                 }  
             case 't':
                 pwmType = optarg;
                 break;
             case 'm':
                pwmMode = optarg;
                break;
              case 'd':
                divisor = atoi(optarg);
                break;
              case 'r':
                range = atoi(optarg);
                break;
              case 'p':
                pin = atoi(optarg);
                globalPin = pin;
                break;
              case 'w':
                pulsewidth = atoi(optarg);
                break;
              case '?':
                 /* getopt_long already printed an error message. */
                 break;
              default:
                //printUsage();
                //exit(0);
                printf("Using default values\n");
              }
    }

    printf("Using: PWM pin: %d, PWM Type %s:, PWM Mode: %s, divisor: %d, range: %d, pulsewidth: %d\n", 
            pin, pwmType, pwmMode, divisor, range, pulsewidth);
    runPWM(pwmType, pwmMode, divisor, range, pin, pulsewidth);
}

void interruptHandler(int sig) {
    // Turn off LED
    pinMode(globalPin, PWM_OUTPUT);
    pwmWrite(globalPin, 0);

    printf("\nExiting...\n");

    exit(0);
}

void runPWM(char* pwmType, char* pwmMode, int divisor, int range, int pin, int pulsewidth) {
    pinMode(pin, PWM_OUTPUT);
    pwmSetRange(range);
    if (strcmp(pwmMode, "Balanced") == 0) {
        pwmSetMode(PWM_MODE_BAL);
    } else {
        pwmSetMode(PWM_MODE_MS);
    }
    pwmSetClock(divisor);
    pwmWrite(pin, pulsewidth);
    while (1) {
//        sleep(1);
        delayMicroseconds(1000);
    }
}
