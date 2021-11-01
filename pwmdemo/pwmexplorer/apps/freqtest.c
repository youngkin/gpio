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
// pulsewidth and the range. 
//
// Given a PWM clock frequency of 100,000 and a range length of 25,000, the LED frequency is 4Hz,
// that is it blinks 4 times per second. If the range length is changed to 400,000, the
// LED frequency is 0.25Hz, or once every 4 seconds. And finally, if the range length is 1,000
// the LED frequency is 100Hz. At 100Hz, the LED will appear to be continuously on, i.e., not
// blinking.
//
// NOTE: There is inadequate error handling in the application. Take care when copying.
// 
// Build: gcc -o freqtest freqtest.c  -lwiringPi -lpthread
// Run: sudo apps/freqtest --pin=<pwmPinNo> --divisor=<2 to 4095> --range=<n> --pulsewidth=<n> --type=<hardware|software> --mode=<balanced|markspace>
// E.g., sudo apps/freqtest --pin=1 --divisor=192 --range=1000 --pulsewidth=50 --type=hardware --mode=balanced
// E.g., or just sudo apps/freqtest if defaults are acceptable
// 
#include <wiringPi.h>
#include <softPwm.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
 
// define magic values
int globalPin = 0;
char* markspace = "markspace";
char* balanced = "balanced";
char* hardware = "hardware";
char* software = "software";
int   OFF      = 0;
int   ON     = 1;

// Define interrupt handlers
void softwareInterruptHandler(int);
void hardwareInterruptHandler(int);

// Define functions to run hardware or software PWM
void runHardwarePWM(char* pwmMode, int divisor, int range, int pin, int pulsewidth); 
void runSoftwarePWM(int pin, int range, int pulsewidth); 

void printUsage() {
    printf("\nUsage:\n");
    printf("\t--help (prints this message)\n");
    printf("\t--type=[software|hardware]\n");
    printf("\t--mode=[markspace|balanced]\n");
    printf("\t--divisor=[2 to 4095]\n");
    printf("\t--range=[n]\n");
    printf("\t--pulsewidth=[0 to 'range']\n");
    printf("\t--pin=[7|21|22|26|23|27|0|1|24|28|29|3|4|5|6|25|2]\n");
    printf("\t\tHardware PWM pins are 0, 1, 23, and 24\n");
}


int main(int argc, char *argv[]) {
    // set default values for options  
    char *pwmType = hardware;
    char *pwmMode = balanced;
    int divisor = 192;
    int range = 1000;
    int pin = 1;
    int pulsewidth = 50;
                                                                     
    int c;
    int help_flag;

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

    // Get command line options
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

    // Set up the WiringPi library, obtain resources, etc
    if (wiringPiSetup() == -1) {
        printf("setup wiringPi failed!");
        return 1;
    }

    if (strcmp(pwmType, hardware) == 0) {
        // register signal handler to catch ctl-C used to end the program for hardware PWM
        signal(SIGINT, hardwareInterruptHandler);
        runHardwarePWM(pwmMode, divisor, range, pin, pulsewidth);
    } else {
        // register interrupt handler to catch ctl-C used to end the program for software PWM
        signal(SIGINT, softwareInterruptHandler);
        runSoftwarePWM(pin, range, pulsewidth);
    }
}

// softwareInterruptHandler catches SIGINT when ctl-C is pressed in order to halt the program gracefully.
void softwareInterruptHandler(int sig) {
    // Had to switch to OUTPUT explicitly instead of using 'softPwmWrite()' as that wasn't reliable.
    pinMode(globalPin, OUTPUT);
    digitalWrite(globalPin, LOW);
    
    printf("\nExiting...\n");
    exit(0);
}

// hardwareInterruptHandler catches SIGINT when ctl-C is pressed in order to halt the program gracefully.
void hardwareInterruptHandler(int sig) {
    // Turn off LED
    pinMode(globalPin, PWM_OUTPUT);
    pwmWrite(globalPin, OFF);

    printf("\nExiting...\n");

    exit(0);
}

// runHardwarePWM starts PWM on a hardware PWM pin
void runHardwarePWM(char* pwmMode, int divisor, int range, int pin, int pulsewidth) {
    pinMode(pin, PWM_OUTPUT);
    pwmSetRange(range);
    if (strcmp(pwmMode, balanced) == 0) {
        pwmSetMode(PWM_MODE_BAL);
    } else {
        pwmSetMode(PWM_MODE_MS);
    }
    pwmSetClock(divisor);
    pwmWrite(pin, pulsewidth);
    while (1) {
        delayMicroseconds(1000);
    }
}

// runSoftwarePWM starts PWM implemented by this function vs. via the board hardware.
// NOTE: WiringPi directly implements software PWM. The PWM runs at 100MHz.
void runSoftwarePWM(int pin, int range, int pulsewidth) {
    softPwmCreate(pin, 0, range);
    softPwmWrite(pin, pulsewidth);
    while (1) {
        delayMicroseconds(1000);
    }
}
