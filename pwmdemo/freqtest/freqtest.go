//
// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by the GPL 3.0
// license that can be found in the LICENSE file.
//
// This program visually demonstrates the the association between rpio.SetFreq() and
// rpio.SetDutyCycle(). rpio.SetFreq() sets the PWM clock frequency. rpio.SetDutyCycle()
// specifies how long a pin is in HIGH state (dutyLen) vs. the length of the containing
// cycle (cycleLen). Duty cycle is the ratio between dutyLen and cycleLen. So a dutyLen
// of 5 with a cycleLen of 10 would have a duty cycle of 0.5, or 50%.
//
// The relationship demonstrated by this program is more specifically the relationship
// between the clock frequency and the cycle length (cycleLen in SetDutyCycle()). This
// ratio sets the frequency of the LED.
//
// Given a clock frequency of 100,000 and a cycle length of 25,000, the LED Hz is 4,
// and it blinks 4 times per second. If the cycle length is changed to 400,000, the
// LED Hz is 0.25, or once every 4 seconds. And finally, if the cycle length is 1,000
// the LED Hz is 100. At 100Hz, the LED will appear to be continuously on, i.e., not
// blinking.
//
// The frequency at the LED dictates where flickering will occur. When flickering occurs depends
// on the type of light source. For example, eliminating flicker in a neon light requires a
// frequency of about 120Hz. Other light sources, like LEDs, don't flicker until a much lower
// threshold, for some people around 50Hz. In my testing, using this program on a Raspberry Pi
// 3B+, I can detect flickering below 45Hz using my peripheral vision. I don't detect
// flickering in an LED until around 35Hz when looking directly at the LED. Using the default
// clock frequency of 9,600,000, 35Hz occurs with a cycle value of 275,000 and 45Hz with a cycle
// value of 210,000.
//
// The range of 4688 to 9,600,000 for the clock frequency was chosen because values outside
// that range caused the LED to exhibit inconsistent behavior. The program will enforce
// this range.
//
// The range of 4 to 38,400,000 for the cycle length is somewhat arbitrary. When the clock
// frequency is set to 9,600,000 and the cycle length is set to 38,400,000, the LED will
// blink at 0.25Hz, or once every 4 seconds. 4 was chosen as the lower bound because
// I arbitrarily decided that the duty cycle would be calculated as the cycle/4. When
// a cycle of 4 is chosen the duty cycle will be 1 (it needs to be an integer > 0).
// With a duty cycle of cycle/4, the power going to the LED will be 25% of full power,
// resulting the brightness to be set to 25% of full brightness. This is bright enough to
// be easily seen, but not so bright as to be uncomfortable to look at.
//
// BCM pin 18 is used as it is a hardware PWM pin. Any hardware PWM pin can be used if desired.
// The other hardware PWM pins are BCM pins 12, 13, and 19.
//
// Run using 'sudo /usr/local/go/bin/go run freqtest.go'

package main

import (
	"bufio"
	"fmt"
	"os"
	"os/signal"
	"strconv"
	"strings"
	"syscall"

	"github.com/stianeikeland/go-rpio/v4"
)

var (
	ledPinGreen = rpio.Pin(13)
	// With these numbers the effective frequency at the LED is 4800 Hz (9,600,000/2000).
	freq  = 9600000
	cycle = 2000
)

func ledInit() {
	ledPinGreen.Mode(rpio.Pwm)
	ledPinGreen.Freq(freq)
	ledPinGreen.DutyCycle(uint32(cycle/4), uint32(cycle))
}

func main() {
	if err := rpio.Open(); err != nil {
		os.Exit(1)
	}
	defer rpio.Close()

	ledInit()

	dutyCycle := uint32((cycle / 4))
	fmt.Printf("\nUsing clock frequency: %d, cycle: %d, duty cycle: %d, LED Hz: %f\n",
		freq, cycle, dutyCycle, float32(freq)/float32(cycle))

	// Initialize signal handling needed to catch ctl-C
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT)
	go interruptHandler(sigs, ledPinGreen)

	reader := bufio.NewReader(os.Stdin)

	fmt.Println("Hit ctl-C to exit")

	for {
		fmt.Printf("Enter clock frequency value (4688 to 9,600,000 (hit <enter> to leave unchanged): ")
		freqStr, err := reader.ReadString('\n')
		if err != nil {
			fmt.Printf("Error reading from StdIn, %s", err)
			os.Exit(1)
		}
		freqStr = strings.TrimSuffix(freqStr, "\n")
		if len(freqStr) > 0 {
			freq, _ = strconv.Atoi(freqStr)
		}
		if freq < 4688 {
			freq = 4688
		}
		if freq > 9600000 {
			freq = 9600000
		}

		fmt.Printf("Enter cycle value (4 to 38,400,000 (hit <enter> to leave unchanged): ")
		cycleStr, err := reader.ReadString('\n')
		if err != nil {
			fmt.Printf("Error reading from StdIn, %s", err)
			os.Exit(1)
		}
		cycleStr = strings.TrimSuffix(cycleStr, "\n")
		if len(cycleStr) > 0 {
			cycle, _ = strconv.Atoi(cycleStr)
		}
		if cycle < 4 {
			cycle = 4
		}
		if cycle > 38400000 {
			cycle = 38400000
		}

		// setting duty cycle to cycle/4 results in an LED of reasonable brightness,
		// not too dim, not too bright, just right
		dutyCycle = uint32((cycle / 4))
		fmt.Printf("\nUsing Clock frequency: %d, cycle: %d, duty cycle: %d, LED Hz: %f\n",
			freq, cycle, dutyCycle, float32(freq)/float32(cycle))
		ledPinGreen.Freq(freq)
		ledPinGreen.DutyCycle(dutyCycle, uint32(cycle))
	}
}

func interruptHandler(sigs chan os.Signal, pin rpio.Pin) {
	<-sigs
	fmt.Println("\nExiting...")
	// Turn off the LED
	pin.DutyCycle(0, uint32(cycle))
	os.Exit(0)
}
