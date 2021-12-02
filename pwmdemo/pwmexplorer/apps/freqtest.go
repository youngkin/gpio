//
// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file.
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
// duty cycle using 'SetDutyCycle()'.
//
// Run example (taking default values for PWM parameters):
// sudo /usr/local/go/bin/go run apps/freqtest.go
// Run example (See 'flag.*Var()' calls below for details regarding the meaning of the flags):
// sudo /usr/local/go/bin/go run apps/freqtest.go -pwmType="hardware" -freq="5000" -range="100" -pulsewidth="2" -pin="18"
//

package main

import (
	"flag"
	"fmt"
	"log"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/stianeikeland/go-rpio/v4"
)

func main() {
	var (
		freq       int
		rrange     int
		pwmPin     int
		pulsewidth int
		pwmMode    bool
	)
	pwmType := ""

	flag.StringVar(&pwmType, "pwmType", "hardware", "defines whether software or hardware PWM should be used")
	flag.IntVar(&freq, "freq", 5000, "desired PWM clock frequency")
	flag.IntVar(&rrange, "range", 10, "PWM range")
	flag.IntVar(&pwmPin, "pin", 18, "BCM pin number")
	flag.IntVar(&pulsewidth, "pulsewidth", 2, "PWM Pulse Width")
	flag.BoolVar(&pwmMode, "pwmmode", rpio.Balanced, "PWM Mode, Balanced (true) or MarkSpace (false)")
	flag.Parse()

	fmt.Printf("Using: PWM pin: %d, PWM Type: %s, freq: %d, range: %d, pulse width: %d, pwm mode: %t\n",
		pwmPin, pwmType, freq, rrange, pulsewidth, pwmMode)

	// Obtain GPIO resources
	if err := rpio.Open(); err != nil {
		log.Fatal(err)
		os.Exit(1)
	}
	defer rpio.Close()
	if pwmType == "software" {
		runSoftwarePWM(pwmPin, rrange, pulsewidth)
		return
	}

	runHardwarePwm(pwmPin, freq, uint32(rrange), uint32(pulsewidth), pwmMode)
}

// runHardwarePWM starts PWM as implemented in the board hardware.
func runHardwarePwm(gpin, freq int, rrange, pulsewidth uint32, pwmMode bool) {
	// Define the pin to be used,
	// followed by setting its mode to PWM,
	// then set directly set the PWM Clock frequency (note lack of divisor),
	// finally set the range and pulse width (pin.DutyCycle())
	// and send the PWM signal to the pin
	pin := rpio.Pin(gpin)
	pin.Mode(rpio.Pwm)
	pin.Freq(freq)
	// To test rpio.SetDutyCycle() comment out the next line and uncomment the one below it.
	rpio.SetDutyCycleWithPwmMode(pin, pulsewidth, rrange, pwmMode)
	//rpio.SetDutyCycle(pin, pulsewidth, rrange)

	// Initialize signal handling needed to catch ctl-C
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT, syscall.SIGKILL)
	go hardwareInterruptHandler(sigs, rrange, pin)

	// Sleep until ctl-C is caught
	for {
		time.Sleep(time.Millisecond * 20)
	}
}

// runSoftwarePWM starts PWM implemented by this function vs. via the board hardware
func runSoftwarePWM(gpin, rrange, pulsewidth int) {
	// Define the pin to be used for software PWM
	pin := rpio.Pin(gpin)
	// Set the pin mode to output so it can accept write
	// requests.
	pin.Output()

	// Initialize signal handling needed to catch ctl-C
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT, syscall.SIGKILL)
	go softwareInterruptHandler(sigs, rrange, pin, rpio.Low)

	for {
		// Execute the software driven duty cycle defined by rrange and
		// pulsewidth.
		//
		// The ratio of time the pin is set to 'on' vs. 'off' determines how bright the LED
		// will be. Lower pulsewidths will result in a dimmer LED. Lower pulsewidths (e.g., 10)
		// may also, depending on the chosen rage, exhibit more flickering vs. higher
		// values (e.g., 10000).
		//
		// The `time.Sleep` multipler is 100. Given the units are microseconds (time.Microsecond)
		// the value of 100 results in a period of 100 microseconds. This equates to a
		// frequency of 10kHz (1/0.0001). This frequency is fixed and can only be changed by
		// modifying this code.
		//
		// Notice that the LED might flicker due to a lack of uniformity in the OS scheduler
		// and the 'time.Sleep()' function. The amount of actual flicker that may occur will
		// depend on the computing power of the hardware and the CPU utilization.
		//
		// Loop until a ctl-C signal is caught
		for {
			rpio.WritePin(pin, rpio.High)
			time.Sleep((time.Microsecond * 100) * time.Duration(pulsewidth))
			rpio.WritePin(pin, rpio.Low)
			time.Sleep((time.Microsecond * 100) * time.Duration(rrange-pulsewidth))

		}
	}
}

// hardwareInterruptHandler handles the SIGINT caught when ctl-C is entered. It will
// return the pin to it's state before the program started and release GPIO resources
// before exiting. It is registered as the SIGINT handler when hardware PWM is
// specified.
func hardwareInterruptHandler(sigs chan os.Signal, rrange uint32, pin rpio.Pin) {
	<-sigs
	fmt.Println("\nExiting...")
	// Turn off the LED
	pin.DutyCycle(0, rrange)
	//pin.Mode(rpio.Output)
	rpio.Close()
	os.Exit(0)

}

// softwareInterruptHandler handles the SIGINT caught when ctl-C is entered. It will
// return the pin to it's state before the program started and release GPIO resources
// before exiting. It is registered as the SIGINT handler when softare PWM is specified.
func softwareInterruptHandler(sigs chan os.Signal, rrange int, pin rpio.Pin, off rpio.State) {
	<-sigs
	fmt.Println("\nExiting...")
	// Turn off the LED
	rpio.WritePin(pin, off)
	rpio.Close()
	os.Exit(0)

}
