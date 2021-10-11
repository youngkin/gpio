//
// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file.
//
// This program visually demostrates the association between the wiringPi functions
// 'pwmSetClock()', 'pwmSetRange()' and 'pwmWrite()'. 
//
// 'pwmSetClock' sets the divisor
// used to scale back the BCM2835 clock input and the desired PWM clock frequency.
// NOTE: The WiringPi library limits the parameter to 'SetClock()', called the divisor,
// to 4095. It uses '&=' to accomplish this, which means a value of 4096 equates to a
// divisor of 0 and 4097 equates to a divisor of 1, and so on.
// 'pwmSetRange()' sets the range for the PWM duty cycle. 'pwmWrite()' sets the
// pulseWidth for the PWM duty cycle. Duty cycle is the ratio between the pulseWidth
// and the range. So a pulseWidth of 5 and a range of 10 would have a duty cycle of
// 0.5 or 50%.
//
// The relationship demonstrated by this program is more specifically the relationship
// between the PWM clock frequency and the range (pwmSetClock() and pwmSetRange()). This
// ratio sets the frequency of the LED.

// Given a clock frequency of 100,000 and a cycle length of 25,000, the LED Hz is 4,
// that is it blinks 4 times per second. If the cycle length is changed to 400,000, the
// LED Hz is 0.25, or once every 4 seconds. And finally, if the cycle length is 1,000
// the LED Hz is 100. At 100Hz, the LED will appear to be continuously on, i.e., not
// blinking.
//
// The range of 4688 to 9,600,000 for the clock frequency was chosen because values outside
// that range caused the LED to exhibit inconsistent behavior. The program will enforce
// this range.
//
// The range of 4 to 38,400,000 for the cycle length is somewhat arbitrary. When the clock
// frequency is set to 9,600,000 and the cycle length is set to 38,400,000, the LED will
// blink at 0.25Hz, or once every 4 seconds. 4 was chosen as the lower bound because
// I arbitrarily chose that the duty cycle would be 25% of the cycle. For an LED this equates
// to 25% brightness. This is bright enough to be easily seen, but not so bright as to be
// uncomfortable to look at.
//
// BCM pin 18 is used as it is a hardware PWM pin. Any hardware PWM pin can be used if desired.
// The other hardware PWM pins are BCM pins 12, 13, and 19.
//
// Build: gcc -o freqtest freqtest.c  -lwiringPi -lpthread
// Run: sudo apps/freqtest --pin=<pinNo> --divisor=<4096 to 9600000> --cycle=<4 to 38400000> --pulseWidth=<n> --type=<hardware|software> --mode=<balanced|Mark/Space>
// E.g., sudo apps/freqtest --pin=1 --divisor=9600000 --cycle=10000 --pulseWidth=1000 --type=hardware --mode=balanced
// 
#include <wiringPi.h>
#include <softPwm.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
 
int globalPin = 0;

void interruptHandler(int);
void runPWM(char*, char*, int, int, int, int);

void printUsage() {
    printf("\nUsage:\n");
    printf("\t--type=[software|hardware]\n");
    printf("\t--mode=[markspace|balanced]\n");
    printf("\t--divisor=[2 to 9600000]\n");
    printf("\t--cycle=[2 to 38000000]\n");
    printf("\t--pulsewidth=[0 to 'cycle']\n");
    printf("\t--pin=[4|5|6|12|13|16|17|18|19|20|21|22|23|24|25|26|27]  (Go pins)\n");
    printf("\t\tFor C use pins 7,21,22,26,23,27,0,1,24,28,29,3,4,5,6,25,2\n");
}


int main(int argc, char *argv[]) {
    char *pwmType = NULL;
    char *pwmMode = NULL;
    int divisor = 0;
    int cycle = 0;
    int pin = 0;
    int pulseWidth = 0;
                                                                     
    int c;
    int help_flag;

    if (argc == 1) {
        printUsage();
        exit(1);
    }

    signal(SIGINT, interruptHandler);

    if (wiringPiSetup() == -1) {
        printf("setup wiringPi failed!");
        return 1;
    }

    while (1)
    {
        static struct option long_options[] =
            {
                {"help",        no_argument,        0,  'h'},
                {"type",        optional_argument,  0,  't'},
                {"mode",        optional_argument,  0,  'm'},
                {"divisor",     optional_argument,  0,  'd'},
                {"cycle",       optional_argument,  0,  'c'},
                {"pin",         optional_argument,  0,  'p'},
                {"pulseWidth",  optional_argument,  0,  'w'},
                 {0, 0, 0, 0}
            };
            /* getopt_long stores the option index here. */
            int option_index = 0;
                   
            //c = getopt_long (argc, argv, "h::t::m::d::c::p::w::",
            c = getopt_long (argc, argv, "t::m::d::c::p::w::",
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
   
              case 'c':
                cycle = atoi(optarg);
                break;
                                               
              case 'p':
                pin = atoi(optarg);
                globalPin = pin;
                break;
                           
              case 'w':
                pulseWidth = atoi(optarg);
                break;
                           
              case '?':
                 /* getopt_long already printed an error message. */
                 break;
                 
              default:
                printUsage();
                exit(0);
              }
    }

    printf("Using: PWM pin: %d, PWM Type %s:, PWM Mode: %s, divisor: %d, cycle: %d, pulseWidth: %d\n", pin, pwmType, pwmMode, divisor, cycle, pulseWidth);
    runPWM(pwmType, pwmMode, divisor, cycle, pin, pulseWidth);
}

void interruptHandler(int sig) {
    // Turn off LED
    pinMode(globalPin, PWM_OUTPUT);
    pwmWrite(globalPin, 0);

    printf("\nExiting...\n");

    exit(0);
}

void runPWM(char* pwmType, char* pwmMode, int divisor, int cycle, int pin, int pulseWidth) {
    pinMode(pin, PWM_OUTPUT);
    pwmSetRange(cycle);
    if (pwmMode == "Balanced") {
        pwmSetMode(PWM_MODE_BAL);
    } else {
        pwmSetMode(PWM_MODE_MS);
    }
    pwmSetClock(divisor);
    pwmWrite(pin, pulseWidth);
    while (1) {
//        sleep(1);
        delayMicroseconds(1000);
    }
}
