//
// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file.
//
// run with sudo /usr/local/go/bin/go run ./apps/freqtest.go -pin=18 -div=9600000 -cycle=2400000 -pulseWidth=4
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
// that is it blinks 4 times per second. If the cycle length is changed to 400,000, the
// LED Hz is 0.25, or once every 4 seconds. And finally, if the cycle length is 1,000
// the LED Hz is 100. At 100Hz, the LED will appear to be continuously on, i.e., not
// blinking.
//
// The range of 4688 to 9,600,000 for the clock frequency was chosen because values outside
// that range caused the LED to exhibit inconsistent behavior. The program will enforce
// this range.
//
// The range of 4 to 38,400,000 for the cycle length is somewhat arbitrary. When the clock
// frequency is set to 9,600,000 and the cycle length is set to 38,400,000, the LED will
// blink at 0.25Hz, or once every 4 seconds. 4 was chosen as the lower bound because
// I arbitrarily chose that the duty cycle would be 25% of the cycle. For an LED this equates
// to 25% brightness. This is bright enough to be easily seen, but not so bright as to be
// uncomfortable to look at.
//
// BCM pin 18 is used as it is a hardware PWM pin. Any hardware PWM pin can be used if desired.
// The other hardware PWM pins are BCM pins 12, 13, and 19.
//
// Run using 'sudo /usr/local/go/bin/go run freqtest.go'

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
	divisorStr := ""
	cycleStr := ""
	pwmPinStr := ""
	pulseWidthStr := ""
	flag.StringVar(&divisorStr, "div", "9600000", "PWM clock frequency divisor")
	flag.StringVar(&cycleStr, "cycle", "2400000", "PWM cycle/period length in microseconds")
	flag.StringVar(&pwmPinStr, "pin", "18", "GPIO PWM pin")
	flag.StringVar(&pulseWidthStr, "pulseWidth", "4", "PWM Pulse Width")
	flag.Parse()
	fmt.Printf("Input: PWM pin: %s, divisor: %s, cycle: %s, pulse width: %s\n", pwmPinStr, divisorStr, cycleStr, pulseWidthStr)

	divisor, cycle, pin, pulse := getParms(divisorStr, cycleStr, pwmPinStr, pulseWidthStr)
	fmt.Printf("Using: PWM pin: %d, divisor: %d, cycle: %d, pulse width: %d\n", pin, divisor, cycle, pulse)

	if err := rpio.Open(); err != nil {
		log.Fatal(err)
		os.Exit(1)
	}
	defer rpio.Close()

	ledPin := rpio.Pin(pin)
	ledPin.Mode(rpio.Pwm)
	ledPin.Freq(divisor)
	dutyCycle := uint32((cycle / pulse))
	ledPin.DutyCycle(dutyCycle, uint32(cycle))

	// Initialize signal handling needed to catch ctl-C
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT, syscall.SIGKILL)
	go interruptHandler(sigs, cycle, ledPin)

	for {
		time.Sleep(time.Millisecond * 20)
	}

	ledPin.DutyCycle(0, uint32(cycle))
	os.Exit(0)
}

func getParms(divisorStr, cycleStr, pwmPinStr, pulseWidthStr string) (div, cycle, pin, pulse int) {
	div, err := strconv.Atoi(divisorStr)
	if err != nil {
		fmt.Printf("Error: err getting divisor: %d from divisorStr: %s", err, div, divisorStr)
		div = 0
	}
	if div < 4688 {
		div = 4688
	}
	if div > 9600000 {
		div = 9600000
	}

	cycle, err = strconv.Atoi(cycleStr)
	if err != nil {
		fmt.Printf("Error: err getting cycle: %d from cycleStr: %s", err, cycle, cycleStr)
		cycle = 0
	}
	if cycle < 4 {
		cycle = 4
	}
	if cycle > 38400000 {
		cycle = 38400000
	}

	pin, err = strconv.Atoi(pwmPinStr)
	if err != nil {
		pin = 18
		fmt.Printf("Error: err getting pwmPin: %d from pwmStr: %s", err, pin, pwmPinStr)
	}

	pulse, err = strconv.Atoi(pulseWidthStr)
	if err != nil {
		pulse = 4
		fmt.Printf("Error: err getting pulse: %d from pulseWidthStr: %s", err, pulse, pulseWidthStr)
	}

	return div, cycle, pin, pulse
}

func interruptHandler(sigs chan os.Signal, cycle int, pin rpio.Pin) {
	<-sigs
	fmt.Println("\nExiting...")
	// Turn off the LED
	fmt.Printf("cycle: %d\n", cycle)
	pin.DutyCycle(0, uint32(cycle))
	pin.Mode(rpio.Output)
	pin.Mode(rpio.Pwm)
	os.Exit(0)

}
