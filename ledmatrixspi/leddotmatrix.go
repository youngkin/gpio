//
// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// See the LICENSE file for details.
//
// Run using 'go run leddotmatrix.go'
//

package main

import (
	"fmt"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/stianeikeland/go-rpio/v4"
)

const csPin = rpio.Pin(uint8(8))
const ROWS = 37
const COLS = 8

var disp1 = [][]byte{
	{0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C}, //0
	{0x08, 0x18, 0x28, 0x08, 0x08, 0x08, 0x08, 0x08}, //1
	{0x7E, 0x2, 0x2, 0x7E, 0x40, 0x40, 0x40, 0x7E},   //2
	{0x3E, 0x2, 0x2, 0x3E, 0x2, 0x2, 0x3E, 0x0},      //3
	{0x8, 0x18, 0x28, 0x48, 0xFE, 0x8, 0x8, 0x8},     //4
	{0x3C, 0x20, 0x20, 0x3C, 0x4, 0x4, 0x3C, 0x0},    //5
	{0x3C, 0x20, 0x20, 0x3C, 0x24, 0x24, 0x3C, 0x0},  //6
	{0x3E, 0x22, 0x4, 0x8, 0x8, 0x8, 0x8, 0x8},       //7
	{0x0, 0x3E, 0x22, 0x22, 0x3E, 0x22, 0x22, 0x3E},  //8
	{0x3E, 0x22, 0x22, 0x3E, 0x2, 0x2, 0x2, 0x3E},    //9
	{0x8, 0x14, 0x22, 0x3E, 0x22, 0x22, 0x22, 0x22},  //A
	{0x3C, 0x22, 0x22, 0x3E, 0x22, 0x22, 0x3C, 0x0},  //B
	{0x3C, 0x40, 0x40, 0x40, 0x40, 0x40, 0x3C, 0x0},  //C
	{0x7C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x7C, 0x0},  //D
	{0x7C, 0x40, 0x40, 0x7C, 0x40, 0x40, 0x40, 0x7C}, //E
	{0x7C, 0x40, 0x40, 0x7C, 0x40, 0x40, 0x40, 0x40}, //F
	{0x3C, 0x40, 0x40, 0x40, 0x40, 0x44, 0x44, 0x3C}, //G
	{0x44, 0x44, 0x44, 0x7C, 0x44, 0x44, 0x44, 0x44}, //H
	{0x7C, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x7C}, //I
	{0x3C, 0x8, 0x8, 0x8, 0x8, 0x8, 0x48, 0x30},      //J
	{0x0, 0x24, 0x28, 0x30, 0x20, 0x30, 0x28, 0x24},  //K
	{0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x7C}, //L
	{0x81, 0xC3, 0xA5, 0x99, 0x81, 0x81, 0x81, 0x81}, //M
	{0x0, 0x42, 0x62, 0x52, 0x4A, 0x46, 0x42, 0x0},   //N
	{0x3C, 0x42, 0x42, 0x42, 0x42, 0x42, 0x42, 0x3C}, //O
	{0x3C, 0x22, 0x22, 0x22, 0x3C, 0x20, 0x20, 0x20}, //P
	{0x1C, 0x22, 0x22, 0x22, 0x22, 0x26, 0x22, 0x1D}, //Q
	{0x3C, 0x22, 0x22, 0x22, 0x3C, 0x24, 0x22, 0x21}, //R
	{0x0, 0x1E, 0x20, 0x20, 0x3E, 0x2, 0x2, 0x3C},    //S
	{0x0, 0x3E, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8},        //T
	{0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x3E}, //U
	{0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x14, 0x8},  //V
	//    {0x42,0x42,0x42,0x42,0x42,0x42,0x22,0x1C},  //U
	//    {0x42,0x42,0x42,0x42,0x42,0x42,0x24,0x18},  //V
	{0x0, 0x49, 0x49, 0x49, 0x49, 0x2A, 0x1C, 0x0},   //W
	{0x0, 0x41, 0x22, 0x14, 0x8, 0x14, 0x22, 0x41},   //X
	{0x41, 0x22, 0x14, 0x8, 0x8, 0x8, 0x8, 0x8},      //Y
	{0x0, 0x7F, 0x2, 0x4, 0x8, 0x10, 0x20, 0x7F},     //Z
	{0x18, 0x24, 0x42, 0xFF, 0x42, 0x24, 0x18, 0x00}, //Theta
}

func main() {
	// stop channel is used to synchronize exiting the
	// program so that the board is reset to the state
	// it was in prior to the program starting.
	stop := make(chan interface{})
	// sigs is the channel used by Go's signals capability
	// to notify the program that a signal has been raised.
	sigs := make(chan os.Signal)

	// signal.Notify() registers the program's interest
	// in receiving signals and provides the channel used
	// to send signals to the program.
	signal.Notify(sigs, syscall.SIGINT, syscall.SIGKILL)
	go signalHandler(sigs, stop)

	// Initialize the rpio library
	if err := rpio.Open(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	if err := rpio.SpiBegin(rpio.Spi0); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	csPin.Output()
	rpio.SpiChipSelect(uint8(csPin)) // Select CE0 as slave
	initMax7219()

	for {
		select {
		case <-stop:
			break
		default:
			for i := 0; i < ROWS; i++ {
				for j := 1; j < COLS+1; j++ {
					//fmt.Printf("Write %x at address %d for i=%d and j=%d\n", disp1[i][j-1], j, i, j)
					writeMax7219(byte(j), disp1[i][j-1])
				}
				time.Sleep(500 * time.Millisecond)
			}
			break
		}
	}
	rpio.SpiEnd(rpio.Spi0)
	rpio.Close()
}

func initMax7219() {
	writeMax7219(0x09, 0x00) // Decode mode register
	writeMax7219(0x0a, 0x03) //  medium brightness
	//    writeMax7219(0x0a,0x0f);// max brightness
	writeMax7219(0x0b, 0x07) // Scan limit register
	writeMax7219(0x0c, 0x01) // Shutdown register
	writeMax7219(0x0f, 0x00) // Display test register, normal mode
	//    writeMax7219(0x0f,0x01);// Display test register, test mode (light all leds)
}

func writeMax7219(addr byte, value byte) {
	csPin.Low()
	writeMax7219Byte(addr)
	writeMax7219Byte(value)
	csPin.High()
}

func writeMax7219Byte(b byte) {
	rpio.SpiTransmit(b)
}

func signalHandler(sigs chan os.Signal, stop chan interface{}) {
	<-sigs
	// notify all listeners that the program is stopping
	close(stop)

	fmt.Println("\nExiting...\n")

	for i := 1; i < COLS+1; i++ {
		writeMax7219(byte(i), 0x00)
	}

	rpio.SpiEnd(rpio.Spi0)
	rpio.Close()

	os.Exit(0)
}