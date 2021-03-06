// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by the GPL 3.0
// license that can be found in the LICENSE file.
//
// Original app code by Sunfounder - https://docs.sunfounder.com/projects/raphael-kit/en/latest/1.1.6_led_dot_matrix_c.html.
// Original bcm_* functions and variable code by Mike McCauley at https://www.airspayce.com/mikem/bcm2835/index.html.
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
// Build: make all

#include "bcmfuncs.h"
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <signal.h>

#define uchar unsigned char
#define uint unsigned int

// NUM_CHARS represents a specific character to create on the LED matrix display. `disp1[NUM_CHARS]` 
// contains the specification of characters 0-9, A-Z, and the greek theta character.
// MATRIX_ROW contains the hex representation to create a display character. Each hex character
// defines which LEDs to turn on in each row of the LED matrix. In the first row  of the `disp1`
// array 0x3C is represented in binary as 0011 1100. This will cause the middle 4 LEDS in the
// first LED matrix display row to be lit and the 2 LEDS closest to each edge will be unlit. 
// 0x42 (0100 0010) specifies the LEDs to be lit in the second row of the LED Matrix display. 
// And so on for the remaining characters in the first NUM_CHARS of the array until each of the rows 
// of the LED Matrix display have been set. The characters in this array row in binary represent the 
// following character in the LED Matrix display. In the representation below the 0's are replaced 
// with spaces:
//
//    1111
//   1    1
//   1    1
//   1    1
//   1    1
//   1    1
//   1    1
//    1111
//
//  If you follow the pattern of 1's in the above rows you can see that they represent the 
//  number 0. Recall that spaces replaced 0's.
uchar disp1[NUM_CHARS][MATRIX_ROW]={
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

uchar scrollDisp[NUM_SCROLL][MATRIX_ROW]={
    {0x80,0x0,0x0,0x0,0x0,0x0,0x0,0x0},
    {0x40,0x80,0x0,0x0,0x0,0x0,0x0,0x0},
    {0x20,0x40,0x80,0x0,0x0,0x0,0x0,0x0},
    {0x10,0x20,0x40,0x80,0x0,0x0,0x0,0x0},
    {0x08,0x10,0x20,0x40,0x80,0x0,0x0,0x0},
    {0x04,0x08,0x10,0x20,0x40,0x80,0x0,0x0},
    {0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x0},
    {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80},
    {0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40},
    {0x00,0x00,0x01,0x02,0x04,0x08,0x10,0x20},
    {0x00,0x00,0x00,0x01,0x02,0x04,0x08,0x10},
    {0x00,0x00,0x00,0x00,0x01,0x02,0x04,0x08},
    {0x00,0x00,0x00,0x00,0x00,0x01,0x02,0x04},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x02},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x01,0X02},
    {0x00,0x00,0x00,0x00,0x00,0x01,0X02,0x04},
    {0x00,0x00,0x00,0x00,0x01,0X02,0x04,0x08},
    {0x00,0x00,0x00,0x01,0X02,0x04,0x08,0x10},
    {0x00,0x00,0x01,0X02,0x04,0x08,0x10,0x20},
    {0x00,0x01,0X02,0x04,0x08,0x10,0x20,0x40},
    {0x01,0X02,0x04,0x08,0x10,0x20,0x40,0x80},
    {0x02,0X04,0x08,0x10,0x20,0x40,0x80,0x00},
    {0x04,0X08,0x10,0x20,0x40,0x80,0x00,0x00},
    {0x08,0X10,0x20,0x40,0x80,0x00,0x00,0x00},
    {0x10,0X20,0x40,0x80,0x00,0x00,0x00,0x00},
    {0x20,0X40,0x80,0x00,0x00,0x00,0x00,0x00},
    {0x40,0X80,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x80,0X00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0X00,0x00,0x00,0x00,0x00,0x00,0x00},
    
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x80,0X40},
    {0x00,0x00,0x00,0x00,0x00,0x80,0X40,0x20},
    {0x00,0x00,0x00,0x00,0x80,0X40,0x20,0x10},
    {0x00,0x00,0x00,0x80,0X40,0x20,0x10,0x08},
    {0x00,0x00,0x80,0X40,0x20,0x10,0x08,0x04},
    {0x00,0x80,0X40,0x20,0x10,0x08,0x04,0x02},
    {0x80,0X40,0x20,0x10,0x08,0x04,0x02,0x01},
    {0X40,0x20,0x10,0x08,0x04,0x02,0x01,0x00},
    {0x20,0x10,0x08,0x04,0x02,0x01,0x00,0x00},
    {0x10,0x08,0x04,0x02,0x01,0x00,0x00,0x00},
    {0x08,0x04,0x02,0x01,0x00,0x00,0x00,0x00},
    {0x04,0x02,0x01,0x00,0x00,0x00,0x00,0x00},
    {0x02,0x01,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    
    {0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    {0x02,0x01,0x00,0x00,0x00,0x00,0x00,0X00},
    {0x04,0x02,0x01,0x00,0x00,0x00,0X00,0x00},
    {0x08,0x04,0x02,0x01,0x00,0X00,0x00,0x00},
    {0x10,0x08,0x04,0x02,0X01,0x00,0x00,0x00},
    {0x20,0x10,0x08,0X04,0x02,0x01,0x00,0x00},
    {0x40,0x20,0X10,0x08,0x04,0x02,0x01,0x00},
    {0x80,0X40,0x20,0x10,0x08,0x04,0x02,0x01},
    {0x00,0x80,0X40,0x20,0x10,0x08,0x04,0x02},
    {0x00,0x00,0x80,0X40,0x20,0x10,0x08,0x04},
    {0x00,0x00,0x00,0x80,0X40,0x20,0x10,0x08},
    {0x00,0x00,0x00,0x00,0x80,0X40,0x20,0x10},
    {0x00,0x00,0x00,0x00,0x00,0x80,0X40,0x20},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x80,0X40},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80},
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
};

void Delay_xms(uint x)
{
    bcm_delay(x);
}
                                                                        
/* Write 1 byte to the MAX7219 display driver. The MAX7219 expects 16 bits, 2 bytes, to be written before data
 * is transferred to the display. See https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf, Table 1.
 * Serial-Data Format (16 bits) for more detail.
 */
void Write_Max7219_byte(uchar data)
{
    bcm_spi_transfer(data);
}

//
// 'address1' is the MAX7219 register to write to. This can reference a
// row on the display (1-8) or control registers 9, a, b, c, and f. The control
// registers are used to specify things like the brightness of the display's LEDs.
// See https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf, Table 2. Register
// Address Map for more details regarding writing to the LED matrix vs. the control registers. Also 
// see 'Init_MAX7219()' for the implementation of control register use in this program.
//
// 'dat1' is the data to be written to a given register address
//
void Write_Max7219(uchar address1,uchar dat1)
{
    bcm_gpio_write(Max7219_pinCS,LOW);  // Enable chip select (CE0) using GPIO pin write (vs. SPI).
                                        // This is needed to enable data transfer to the SPI device connected
                                        // to SPI0 CE0 pin. Chip select is also known as chip enable (CE).
    Write_Max7219_byte(address1);       // Choose row in the Max7219 address register.
    Write_Max7219_byte(dat1);           // Write data to the selected address register.
    bcm_gpio_write(Max7219_pinCS,HIGH); // Disable Chip Select. At this point address1 and dat1 have been written
                                        // into the MAX7219's shift register. When the CS pin is set to high
                                        // this data will be transferred to the LED display.
}

//
// Initialize control registers on the MAX7219 display driver. 
// See https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf, Table 2. Register
// Address Map, for details.
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

// Initialize the SPI0 interface on the BCM2835 board
void init_spi()
{
    bcm_spi_begin();                                    // Defines which pins will be used for SPI and sets them to SPI mode
                                                        // (Alternate function 0).
    bcm_spi_setBitOrder(BCM_SPI_BIT_ORDER_MSBFIRST);    // Using most significant bit ordering. The MAX7219 uses MSB ordering.
                                                        // See https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf,
                                                        // see Table 1 page 6, for more details. More importantly, the BCM2835
                                                        // board only supports MSB addressing.
    bcm_spi_setDataMode(BCM_SPI_MODE0);                 // Set clock polarity and phase. Mostly don't worry about this.
    bcm_spi_setClockDivider(BCM_SPI_CLOCK_DIVIDER_256); // Set the clock speed. Mostly don't worry about this.
                                                        // See leddotmatrixnolib.h for details.
    bcm_gpio_fsel(Max7219_pinCS, BCM_GPIO_FSEL_OUTP);   // set chip select pin to OUTPUT so it can be set HIGH/LOW.
}

// NOTE: the use of the SPI0 set of pins is hardcoded (i.e., GPIO pins 7-11 are used).
int main(void)
{
    signal(SIGINT, interruptHandler);

    uchar i,j;

    if (!bcm_init())
    {
        printf("Unable to init bcm2835.\n");
        return 1;
    }
    init_spi();
    Delay_xms(50);
    Init_MAX7219();
    // Iterate through the disp1 array writing to the MAX7219 display driver. For each ROW written all the
    // delay 1000ms in order to provide time to see the character displayed on the LED Matrix display.
    while(1)
    {
        for(j=0;j<NUM_CHARS;j++)
        {
            for(i=1;i<MATRIX_ROW+1;i++)
            {
                Write_Max7219(i,disp1[j][i-1]);
            }
            Delay_xms(300);
        }

        for(j=0;j<NUM_SCROLL;j++)
        {
            for(i=1;i<MATRIX_ROW+1;i++)
            {
                Write_Max7219(i,scrollDisp[j][i-1]);
            }
            Delay_xms(25);
        }
    }

    // release resources
    bcm_spi_end();
    bcm_close(); // reverses bcm_init()
    return 0;
}

// interruptHandler catches SIGINT when ctl-C is pressed in order to halt the program gracefully.
void interruptHandler(int sig) {
    // Clear display
    for(int i=1;i<9;i++)
    {
        Write_Max7219(i, 0x0);
    }

    bcm_spi_end();
    bcm_close();

    printf("\nExiting...\n");
    exit(0);
}

