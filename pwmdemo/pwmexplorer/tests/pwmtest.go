//
// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file.
//

package main

import (
	"fmt"
	"log"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/stianeikeland/go-rpio/v4"
)

const cycle = 2400000

//const cycle = 2400000

func main() {
	if err := rpio.Open(); err != nil {
		log.Fatal(err)
		os.Exit(1)
	}
	defer rpio.Close()

	ledPin := rpio.Pin(18)
	ledPin.Mode(rpio.Pwm)

	//ledPin.Freq(64000)
	ledPin.Freq(9600000)
	dutyCycle := uint32((cycle / 4))
	ledPin.DutyCycle(dutyCycle, uint32(cycle))

	// Initialize signal handling needed to catch ctl-C
	sigs := make(chan os.Signal, 1)
	signal.Notify(sigs, syscall.SIGINT, syscall.SIGKILL)
	go pwmInterruptHandler(sigs, ledPin)

	for {
		time.Sleep(time.Millisecond * 20)
	}

	ledPin.DutyCycle(0, uint32(cycle))
	os.Exit(0)
	//	}
}

func pwmInterruptHandler(sigs chan os.Signal, pin rpio.Pin) {
	<-sigs
	fmt.Println("\nExiting...")
	// Turn off the LED
	pin.DutyCycle(0, uint32(cycle))
	pin.Mode(rpio.Output)
	pin.Mode(rpio.Pwm)
	os.Exit(0)

}
