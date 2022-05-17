//
// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by the GPL 3.0
// license that can be found in the LICENSE file.
//
// Run using 'go run ledbargraph.go'
//
// This program demonstrates how to drive an LED Bar Graph LED display. See
// https://docs.sunfounder.com/projects/raphael-kit/en/latest/components/component_bar_graph.html
// for details.
//
// This program assumes the BCM board is wired up as specified
// in the associated SunFounder project -
// https://docs.sunfounder.com/projects/raphael-kit/en/latest/1.1.3_led_bar_graph_c.html

package main

import (
	"fmt"
	"math/rand"
	"os"
	"os/signal"
	"syscall"
	"time"

	"github.com/stianeikeland/go-rpio/v4"
)

// 'pins' references GPIO/BCM pins, except for  pins 2,
// 3, and 8 which map to the SDA1, SCL1, and SPCE0 BCM
// pins respectively. Here are the associated WiringPi
// pin numbers as a cross reference (from the C program).
// int pins[10] = {0,1,2,3,4,5,6,8,9,10};
var pins = []int{17, 18, 27, 22, 23, 24, 25, 2, 3, 8}
var gpins = []rpio.Pin{}

// initPins will briefly flash all of the pins on before
// turning them all off.
func initPins() {
	for _, pin := range pins {
		pin := pin
		gpin := rpio.Pin(pin)
		gpin.Output()
		gpins = append(gpins, gpin)
	}
	for i, _ := range gpins {
		gpin := gpins[i]
		gpin.Low()
	}
	time.Sleep(time.Millisecond * 300)
	for _, gpin := range gpins {
		gpin.High()
	}
	time.Sleep(time.Millisecond * 300)
}

// randBarGraph will randonly light up 'pins'
func randBarGraph(upper int, stop chan interface{}) {
	s1 := rand.NewSource(time.Now().UnixNano())
	for {
		select {
		case <-stop:
			break
		default:
			r1 := rand.New(s1)
			rnum := r1.Intn(upper)
			gpins[rnum].Low()
			time.Sleep(time.Millisecond * 30)
			gpins[rnum].High()
			time.Sleep(time.Millisecond * 30)
		}
	}
}

// ledAll lights up all the leds in sequence.
func ledAll(stop chan interface{}) {
	for i, _ := range gpins {
		select {
		case <-stop:
			break
		default:
			gpins[i].Low()
			time.Sleep(time.Millisecond * 300)
			gpins[i].High()
			time.Sleep(time.Millisecond * 300)
		}
	}
}
func main() {
	// Initialize the rpio library
	if err := rpio.Open(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}

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
	go signalHandler(sigs, stop)

	initPins()
	ledAll(stop)
	randBarGraph(10, stop)
}

func signalHandler(sigs chan os.Signal, stop chan interface{}) {
	<-sigs
	// notify all listeners that the program is stopping
	close(stop)

	fmt.Println("\nExiting...\n")
	for _, gpin := range gpins {
		// turn off LEDs
		gpin.High()
	}
	// Release rpio library resources
	rpio.Close()

	os.Exit(0)
}
