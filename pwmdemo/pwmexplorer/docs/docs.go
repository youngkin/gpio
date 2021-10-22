package docs

const GeneralHelp = `
This application can be used to explore the various PWM settings and how they interact with each other. The main settings are:
1.  PWM Pin - this is the GPIO PWM hardware pin to use for the test.
2.  Non-PWM Pin - this is used to specify a software PWM pin to use. It is mutually exclusive with PWM Pin.
3.  Clock Frequency/Clock Divisor - this is used to set the PWM Clock frequency
4.  PWM Mode - this is used to specify whether the Balanced or Mark/Space algorithm will be used.
5.  Range - this is used to specify the range of the duty cycle.
6.  Pulse Width - this is used to specify the pulse width or duty of the duty cycle.
7.  PWM Type - this is used to specify whether hardware or software PWM will be used.
`

const PWMPinHelp = `
PWM Pin lets you to choose a PWM hardware pin to use. The pins available in the dropdown are specific to the language chosen. C uses the WiringPi library which uses its own pin numbering scheme. Go uses the standard GPIO pin numbers.
`

//const PWMPinHelpCodeGo = `
//[ColorRed]var[-] [ColorAqua]pin[-] [ColorRed] = [-][ColorLightSeaGreen]18[-]
//`

const PWMPinHelpCodeGo = `
[green]// 1. Initialize an integer const with a valid GPIO(BCM) pin number
// 2. Obtain an rpio.Pin instance using the 'pin' (int) defined above
[red::b]const [aqua::-]pin[orange] int =  [aqua]18[yellow]
[aqua]gpin [orange]= [teal]rpio.[yellow]Pin[wheat]([aqua]pin[wheat])[yellow]
`

const NonPWMPinHelp = `
Non-PWM Pin allows you to specify a PWM pin to use, even a hardware pin. PWM Pin and Non-PWM Pin are mutually exclusive and the program will prevent you from specifying both. As with PWM Pin above, the numbering scheme is specific to the language, C or Go, chosen. The program offers no protection against using the wrong pin numbering scheme so be careful what you specify. If the pin chosen doesn’t behave as expected it may be that you used the wrong pin numbering scheme.

A non-PWM pin is instantiated in exactly the same way as a PWM pin.
`

const NonPWMPinHelpCodeGo = `
[green]// 1. Initialize an integer const with a valid GPIO(BCM) pin number
// 2. Obtain an rpio.Pin instance using the 'pin' (int) defined above
[red::b]const [aqua::-]pin[orange] int =  [aqua]18[yellow]
[aqua]gpin [orange]= [teal]rpio.[yellow]Pin[wheat]([aqua]pin[wheat])[yellow]
`

const ClockFreqHelp = `
Clock Divisor is used to set the PWM clock frequency. The go-rpio library supports specifying the PWM clock frequency directly. The C WiringPi library uses the concept of divisor defined above to set the PWM clock frequency. You can calculate the divisor to use by dividing the Raspberry Pi 3B’s oscillator clock’s frequency of 19,200,000 Hertz by the desired PWM clock frequency. For example. to get a 100kHz PWM clock frequency divide 19,200,000 by 100,000. This calculation gives the Clock Divisor to use, 192 in this case. To avoid confusion, when C is the chosen language the label will be Clock Divisor. When Go is the chosen language this items label will be Clock Frequency.

The clock frequency must be an integer between 4688 and 9600000. In Go this can be set directly. In C it must be set by a divisor whose calculation must follow this form - 19200000/[4095 to 2], e.g., 19200000/4095 = 4688.
`

const ClockFreqHelpCodeGo = `
[green]// 1. Initialize an integer const with a valid GPIO(BCM) pin number
// 2. Obtain an rpio.Pin instance using the 'pin' (int) defined above
// 3. Set the desired clock frequency. 'freq' is an int
[red::b]const [aqua::-]pin[orange] int =  [aqua]18[yellow]
[aqua]gpin [orange]= [teal]rpio.[yellow]Pin[wheat]([aqua]pin[wheat])[yellow]
[aqua]gpin.[yellow]Freq[wheat]([aqua]freq[wheat])[yellow]
`
