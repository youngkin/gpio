//
// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by a MIT-style
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
	"errors"
	"flag"
	"fmt"
	"log"
	"os"
	"strconv"
	"strings"
	"time"

	"github.com/stianeikeland/go-rpio/v4"
)

var (
	ledPinGreen rpio.Pin
	// With these numbers the effective frequency is 0.5 Hz, 2 blinks per second
	divisor = 9600000
	cycle   = 2000
	pwmPin  = 18
)

func ledInit() {
	//	ledPinGreen = rpio.Pin(pwmPin)
	//	ledPinGreen.Mode(rpio.Pwm)
	//	ledPinGreen.Freq(divisor)
	//	ledPinGreen.DutyCycle(uint32(cycle/4), uint32(cycle))
}

func main() {
	divisorStr := ""
	cycleStr := ""
	pwmPinStr := ""
	pulseWidthStr := ""
	flag.StringVar(&divisorStr, "div", "9600000", "PWM clock frequency divisor")
	flag.StringVar(&cycleStr, "cycle", "38400000", "PWM cycle/period length in microseconds")
	flag.StringVar(&pwmPinStr, "pin", "18", "GPIO PWM pin")
	flag.StringVar(&pulseWidthStr, "pulseWidth", "4", "PWM Pulse Width")

	flag.Parse()

	fmt.Printf("PWM pin: %s, divisor: %s, cycle: %s, pulse width: %s\n", pwmPinStr, divisorStr, cycleStr, pulseWidthStr)

	if err := rpio.Open(); err != nil {
		log.Fatal(err)
		os.Exit(1)
	}
	defer rpio.Close()

	ledInit()

	divisorStr = strings.TrimSuffix(divisorStr, "\n")
	if len(divisorStr) > 0 {
		divisor, _ = strconv.Atoi(divisorStr)
	}
	if divisor < 4688 {
		divisor = 4688
	}
	if divisor > 9600000 {
		divisor = 9600000
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

	pwmPinStr = strings.TrimSuffix(pwmPinStr, "\n")
	err := errors.New("")
	if len(pwmPinStr) > 0 {
		pwmPin, err = strconv.Atoi(pwmPinStr)
		if err != nil {
			pwmPin = 18
			fmt.Printf("Error: err getting pwmPin: %d from pwmStr: %s", err, pwmPin, pwmPinStr)
		}
	}

	ledPinGreen = rpio.Pin(pwmPin)

	dutyCycle := uint32((cycle / 4))
	fmt.Printf("\nUsing PWM pin: %d, Clock divisor: %d, cycle: %d, duty cycle: %d, LED Hz: %f\n",
		ledPinGreen, divisor, cycle, dutyCycle, float32(divisor)/float32(cycle))
	fmt.Printf("PWM pin: %s, divisor: %s, cycle: %s, pulse width: %s\n", pwmPinStr, divisorStr, cycleStr, pulseWidthStr)
	ledPinGreen.Freq(divisor)
	ledPinGreen.DutyCycle(dutyCycle, uint32(cycle))

	time.Sleep(time.Second * 10)

	ledPinGreen.DutyCycle(0, uint32(cycle))
	os.Exit(0)
	//	}
}
