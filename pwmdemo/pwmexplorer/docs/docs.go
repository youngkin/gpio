package docs

const GeneralHelp = `This application can be used to explore the various PWM settings and how they interact with each other. The main settings are:
1.  PWM Pin - this is the GPIO PWM hardware pin to use for the test.
2.  Non-PWM Pin - this is used to specify a software PWM pin to use. It is mutually exclusive with PWM Pin.
3.  Clock Frequency/Clock Divisor - this is used to set the PWM Clock frequency
4.  PWM Mode - this is used to specify whether the Balanced or Mark/Space algorithm will be used.
5.  Range - this is used to specify the range of the duty cycle.
6.  Pulse Width - this is used to specify the pulse width or duty of the duty cycle.
7.  PWM Type - this is used to specify whether hardware or software PWM will be used.
`

const PWMPinHelp = `PWM Pin lets you to choose a PWM hardware pin to use. The pins available in the dropdown are specific to the language chosen. C uses the WiringPi library which uses its own pin numbering scheme. Go uses the standard GPIO pin numbers.
`

const PWMPinHelpCodeGo = `[green]// Obtain GPIO resources
[orange]if [aqua]err [orange]:= [teal]rpio.[yellow]Open[wheat](); [aqua]err [orange]!= [blue]nil [wheat]{
    [teal]log.[yellow]Fatal[wheat]([aqua]err[wheat])[yellow]
    [teal]os.[yellow]Exit[wheat]([orange]1[wheat])
}
[green]// Release rpio resources obtained with 'rpio.Open()'
[red]defer [teal]rpio.[yellow]Close()

[green]// Initialize an integer const with a valid GPIO(BCM) pin number
// Obtain an rpio.Pin instance using the 'pin' (int) defined above
[red::b]const [aqua::-]pin[orange] int =  [aqua]18[yellow]
[aqua]gpin [orange]= [teal]rpio.[yellow]Pin[wheat]([aqua]pin[wheat])[yellow]
`

const PWMPinHelpCodeC = `[green]// Obtain GPIO resources
[orange]if [wheat]([yellow]wiringPiSetup[wheat]() == [red]-1[wheat]) {
    [yellow]printf[wheat]([green]"setup wiringPi failed!"[wheat]);[yellow]
    [red]return 1[wheat]);

[green::bu]// PWM_OUTPUT sets the mode to PWM
[yellow::bu]pinMode[wheat]([aqua]pin, PWM_OUTPUT[wheat]);[yellow::-]
}
`

const NonPWMPinHelp = `Non-PWM Pin allows you to specify a PWM pin to use, even a hardware pin. PWM Pin and Non-PWM Pin are mutually exclusive and the program will prevent you from specifying both. As with PWM Pin above, the numbering scheme is specific to the language, C or Go, chosen. The program offers no protection against using the wrong pin numbering scheme so be careful what you specify. If the pin chosen doesn’t behave as expected it may be that you used the wrong pin numbering scheme.

In Go, a non-PWM pin is instantiated in exactly the same way as a PWM pin.
`

const NonPWMPinHelpCodeGo = `[green]// Obtain GPIO resources
[orange]if [aqua]err [orange]:= [teal]rpio.[yellow]Open[wheat](); [aqua]err [orange]!= [blue]nil [wheat]{
    [teal]log.[yellow]Fatal[wheat]([aqua]err[wheat])[yellow]
    [teal]os.[yellow]Exit[wheat]([orange]1[wheat])
}
[green]// Release rpio resources obtained with 'rpio.Open()'
[red]defer [teal]rpio.[yellow]Close()

[green]// Initialize an integer const with a valid GPIO(BCM) pin number
// Obtain an rpio.Pin instance using the 'pin' (int) defined above
[red::b]const [aqua::-]pin[orange] int =  [aqua]18[yellow]
[aqua]gpin [orange]= [teal]rpio.[yellow]Pin[wheat]([aqua]pin[wheat])[yellow::-]
`

const NonPWMPinHelpCodeC = `[green]// Obtain GPIO resources
[aqua]int pin[wheat] = [red]0[wheat];
[orange]if [wheat]([yellow]wiringPiSetup[wheat]() == [red]-1[wheat]) {
    [yellow]printf[wheat]([green]"setup wiringPi failed!"[wheat]);[yellow]
    [red]return 1[wheat]);
}

[green::bu]// softPwmCreate() does a number of things. First it sets the 
// the non-PWM pin. Second it sets the initial pulsewidth to use.
// Finally, it sets the range. So it not only sets the non-PWM pin
// to use, it also sets the duty cycle. The initial pulsewidth in
// this example is set to 0, so the LED will be 'off'. The Range
// is set to 500. This is completely arbitrary.
[yellow::bu]softPwmCreate[wheat]([aqua]pin, 0, 500[wheat]);[yellow::-]
`

const ClockFreqHelp = `Clock Divisor is used to set the PWM clock frequency. The go-rpio library supports specifying the PWM clock frequency directly. The C WiringPi library uses the concept of divisor defined above to set the PWM clock frequency. You can calculate the divisor to use by dividing the Raspberry Pi 3B’s oscillator clock’s frequency of 19,200,000 Hertz by the desired PWM clock frequency. For example. to get a 100kHz PWM clock frequency divide 19,200,000 by 100,000. This calculation gives the Clock Divisor to use, 192 in this case. To avoid confusion, when C is the chosen language the label will be Clock Divisor. When Go is the chosen language this items label will be Clock Frequency.

The clock frequency must be an integer between 4688 and 9600000. In Go this can be set directly. In C it must be set by a divisor whose calculation must follow this form - 19200000/[4095 to 2], e.g., 19200000/4095 = 4688. In C, the acceptable divisor range is 2 to 4095.

In both C and Go, for software PWM, neither the ClockFrequency nor Clock Divisor are used. They are hardcoded directly into their respective PWM libraries.
`

const ClockFreqHelpCodeGo = `[green]// Obtain GPIO resources
[orange]if [aqua]err [orange]:= [teal]rpio.[yellow]Open[wheat](); [aqua]err [orange]!= [blue]nil [wheat]{
    [teal]log.[yellow]Fatal[wheat]([aqua]err[wheat])[yellow]
    [teal]os.[yellow]Exit[wheat]([orange]1[wheat])
}
[green]// Release rpio resources obtained with 'rpio.Open()'
[red]defer [teal]rpio.[yellow]Close()

[green]// Initialize an integer const with a valid GPIO(BCM) pin number
// Obtain an rpio.Pin instance using the 'pin' (int) defined above
[::bu]// Set the desired clock frequency. 'freq' is an int. Recall that
// in software PWM the clock frequency is hardcoded.
[red::b]const [aqua::-]pin[orange] int =  [aqua]18[yellow]
[red::b]const [aqua::-]freq[orange] int =  [aqua]4095[yellow]
[aqua]gpin [orange]= [teal]rpio.[yellow]Pin[wheat]([aqua]pin[wheat])[yellow]
[aqua::bu]gpin.[yellow]Freq[wheat]([aqua]freq[wheat])[yellow::-]
`

const ClockFreqHelpCodeC = `[green]// Obtain GPIO resources
[aqua]int pin[wheat] = [red]0[wheat];
[orange]if [wheat]([yellow]wiringPiSetup[wheat]() == [red]-1[wheat]) {
    [yellow]printf[wheat]([green]"setup wiringPi failed!"[wheat]);[yellow]
    [red]return 1[wheat]);
}

[green]// PWM_OUTPUT sets the mode to PWM
[green::bu]// pwmSetClock(divisor) sets the PWM clock
// frequency by specifying the denominator to be used to
// divide the clock source frequency. Recall that for software
// PWM the clock frequency is hardcoded.
[yellow::-]pinMode[wheat]([aqua]pin, PWM_OUTPUT[wheat]);[yellow::-]
[yellow::bu]pwmSetClock[wheat]([aqua]divisor[wheat]);[yellow::-]
`

const PWMModeHelp = `PWM Mode specifies whether Mark/Space or Balanced modes will be used. 

[red::bu]Note:[-:-:-] some combinations of language, pin type (PWM vs. non-PWM), and PWM Type (hardware/software) don’t support Balanced or Markspace mode. When this is the case a message will be displayed in the Messages area. 

The Go go-rpio library doesn’t support balanced mode except when PWM Type software is chosen.`

const PWMModeHelpCodeGo = `[green]// Obtain GPIO resources
[orange]if [aqua]err [orange]:= [teal]rpio.[yellow]Open[wheat](); [aqua]err [orange]!= [blue]nil [wheat]{
    [teal]log.[yellow]Fatal[wheat]([aqua]err[wheat])[yellow]
    [teal]os.[yellow]Exit[wheat]([orange]1[wheat])
}
[green]// Release rpio resources obtained with 'rpio.Open()'
[red]defer [teal]rpio.[yellow]Close()

[green]// Initialize an integer const with a valid GPIO(BCM) pin number
// Obtain an rpio.Pin instance using the 'pin' (int) defined above    
// Set the desired clock frequency. 'freq' is an int                 
[::bu]// Set the pin to PWM. Note that in Go there is no option to set
// PWM Mode directly. Calling 'rpio.Pwm()' implicitly means markspace
// mode is being used. See the github repo 'github.com/youngkin/gpio', the 
// file 'pwmdemo/pwmexplorer/apps/freqtest.go' for the implementation 
// details for software PWM in Go.
[red::b]const [aqua::-]pin[orange] int =  [aqua]18[yellow]             
[aqua]gpin [orange]= [teal]rpio.[yellow]Pin[wheat]([aqua]pin[wheat])[yellow]
[aqua]gpin.[yellow]Freq[wheat]([aqua]freq[wheat])[yellow::-]
[aqua::bu]gpin.[yellow]Mode[wheat]([aqua]rpio.Pwm[wheat])[yellow::-]
`

const PWMModeHelpCodeC = `[green]// Obtain GPIO resources
[aqua]int pin[wheat] = [red]0[wheat];
[orange]if [wheat]([yellow]wiringPiSetup[wheat]() == [red]-1[wheat]) {
    [yellow]printf[wheat]([green]"setup wiringPi failed!"[wheat]);[yellow]
    [red]return 1[wheat]);
}

[green]// 'PWM_OUTPUT' sets the mode to PWM
[green]// 'pwmSetClock(divisor)' sets the PWM clock
// frequency by specifying the denominator to be used to
// divide the clock source frequency. Recall that for software
// PWM the clock frequency is hardcoded.
[green::bu]// 'pwmSetMode()' is used to set the mode to 
// balanced, PWM_MODE_BAL, or markspace, PWM_MODE_MS.
[yellow::-]pinMode[wheat]([aqua]pin, PWM_OUTPUT[wheat]);[yellow::-]
[yellow]pwmSetClock[wheat]([aqua]divisor[wheat]);
[yellow::bu]pwmSetMode[wheat]([red]PWM_MODE_BAL[wheat]);
`

const RangeHelp = `Range can be thought of as a counter that counts PWM clock pulses. The ratio of range to PWM clock frequency can be thought of as the frequency of the signal sent to a PWM pin. Sometimes the term cycle length used as as an alias for range.  Along with Pulse Width, Range is used to calculate the Duty Cycle.
`

const RangeHelpCodeGo = `[green]// Obtain GPIO resources
[orange]if [aqua]err [orange]:= [teal]rpio.[yellow]Open[wheat](); [aqua]err [orange]!= [blue]nil [wheat]{
    [teal]log.[yellow]Fatal[wheat]([aqua]err[wheat])[yellow]
    [teal]os.[yellow]Exit[wheat]([orange]1[wheat])
}
[green]// Release rpio resources obtained with 'rpio.Open()'
[red]defer [teal]rpio.[yellow]Close()

[green]// Initialize an integer const with a valid GPIO(BCM) pin number
// Obtain an rpio.Pin instance using the 'pin' (int) defined above    
// Set the desired clock frequency. 'freq' is an int                 
// Set the pin to PWM
[::bu]// Set the desired Range of the Duty Cycle
[red::b]const [aqua::-]pin[orange] int =  [aqua]18[yellow]             
[aqua]gpin [orange]= [teal]rpio.[yellow]Pin[wheat]([aqua]pin[wheat])[yellow]
[aqua]gpin.[yellow]Mode[wheat]([aqua]rpio.Pwm[wheat])[yellow::-]
[aqua]gpin.[yellow]Freq[wheat]([aqua]freq[wheat])[yellow::-]
[aqua::bu]gpin.[yellow]DutyCycle[wheat][aqua](pulsewidth, [::r]rrange[wheat::bu])[yellow]
`

const RangeHelpCodeC = `[green]// Obtain GPIO resources
[aqua]int pin[wheat] = [red]0[wheat];
[orange]if [wheat]([yellow]wiringPiSetup[wheat]() == [red]-1[wheat]) {
    [yellow]printf[wheat]([green]"setup wiringPi failed!"[wheat]);[yellow]
    [red]return 1[wheat]);
}

[green]// 'PWM_OUTPUT' sets the mode to PWM
[green]// 'pwmSetClock(divisor)' sets the PWM clock
// frequency by specifying the denominator to be used to
// divide the clock source frequency. Recall that for software
// PWM the clock frequency is hardcoded.
[green]// 'pwmSetMode()' is used to set the mode to
// balanced, PWM_MODE_BAL, or markspace, PWM_MODE_MS.
[green::bu]// 'pwmSetRange()' sets the Range of the Duty Cycle
[yellow::bu]pinMode[wheat]([aqua]pin, PWM_OUTPUT[wheat]);[yellow::-]
[yellow]pwmSetClock[wheat]([aqua]divisor[wheat]);
[yellow]pwmSetMode[wheat]([red]PWM_MODE_BAL[wheat]);
[yellow::bu]pwmSetRange[wheat]([aqua]range[wheat]);
`

const PulseWidthHelpGo = `Pulse width is the duration of a pulse. In the various software libraries it's also called width, value, data, and duty length. Along with Range, Pulse Width is used to calculate the Duty Cycle.
`

const PulseWidthHelpC = `Pulse width is the duration of a pulse. In the various software libraries it's also called width, value, data, and duty length. It is used to set the Duty Cycle. For software PWM all of the hardware PWM calls in the 'Code' section below are replaced as follows:

[green]// Obtain GPIO resources
[aqua]int pin[wheat] = [red]0[wheat];
[orange]if [wheat]([yellow]wiringPiSetup[wheat]() == [red]-1[wheat]) {
    [yellow]printf[wheat]([green]"setup wiringPi failed!"[wheat]);[yellow]
    [red]return 1[wheat]);
}

[aqua]int range[wheat] = [red]0[wheat];
[aqua]int pulsewidth[wheat] = [red]0[wheat];
[yellow::bu]softPwmCreate[wheat]([aqua]pin, pulsewidth, range[wheat]);[yellow::-]
[yellow::bu]softPwmWrite[wheat]([aqua]pin, pulsewidth[wheat]);[yellow::-]
`

const PulseWidthHelpCodeGo = `[green]// Obtain GPIO resources
[orange]if [aqua]err [orange]:= [teal]rpio.[yellow]Open[wheat](); [aqua]err [orange]!= [blue]nil [wheat]{
    [teal]log.[yellow]Fatal[wheat]([aqua]err[wheat])[yellow]
    [teal]os.[yellow]Exit[wheat]([orange]1[wheat])
}
[green]// Release rpio resources obtained with 'rpio.Open()'
[red]defer [teal]rpio.[yellow]Close()

[green]// Initialize an integer const with a valid GPIO(BCM) pin number
// Obtain an rpio.Pin instance using the 'pin' (int) defined above
// Set the desired clock frequency. 'freq' is an int
// Set the pin to PWM
[::bu]// Set the desired Pulse Width of the Duty Cycle
[red::b]const [aqua::-]pin[orange] int =  [aqua]18[yellow]
[aqua]gpin [orange]= [teal]rpio.[yellow]Pin[wheat]([aqua]pin[wheat])[yellow]
[aqua]gpin.[yellow]Mode[wheat]([aqua]rpio.Pwm[wheat])[yellow::-]
[aqua]gpin.[yellow]Freq[wheat]([aqua]freq[wheat])[yellow::-]
[aqua::bu]gpin.[yellow]DutyCycle[wheat][aqua]([::r]pulsewidth[::bu], rrange[wheat])[yellow]
`

const PulseWidthHelpCodeC = `[green]// Obtain GPIO resources
[aqua]int pin[wheat] = [red]0[wheat];
[orange]if [wheat]([yellow]wiringPiSetup[wheat]() == [red]-1[wheat]) {
    [yellow]printf[wheat]([green]"setup wiringPi failed!"[wheat]);[yellow]
    [red]return 1[wheat]);
}

[green]// 'PWM_OUTPUT' sets the mode to PWM
[green]// 'pwmSetClock(divisor)' sets the PWM clock
// frequency by specifying the denominator to be used to
// divide the clock source frequency. Recall that for software
// PWM the clock frequency is hardcoded.
[green]// 'pwmSetMode()' is used to set the mode to
// balanced, PWM_MODE_BAL, or markspace, PWM_MODE_MS.
[green]// 'pwmSetRange()' sets the Range of the Duty Cycle
[green::bu]// 'pwmWrite()' sets the Pulse Width of the Duty Cycle
// and sends the corresponding signal to the specified pin.
[yellow::bu]pinMode[wheat]([aqua]pin, PWM_OUTPUT[wheat]);[yellow::-]
[yellow]pwmSetClock[wheat]([aqua]divisor[wheat]);
[yellow]pwmSetMode[wheat]([red]PWM_MODE_BAL[wheat]);
[yellow]pwmSetRange[wheat]([aqua]range[wheat]);
[yellow]pwmWrite[wheat]([aqua]pin, pulsewidth[wheat]);
`

const PWMTypeHelp = `PWM Type specifies whether hardware or software PWM is to be used.
`

const PWMTypeHelpCodeGo = `N/A - PWM Type is inferred when writing the code to use software or hardware PWM. It's use here is strictly limited to directing the application whether to call the software or hardware PWM code branch.
`
