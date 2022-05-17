//
// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by the GPL 3.0
// license that can be found in the LICENSE file.
//
// Run using 'go run blinkingled.go'
//
package main

import (
	"fmt"
	"os"
	"time"

	"github.com/stianeikeland/go-rpio/v4"
)

func main() {
	// Initialize the go-rpio library. By default it uses BCM pin numbering.
	if err := rpio.Open(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

	// Release resources held by the go-rpio library obtained above after
	// 'main()' exits.
	defer rpio.Close()

	// Select the GPIO pin to use, BCM pin 17
	pin := rpio.Pin(17)

	// Set the pin (BCM pin 17) to OUTPUT mode to allow writes to the pin,
	// e.g., set the pin to LOW or HIGH
	pin.Output()

	for i := 0; i < 5; i++ {
		// Setting the GPIO pin to LOW allows current to flow from the power source thru
		// the anode to cathode turning on the LED.
		pin.Low()
		//        pin.Write(rpio.Low)
		fmt.Printf("LED on, Pin value should be 0: %d\n", pin.Read())
		time.Sleep(time.Millisecond * 500)
		pin.High()
		//        pin.Write(rpio.High)
		fmt.Printf("\tLED off, Pin value should be 1: %d\n", pin.Read())
		time.Sleep(time.Millisecond * 500)
	}

	// Turn off the LED
	pin.High()
}
