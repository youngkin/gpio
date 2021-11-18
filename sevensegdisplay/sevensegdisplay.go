//
// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file.
//
// This program is designed to be used with a 74HC595 Shift Register and a common cathode
// seven segment display.
//
// Run: go run sevensegdisplay.go
//
package main

import (
	"bufio"
	"fmt"
	"os"
	"os/signal"
	"strings"
	"syscall"
	"time"

	"github.com/stianeikeland/go-rpio/v4"
)

const ()

// segcode contains the hexidecimal codes that will be left-shifted into the shift register. They
// correspond to the 7-segment display numbers 0-F and the decimal point respectively.
var segcode = []int{0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77,
	0x7c, 0x39, 0x5e, 0x79, 0x71, 0x80}

func main() {
	// Initialize the go-rpio library, exiting if there's a problem.
	if err := rpio.Open(); err != nil {
		os.Exit(1)

	}
	// Release go-rpio resources prior to exiting program
	defer rpio.Close()

	// sdiPin   => Serial data input pin (aka SER or DS)
	// rclkPin  => Output Register Clock/Latch pin (st_cp)
	// srclkPin => Shift Register Clock pin (sh_cp)
	// srclrPin => Shift Register Clear pin
	// oePin    => Output Enable Pin
	sdiPin, rclkPin, srclkPin, srclrPin, oePin := initShiftRegister()

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
	go signalHandler(sigs, stop, rclkPin, srclrPin)

	reader := bufio.NewReader(os.Stdin)
	for {
		select {
		// Listen for an exit message from the signal/interrupt handler. Receipt of
		// this message directs the main goroutine to exit.
		case <-stop:
			break
		default:
			fmt.Printf("\nEnter [n]umbers, [s]hift register clear, [z]ero clear, [w]rite ones, [o]e toggle, [q]uit: ")
			choice, err := reader.ReadString('\n')
			if err != nil {
				fmt.Printf("Error reading from terminal, %s", err)
				os.Exit(1)

			}
			choice = strings.TrimSuffix(choice, "\n")

			switch choice {
			case "n":
				fmt.Printf("\tDemonstrate displaying hexidecimal digits 0 thru F and the decimal point.\n")
				fmt.Printf("\tEach number will be displayed in turn and then cleared.\n")
				time.Sleep(time.Second)
				testWriteNums(sdiPin, rclkPin, srclkPin, srclrPin)
				break
			case "s":
				fmt.Printf("\tDemonstrate the effects of clearing the shift register via SRCLR.\n")
				fmt.Printf("\tThe display will briefly display '8' and then cleared.\n")
				time.Sleep(time.Second)
				testShiftRegClr(sdiPin, rclkPin, srclkPin, srclrPin)
				break
			case "z":
				fmt.Printf("\tDemonstrate the effects of clearing the shift register by writing zeros to the shift register.\n")
				fmt.Printf("\tThe display will briefly display '8' and then cleared.\n")
				time.Sleep(time.Second)
				testZeroClr(sdiPin, rclkPin, srclkPin)
				break
			case "w":
				fmt.Printf("\tDemonstrate effects of writing all 1's to the shift register.\n")
				fmt.Printf("\tThe display will briefly display '8.' and then cleared.\n")
				time.Sleep(time.Second)
				testWriteOnes(sdiPin, rclkPin, srclkPin, srclrPin)
				break
			case "o":
				fmt.Printf("\tDemonstrate effects of toggling the Output Enable pin.\n")
				fmt.Printf("\tThe display will display '8', the OE pin will be toggled which will\n")
				fmt.Printf("\tcause the display to clear, then the OE pin will be toggled again\n")
				fmt.Printf("\twhich will cause '8' to once again be displayed, and then the display will\n")
				fmt.Printf("\tbe cleared.\n")
				time.Sleep(time.Second)
				testOEToggle(sdiPin, rclkPin, srclkPin, srclrPin, oePin)
				break
			case "q":
				fmt.Println("Goodbye!")
				os.Exit(0)
			default:
				fmt.Printf("\tInvalid choice, try again\n")
			}
		}

	}
}

func initShiftRegister() (sdiPin, rclkPin, srclkPin, srclrPin, oePin rpio.Pin) {
	sdiPin = rpio.Pin(17) // Associate GPIO pins with the go-rpio implemetation
	rclkPin = rpio.Pin(18)
	srclkPin = rpio.Pin(27)
	srclrPin = rpio.Pin(19)
	oePin = rpio.Pin(21)

	sdiPin.Output() // Pins are set to OUTPUT so they can be written to
	rclkPin.Output()
	srclkPin.Output()
	srclrPin.Output()
	oePin.Output()

	sdiPin.Low()
	rclkPin.Low()
	srclkPin.Low()
	srclrPin.High() // SRCLR must be set to HIGH before the sdiPin can be written to
	oePin.Low()     // OE must be set to low to enable the output register

	return sdiPin, rclkPin, srclkPin, srclrPin, oePin
}

// testWriteNums displays hexidecimal digits 0-F in turn followed by a decimal point. The
// test ends with the registers and display being cleared.
func testWriteNums(sdiPin, rclkPin, srclkPin, srclrPin rpio.Pin) {
	writeNums(sdiPin, rclkPin, srclkPin)
	shiftRegClr(rclkPin, srclrPin)
}

// testShiftRegClr first writes an '8' to the 7-segment display and then clears the
// display by clearing the shift register via the SRCLR pin
func testShiftRegClr(sdiPin, rclkPin, srclkPin, srclrPin rpio.Pin) {
	hc595_shift(segcode[8], sdiPin, rclkPin, srclkPin)
	time.Sleep(time.Millisecond * 500) // Sleep a while so the effect can be observed
	shiftRegClr(rclkPin, srclrPin)
}

// testZeroClr writes zeros into the shift register to demonstrate writing zeros to the
// shift register as an alternative to SRCLR. It first writes an '8' to the display so
// the effect is visible.
func testZeroClr(sdiPin, rclkPin, srclkPin rpio.Pin) {
	hc595_shift(segcode[8], sdiPin, rclkPin, srclkPin)
	time.Sleep(time.Millisecond * 500) // Sleep a while so the effect can be observed
	// populate the shift register 1 bit at a time with zeros
	hc595_shift(0, sdiPin, rclkPin, srclkPin)
}

// testWriteOnes displays '8.' before clearing the display
func testWriteOnes(sdiPin, rclkPin, srclkPin, srclrPin rpio.Pin) {
	// populate the shift register 1 bit at a time with ones
	hc595_shift(0xff, sdiPin, rclkPin, srclkPin)
	time.Sleep(time.Second)
	shiftRegClr(rclkPin, srclrPin)
}

// testOEToggle demonstrates the effect of toggling the OE pin on the shift register.
// First it writes an '8' to the display, toggles the OE pin to HIGH, pauses, then
// toggles the OE pin back to low to demonstrate that the contents of the output register
// were only blocked, not cleared.
func testOEToggle(sdiPin, rclkPin, srclkPin, srclrPin, oePin rpio.Pin) {
	hc595_shift(segcode[8], sdiPin, rclkPin, srclkPin)
	time.Sleep(time.Millisecond * 500)
	oePin.High()
	time.Sleep(time.Millisecond * 500)
	oePin.Low()
	time.Sleep(time.Millisecond * 500)
	shiftRegClr(rclkPin, srclrPin)
}

// hc595_shift takes 'dat' and writes it 1 bit at a time into the
// shift register. This is accomplished by first writing the bit to
// the serial data input pin (SDI) and the advancing the shift register
// clock (SRCLK) by toggling it from LOW to HIGH to LOW again. The
// contents of the shift register are finally made available to connected
// devices by toggling the output register clock (RCLK).
func hc595_shift(dat int, sdiPin, rclkPin, srclkPin rpio.Pin) {
	// Populate the input shift registers 1 bit at a time
	for i := 0; i < 8; i++ {
		// Populate shift register, bit 'i' (0 thru 7)
		sdiPin.Write(rpio.State(0x80 & (dat << i)))
		// Advance shift register clock
		srclkPin.Write(1)
		time.Sleep(time.Microsecond)
		srclkPin.Write(0)
	}
	// Advance storage/output register clock to transfer input shift register
	// contents to the output register
	rclkPin.Write(1)
	time.Sleep(time.Microsecond)
	rclkPin.Write(0)
}

// writeNums writes hexidecimal digits 0-F and a decimal point to a 7-segment display
// by way of the shift register.
func writeNums(sdiPin, rclkPin, srclkPin rpio.Pin) {
	for i := 0; i < 17; i++ {
		hc595_shift(segcode[i], sdiPin, rclkPin, srclkPin)
		time.Sleep(time.Millisecond * 500)
	}
}

// shiftRegClr clears the contents of the shift register with the side effect of clearing
// the attached 7-segment display
func shiftRegClr(rclkPin, srclrPin rpio.Pin) {
	srclrPin.Low()
	rclkPin.High()
	time.Sleep(time.Microsecond)
	rclkPin.Low()
	srclrPin.High() // Need to reset this back to high to reenable the shift register
}

// Handles 'ctl-C' entered at the terminal by exiting the program after directing the main
// goroutine (listening on the 'stop' channel) to exit.
func signalHandler(sigs chan os.Signal, stop chan interface{}, rclkPin, srclrPin rpio.Pin) {
	<-sigs
	// notify all listeners that the program is stopping
	close(stop)

	fmt.Printf("\n!!!INTERRUPTED!!! Clear display, then exit\n")
	shiftRegClr(rclkPin, srclrPin)
	// Release rpio library resources
	rpio.Close()

	os.Exit(0)

}
