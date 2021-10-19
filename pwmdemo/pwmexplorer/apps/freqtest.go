//
// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file.
//
// run with sudo /usr/local/go/bin/go run ./apps/freqtest.go -pin=18 -freq=9600000 -range=2400000 -pulsewidth=4
//
// This program visually demonstrates the the association between rpio.SetFreq() and
// rpio.SetDutyCycle(). rpio.SetFreq() sets the PWM clock frequency. rpio.SetDutyCycle()
// specifies how long a pin is in HIGH state (pulsewidth) vs. the length of the containing
// range.
//
// The Raspberry Pi 3B+ has a number of available clocks. The go-rpio library uses a clock called
// the Oscillator. This clock has a frequency of 19.2MHz. This freqency can be stepped down to a
// frequency which is more suitable for a given device like a motor which might require an input
// frequency of 100kHz. Using the go-rpio library it is possible to directly set the desired frequency
// of the PWM clock using 'SetFreq()'.
//
// Given a PWM clock frequency of 100,000 and a range length of 25,000, the LED frequency is 4Hz,
// that is it blinks 4 times per second. If the range length is changed to 400,000, the
// LED frequency is 0.25Hz, or once every 4 seconds. And finally, if the range length is 1,000
// the LED frequency is 100Hz. At 100Hz, the LED will appear to be continuously on, i.e., not
// blinking.  Using the go-rpio library range and pulse width are set indirectly by setting the
// duty cycle using 'SetDutyCycl()'.
//
// For reasons I can't find in the Broadcomm documentation, go-rpio suggests limiting the frequency
// requested in 'SetFreq()' to the range of 4688Hz and 9.6MHz. I have confirmed that setting
// frequencies outside this range can lead to unexpected behavior. As a result this program imposes
// this limit on the requested frequency by setting either the lower or upper bound to stay within
// this limit if the requested frequency is outside these limits.
//
// Run example:
// sudo /usr/local/go/bin/go run apps/freqtest.go -pwmType="hardware" -range="10000" -pulsewidth="25" -pin="18"
//

package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"os/signal"
	"strconv"
	"syscall"
	"time"

	"github.com/stianeikeland/go-rpio/v4"
)

func main() {
	pwmType := ""
	freqStr := ""
	rrangeStr := ""
	pwmPinStr := ""
	pulsewidthStr := ""
	flag.StringVar(&pwmType, "pwmType", "hardware", "defines whether software or hardware PWM should be used")
	flag.StringVar(&freqStr, "freq", "9600000", "desired PWM clock frequency")
	flag.StringVar(&rrangeStr, "range", "2400000", "PWM range")
	flag.StringVar(&pwmPinStr, "pin", "18", "BCM pin number")
	flag.StringVar(&pulsewidthStr, "pulsewidth", "4", "PWM Pulse Width")
	flag.Parse()

	freq, rrange, pin, pulsewidth := getParms(freqStr, rrangeStr, pwmPinStr, pulsewidthStr)
	fmt.Printf("Using: PWM pin: %s, PWM Type: %s, freq: %s, range: %s, pulse width: %s\n",
		pwmPinStr, pwmType, freqStr, rrangeStr, pulsewidthStr)

	if err := rpio.Open(); err != nil {
		log.Fatal(err)
		os.Exit(1)
	}
	if pwmType == "software" {
		runSoftwarePWM(pin, rrange, pulsewidth)
	}

	runHardwarePwm(pin, freq, uint32(rrange), uint32(pulsewidth))
}

func getParms(freqStr, rrangeStr, pinStr, pulsewidthStr string) (freq, rrange, pin, pulsewidth int) {
	freq, err := strconv.Atoi(freqStr)
	if err != nil {
		fmt.Printf("Error: error getting freq: %d from freqStr: %s\n", err, freq, freqStr)
		freq = 9600000
	}
	if freq < 4688 {
		freq = 4688
	}
	if freq > 9600000 {
		freq = 9600000
	}

	rrange, err = strconv.Atoi(rrangeStr)
	if err != nil {
		fmt.Printf("Error: error getting rrange: %d from rrangeStr: %s\n", err, rrange, rrangeStr)
		rrange = 2400000
	}

	pin, err = strconv.Atoi(pinStr)
	if err != nil {
		pin = 18
		fmt.Printf("Error: error getting pin: %d from pinStr: %s\n", err, pin, pinStr)
	}

	pulsewidth, err = strconv.Atoi(pulsewidthStr)
	if err != nil {
		pulsewidth = 4
		fmt.Printf("Error: error getting pulsewidth: %d from pulsewidthStr: %s\n", err, pulsewidth, pulsewidthStr)
	}

	return freq, rrange, pin, pulsewidth
}

func runHardwarePwm(gpin, freq int, rrange, pulsewidth uint32) {
	pin := rpio.Pin(gpin)
	pin.Mode(rpio.Pwm)
	pin.Freq(freq)
	pin.DutyCycle(pulsewidth, rrange)

	// Initialize signal handling needed to catch ctl-C
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT, syscall.SIGKILL)
	go hardwareInterruptHandler(sigs, rrange, pin)

	for {
		time.Sleep(time.Millisecond * 20)
	}
}

func runSoftwarePWM(gpin, rrange, pulsewidth int) {
	pin := rpio.Pin(gpin)
	pin.Output()
	on := rpio.Low
	off := rpio.High
	if gpin == 18 || gpin == 12 || gpin == 13 || gpin == 19 {
		on = rpio.High
		off = rpio.Low
	}

	// Initialize signal handling needed to catch ctl-C
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT, syscall.SIGKILL)
	go softwareInterruptHandler(sigs, rrange, pin, off)

	for {
		// Set LED to a specific brightness. Notice how the LED will flicker due to
		// lack of uniformity in the OS scheduler and the 'time.Sleep()' function.
		// The amount of actual flickering will depend on the computing power of the
		// hardware and the CPU utilization.
		//
		// The ratio of time the pin is set to 'on' vs. 'off' determines how bright the LED
		// will be. Lower pulsewidths will result in a dimmer LED. Lower pulsewidths (e.g., 10)
		// will also exhibit more flickering vs. higher values (e.g., 10000).
		//
		// The `time.Sleep` multipler is 100. Given the units are microseconds (time.Microsecond)
		// the value of 100 results in a period of 100 microseconds. This equates to a
		// frequency of 10kHz (1/0.0001). This frequency is fixed and can only be changed by
		// modifying this code.
		for {
			rpio.WritePin(pin, on)
			time.Sleep((time.Microsecond * 100) * time.Duration(pulsewidth))
			rpio.WritePin(pin, off)
			time.Sleep((time.Microsecond * 100) * time.Duration(rrange-pulsewidth))

		}
		// turn LED off
		rpio.WritePin(pin, off)
		time.Sleep(time.Second)

	}
}

func hardwareInterruptHandler(sigs chan os.Signal, rrange uint32, pin rpio.Pin) {
	<-sigs
	fmt.Println("\nExiting...")
	// Turn off the LED
	pin.DutyCycle(0, rrange)
	pin.Mode(rpio.Output)
	pin.Mode(rpio.Pwm)
	rpio.Close()
	os.Exit(0)

}

func softwareInterruptHandler(sigs chan os.Signal, rrange int, pin rpio.Pin, off rpio.State) {
	<-sigs
	fmt.Println("\nExiting...")
	// Turn off the LED
	rpio.WritePin(pin, off)
	rpio.Close()
	os.Exit(0)

}
