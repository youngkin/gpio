//
// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by the GPL 3.0
// license that can be found in the LICENSE file.
//
// Inspiration for this code goes to Sunfounder,
// https://docs.sunfounder.com/projects/raphael-kit/en/latest/1.1.4_7-segment_display_c.html
//
// This program is designed to be used with a 74HC595 Shift Register and a common cathode
// seven segment display.
//
// Build: gcc -o sevensegdisplay sevensegdisplay.c  -lwiringPi -lpthread
// Run: ./sevensegdisplay 
// 
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <wiringPi.h>
#include <stdio.h>

// Define interrupt handler, int parm is the signal
void interruptHandler(int);

#define   SDI   0   // Serial data input pin (aka SER or DS)
#define   RCLK  1   // Output Register Clock/Latch pin (st_cp)
#define   SRCLK 2   // Shift Register Clock pin (sh_cp)
#define   SRCLR 24  // Shift Register Clear pin
#define   OE    29  // Output Enable pin

// 'SegCode' contains the hex values that will be written to the shift register to then
// be transferred to the seven segment display.
unsigned char SegCode[17] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,
    0x7c,0x39,0x5e,0x79,0x71,0x80};

void toggle8(void);
void init(void);
void hc595_shift(unsigned char);
void shiftRegClr(void);

// Initializes the shift register and 7-segment display
void init(void){
    pinMode(SDI, OUTPUT);   
    pinMode(RCLK, OUTPUT);  
    pinMode(SRCLK, OUTPUT); 
    pinMode(SRCLR, OUTPUT); 
    pinMode(OE, OUTPUT);    
    digitalWrite(SDI, 0);
    digitalWrite(RCLK, 0);
    digitalWrite(SRCLK, 0);
    digitalWrite(SRCLR, 1); // Enable shift register
    digitalWrite(OE, 0);    // Enable output register (1 disables)
}

// toggle8 briefly illuminates the number '8' on the 7-segment display before clearing it.
void toggle8(void) {
    hc595_shift(SegCode[8]);
    delay(1000);
    shiftRegClr();
    delay(1000);
}

// shiftRegClr clears the shift register by setting the shift register clear (SRCLR) pin 
// to LOW. The cleared register is then transferred to the output register by toggling
// the output register clock (RCLK). NOTE: SRCLR must be set back to HIGH before it can
// receive any data.
void shiftRegClr(void) {
    digitalWrite(SRCLR, 0);

    digitalWrite(RCLK, 1);
    delay(1);
    digitalWrite(RCLK, 0);

    // reset to HIGH to reenable shift register
    digitalWrite(SRCLR, 1);
    delay(100);
}

// zeroClear clears the display by setting all output pins (Qa to Qh) to LOW. The net
// effect, after the shift register is written to the output register, is that all pins
// in the 7-segment display are turned off.
void zeroClear(void) {
    // Shift '0' into all 8 bits into shift register and...
    for (int i = 0; i < 8; i++) {
        digitalWrite(SDI, 0x80 & (0 << i));
        digitalWrite(SRCLK, 1);
        delay(1);
        digitalWrite(SRCLK, 0);
    }
    //... then transfer shift register to storage/output register
    digitalWrite(RCLK, 1);
    delay(1);
    digitalWrite(RCLK, 0);
}

// writeAllOnes sets all shift register pins (Qa to Qh) to HIGH. The net effect of this
// is to illuminate all LEDs in the 7-segment display, displaying '8.'.
void writeAllOnes(void) {
    // Shift '1' into all 8 bits into shift register and...
    for (int i = 0; i < 8; i++) {
        digitalWrite(SDI, 1);
        digitalWrite(SRCLK, 1);
        delay(1);
        digitalWrite(SRCLK, 0);
    }
    //... then transfer shift register to storage/output register
    digitalWrite(RCLK, 1);
    delay(1);
    digitalWrite(RCLK, 0);
}

// oeToggle sets the OE pin to HIGH which has the effect of blocking the contents of the
// output register from being available to any connected devices (e.g., the 7-segment display).
// It then sets the OE pin to LOW which makes the output register contents available to any
// connected devices. Toggling this pin does not clear the contents of the output register,
// it only blocks their availability until it is set to LOW.
void oeToggle(void) {
    printf("\t\tBlock output register (OE = 1, clear display)\n");
    digitalWrite(OE, 1);
    delay(1000);
    printf("\t\tUnblock output register (OE = 0, display output register contents)\n");
    digitalWrite(OE, 0);
    delay(1000);
}

// hc595_shift takes 'dat' and writes it 1 bit at a time into the
// shift register. This is accomplished by first writing the bit to
// the serial data input pin (SDI) and the advancing the shift register
// clock (SRCLK) by toggling it from LOW to HIGH to LOW again. The 
// contents of the shift register are finally made available to connected
// devices by toggling the output register clock (RCLK).
void hc595_shift(unsigned char dat){
    int i;
    // Populate the shift registers 1 bit at a time
    for(i=0;i<8;i++){
        // Populate shift register, bit 'i' (0 thru 7)
        digitalWrite(SDI, 0x80 & (dat << i));
        // Advance shift register clock
        digitalWrite(SRCLK, 1);
        delay(1);
        digitalWrite(SRCLK, 0);

    }
    // Advance storage/output register clock to transfer shift register contents to the 
    // output register
    digitalWrite(RCLK, 1);
    delay(1);
    digitalWrite(RCLK, 0);

}

// testSRCLR demonstrates the usage of the shift register clear pin (SRCLR), in conjuction with
// the output register clock (RCLK), to clear the shift and output registers' contents. It first
// writes the digit '8' to the registers prior to clearing them. The effect will be that an '8'
// will be briefly displayed before being cleared from the 7-segment display.
void testSRCLR(void) {
    printf("\tDisplaying '8'\n");
    hc595_shift(SegCode[8]);
    delay(1000);
    printf("\tClear Shift Register\n");
    shiftRegClr();
}

// testZerosClear clears the shift and output registers by writing zeros to all output pins
// (Qa-Qh). It first writes the digit '8' to the display before clearing the output pins.
void testZerosClear(void) {
    printf("\tDisplaying '8'\n");
    hc595_shift(SegCode[8]);
    delay(1000);
    printf("\tWrite zeros to Shift Register\n");
    zeroClear();
}

// testWriteAllOnes demonstrates the effect of setting all output pins (Qa-Qh) to 1. The result
// will be '8.' will be displayed on the 7-segment display. The test ends with the clearing of
// all register contents (clearing the display).
void testWriteAllOnes(void) {
    printf("\tDisplaying '8'\n");
    hc595_shift(SegCode[8]);
    delay(1000);
    printf("\tWrite 1's to Shift Register\n");
    writeAllOnes();
    delay(1000);
    printf("\tClear Shift Register\n");
    shiftRegClr();
}

// testOEToggle demonstrates the effect of toggling the OutputEnable pin. It does this by first
// displaying '8' on the 7-segment display, then setting the OE pin to HIGH (blocking the availability
// of the output register contents), and finally resetting the OE pin to LOW restoring the
// availability of the register contents to connected devices, in this case the 7-segment display.
void testOEToggle(void) {
    printf("\tDisplaying '8'\n");
    hc595_shift(SegCode[8]);
    delay(1000);
    oeToggle();
    delay(1000);
    printf("\tClear Shift Register\n");
    shiftRegClr();
}

// testWriteNums displays in turn hexidecimal digits 0-F followed by a decimal point. The
// test ends with the registers and display being cleared.
void testWriteNums(void) {
    for(int i=0;i<17;i++){
        if (i == 16) {
            printf("\tDisplaying decimal point\n");
        } else {
            printf("\tDisplaying %1X\n", i); // %X means hex output
        }
        hc595_shift(SegCode[i]);
        delay(500);
    }
    printf("\tClear Shift Register\n");
    shiftRegClr();
}

int main(void){
    // Register handler to capture 'ctl-C' keyboard input.
    signal(SIGINT, interruptHandler);

    // Initialize WiringPi library
    if(wiringPiSetup() == -1){ 
        printf("setup wiringPi failed !");
        return 1;
    }

    init();

    int c;
    unsigned int run = 1;
    while (run) {
        printf("\nEnter [n]umbers, [s]hift register clear, [z]ero clear, [w]rite ones, [o]e toggle, [q]uit: ");
        char *line = NULL;
        size_t len = 0;
        ssize_t bytesRead = getline(&line, &len, stdin);
        if (bytesRead < 2) {
            printf("invalid option chosen, try again\n");
            continue;
        }
        char cStr[2] = "\0"; // create a null terminated 2 char string
        cStr[0] = line[0];
        free(line);

        switch(cStr[0]) {
            case 'n':
                printf("\tDemostrate displaying hexidecimal digits 0 thru F and decimal point\n");
                testWriteNums();
                break;
            case 's':
                printf("\tDemonstrate effects of clearing the Shift Register\n");
                testSRCLR();
                break;
            case 'z':
                printf("\tDemonstrate effects of writing zeros to clear Shift Register\n");
                testZerosClear();
                break;
            case 'w':
                printf("\tDemonstrate effects of writing all 1's (displays '8' and the decimal point)\n");
                testWriteAllOnes();
                break;
            case 'o':
                printf("\tDemonstrate effects of toggling the Output Enable pin\n");
                testOEToggle();
                break;
            case 'q':
                printf("\tQuitting program\n\n");
                run = 0;
                break;
            default:
                printf("\tinvalid option %d chosen, try again\n", c);
                break;
        }
    }

    return 0;
}

// interruptHandler catches SIGINT when ctl-C is pressed in order to halt the program gracefully.
void interruptHandler(int sig) {
    printf("\n!!!INTERRUPTED!!! Write '8', clear display, then exit\n");
    toggle8();
    exit(1);
}
