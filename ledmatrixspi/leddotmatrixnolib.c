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
// Build: gcc -o leddotmatrixnolib leddotmatrixnolib.c  -lpthread
// Build DEBUG version: gcc -o leddotmatrixnolib leddotmatrixnolib.c  -lpthread -D DEBUG 

#include "leddotmatrixnolib.h"
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

// ROWS represents a specific character to create on the LED matrix display.
// COLS contains the hex representation to create a display character. Each hex character
// defines which LEDs to turn on in each row of the LED matrix. In the first ROW of the
// array 0x3C is represented in binary as 0011 1100. This will cause the middle 4 LEDS in the
// first LED matrix display row to be lit and the 2 LEDS closest to each edge will be unlit. 
// 0x42 (0100 0010) specifies the LEDs to be lit in the second row of the LED Matrix display. 
// And so on for the remaining characters in the first ROW of the array until each of the rows 
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

// Initialize the SPI interface on the BCM2835 board
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
    // delay 1000ms in order to provide time to see the displayed character.
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
    bcm_spi_end();
    bcm_close();
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

/* Physical address and size of the peripherals block. These addresses come from /dev/devicetree and are physical
 * addresses. See the BCM2835 datasheet, section 1.2, at
 * https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf for details.
 * May be overridden on RPi2
 */
off_t bcm_peripherals_base = BCM_PERI_BASE;
size_t bcm_peripherals_size = BCM_PERI_SIZE;

/* Virtual memory address of the mapped peripherals block. It is set from bcm_peripherals_base and 
 * bcm_peripheral_size using the mapmem() function called from bcm_init().
 */
uint32_t *bcm_peripherals = (uint32_t *)MAP_FAILED;

/* These are the register bases within the peripherals block. Initialize to MAP_FAILED to indicate that the mapping from the
 * physical address space to virtual memory has failed. These will be set to the actual virtual memory offsets if mapping
 * is successful (in mapmem()). NOTE: only bcm_gpio will be set if /dev/gpiomem is used instead of /dev/mem. The other offsets
 * (e.g., bcm_pwm, bcm_spi0, etc)can only be accessed when running this program as root as root is required to access /dev/mem.
 */
volatile uint32_t *bcm_gpio        = (uint32_t *)MAP_FAILED;
volatile uint32_t *bcm_pwm         = (uint32_t *)MAP_FAILED;
volatile uint32_t *bcm_clk         = (uint32_t *)MAP_FAILED;
volatile uint32_t *bcm_pads        = (uint32_t *)MAP_FAILED;
volatile uint32_t *bcm_spi0        = (uint32_t *)MAP_FAILED;
volatile uint32_t *bcm_bsc0        = (uint32_t *)MAP_FAILED;
volatile uint32_t *bcm_bsc1        = (uint32_t *)MAP_FAILED;
volatile uint32_t *bcm_st          = (uint32_t *)MAP_FAILED;
volatile uint32_t *bcm_aux         = (uint32_t *)MAP_FAILED;
volatile uint32_t *bcm_spi1        = (uint32_t *)MAP_FAILED;

/* RPI 4 has different pullup registers - we need to know if we have that type */
static uint8_t pud_type_rpi4 = 0;

/* SPI bit order. BCM2835 SPI0 only supports MSBFIRST, so we instead 
 * have a software based bit reversal, based on a contribution by Damiano Benedetti
 */
static uint8_t bcm_spi_bit_order = BCM_SPI_BIT_ORDER_MSBFIRST;
static uint8_t bcm_byte_reverse_table[] = 
{
    0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
    0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
    0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
    0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
    0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
    0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
    0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
    0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
    0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
    0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
    0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
    0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
    0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
    0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
    0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
    0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
    0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
    0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
    0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
    0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
    0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
    0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
    0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
    0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
    0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
    0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
    0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
    0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
    0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
    0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
    0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
    0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff

};

/* Ensure data written is in MSB first format. */
static uint8_t bcm_correct_order(uint8_t b)
{
    if (bcm_spi_bit_order == BCM_SPI_BIT_ORDER_LSBFIRST)
        return bcm_byte_reverse_table[b];
    else
        return b;
}

/* Map 'size' bytes starting at 'off' in file 'fd' to memory.
// Return mapped address on success, MAP_FAILED otherwise.
// On error print message.
*/
static void *mapmem(const char *msg, size_t size, int fd, off_t off)
{
    void *map = mmap(NULL, size, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, off);
    if (map == MAP_FAILED)
        fprintf(stderr, "bcm_init: %s mmap failed: %s\n", msg, strerror(errno));
    return map;
}

/* Release the memory mapped in mapmem(). */
static void unmapmem(void **pmem, size_t size)
{
    if (*pmem == MAP_FAILED) return;
    munmap(*pmem, size);
    *pmem = MAP_FAILED;
}

/* Initialize the library be performing the following steps:
 *  1.  Find the base address and and the size of associated memory by using info
 *      from the device tree (BCM_RPI2_DT_FILENAME). This information is in the
 *      'ranges' property in the 'soc' node. See the device tree spec at 
 *      https://www.devicetree.org/specifications/. Release v0.4-rc1 has more info
 *      about this in section 2.3.8 - 'ranges'.  
 *  2.  Attempt to map the entire BCM2835 address range from /dev/mem. The base address
 *      and size are used to create the memory map for the BCM2835. 'root' access is
 *      needed to map from /dev/mem.
 *  3.  If the program is not run as 'root' then /dev/gpiomem is used instead. Note
 *      that in this case only the GPIO register is accessible (i.e., the pins). Modes
 *      such as PWM and SPI are not available. Attempts to use these modes will result
 *      in the program exiting with an error.
 */
int bcm_init(void)
{
    int  memfd;
    int  ok;
    FILE *fp;

    /* Figure out the base and size of the peripheral address block
     * using the device-tree. Required for RPi2/3/4, optional for RPi 1
     */
    if ((fp = fopen(BCM_RPI2_DT_FILENAME , "rb"))) //"rb" == read binary file
    {
        unsigned char buf[16];
        uint32_t base_address;
        uint32_t peri_size;
        if (fread(buf, 1, sizeof(buf), fp) >= 8)
        {
#ifdef DEBUG
            for (int i = 0; i < 16; i++)
            {
                printf("device tree soc/ranges property buf[%d]: %#06x\n", i, buf[i]);
            }
#endif
            // buf offsets 0-3 contains the starting offset of the BCM2835 peripherals on the
            // system bus. See the BCM2835 datasheet, Section 1.2, for more details
            // (https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf).
            base_address = (buf[4] << 24) |
                (buf[5] << 16) |
                (buf[6] << 8) |
                (buf[7] << 0);

            peri_size = (buf[8] << 24) |
                (buf[9] << 16) |
                (buf[10] << 8) |
                (buf[11] << 0);

            if (!base_address)
            {
                /* looks like RPI 4 */
                base_address = (buf[8] << 24) |
                    (buf[9] << 16) |
                    (buf[10] << 8) |
                    (buf[11] << 0);

                peri_size = (buf[12] << 24) |
                    (buf[13] << 16) |
                    (buf[14] << 8) |
                    (buf[15] << 0);

            }
            /* check for valid known range formats. The starting offset of the
             * peripheral bus address range should be 0x7e000000. 
             */
            if ((buf[0] == 0x7e) &&
                    (buf[1] == 0x00) &&
                    (buf[2] == 0x00) &&
                    (buf[3] == 0x00) &&
                    ((base_address == BCM_PERI_BASE) || (base_address == BCM_RPI2_PERI_BASE) || (base_address == BCM_RPI4_PERI_BASE)))
            {
                bcm_peripherals_base = (off_t)base_address;
                bcm_peripherals_size = (size_t)peri_size;
                if( base_address == BCM_RPI4_PERI_BASE  )
                {
                    pud_type_rpi4 = 1;

                }
#ifdef DEBUG
                printf("base_address: %p\n", base_address);
                printf("bcm_peripherals_base: %p\n", bcm_peripherals_base);
                printf("peri_size: %p\n", peri_size);
                printf("bcm_peripherals_size: %p\n", bcm_peripherals_size);
#endif
            }
        }

        fclose(fp);

    }
    /* otherwise we are prob on RPi 1 with BCM2835, and use the hardwired defaults */

    /* Now get ready to map the peripherals block 
     * If we are not root, try for the new /dev/gpiomem interface and accept
     * the fact that we can only access GPIO
     * else try for the /dev/mem interface and get access to everything
     */
    memfd = -1;
    ok = 0;
    if (geteuid() == 0)
    {
        /* Open the master /dev/mem device */
        if ((memfd = open("/dev/mem", O_RDWR | O_SYNC) ) < 0) 
        {
            fprintf(stderr, "bcm_init: Unable to open /dev/mem: %s\n",
                    strerror(errno)) ;
            goto exit;

        }

        /* The base physical address of the peripherals block is mapped to VM */
        bcm_peripherals = mapmem("gpio", bcm_peripherals_size, memfd, bcm_peripherals_base);
        if (bcm_peripherals == MAP_FAILED) goto exit;

        /* Now compute the base addresses of various peripherals, 
         * which are at fixed offsets within the mapped peripherals block
         *
         * The address offsets in the SPI address map are specified in bytes.
         * Since the offsets below are defined as uint32_t's, and that's 4 bytes, all of
         * offsets have to be divided by 4 to get the uint32_t offset.
         */
        bcm_gpio = bcm_peripherals + BCM_GPIO_BASE/4;
#ifdef DEBUG
        printf("bcm_gpio: %p=%p+%p\n", bcm_gpio, bcm_peripherals, BCM_GPIO_BASE/4);
#endif
        bcm_pwm  = bcm_peripherals + BCM_GPIO_PWM/4;
        bcm_clk  = bcm_peripherals + BCM_CLOCK_BASE/4;
        bcm_pads = bcm_peripherals + BCM_GPIO_PADS/4;
        bcm_spi0 = bcm_peripherals + BCM_SPI0_BASE/4;
        bcm_bsc0 = bcm_peripherals + BCM_BSC0_BASE/4; /* I2C */
        bcm_bsc1 = bcm_peripherals + BCM_BSC1_BASE/4; /* I2C */
        bcm_st   = bcm_peripherals + BCM_ST_BASE/4;
        bcm_aux  = bcm_peripherals + BCM_AUX_BASE/4;
        bcm_spi1 = bcm_peripherals + BCM_SPI1_BASE/4;

        ok = 1;

    }
    else
    {
        /* Not root, try /dev/gpiomem */
        if ((memfd = open("/dev/gpiomem", O_RDWR | O_SYNC) ) < 0) 
        {
            fprintf(stderr, "bcm_init: Unable to open /dev/gpiomem: %s\n",
                    strerror(errno)) ;
            goto exit;

        }

        /* Base of the peripherals block is mapped to VM.
         * When using /dev/gpiomem there are no peripherals other than the GPIO pins so offsets are
         * not applicable. Likewise, 'bcm_peripherals_base' is no longer relevant. The starting address
         * of GPIO is at the beginning of the memory returned, not at some base offset in physical memory.
         */
        bcm_peripherals_base = 0;
        bcm_peripherals = mapmem("gpio", bcm_peripherals_size, memfd, bcm_peripherals_base);
        if (bcm_peripherals == MAP_FAILED) goto exit;
        bcm_gpio = bcm_peripherals;
        ok = 1;

    }

exit:
    if (memfd >= 0)
        close(memfd);

    if (!ok)
        bcm_close();

    return ok;

}

/* Close this library and deallocate everything */
int bcm_close(void)
{
    unmapmem((void**) &bcm_peripherals, bcm_peripherals_size);
    bcm_peripherals = MAP_FAILED;
    bcm_gpio = MAP_FAILED;
    bcm_pwm  = MAP_FAILED;
    bcm_clk  = MAP_FAILED;
    bcm_pads = MAP_FAILED;
    bcm_spi0 = MAP_FAILED;
    bcm_bsc0 = MAP_FAILED;
    bcm_bsc1 = MAP_FAILED;
    bcm_st   = MAP_FAILED;
    bcm_aux  = MAP_FAILED;
    bcm_spi1 = MAP_FAILED;
    return 1; /* Success */
}    

/* Set/clear only the bits in the value covered by the mask. Bits to be masked are set to 1.
 * This is not atomic - can be interrupted.
 */
void bcm_peri_set_bits(volatile uint32_t* paddr, uint32_t value, uint32_t mask)
{
    // Read the byte at paddr.
    uint32_t v = bcm_peri_read(paddr);

    // Using the mask, e.g.,:
    //       v          = 1100 1011
    //       mask       = 0000 1100 // specifies which bits will be modified (e.g., bits 2 & 3)
    //       value      = 0000 0100 // speciies the new values for the bits specified by 'mask'
    //
    //       ~mask      = 1111 0011
    //       v&~mask    = 1100 0011 // resets any of the bits in 'v' to be reset by 'value' (e.g., bits 2 & 3)
    //       value&mask = 0000 0100 // provides the new values for the bits specified in 'mask' (e.g., 2 & 3)
    //
    //       (v&~mask) | (value&mask) = 1100 0111 // results in the new value, bit 3 set to 0 and bit 2 set to 1
    //
    v = (v & ~mask) | (value & mask);
    bcm_peri_write(paddr, v);

}

/* Function select
// pin is a BCM2835 GPIO pin number NOT RPi pin number
// There are 6 control registers, each control the functions of a block
//      of 10 pins.
//      Each control register has 10 sets of 3 bits per GPIO pin:
//
//      000 = GPIO Pin X is an input
//      001 = GPIO Pin X is an output
//      100 = GPIO Pin X takes alternate function 0
//      101 = GPIO Pin X takes alternate function 1
//      110 = GPIO Pin X takes alternate function 2
//      111 = GPIO Pin X takes alternate function 3
//      011 = GPIO Pin X takes alternate function 4
//      010 = GPIO Pin X takes alternate function 5
//
// So the 3 bits for port X are:
//      X / 10 + ((X % 10) * 3)
*/
void bcm_gpio_fsel(uint8_t pin, uint8_t mode)
{
    /* Function selects are 10 pins per 32 bit word, 3 bits per pin */
    volatile uint32_t* paddr = bcm_gpio + BCM_GPFSEL0/4 + (pin/10);
    uint8_t   shift = (pin % 10) * 3;
    uint32_t  mask = BCM_GPIO_FSEL_MASK << shift;
    uint32_t  value = mode << shift;
    bcm_peri_set_bits(paddr, value, mask);
}

void bcm_spi_setBitOrder(uint8_t order)
{
    bcm_spi_bit_order = order;

}

/* defaults to 0, which means a divider of 65536.
 * The divisor must be a power of 2. Odd numbers
 * rounded down. The maximum SPI clock rate is
 * of the APB clock
 */
void bcm_spi_setClockDivider(uint16_t divider)
{
    // SPI register address map offsets are specified in bytes and the associated offsets
    // in the program are specified as uint32_t or 4 bytes. Dividing by 4 is
    // therefore needed to get the 4 byte offset into the register address map.
    volatile uint32_t* paddr = bcm_spi0 + BCM_SPI0_CLK/4;
    bcm_peri_write(paddr, divider);
}

/* Set Clock Polarity and Phase. */ 
void bcm_spi_setDataMode(uint8_t mode)
{
    volatile uint32_t* paddr = bcm_spi0 + BCM_SPI0_CS/4;
    /* Mask in the CPO and CPHA bits of CS */
    bcm_peri_set_bits(paddr, mode << 2, BCM_SPI0_CS_CPOL | BCM_SPI0_CS_CPHA);

}

int bcm_spi_begin(void) {
    volatile uint32_t* paddr;

    if (bcm_spi0 == MAP_FAILED)
        return 0; /* bcm_init() failed, or not root */


    /* Set the SPI0 pins to the Alt 0 function to enable SPI0 access on them. 
     * See the BCM2835 datasheet, section 6.2, for more about alternate function
     * assignments (https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf).
     */ 
    bcm_gpio_fsel(BCM_GPIO_P1_26, BCM_GPIO_FSEL_ALT0); /* CE1 */
    bcm_gpio_fsel(BCM_GPIO_P1_24, BCM_GPIO_FSEL_ALT0); /* CE0 */
    bcm_gpio_fsel(BCM_GPIO_P1_21, BCM_GPIO_FSEL_ALT0); /* MISO */
    bcm_gpio_fsel(BCM_GPIO_P1_19, BCM_GPIO_FSEL_ALT0); /* MOSI */
    bcm_gpio_fsel(BCM_GPIO_P1_23, BCM_GPIO_FSEL_ALT0); /* CLK */

    /* Set the SPI CS register to the some sensible defaults. The address offsets
     * in the SPI address map are specified in bytes.
     * Since BCM_SPIO_CS is defined as a uint32_t, and that's 4 bytes, BCM_SPI0_CS
     * has to be divided by 4 to get the uint32_t offset.
     */
    paddr = bcm_spi0 + BCM_SPI0_CS/4;
    bcm_peri_write(paddr, 0); /* All 0s */

    /* Clear TX and RX fifos to prepare them for data transfers. */
    bcm_peri_write_nb(paddr, BCM_SPI0_CS_CLEAR);

    return 1; // OK

}

void bcm_spi_end(void)
{  
    /* Set all the SPI0 pins back to input */
    bcm_gpio_fsel(BCM_GPIO_P1_26, BCM_GPIO_FSEL_INPT); /* CE1 */
    bcm_gpio_fsel(BCM_GPIO_P1_24, BCM_GPIO_FSEL_INPT); /* CE0 */
    bcm_gpio_fsel(BCM_GPIO_P1_21, BCM_GPIO_FSEL_INPT); /* MISO */
    bcm_gpio_fsel(BCM_GPIO_P1_19, BCM_GPIO_FSEL_INPT); /* MOSI */
    bcm_gpio_fsel(BCM_GPIO_P1_23, BCM_GPIO_FSEL_INPT); /* CLK */
}

/* Write with memory barriers to peripheral */  
void bcm_peri_write(volatile uint32_t* paddr, uint32_t value)
{
    __sync_synchronize();
    *paddr = value;
    __sync_synchronize();

}

/* write to peripheral without the write barrier */
void bcm_peri_write_nb(volatile uint32_t* paddr, uint32_t value)
{
    *paddr = value;
}

/* Read with memory barriers from peripheral
 *
 */
uint32_t bcm_peri_read(volatile uint32_t* paddr)
{
    uint32_t ret;
    __sync_synchronize();
    ret = *paddr;
    __sync_synchronize();
    return ret;

}

/* read from peripheral without the read barrier
 *  * This can only be used if more reads to THE SAME peripheral
 *   * will follow.  The sequence must terminate with memory barrier
 *    * before any read or write to another peripheral can occur.
 *     * The MB can be explicit, or one of the barrier read/write calls.
 *      */
uint32_t bcm_peri_read_nb(volatile uint32_t* paddr)
{
    return *paddr;
}


/* Set the state of an output */
void bcm_gpio_write(uint8_t pin, uint8_t on)
{
    if (on)
        bcm_gpio_set(pin);
    else
        bcm_gpio_clr(pin);

}

/* Set output pin */
void bcm_gpio_set(uint8_t pin)
{
    volatile uint32_t* paddr = bcm_gpio + BCM_GPSET0/4 + pin/32;
    uint8_t shift = pin % 32;
    bcm_peri_write(paddr, 1 << shift);

}

/* Clear output pin */
void bcm_gpio_clr(uint8_t pin)
{
    volatile uint32_t* paddr = bcm_gpio + BCM_GPCLR0/4 + pin/32;
    uint8_t shift = pin % 32;
    bcm_peri_write(paddr, 1 << shift);

}

/* Some convenient arduino-like functions
 * milliseconds
 */
void bcm_delay(unsigned int millis)
{
    struct timespec sleeper;

    sleeper.tv_sec  = (time_t)(millis / 1000);
    sleeper.tv_nsec = (long)(millis % 1000) * 1000000;
    nanosleep(&sleeper, NULL);

}

/* Writes (and reads) a single byte to SPI */
uint8_t bcm_spi_transfer(uint8_t value)
{
    volatile uint32_t* paddr = bcm_spi0 + BCM_SPI0_CS/4;
    volatile uint32_t* fifo = bcm_spi0 + BCM_SPI0_FIFO/4;
    uint32_t ret;

    /* This is Polled transfer as per section 10.6.1
     * BUG ALERT: what happens if we get interupted in this section, and someone else
     * accesses a different peripheral? 
     * Clear TX and RX fifos
     **/
    bcm_peri_set_bits(paddr, BCM_SPI0_CS_CLEAR, BCM_SPI0_CS_CLEAR);

    /* Set TA = 1 */
    bcm_peri_set_bits(paddr, BCM_SPI0_CS_TA, BCM_SPI0_CS_TA);

    /* Maybe wait for TXD */
    while (!(bcm_peri_read(paddr) & BCM_SPI0_CS_TXD))
        ;

    /* Write to FIFO, no barrier */
    bcm_peri_write_nb(fifo, bcm_correct_order(value));

    /* Wait for DONE to be set */
    while (!(bcm_peri_read_nb(paddr) & BCM_SPI0_CS_DONE))
        ;

    /* Read any byte that was sent back by the slave while we sere sending to it */
    ret = bcm_correct_order(bcm_peri_read_nb(fifo));

    /* Set TA = 0, and also set the barrier */
    bcm_peri_set_bits(paddr, 0, BCM_SPI0_CS_TA);

    return ret;
}



