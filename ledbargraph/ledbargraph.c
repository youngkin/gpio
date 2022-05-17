//
// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by the GPL 3.0
// license that can be found in the LICENSE file.
//
// Credit for the majority of this code goes to Sunfounder,
// https://docs.sunfounder.com/projects/raphael-kit/en/latest/1.1.3_led_bar_graph_c.html
//
// Build: gcc -o ledbargraph ledbargraph.c  -lwiringPi -lpthread
// Run: ./ledbargraph 
// 
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <wiringPi.h>
#include <stdio.h>

// Define interrupt handler, int parm is the signal
void interruptHandler(int);

int pins[10] = {0,1,2,3,4,5,6,8,9,10};

// Initialize all pins to output mode, turn on then turn off
void init(void) {
    for (int i = 0; i < 10; i++) {
        pinMode(pins[i], OUTPUT);
        digitalWrite(pins[i], LOW);
    }
    delay(500);
    for (int i = 0; i < 10; i++) {
        pinMode(pins[i], OUTPUT);
        digitalWrite(pins[i], HIGH);
    }
    delay(500);
}

void randomBarGraph(int lower, int upper) {
    srand(time(0));

    while(1) {
        int rnum = rand();
        int num = (rnum % (upper-lower+1)) + lower;
        digitalWrite(pins[num], LOW);
        delay(30);
        digitalWrite(pins[num], HIGH);
        delay(30);
    }
}
void oddLedBarGraph(void){
    for(int i=0;i<5;i++){
        int j=i*2;
        digitalWrite(pins[j],LOW);
        delay(300);
        digitalWrite(pins[j],HIGH);

    }

}
void evenLedBarGraph(void){
    for(int i=0;i<5;i++){
        int j=i*2+1;
        digitalWrite(pins[j],LOW);
        delay(300);
        digitalWrite(pins[j],HIGH);

    }

}
void allLedBarGraph(void){
    for(int i=0;i<10;i++){
        digitalWrite(pins[i],LOW);
        delay(300);
        digitalWrite(pins[i],HIGH);

    }

}
int main(void)
{
    signal(SIGINT, interruptHandler);

    if(wiringPiSetup() == -1){ //when initialize wiring failed,print message to screen
        printf("setup wiringPi failed !");
        return 1;
    }

    init();

    while(1){
        oddLedBarGraph();
        delay(300);
        evenLedBarGraph();
        delay(300);
        allLedBarGraph();
        delay(300);
        randomBarGraph(0, 10);
    }
    return 0;

}

// interruptHandler catches SIGINT when ctl-C is pressed in order to halt the program gracefully.
void interruptHandler(int sig) {
    for (int i = 0; i < 10; i++) {
        pinMode(pins[i], OUTPUT);
        digitalWrite(pins[i], HIGH);
    }

    printf("\nExiting...\n");
    exit(0);
}
