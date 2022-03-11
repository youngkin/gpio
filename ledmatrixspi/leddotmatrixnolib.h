// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file.
//
//  This is a highly modified version of the bcm2835.h from the BCM2835 C library by
//  Mice McCauley. See https://www.airspayce.com/mikem/bcm2835/index.html for the original.
//
//  The modifications include renaming the variables and #defines to avoid conflicts with the
//  original library. The file was also modified to keep only the #defines, enums, and typedefs
//  needed by this project. The point of this project is to demonstrate how to program the BCM2835
//  board directly without using any libraries (like the BCM2835 library). The original code has
//  also been extensively commented in order to explain to newcomers to BCM2835 board 
//  programming what is going on.
//
/* Defines for BCM */
#ifndef BCM_H
#define BCM_H

#include <stdlib.h>
#include <stdint.h>

#define uchar unsigned char
#define uint unsigned int

/*! On all recent OSs, the base physical address of the peripherals is read from a /proc file */
#define BCM_RPI2_DT_FILENAME "/proc/device-tree/soc/ranges"

/*! This means pin HIGH, true, 3.3volts on a pin. */
#define HIGH 0x1
/*! This means pin LOW, false, 0volts on a pin. */
#define LOW  0x0

/*! Physical addresses for various peripheral register sets
 *  Base Physical Address of the BCM 2835 peripheral registers
 *  Note this is different for the RPi2 BCM2836, where this is derived from /proc/device-tree/soc/ranges
 *  If /proc/device-tree/soc/ranges exists on a RPi 1 OS, it would be expected to contain the
 *  following numbers:
 */
/*! Peripherals block base address on RPi 1 */
#define BCM_PERI_BASE               0x20000000
/*! Size of the peripherals block on RPi 1 */
#define BCM_PERI_SIZE               0x01000000
/*! Alternate base address for RPI  2 / 3 */
#define BCM_RPI2_PERI_BASE          0x3F000000
/*! Alternate base address for RPI  4 */
#define BCM_RPI4_PERI_BASE          0xFE000000
/*! Alternate size for RPI  4 */
#define BCM_RPI4_PERI_SIZE          0x01800000

/*! Offsets for the bases of various peripherals within the peripherals block */

/*   Base Address of the System Timer registers */
#define BCM_ST_BASE                 0x3000
/*! Base Address of the Pads registers */
#define BCM_GPIO_PADS               0x100000
/*! Base Address of the Clock/timer registers */
#define BCM_CLOCK_BASE              0x101000
/*! Base Address of the GPIO registers */
#define BCM_GPIO_BASE               0x200000
/*! Base Address of the SPI0 registers */
#define BCM_SPI0_BASE               0x204000
/*! Base Address of the BSC0 registers */
#define BCM_BSC0_BASE               0x205000
/*! Base Address of the PWM registers */
#define BCM_GPIO_PWM                0x20C000
/*! Base Address of the AUX registers */
#define BCM_AUX_BASE                0x215000
/*! Base Address of the AUX_SPI1 registers */
#define BCM_SPI1_BASE               0x215080
/*! Base Address of the AUX_SPI2 registers */
#define BCM_SPI2_BASE               0x2150C0
/*! Base Address of the BSC1 registers */
#define BCM_BSC1_BASE               0x804000

/* Defines for GPIO
 *    The BCM2835 has 54 GPIO pins.
 *    BCM2835 data sheet, Page 90 onwards - https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
 */
/*! GPIO register offsets from BCM_GPIO_BASE. 
 *   Offsets into the GPIO Peripheral block in bytes per 6.1 Register View 
 */
#define BCM_GPFSEL0                      0x0000 /*!< GPIO Function Select 0 */
#define BCM_GPSET0                       0x001c /*!< GPIO Pin Output Set 0 */
#define BCM_GPCLR0                       0x0028 /*!< GPIO Pin Output Clear 0 */

/*! \brief bcmSPIBitOrder SPI Bit order
 *   Specifies the SPI data bit ordering for bcm_spi_setBitOrder()
 */
typedef enum
{
    BCM_SPI_BIT_ORDER_LSBFIRST = 0,  /*!< LSB First */
    BCM_SPI_BIT_ORDER_MSBFIRST = 1   /*!< MSB First */

}bcmSPIBitOrder;

/* See https://pinout.xyz for pin numbering details
 * This enum maps from the Pi pin numbering scheme to the GPIO pin numbering
 * scheme. For example, BCM_GPIO_P1_19 maps the Pi pin 19 to the GPIO pin 10.
 */
typedef enum
{
    BCM_GPIO_P1_19        = 10,  /*!< Version 1, Pin P1-19, MOSI when SPI0 in use */
    BCM_GPIO_P1_21        =  9,  /*!< Version 1, Pin P1-21, MISO when SPI0 in use */
    BCM_GPIO_P1_23        = 11,  /*!< Version 1, Pin P1-23, CLK when SPI0 in use */
    BCM_GPIO_P1_24        =  8,  /*!< Version 1, Pin P1-24, CE0 when SPI0 in use */
    BCM_GPIO_P1_26        =  7,  /*!< Version 1, Pin P1-26, CE1 when SPI0 in use */
} GPIOPin;

typedef enum
{
    BCM_GPIO_FSEL_INPT  = 0x00,   /*!< Input 0b000 */
    BCM_GPIO_FSEL_OUTP  = 0x01,   /*!< Output 0b001 */
    BCM_GPIO_FSEL_ALT0  = 0x04,   /*!< Alternate function 0 0b100 */
    BCM_GPIO_FSEL_ALT1  = 0x05,   /*!< Alternate function 1 0b101 */
    BCM_GPIO_FSEL_ALT2  = 0x06,   /*!< Alternate function 2 0b110, */
    BCM_GPIO_FSEL_ALT3  = 0x07,   /*!< Alternate function 3 0b111 */
    BCM_GPIO_FSEL_ALT4  = 0x03,   /*!< Alternate function 4 0b011 */
    BCM_GPIO_FSEL_ALT5  = 0x02,   /*!< Alternate function 5 0b010 */
    BCM_GPIO_FSEL_MASK  = 0x07    /*!< Function select bits mask 0b111 */


} bcmFunctionSelect;

/* Defines for SPI
 * GPIO register offsets from BCM_SPI0_BASE. 
 * Offsets into the SPI Peripheral block in bytes per 10.5 SPI Register Map
 *  https://www.raspberrypi.org/app/uploads/2012/02/BCM2835-ARM-Peripherals.pdf
 */
#define BCM_SPI0_CS                      0x0000 /*!< SPI Master Control and Status */
#define BCM_SPI0_FIFO                    0x0004 /*!< SPI Master TX and RX FIFOs */
#define BCM_SPI0_CLK                     0x0008 /*!< SPI Master Clock Divider */
#define BCM_SPI0_DLEN                    0x000c /*!< SPI Master Data Length */
#define BCM_SPI0_LTOH                    0x0010 /*!< SPI LOSSI mode TOH */
#define BCM_SPI0_DC                      0x0014 /*!< SPI DMA DREQ Controls */

/* Register masks for SPI0_CS (Master control & status) */
#define BCM_SPI0_CS_LEN_LONG             0x02000000 /*!< Enable Long data word in Lossi mode if DMA_LEN is set */
#define BCM_SPI0_CS_DMA_LEN              0x01000000 /*!< Enable DMA mode in Lossi mode */
#define BCM_SPI0_CS_CSPOL2               0x00800000 /*!< Chip Select 2 Polarity */
#define BCM_SPI0_CS_CSPOL1               0x00400000 /*!< Chip Select 1 Polarity */
#define BCM_SPI0_CS_CSPOL0               0x00200000 /*!< Chip Select 0 Polarity */
#define BCM_SPI0_CS_RXF                  0x00100000 /*!< RXF - RX FIFO Full */
#define BCM_SPI0_CS_RXR                  0x00080000 /*!< RXR RX FIFO needs Reading (full) */
#define BCM_SPI0_CS_TXD                  0x00040000 /*!< TXD TX FIFO can accept Data */
#define BCM_SPI0_CS_RXD                  0x00020000 /*!< RXD RX FIFO contains Data */
#define BCM_SPI0_CS_DONE                 0x00010000 /*!< Done transfer Done */
#define BCM_SPI0_CS_TE_EN                0x00008000 /*!< Unused */
#define BCM_SPI0_CS_LMONO                0x00004000 /*!< Unused */
#define BCM_SPI0_CS_LEN                  0x00002000 /*!< LEN LoSSI enable */
#define BCM_SPI0_CS_REN                  0x00001000 /*!< REN Read Enable */
#define BCM_SPI0_CS_ADCS                 0x00000800 /*!< ADCS Automatically Deassert Chip Select */
#define BCM_SPI0_CS_INTR                 0x00000400 /*!< INTR Interrupt on RXR */
#define BCM_SPI0_CS_INTD                 0x00000200 /*!< INTD Interrupt on Done */
#define BCM_SPI0_CS_DMAEN                0x00000100 /*!< DMAEN DMA Enable */
#define BCM_SPI0_CS_TA                   0x00000080 /*!< Transfer Active */
#define BCM_SPI0_CS_CSPOL                0x00000040 /*!< Chip Select Polarity */
#define BCM_SPI0_CS_CLEAR                0x00000030 /*!< Clear FIFO Clear RX and TX */
#define BCM_SPI0_CS_CLEAR_RX             0x00000020 /*!< Clear FIFO Clear RX  */
#define BCM_SPI0_CS_CLEAR_TX             0x00000010 /*!< Clear FIFO Clear TX  */
#define BCM_SPI0_CS_CPOL                 0x00000008 /*!< Clock Polarity */
#define BCM_SPI0_CS_CPHA                 0x00000004 /*!< Clock Phase */
#define BCM_SPI0_CS_CS                   0x00000003 /*!< Chip Select */

/*! \brief SPI Data mode
 *   Specify the SPI data mode to be passed to bcm_spi_setDataMode()
 *   (clock polarity and phase).
 */
typedef enum
{
    BCM_SPI_MODE0 = 0,  /*!< CPOL = 0, CPHA = 0 */
    BCM_SPI_MODE1 = 1,  /*!< CPOL = 0, CPHA = 1 */
    BCM_SPI_MODE2 = 2,  /*!< CPOL = 1, CPHA = 0 */
    BCM_SPI_MODE3 = 3   /*!< CPOL = 1, CPHA = 1 */

}bcmSPIMode;

/*! \brief bcmSPIChipSelect
 *   Specify the SPI chip select pin(s) (GPIO poins 7 & 8, SPI0CE1 & SPICE0)
 *   */
typedef enum
{
    BCM_SPI_CS0 = 0,     /*!< Chip Select 0 */
    BCM_SPI_CS1 = 1,     /*!< Chip Select 1 */
    BCM_SPI_CS2 = 2,     /*!< Chip Select 2 (ie pins CS1 and CS2 are asserted) */
    BCM_SPI_CS_NONE = 3  /*!< No CS, control it yourself */

} bcmSPIChipSelect;

/*! \brief bcmSPIClockDivider
 *  Specifies the divider used to generate the SPI clock from the system clock.
 *  Figures below give the divider, clock period and clock frequency.
 *  Clock divided is based on nominal core clock rate of 250MHz on RPi1 and RPi2, and 400MHz on RPi3.
 *  It is reported that (contrary to the documentation) any even divider may used.
 *  The frequencies shown for each divider have been confirmed by measurement on RPi1 and RPi2.
 *  The system clock frequency on RPi3 is different, so the frequency you get from a given divider will be different.
 *  See comments in 'SPI Pins' for information about reliable SPI speeds. Also see
 *  https://www.airspayce.com/mikem/bcm2835/group__constants.html#gaf2e0ca069b8caef24602a02e8a00884e for details.
 *  Note: it is possible to change the core clock rate of the RPi 3 back to 250MHz, by putting 
 *  \code
 *  core_freq=250
 *  \endcode
 *  in the config.txt
 */
typedef enum
{
    BCM_SPI_CLOCK_DIVIDER_65536 = 0,       /*!< 65536 = 3.814697260kHz on Rpi2, 6.1035156kHz on RPI3 */
    BCM_SPI_CLOCK_DIVIDER_32768 = 32768,   /*!< 32768 = 7.629394531kHz on Rpi2, 12.20703125kHz on RPI3 */
    BCM_SPI_CLOCK_DIVIDER_16384 = 16384,   /*!< 16384 = 15.25878906kHz on Rpi2, 24.4140625kHz on RPI3 */
    BCM_SPI_CLOCK_DIVIDER_8192  = 8192,    /*!< 8192 = 30.51757813kHz on Rpi2, 48.828125kHz on RPI3 */
    BCM_SPI_CLOCK_DIVIDER_4096  = 4096,    /*!< 4096 = 61.03515625kHz on Rpi2, 97.65625kHz on RPI3 */
    BCM_SPI_CLOCK_DIVIDER_2048  = 2048,    /*!< 2048 = 122.0703125kHz on Rpi2, 195.3125kHz on RPI3 */
    BCM_SPI_CLOCK_DIVIDER_1024  = 1024,    /*!< 1024 = 244.140625kHz on Rpi2, 390.625kHz on RPI3 */
    BCM_SPI_CLOCK_DIVIDER_512   = 512,     /*!< 512 = 488.28125kHz on Rpi2, 781.25kHz on RPI3 */
    BCM_SPI_CLOCK_DIVIDER_256   = 256,     /*!< 256 = 976.5625kHz on Rpi2, 1.5625MHz on RPI3 */
    BCM_SPI_CLOCK_DIVIDER_128   = 128,     /*!< 128 = 1.953125MHz on Rpi2, 3.125MHz on RPI3 */
    BCM_SPI_CLOCK_DIVIDER_64    = 64,      /*!< 64 = 3.90625MHz on Rpi2, 6.250MHz on RPI3 */
    BCM_SPI_CLOCK_DIVIDER_32    = 32,      /*!< 32 = 7.8125MHz on Rpi2, 12.5MHz on RPI3 */
    BCM_SPI_CLOCK_DIVIDER_16    = 16,      /*!< 16 = 15.625MHz on Rpi2, 25MHz on RPI3 */
    BCM_SPI_CLOCK_DIVIDER_8     = 8,       /*!< 8 = 31.25MHz on Rpi2, 50MHz on RPI3 */
    BCM_SPI_CLOCK_DIVIDER_4     = 4,       /*!< 4 = 62.5MHz on Rpi2, 100MHz on RPI3. Dont expect this speed to work reliably. */
    BCM_SPI_CLOCK_DIVIDER_2     = 2,       /*!< 2 = 125MHz on Rpi2, 200MHz on RPI3, fastest you can get. Dont expect this speed to work reliably.*/
    BCM_SPI_CLOCK_DIVIDER_1     = 1        /*!< 1 = 3.814697260kHz on Rpi2, 6.1035156kHz on RPI3, same as 0/65536 */

} bcmSPIClockDivider;

/*! Physical address and size of the peripherals block
 *   May be overridden on RPi2
 */
extern off_t bcm_peripherals_base;
/*! Size of the peripherals block to be mapped */
extern size_t bcm_peripherals_size;

/*! Virtual memory address of the mapped peripherals block */
extern uint32_t *bcm_peripherals;

/*! Base of the ST (System Timer) registers.
 *   Available after bcm_init has been called (as root)
 *   */
extern volatile uint32_t *bcm_st;

/*! Base of the GPIO registers.
 *   Available after bcm_init has been called
 *   */
extern volatile uint32_t *bcm_gpio;

/*! Base of the PWM registers.
 *   Available after bcm_init has been called (as root)
 *   */
extern volatile uint32_t *bcm_pwm;

/*! Base of the CLK registers.
 *   Available after bcm_init has been called (as root)
 *   */
extern volatile uint32_t *bcm_clk;

/*! Base of the PADS registers.
 *   Available after bcm_init has been called (as root)
 *   */
extern volatile uint32_t *bcm_pads;

/*! Base of the SPI0 registers.
 *   Available after bcm_init has been called (as root)
 *   */
extern volatile uint32_t *bcm_spi0;

/*! Base of the BSC0 registers.
 *   Available after bcm_init has been called (as root)
 *   */
extern volatile uint32_t *bcm_bsc0;

/*! Base of the BSC1 registers.
 *   Available after bcm_init has been called (as root)
 *   */
extern volatile uint32_t *bcm_bsc1;

/*! Base of the AUX registers.
 *   Available after bcm_init has been called (as root)
 *   */
extern volatile uint32_t *bcm_aux;

/*! Base of the SPI1 registers.
 *   Available after bcm_init has been called (as root)
 *   */
extern volatile uint32_t *bcm_spi1;

/*! Sets the SPI data mode
 * Sets the clock polariy and phase
 * \param[in] mode The desired data mode, one of BCM2835_SPI_MODE*, 
 * see \ref bcmSPIMode
 */
extern void bcm_spi_setDataMode(uint8_t mode);

#define Max7219_pinCS  BCM_GPIO_P1_24 /* Pi pin 24, GPIO pin 8 - from bcm2835.h */
#define ROWS 37 /* number of characters to display on the LED matrix */
#define COLS 8 /* LED matrix row values (i.e., which LEDs to light on a given matrix row) */

/* Gracefully handle interrupts, releases all resources prior to exiting */
void interruptHandler(int);

/* Sets bit values in the various BCM2835 registers */
void bcm_peri_set_bits(volatile uint32_t*, uint32_t, uint32_t);

/* Sets the function of a given pin (e.g., ALT0) */
void bcm_gpio_fsel(uint8_t, uint8_t);

/* Writes data to a peripheral with memory barrier (e.g., and LED matrix) */
void bcm_peri_write(volatile uint32_t*, uint32_t);

/* Writes data to a peripheral without a memory barrier (e.g., and LED matrix) */
void bcm_peri_write_nb(volatile uint32_t*, uint32_t);

/* reads data from a peripheral */
uint32_t bcm_peri_read(volatile uint32_t*);

/* sets a GPIO pin to HIGH voltage */
void bcm_gpio_set(uint8_t);

/* sets the GPIO pin to LOW voltage */
void bcm_gpio_clr(uint8_t);

/* delays execution until the specified milliseconds have passed */
void bcm_delay(unsigned int);

/* initializes gpio pins to function in SPI mode */
int bcm_spi_begin(void);

/* initializes the BCM2835 board */
int bcm_init(void);

/* resets gpio pins back to input state */
void bcm_spi_end(void);

/* releases mapped memory and resets bcm memory addresses to default values */
int bcm_close(void);

/*! Transfers one byte to and from the currently selected SPI slave.
 *  Asserts the currently selected CS pins (as previously set by bcm_spi_chipSelect) 
 *  during the transfer.
 *  Clocks the 8 bit value out on MOSI, and simultaneously clocks in data from MISO. 
 *  Returns the read data byte from the slave.
 *  Uses polled transfer as per section 10.6.1 of the BCM 2835 ARM Peripherls manual
 *  \param[in] value The 8 bit data byte to write to MOSI
 *  \return The 8 bit byte simultaneously read from  MISO
 *  \sa bcm_spi_transfern()
 */
extern uint8_t bcm_spi_transfer(uint8_t value);

/* sets the SPI interface to LSB or MSB */
void bcm_spi_setBitOrder(uint8_t);

/* sets the SPI clock frequency (divides the RPi clock speed by the specified divisor) */
void bcm_spi_setClockDivider(uint16_t);


/*! Sets the output state of the specified pin
 *       \param[in] pin GPIO number, or one of RPI_GPIO_P1_* from \ref RPiGPIOPin.
 *       \param[in] on HIGH sets the output to HIGH and LOW to LOW.
 */
void bcm_gpio_write(uint8_t pin, uint8_t on);

#endif /* BCM_H */
