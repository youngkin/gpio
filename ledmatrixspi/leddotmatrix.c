//
// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file.
//
// Original code by Sunfounder - https://docs.sunfounder.com/projects/raphael-kit/en/latest/1.1.6_led_dot_matrix_c.html.
// This code was modified to include details about how the code works and to handle ctl-C signal to halt the
// program gracefully.
//
// This program demonstrates controlling a MAX7219 LED display by causing it to display
// the numbers 0-9 and the letters A-Z.
//
// NOTE: There is inadequate error handling in the application. Take care when copying.
//
// References:
//  1. https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf - MAX7219 LED display datasheet
//  2. https://www.airspayce.com/mikem/bcm2835/index.html - BCM2835 library documentation
//
// Build: gcc -o ledmatrixspi ledmatrixspi.c  -lbcm2835 -lpthread
// Run: sudo ./ledmatrixspi

#include <bcm2835.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>

#define uchar unsigned char
#define uint unsigned int

#define Max7219_pinCS  RPI_GPIO_P1_24 // Pi pin 24, GPIO pin 8 - from bcm2835.h
#define ROWS 37
#define COLS 8

void interruptHandler(int);

uchar disp1[ROWS][COLS]={
    {0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x3C},  //0
    {0x08,0x18,0x28,0x08,0x08,0x08,0x08,0x08},  //1
    {0x7E,0x2,0x2,0x7E,0x40,0x40,0x40,0x7E},    //2
    {0x3E,0x2,0x2,0x3E,0x2,0x2,0x3E,0x0},       //3
    {0x8,0x18,0x28,0x48,0xFE,0x8,0x8,0x8},      //4
    {0x3C,0x20,0x20,0x3C,0x4,0x4,0x3C,0x0},     //5
    {0x3C,0x20,0x20,0x3C,0x24,0x24,0x3C,0x0},   //6
    {0x3E,0x22,0x4,0x8,0x8,0x8,0x8,0x8},        //7
    {0x0,0x3E,0x22,0x22,0x3E,0x22,0x22,0x3E},   //8
    {0x3E,0x22,0x22,0x3E,0x2,0x2,0x2,0x3E},     //9
    {0x8,0x14,0x22,0x3E,0x22,0x22,0x22,0x22},   //A
    {0x3C,0x22,0x22,0x3E,0x22,0x22,0x3C,0x0},   //B
    {0x3C,0x40,0x40,0x40,0x40,0x40,0x3C,0x0},   //C
    {0x7C,0x42,0x42,0x42,0x42,0x42,0x7C,0x0},   //D
    {0x7C,0x40,0x40,0x7C,0x40,0x40,0x40,0x7C},  //E
    {0x7C,0x40,0x40,0x7C,0x40,0x40,0x40,0x40},  //F
    {0x3C,0x40,0x40,0x40,0x40,0x44,0x44,0x3C},  //G
    {0x44,0x44,0x44,0x7C,0x44,0x44,0x44,0x44},  //H
    {0x7C,0x10,0x10,0x10,0x10,0x10,0x10,0x7C},  //I
    {0x3C,0x8,0x8,0x8,0x8,0x8,0x48,0x30},       //J
    {0x0,0x24,0x28,0x30,0x20,0x30,0x28,0x24},   //K
    {0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x7C},  //L
    {0x81,0xC3,0xA5,0x99,0x81,0x81,0x81,0x81},  //M
    {0x0,0x42,0x62,0x52,0x4A,0x46,0x42,0x0},    //N
    {0x3C,0x42,0x42,0x42,0x42,0x42,0x42,0x3C},  //O
    {0x3C,0x22,0x22,0x22,0x3C,0x20,0x20,0x20},  //P
    {0x1C,0x22,0x22,0x22,0x22,0x26,0x22,0x1D},  //Q
    {0x3C,0x22,0x22,0x22,0x3C,0x24,0x22,0x21},  //R
    {0x0,0x1E,0x20,0x20,0x3E,0x2,0x2,0x3C},     //S
    {0x0,0x3E,0x8,0x8,0x8,0x8,0x8,0x8},         //T
    {0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x3E},  //U
    {0x22,0x22,0x22,0x22,0x22,0x22,0x14,0x8},   //V
//    {0x42,0x42,0x42,0x42,0x42,0x42,0x22,0x1C},  //U
//    {0x42,0x42,0x42,0x42,0x42,0x42,0x24,0x18},  //V
    {0x0,0x49,0x49,0x49,0x49,0x2A,0x1C,0x0},    //W
    {0x0,0x41,0x22,0x14,0x8,0x14,0x22,0x41},    //X
    {0x41,0x22,0x14,0x8,0x8,0x8,0x8,0x8},       //Y
    {0x0,0x7F,0x2,0x4,0x8,0x10,0x20,0x7F},      //Z
    {0x18,0x24,0x42,0xFF,0x42,0x24,0x18},       //Theta
};

void Delay_xms(uint x)
{
        bcm2835_delay(x);

}
//------------------------

void Write_Max7219_byte(uchar DATA)
{
    bcm2835_gpio_write(Max7219_pinCS,LOW); //Enable chip select (CE0)
    bcm2835_spi_transfer(DATA);
}

//
// address1 is the MAX7219 register to write to. This can reference a
// row on the display (1-8) or control registers 9, a, b, c, and f.
// See https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf for more
// details.
//
// dat1 is the data to be written to a give register address
//
void Write_Max7219(uchar address1,uchar dat1)
{
    bcm2835_gpio_write(Max7219_pinCS,LOW);  // Enable Chip Select
    Write_Max7219_byte(address1);           // Choose row in the address register
    Write_Max7219_byte(dat1);               // Write fill columns in selected row
    bcm2835_gpio_write(Max7219_pinCS,HIGH); // Disable Chip Select
}

//
// Initialize control registers. See https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf
// for details.
// 
void Init_MAX7219()
{
    Write_Max7219(0x09,0x00); // Decode mode register
    Write_Max7219(0x0a,0x03); //  medium brightness
//    Write_Max7219(0x0a,0x0f);// max brightness
    Write_Max7219(0x0b,0x07); // Scan limit register
    Write_Max7219(0x0c,0x01); // Shutdown register
    Write_Max7219(0x0f,0x00); // Display test register, normal mode
//    Write_Max7219(0x0f,0x01);// Display test register, test mode (light all leds)
}

void Init_BCM2835()
{
    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_256);
    bcm2835_gpio_fsel(Max7219_pinCS, BCM2835_GPIO_FSEL_OUTP); // set chip select pin to OUTPUT
//    bcm2835_gpio_write(disp1[0][0],HIGH); // What does this do? It writes HIGH to GPIO pin 60 (0x3C - disp1[0][0]).
}

int main(void)
{
    signal(SIGINT, interruptHandler);

    uchar i,j;

    if (!bcm2835_init())
    {
        printf("Unable to init bcm2835.\n");
        return 1;
    }
    Init_BCM2835();
    Delay_xms(50);
    Init_MAX7219();
    while(1)
    {
        for(j=0;j<ROWS;j++)
        {
            for(i=1;i<COLS+1;i++)
            {
                Write_Max7219(i,disp1[j][i-1]);
            }
            Delay_xms(1000);
        }
    }
    bcm2835_spi_end();
    bcm2835_close();
    return 0;
}

// interruptHandler catches SIGINT when ctl-C is pressed in order to halt the program gracefully.
void interruptHandler(int sig) {
    // Clear display
    for(int i=1;i<9;i++)
    {
        Write_Max7219(i, 0x0);
    }

    bcm2835_spi_end();
    bcm2835_close();

    printf("\nExiting...\n");
    exit(0);
}
