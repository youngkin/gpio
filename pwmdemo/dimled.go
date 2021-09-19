// Copyright (c) 2020 Richard Youngkin. All rights reserved.
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file.
//
// This program uses a direct implementation of software PWM to vary
// an LED's brightness.
//
// Run using 'go run dimled.go'
//
package main

import (
	"bufio"
	"fmt"
	"os"
	"os/signal"
	"strconv"
	"strings"
	"syscall"
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

	// Set the pin (BCM pin 17) to OUTPUT mode to allow writes to the pin,
	// e.g., to set the pin to LOW or HIGH
	pin := rpio.Pin(17)
	pin.Output()
	pin.Low()

	// Initialize signal handling needed to catch ctl-C
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT)
	go interruptHandler(sigs, pin)

	reader := bufio.NewReader(os.Stdin)
	fmt.Println("Hit ctl-C to exit\nEnter a brightness brightVal between 10 and 10000 (e.g., 25):\n")
	// Bad practice to ignore errors!
	brightValStr, _ := reader.ReadString('\n')
	brightValStr = strings.TrimSuffix(brightValStr, "\n")
	brightVal, _ := strconv.Atoi(brightValStr)

	// The LED will be dim for about 5 seconds, and bright for 1 second.
	for {
		// Set LED to a dimmer brightVal. Notice how the LED will flicker due to
		// lack of uniformity in the OS scheduler and the 'time.Sleep()' function.
		// The amount of actual flickering will depend on the computing power of the
		// hardware and the CPU utilization.
		//
		// The ratio of time the pin is set to LOW vs. HIGH determines how bright the LED
		// will be. Lower ratios will result in a dimmer LED. Lower ratios (e.g., 10) will
		// also exhibit more flickering vs. higher values (e.g., 10000).
		for i := 0; i < 500; i++ {
			pin.Low()
			time.Sleep(time.Microsecond * time.Duration(brightVal))
			pin.High()
			// brightVal is the duty cycle. 10ms is the range. To have the total duration
			// equal the range this sleep needs to subtract the duty cycle from the range.
			time.Sleep(time.Millisecond*10 - time.Microsecond*time.Duration(brightVal))

		}
		// Set LED to full bright
		pin.Low()
		time.Sleep(time.Second)

	}
}

func interruptHandler(sigs chan os.Signal, pin rpio.Pin) {
	<-sigs
	fmt.Println("\nExiting...")
	// Turn off the LED
	pin.High()
	os.Exit(0)
}
