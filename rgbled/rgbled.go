// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by the GPL 3.0
// license that can be found in the LICENSE file.
//
// Hardware PWM pins include WiringPi pins 1, 26, 23, 24 (BCM 18, 12, 13, and 19 respectively).
// For practical purposes only 2 are usable as the other 2 are linked, turn one on they both turn on.
// Pins 24 & 26 (BCM 19 & 12) and Pins 1 & 23 (BCM 13 and 18) are independent. Pins 1 and 26 (BCM 18 & 12)
// are linked. Set one and the other will also be set. Ditto for pins 23 & 24 (BCM 13 & 19). In addition,
// writing to linked pins leads to inconsistent results. The independent pin will always
// be set, but the dependent pins won't always be set correctly. This could be a result of timing issues
// that affect the ordering of when signals are sent to pins on the same channel.
//

package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"

	"github.com/stianeikeland/go-rpio/v4"
)

var (
	ledPinRed   = rpio.Pin(19)
	ledPinGreen = rpio.Pin(18)
	ledPinBlue  = rpio.Pin(13)
	freq        = 100000
	cycle       = 1024
)

func ledColorSet(redVal, greenVal, blueVal uint32) {
	// This doesn't work as expected because GPIO pins 19 & 13 are essentially linked. This
	// is also true for GPIO pins 18 & 12, but since pin 12 isn't used here pin 18, green,
	// works as expected. So, when a blueVal or redVal are provided, the same value
	// is propagated to the linked pin. So for example, if redVal is 0 and blueVal
	// is 255, both the red and blue leds will light up (purple).
	//
	// Uncomment these lines if you want to see this behavior.
	ledPinRed.Mode(rpio.Pwm)
	ledPinGreen.Mode(rpio.Pwm)
	ledPinBlue.Mode(rpio.Pwm)

	ledPinRed.DutyCycle(redVal, uint32(cycle))
	ledPinGreen.DutyCycle(greenVal, uint32(cycle))
	ledPinBlue.DutyCycle(blueVal, uint32(cycle))

	// Given the explanation above, a workaround is to set only accept a redVal of
	// 255. When this is the case the blue and green pins are changed to output mode
	// and turned off, resulting in only a red LED. If redVal isn't 255, then it's
	// disabled (like blue and green above) and the blue/green pins are reenabled
	// and set to their respective values. This is obviously a hack since the red
	// pin can only be set to 255 and can't be used in conjuction with the blue and
	// green pins.
	//
	// Comment these lines if you don't want to see this behavior.
	//	if redVal == 255 {
	//		ledPinRed.Mode(rpio.Pwm)
	//		ledPinRed.DutyCycle(redVal, cycle)
	//
	//		//ledPinGreen.Mode(rpio.Output)
	//		ledPinGreen.DutyCycle(greenVal, cycl)
	//		ledPinBlue.Mode(rpio.Output)
	//		ledPinBlue.Low()
	//	} else {
	//		ledPinRed.Mode(rpio.Output)
	//		ledPinRed.Low()
	//
	//		ledPinGreen.Mode(rpio.Pwm)
	//		ledPinBlue.Mode(rpio.Pwm)
	//		ledPinGreen.DutyCycle(greenVal, cycle)
	//		ledPinBlue.DutyCycle(blueVal, cycle)
	//	}
}

func ledInit() {
	ledPinRed.Mode(rpio.Pwm)
	ledPinRed.Freq(freq)
	ledPinRed.DutyCycle(0, uint32(cycle))

	ledPinGreen.Mode(rpio.Pwm)
	ledPinGreen.Freq(freq)
	ledPinGreen.DutyCycle(1, uint32(cycle))

	ledPinBlue.Mode(rpio.Pwm)
	ledPinBlue.Freq(freq)
	ledPinBlue.DutyCycle(1023, uint32(cycle))
}

func main() {
	if err := rpio.Open(); err != nil {
		os.Exit(1)
	}
	defer rpio.Close()

	ledInit()

	reader := bufio.NewReader(os.Stdin)

	for {
		//
		// Get RGB values
		//
		fmt.Println("Enter red value (0 to 1023):")
		red, err := reader.ReadString('\n')
		if err != nil {
			fmt.Printf("Error reading from StdIn, %s", err)
			os.Exit(1)
		}
		red = strings.TrimSuffix(red, "\n")

		fmt.Println("Enter green value (0 to 1023):")
		green, err := reader.ReadString('\n')
		if err != nil {
			fmt.Printf("Error reading from StdIn, %s", err)
			os.Exit(1)
		}
		green = strings.TrimSuffix(green, "\n")

		fmt.Println("Enter blue value (0 to 1023):")
		blue, err := reader.ReadString('\n')
		if err != nil {
			fmt.Printf("Error reading from StdIn, %s", err)
			os.Exit(1)
		}
		blue = strings.TrimSuffix(blue, "\n")

		fmt.Printf("You entered red: %s, green: %s, blue: %s\n", red, green, blue)

		//
		// Convert string RGB values to int
		//
		blueNum, _ := strconv.Atoi(blue)
		redNum, _ := strconv.Atoi(red)
		greenNum, _ := strconv.Atoi(green)
		fmt.Printf("red: %x, green: %x, blue: %x\n", redNum, greenNum, blueNum)

		//
		// Set LED color
		//
		ledColorSet(uint32(redNum), uint32(greenNum), uint32(blueNum))

		//
		// Quit?
		//
		fmt.Printf("Enter 'q' to quit\n")
		quit, err := reader.ReadString('\n')
		if err != nil {
			fmt.Printf("Error reading from StdIn, %s, err")
			os.Exit(1)
		}
		if quit == "q\n" {
			ledColorSet(0, 0, 0)
			break
		}
	}
}
