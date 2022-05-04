// Author: Mike McCauley
// Changes by Rich Youngkin
// Copyright (C) 2011-2013 Mike McCauley
//
//
// Original bcm_* functions and variable code by Mike McCauley at https://www.airspayce.com/mikem/bcm2835/index.html.
// This code was modified to include details about how the code works.
//
// References:
//  1. https://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf - MAX7219 LED display datasheet
//  2. https://www.airspayce.com/mikem/bcm2835/index.html - BCM2835 library documentation
//
// Build: make all
// Build DEBUG version: gcc -c -o bcmfuncs.o  bcmfuncs.c -D DEBUG 

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

/* Physical address and size of the peripherals block. These addresses come from /dev/devicetree and are physical
 * addresses. See the BCM2835 datasheet, section 1.2, Address map, at
 * https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf for details.
 * May be overridden on RPi2
 */
off_t bcm_peripherals_base = BCM_PERI_BASE;
size_t bcm_peripherals_size = BCM_PERI_SIZE;

/* Virtual memory address of the mapped peripherals block. It is set using bcm_peripherals_base and 
 * bcm_peripheral_size in the mapmem() function called from bcm_init().
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
            // buf offsets 0-3 contains the starting offset of the BCM2835 peripherals on the
            // system bus. See the BCM2835 datasheet, Section 1.2, Address map, for more details
            // (https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf).
            // Verify that the starting offset of the peripheral buss is 0x7e000000 as documented above.
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
    if (geteuid() == 0) // 'root'
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
         * The offsets below are defined as uint32_t's. Access to the registers is done via
         * pointer arithmetic. How pointer arithmetic works is dependent on the type of the pointer
         * variable being acted upon. For uint32_t types, they're 4 bytes, so pointers will be adjusted
         * by 4 bytes with each operation. Given this, all offsets, which are in bytes, need to be 
         * divided by 4 to account for how pointer arithmetic works on uint32_t's.
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
    //       (v&~mask) | (value&mask) = 1100 0111 // results in the new value, bit 3 set to 0 and bit 2 set to 1
    //
    v = (v & ~mask) | (value & mask);
    bcm_peri_write(paddr, v);

}

/* Function select
// pin is a BCM2835 GPIO pin number NOT RPi pin number
// See https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf, section 6, GPIO, for details.
// There are 6 control registers (section 6, Register View table), each control the functions of a block
//      of 10 pins (table 6.1, GPIO Function Select Registers).
//      Each control register has 10 sets of 3 bits per GPIO pin, 32 bits per control register:
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
// So the offset for the 3 bits for port X are:
//      (BCM_GPFSEL0/4) 0 + X / 10 + ((X % 10) * 3) (e.g., GPIO pin 19's offset is 28 from the beginning of 
//      GPIO Alternate function select register 1, register offset 27)
*/
void bcm_gpio_fsel(uint8_t pin, uint8_t mode)
{
    // Example with GPIO pin 10 (BCM_GPIO_P1_19 is GPIO pin 10) which is the MOSI pin at GPFSEL1 (BCM_GPFSEL0 + 0x4)
    // See the GPIO Register address space - section 6.1, Register view, in the BCM2835 datasheet - 
    // https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf.
    //
    // NOTE: BCM_GPFSEL0 (0x0) is still used in the offset calculations below. pin#/10 steps the address forward
    // as necessary (e.g., (pin)9/10=0, (pin)19/10=1, and so on steps the address forward to the appropriate
    // GPIO Function Select register)
    //      10(pin)/10 = 1
    //          paddr = bcm_gpio + 1 + 0( (BCM_GPFSEL0 + 0x0)/4 )
    //      10(pin)%10 = 0
    //      shift = 0(from previous calc)*3 = 0
    //      GPIO_FSEL_MASK = 0111 (0x7)
    //      mask    = 0000 0000 0000 0000 0000 0000 0000 0111 (shifted left by 0 bits)
    //      mode    = 0000 0000 0000 0000 0000 0000 0000 0100 ==> BCM_GPIO_FSEL_ALT0 (0x4)
    //      new
    //      value   = 0000 0000 0000 0000 0000 0000 0000 0100
    //      current 
    //      value   = 0000 0000 0000 0000 0000 0000 1111 0000
    //      -------------------------------------------------
    //      result  = 0000 0000 0000 0000 0000 0000 1111 0100 (in GPIO Alt function register 1 after bcm_peri_set_bits())
    //
    // So, GPIO pin 10 is in GPIO Alt function select register 1 which is at an offset of 0x4 or
    // the second 32 bit block from bcm_gpio. GPIO pin 10 function settings are at bit offsets
    // 0-2. 'value' above, coupled with 'mask', will set bits 0-2 to 100 in function select register 1
    // which corresponds to ALT0 on GPIO pin 10.
    //
    //  The offsets below are defined as uint32_t's. Access to the registers is done via
    //  pointer arithmetic. How pointer arithmetic works is dependent on the type of the pointer
    //  variable being acted upon. For uint32_t types, they're 4 bytes, so pointers will be adjusted
    //  by 4 bytes with each operation. Given this, all offsets, which are in bytes, need to be
    //  divided by 4 to account for how pointer arithmetic works on uint32_t's.

    volatile uint32_t* paddr = bcm_gpio + BCM_GPFSEL0/4 + (pin/10);
    uint8_t   shift = (pin % 10) * 3;
    uint32_t  mask = BCM_GPIO_FSEL_MASK << shift;
    uint32_t  value = mode << shift;
    bcm_peri_set_bits(paddr, value, mask);
}

// MSB or LSB (BCM2835 only accepts MSB but bcm_correct_order() is used to ensure this)
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
    /*
     * The offsets below are defined as uint32_t's. Access to the registers is done via
     * pointer arithmetic. How pointer arithmetic works is dependent on the type of the pointer
     * variable being acted upon. For uint32_t types, they're 4 bytes, so pointers will be adjusted
     * by 4 bytes with each operation. Given this, all offsets, which are in bytes, need to be 
     * divided by 4 to account for how pointer arithmetic works on uint32_t's.
     */
    volatile uint32_t* paddr = bcm_spi0 + BCM_SPI0_CLK/4;
    bcm_peri_write(paddr, divider);
}

/* Set Clock Polarity and Phase. */ 
void bcm_spi_setDataMode(uint8_t mode)
{
    //  The offsets below are defined as uint32_t's. Access to the registers is done via
    //  pointer arithmetic. How pointer arithmetic works is dependent on the type of the pointer
    //  variable being acted upon. For uint32_t types, they're 4 bytes, so pointers will be adjusted
    //  by 4 bytes with each operation. Given this, all offsets, which are in bytes, need to be
    //  divided by 4 to account for how pointer arithmetic works on uint32_t's.
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

    /* Set the SPI CS register to the some sensible defaults.
     * The offsets below are defined as uint32_t's. Access to the registers is done via
     * pointer arithmetic. How pointer arithmetic works is dependent on the type of the pointer
     * variable being acted upon. For uint32_t types, they're 4 bytes, so pointers will be adjusted
     * by 4 bytes with each operation. Given this, all offsets, which are in bytes, need to be 
     * divided by 4 to account for how pointer arithmetic works on uint32_t's.
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

/* Read with memory barriers from peripheral */
uint32_t bcm_peri_read(volatile uint32_t* paddr)
{
    uint32_t ret;
    __sync_synchronize();
    ret = *paddr;
    __sync_synchronize();
    return ret;

}

/* read from peripheral without the read barrier
 * This can only be used if more reads to THE SAME peripheral
 * will follow.  The sequence must terminate with memory barrier
 * before any read or write to another peripheral can occur.
 * The MB can be explicit, or one of the barrier read/write calls.
 */
uint32_t bcm_peri_read_nb(volatile uint32_t* paddr)
{
    return *paddr;
}


/* Set the state of an output pin (e.g., HIGH or LOW voltage) */
void bcm_gpio_write(uint8_t pin, uint8_t on)
{
    if (on)
        bcm_gpio_set(pin);
    else
        bcm_gpio_clr(pin);

}

/* Set output pin to HIGH voltage */
void bcm_gpio_set(uint8_t pin)
{
    //  The offsets below are defined as uint32_t's. Access to the registers is done via
    //  pointer arithmetic. How pointer arithmetic works is dependent on the type of the pointer
    //  variable being acted upon. For uint32_t types, they're 4 bytes, so pointers will be adjusted
    //  by 4 bytes with each operation. Given this, all offsets, which are in bytes, need to be
    //  divided by 4 to account for how pointer arithmetic works on uint32_t's.
    volatile uint32_t* paddr = bcm_gpio + BCM_GPSET0/4 + pin/32;
    uint8_t shift = pin % 32;
    bcm_peri_write(paddr, 1 << shift);

}

/* Clear output pin (i.e., to LOW voltage) */
void bcm_gpio_clr(uint8_t pin)
{
    //  The offsets below are defined as uint32_t's. Access to the registers is done via
    //  pointer arithmetic. How pointer arithmetic works is dependent on the type of the pointer
    //  variable being acted upon. For uint32_t types, they're 4 bytes, so pointers will be adjusted
    //  by 4 bytes with each operation. Given this, all offsets, which are in bytes, need to be
    //  divided by 4 to account for how pointer arithmetic works on uint32_t's.
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
    //  The offsets below are defined as uint32_t's. Access to the registers is done via
    //  pointer arithmetic. How pointer arithmetic works is dependent on the type of the pointer
    //  variable being acted upon. For uint32_t types, they're 4 bytes, so pointers will be adjusted
    //  by 4 bytes with each operation. Given this, all offsets, which are in bytes, need to be
    //  divided by 4 to account for how pointer arithmetic works on uint32_t's.
    volatile uint32_t* paddr = bcm_spi0 + BCM_SPI0_CS/4;
    volatile uint32_t* fifo = bcm_spi0 + BCM_SPI0_FIFO/4;
    uint32_t ret;

    /* This is Polled transfer as per section 10.6.1 in the BCM2835 datasheet at
     * https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
     * BUG ALERT: what happens if we get interupted in this section, and someone else
     * accesses a different peripheral? 
     * Clear TX and RX fifos
     **/
    bcm_peri_set_bits(paddr, BCM_SPI0_CS_CLEAR, BCM_SPI0_CS_CLEAR);

    /* Set TA = 1, data transfer is active */
    bcm_peri_set_bits(paddr, BCM_SPI0_CS_TA, BCM_SPI0_CS_TA);

    /* Maybe wait for TXD (e.g., if it can't accept any data because the FIFO is full) */
    while (!(bcm_peri_read(paddr) & BCM_SPI0_CS_TXD))
        ;

    /* Write to FIFO, no barrier */
    bcm_peri_write_nb(fifo, bcm_correct_order(value));

    /* Wait for DONE to be set */
    while (!(bcm_peri_read_nb(paddr) & BCM_SPI0_CS_DONE))
        ;

    /* Read any byte that was sent back by the slave while we sere sending to it */
    ret = bcm_correct_order(bcm_peri_read_nb(fifo));

    /* Set TA = 0 (transfer is finished), and also set the barrier */
    bcm_peri_set_bits(paddr, 0, BCM_SPI0_CS_TA);

    return ret;
}

