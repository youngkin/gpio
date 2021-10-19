//
// Copyright (c) 2021 Richard Youngkin. All rights reserved.
// Use of this source code is governed by a MIT-style
// license that can be found in the LICENSE file.
//

// run with sudo /usr/local/go/bin/go run ./apps/freqtest.go -pin=18 -freq=9600000 -range=2400000 -pulsewidth=4
package main

import (
	"bytes"
	"fmt"
	"os/exec"
	"strconv"
	"syscall"

	"github.com/gdamore/tcell/v2"
	"github.com/rivo/tview"
)

// PWMApp contains the state of the application
type PWMApp struct {
	langs    *tview.DropDown
	pwmParms *tview.Form
	topics   *tview.Form
	msg      *tview.TextView
	helpView *tview.TextView
	codeView *tview.TextView
}

// These constants are the strings used in the various PWM configuration
// entries.
const (
	cLang       = "C"
	goLang      = "Go"
	pwmModeBal  = "balanced"
	pwmModeMS   = "markspace"
	hardware    = "hardware"
	software    = "software"
	pwmModeHelp = "PWM Mode"
)

// These constants are the labels for the Help Topics
const (
	HelpGeneral = iota
	HelpPWMPin
	HelpNonPWMPin
	HelpClockDivisor
	HelpPWMMode
	HelpRange
	HelpPulseWidth
)

// These constants are the labels for the language settings
const (
	C = iota
	Go
)

// These constants are the positions of the Go and C entries in
// the Language dropdown
const (
	cIdx = iota
	goIdx
)

// These constants are the positions of the mode entries in the
// PWM Mode dropdown
const (
	markspaceIdx = iota
	balancedIdx
)

// These constants are the positions of the type entries in the
// PWM Type dropdown
const (
	hardwareIdx = iota
	softwareIdx
)

// These constants are the positions of the various entries in the
// PWM Parameters form
const (
	pwmPinIdx = iota
	nonPwmPinIdx
	clockDivisorIdx
	pwmModeIdx
	rangeIdx
	pulseWidthIdx
	pwmTypeIdx
)

// This is the position of the unselected entry in the PWM Pin dropdown
const pwmPinUnselected = 4

// SetHelpTopic gets called when a help topic is chosen or changes
func (p *PWMApp) SetHelpTopic(option string, optionIndex int) {
	if p.pwmParms == nil {
		return // app is still initializing
	}

	p.helpView.Clear()
	p.codeView.Clear()

	_, lang := p.langs.GetCurrentOption()
	_, modeOpt := p.pwmParms.GetFormItem(3).(*tview.DropDown).GetCurrentOption()

	switch optionIndex {
	case 0:
		p.helpView.Write([]byte("General information blah blah blah."))
		p.codeView.Write([]byte(""))
	case 1:
		p.helpView.Write([]byte("The PWM Pin is blah blah blah."))
		p.codeView.Write([]byte("...\npin = rpio.Pin(18)\npin.Mode(rpio.PWM)\n\n..."))
	case 2:
		p.helpView.Write([]byte("The Non-PWM Pin is blah blah blah."))
		p.codeView.Write([]byte("...\npin = rpio.Pin(18)\npin.Mode(rpio.OUTPUT)\n\n..."))
	case 3:
		p.helpView.Write([]byte("The Clock Divisor is blah blah blah."))
		p.codeView.Write([]byte("...\npin.Freq(clockDivisor))\n\n..."))
	case 4:
		if lang == goLang && pwmModeMS == modeOpt {
			p.helpView.Write([]byte("[red]PWM Mode 'Mark/Space' isn't available in Go.[white]\n"))
		}
		p.helpView.Write([]byte("PWM Mode is blah blah blah. "))
		p.codeView.Write([]byte("...\npin = pin.Mode(rpio.MarkSpace))\n...\n"))
	case 5:
		p.helpView.Write([]byte("Range is blah blah blah."))
		p.codeView.Write([]byte("...\npin.DutyCycle(pulsewidth, rrange))\n\n..."))
	case 6:
		p.helpView.Write([]byte("Pulse width is blah blah blah blah blah blah blah blah blah blah blah blah blah blah blah blah blah blah blah balh balh balh balh ablah blah blah."))
		p.codeView.Write([]byte("...\npin.DutyCycle(pulsewidth, rrange))\n\n..."))
	}

	p.msg.Clear()
}

// SetPWMMode gets called when the PWM Mode is set/changed (balanced/markspace)
func (p *PWMApp) SetPWMMode(option string, optionIdx int) {
	//TODO: Need to handle case where a non PWM pin is currently set. Will need to unset it
	//TODO: and post a message. This should be done first so there's no interaction between
	//TODO: the other checks below.
	//TODO:
	//TODO: p.msg.SetText() will have to be constructed from a help message as is done in
	//TODO: SetNonPWMPin().
	if p.topics == nil || p.pwmParms == nil || optionIdx == -1 {
		return // app is still initializing
	}

	_, lang := p.langs.GetCurrentOption()
	helpIdx, helpItem := p.topics.GetFormItem(HelpGeneral).(*tview.DropDown).GetCurrentOption()
	pwmModeDropdown := p.pwmParms.GetFormItem(pwmModeIdx).(*tview.DropDown)
	if lang == goLang && option == pwmModeBal {
		p.SetHelpTopic(helpItem, helpIdx)
		pwmModeDropdown.SetCurrentOption(markspaceIdx)
		p.msg.SetText("Warning: PWM mode 'Balanced' is not available in 'Go'.")
		return
	}

	pwmTypeDropdown := p.pwmParms.GetFormItem(pwmTypeIdx).(*tview.DropDown)
	_, pwmType := pwmTypeDropdown.GetCurrentOption()
	if pwmType == software && lang == cLang {
		if option == pwmModeBal {
			pwmModeDropdown.SetCurrentOption(markspaceIdx)
			p.msg.SetText("Warning: only PWM mode 'markspace' is available for PWM type 'software' using 'C'.")
			return
		}
	}
	// Reset message if everything worked
	p.msg.SetText("Messages: Use mouse to navigate screen.")
}

// SetPWMType gets called when the PWM type is set/changed (hardware/software)
func (p *PWMApp) SetPWMType(option string, optionIdx int) {
	if p.topics == nil || p.pwmParms == nil || optionIdx == -1 {
		return // app is still initializing
	}

	_, lang := p.langs.GetCurrentOption()
	//TODO: Use the current language to set the language specific help topic, e.g.,
	//TODO: code for PWM Type
	helpIdx, helpItem := p.topics.GetFormItem(HelpGeneral).(*tview.DropDown).GetCurrentOption()
	pwmModeDropdown := p.pwmParms.GetFormItem(pwmModeIdx).(*tview.DropDown)
	if lang == cLang && option == software {
		//TODO: See above TODO item
		p.SetHelpTopic(helpItem, helpIdx)
		_, currentMode := pwmModeDropdown.GetCurrentOption()
		if currentMode == pwmModeBal {
			pwmModeDropdown.SetCurrentOption(markspaceIdx)
			p.msg.SetText("Warning: PWM mode 'Balanced' is not available in 'C' when PWM type 'software' is specified.")
			return
		}
	}

	nonPWMPinInput := p.pwmParms.GetFormItem(nonPwmPinIdx).(*tview.InputField)
	if nonPWMPinInput.GetText() != "" && optionIdx == hardwareIdx {
		pwmModeDropdown.SetCurrentOption(markspaceIdx)
		p.msg.SetText("Warning: PWM mode 'balanced' is not available when using a non-PWM Pin")
		return
	}

	// Reset message if everything worked
	p.msg.SetText("Messages: Use mouse to navigate screen.")
}

// SetPWMPin gets called when the PWM pin is selected
func (p *PWMApp) SetPWMPin(option string, pinIdx int) {
	//TODO: Need to handle case where a non PWM pin is currently set. Will need to unset it
	//TODO: and post a message. This should be done first so there's no interaction between
	//TODO: the other checks below.
	//TODO:
	//TODO: p.msg.SetText() will have to be constructed from a help message as is done in
	//TODO: SetNonPWMPin().
	if p.topics == nil || p.pwmParms == nil {
		return // app is still initializing
	}
	nonPWMPinInput := p.pwmParms.GetFormItem(nonPwmPinIdx).(*tview.InputField)
	if nonPWMPinInput.GetText() != "" && option != "" {
		nonPWMPinInput.SetText("")
		p.msg.SetText("Warning: PWM pin specified. The Non PWM Pin will be ignored!")
	}
}

// SetNonPWMPin gets called when a non PWM pin is selected
func (p *PWMApp) SetNonPWMPin(option string) {
	if p.topics == nil || p.pwmParms == nil {
		return // app is still initializing
	}

	helpMsg := "Non PWM pin chosen, Clock Frequency/Frequency setting will be ignored."
	pwmPinDropdown := p.pwmParms.GetFormItem(pwmPinIdx).(*tview.DropDown)
	_, pwmPinOpt := pwmPinDropdown.GetCurrentOption()
	if pwmPinOpt != "" && option != "" {
		pwmPinDropdown.SetCurrentOption(pwmPinUnselected)
		helpMsg += "\n* Warning: Non-PWM pin will be used. The PWM Pin will be ignored!"
	}

	pwmModeDropdown := p.pwmParms.GetFormItem(pwmModeIdx).(*tview.DropDown)
	pwmModeDropdown.SetCurrentOption(markspaceIdx)
	helpMsg += "\n* Warning: Only PWM Mode 'markspace' is available for non-PWM pins"

	pwmTypeDropdown := p.pwmParms.GetFormItem(pwmTypeIdx).(*tview.DropDown)
	pwmTypeDropdown.SetCurrentOption(softwareIdx)
	helpMsg += "\n* Warning: Only PWM type 'software' is available for non-PWM pins"

	if helpMsg != "" {
		p.msg.SetText(helpMsg)

	}
}

// SetLanguage is called when a language option is selected or changed
func (p *PWMApp) SetLanguage(lang string, langIdx int) {
	if p.helpView == nil || p.pwmParms == nil {
		return // app is still initializing
	}

	pwmPinDropdown := p.pwmParms.GetFormItem(pwmPinIdx).(*tview.DropDown)
	if lang == cLang {
		pwmPinDropdown.SetOptions([]string{"1", "23", "24", "26", ""}, p.SetPWMPin)
	} else {
		pwmPinDropdown.SetOptions([]string{"12", "18", "13", "19", ""}, p.SetPWMPin)
	}

	pwmModeDropdown := p.pwmParms.GetFormItem(pwmModeIdx).(*tview.DropDown)
	_, pwmMode := pwmModeDropdown.GetCurrentOption()
	if pwmMode == pwmModeBal && lang == goLang {
		pwmModeDropdown.SetCurrentOption(markspaceIdx)
		p.msg.SetText("Warning: PWM mode 'balanced' is not available in 'Go'.")
		return
	}

	pwmTypeDropdown := p.pwmParms.GetFormItem(pwmTypeIdx).(*tview.DropDown)
	_, pwmType := pwmTypeDropdown.GetCurrentOption()
	if pwmType == software && lang == cLang {
		pwmModeDropdown.SetCurrentOption(markspaceIdx)
		p.msg.SetText("Warning: only PWM mode 'markspace' is available for PWM type 'software' using 'C'.")
		return
	}

	clockInput := p.pwmParms.GetFormItem(clockDivisorIdx).(*tview.InputField)
	if lang == goLang {
		clockInput.SetLabel("Clock Frequency")
	} else {
		clockInput.SetLabel("Clock Divisor")
	}

	//	helpTopic := p.topics.GetFormItem(0).(*tview.DropDown)
	//	helpIdx, hItem := helpTopic.GetCurrentOption()
	//	if hItem == pwmModeHelp {
	//		p.SetHelpTopic(hItem, helpIdx)
	//	}

	// Reset message if everything worked
	p.msg.SetText("Messages: Use mouse to navigate screen.")
}

func main() {
	msg := tview.NewTextView().SetTextColor(tcell.ColorRed)
	langDropDown := tview.NewDropDown().SetLabel("Language:").AddOption(cLang, nil).AddOption(goLang, nil)
	langDropDown.SetCurrentOption(1)
	language := tview.NewForm().AddFormItem(langDropDown)

	pwmApp := PWMApp{msg: msg, langs: langDropDown}
	langDropDown.SetSelectedFunc(pwmApp.SetLanguage)

	ui := tview.NewApplication()

	helpView := getHelpView(ui)

	codeView := getHelpView(ui)

	helpView.SetTitle("Help").SetBorder(true).SetTitleColor(tcell.ColorYellow)
	codeView.SetTitle("Code").SetBorder(true)
	// Go is the default language, set pins accordingly
	msg.SetText("Messages: Use mouse to navigate screen.")

	pwmApp.helpView = helpView
	pwmApp.codeView = codeView

	// Text/Code Grid
	tcodeGrid := getTextCodeGrid(helpView, codeView)

	topics := getTopicsForm(&pwmApp)
	helpView.Write([]byte(generalHelp))
	pwmApp.topics = topics

	parms := getParmsForm(&pwmApp)
	pwmApp.pwmParms = parms

	//	var stopTest chan interface{}
	//	running := false

	buttons := getButtonForm(ui, &pwmApp, msg)

	// Main Grid
	grid := tview.NewGrid().
		SetRows(2, 0, 6, 3).
		SetColumns(-2, -3, -3, -9).
		// SetColumns(-2, -3, -12).
		SetBorders(true).
		AddItem(tview.NewTextView().SetTextAlign(tview.AlignCenter).SetText("PWM Explorer").SetTextColor(tcell.ColorGreen), 0, 0, 1, 4, 0, 0, false).
		AddItem(msg, 2, 0, 1, 4, 0, 0, false).
		AddItem(buttons, 3, 0, 1, 4, 0, 0, false)

	// Layout for screens narrower than 100 cells (language and help are hidden).
	grid.AddItem(language, 0, 0, 0, 0, 0, 0, false).
		AddItem(parms, 0, 1, 1, 3, 0, 0, false).
		AddItem(topics, 0, 2, 0, 0, 0, 0, false).
		AddItem(tcodeGrid, 0, 3, 0, 0, 0, 0, false)
		// AddItem(topicGrid, 0, 2, 0, 0, 0, 0, false)
		// AddItem(hGrid, 0, 2, 0, 0, 0, 0, false)

	// Layout for screens wider than 100 cells.
	grid.AddItem(language, 1, 0, 1, 1, 0, 100, false).
		AddItem(parms, 1, 1, 1, 1, 0, 100, false).
		AddItem(topics, 1, 2, 1, 1, 0, 100, false).
		AddItem(tcodeGrid, 1, 3, 1, 1, 0, 100, false)

	if err := ui.SetRoot(grid, true).EnableMouse(true).Run(); err != nil {
		panic(err)
	}
}

func newPrimitive(text string) tview.Primitive {
	return tview.NewTextView().
		SetTextAlign(tview.AlignCenter).
		SetText(text)
}

func buildGoCommand(pin, freq, rrange, pulsewidth, pwmType string) []string {
	return []string{"/usr/local/go/bin/go", "run", "./apps/freqtest.go",
		fmt.Sprintf("-pin=%s", pin), fmt.Sprintf("-freq=%s", freq),
		fmt.Sprintf("-range=%s", rrange), fmt.Sprintf("-pulsewidth=%s", pulsewidth),
		fmt.Sprintf("-pwmType=%s", pwmType),
	}
}

func buildCCommand(pin, divisor, rrange, pulsewidth, pwmType, pwmMode string) []string {
	return []string{"./apps/freqtest",
		fmt.Sprintf("--pin=%s", pin), fmt.Sprintf("--divisor=%s", divisor),
		fmt.Sprintf("--range=%s", rrange), fmt.Sprintf("--pulsewidth=%s", pulsewidth),
		fmt.Sprintf("--type=%s", pwmType), fmt.Sprintf("--mode=%s", pwmMode),
	}
}

func getHelpView(ui *tview.Application) *tview.TextView {
	return tview.NewTextView().
		SetDynamicColors(true).
		SetRegions(true).
		SetWordWrap(true).
		SetChangedFunc(func() {
			ui.Draw()
		})
}

func getTextCodeGrid(helpView, codeView *tview.TextView) *tview.Grid {
	return tview.NewGrid().
		SetRows(-1, -1).
		SetColumns(0).
		AddItem(helpView, 0, 0, 1, 1, 0, 0, false).
		AddItem(codeView, 1, 0, 1, 1, 0, 0, false)
}

func getTopicsForm(pwmApp *PWMApp) *tview.Form {
	return tview.NewForm().
		AddDropDown("Help Topics:",
			[]string{"General", "PWM Pin", "Non-PWM Pin", "Clock Divisor", "PWM Mode", "Range", "PulseWidth"},
			0, pwmApp.SetHelpTopic)
}

func getParmsForm(pwmApp *PWMApp) *tview.Form {
	return tview.NewForm().
		// Go is the default language, set the pins accordingly
		AddDropDown("PWM Pin:", []string{"12", "18", "13", "19", ""}, -1, pwmApp.SetPWMPin).
		AddInputField("Non-PWM Pin:", "", 2, nil, pwmApp.SetNonPWMPin).
		AddInputField("Clock Frequency:", "", 11, nil, nil).
		AddDropDown("PWM Mode:", []string{pwmModeMS, pwmModeBal}, -1, pwmApp.SetPWMMode).
		AddInputField("Range:", "", 12, nil, nil).
		AddInputField("Pulse Width:", "", 12, nil, nil).
		AddDropDown("PWM Type:", []string{hardware, software}, 0, pwmApp.SetPWMType)
}

func getButtonForm(ui *tview.Application, pwmApp *PWMApp, msg *tview.TextView) *tview.Form {
	var stopTest chan interface{}
	running := false
	return tview.NewForm().
		AddButton("Start", func() {
			go func() {
				if running == true {
					msg.SetText("There is a running test. Stop that test and try again")
					return
				}

				stopTest = make(chan interface{})

				_, lang := pwmApp.langs.GetCurrentOption()
				_, pwmPin := pwmApp.pwmParms.GetFormItem(0).(*tview.DropDown).GetCurrentOption()
				nonPwmPin := pwmApp.pwmParms.GetFormItem(1).(*tview.InputField).GetText()
				divisor := pwmApp.pwmParms.GetFormItem(2).(*tview.InputField).GetText()
				_, pwmMode := pwmApp.pwmParms.GetFormItem(3).(*tview.DropDown).GetCurrentOption()
				rrange := pwmApp.pwmParms.GetFormItem(4).(*tview.InputField).GetText()
				pulsewidth := pwmApp.pwmParms.GetFormItem(5).(*tview.InputField).GetText()
				_, pwmType := pwmApp.pwmParms.GetFormItem(6).(*tview.DropDown).GetCurrentOption()

				var pwmClockFreq float32 = 0.0
				divisorInt, _ := strconv.Atoi(divisor)
				if lang == cLang {
					pwmClockFreq = 19200000 / float32(divisorInt) //19200000 is the oscillator clock frequency used as clock source
				} else {
					pwmClockFreq = float32(divisorInt)
				}
				rrangeInt, _ := strconv.Atoi(rrange)
				freqMsg := fmt.Sprintf("PWM Clock Frequency(Hz): %9.2f, GPIO Pin Frequency(Hz): %9.2f",
					pwmClockFreq, pwmClockFreq/float32(rrangeInt))

				// if nonPwmPin is populated use it
				pin := ""
				pinWarningText := ""
				if nonPwmPin != "" {
					pin = nonPwmPin
					pinWarningText = "Note: non-PWM pin being used\n"
				} else {
					pin = pwmPin
				}
				if lang == goLang {
					msg.SetText(fmt.Sprintf("Command line: %v\n%s%s", buildGoCommand(pin, divisor, rrange, pulsewidth, pwmType),
						freqMsg, pinWarningText))
				} else {
					msg.SetText(fmt.Sprintf("Command line: %v\n%s%s", buildCCommand(pin, divisor, rrange, pulsewidth, pwmType,
						pwmMode), freqMsg, pinWarningText))
				}

				var out bytes.Buffer
				var cmd *exec.Cmd
				if lang == goLang {
					cmd = exec.Command("sudo", buildGoCommand(pin, divisor, rrange, pulsewidth, pwmType)...)
				} else {
					cmd = exec.Command("sudo", buildCCommand(pin, divisor, rrange, pulsewidth, pwmType, pwmMode)...)
				}
				cmd.SysProcAttr = &syscall.SysProcAttr{Setpgid: true}
				cmd.Stdout = &out

				if err := cmd.Start(); err != nil {
					msg.SetText(fmt.Sprintf("Error starting test: %s", err))
					running = false
					return
				}
				running = true

				<-stopTest
				// Not sure why sending a signal to the process via cmd.Process.Signal() doesn't stop
				// the process but using syscall.Kill() to send the same signal works.
				//				if err := cmd.Process.Signal(syscall.SIGINT); err != nil {
				//					msg.SetText(fmt.Sprintf("Error sending interrupt signal: %s", err))
				//					return
				//				}
				if err := syscall.Kill(-cmd.Process.Pid, syscall.SIGINT); err != nil {
					msg.SetText(fmt.Sprintf("Error sending interrupt signal: %s", err))
				}
				running = false
				msg.SetText("Test stopped")
			}()
		}).
		AddButton("Stop", func() {
			go func() {
				if running {
					close(stopTest)
					running = false
					return
				}
				msg.SetText("No tests running")
			}()
		}).
		AddButton("Reset", func() {
			pwmApp.pwmParms.GetFormItem(0).(*tview.DropDown).SetCurrentOption(-1)
			pwmApp.pwmParms.GetFormItem(1).(*tview.InputField).SetText("")
			pwmApp.pwmParms.GetFormItem(2).(*tview.InputField).SetText("")
			pwmApp.pwmParms.GetFormItem(3).(*tview.DropDown).SetCurrentOption(-1)
			pwmApp.pwmParms.GetFormItem(4).(*tview.InputField).SetText("")
			pwmApp.pwmParms.GetFormItem(5).(*tview.InputField).SetText("")
			pwmApp.pwmParms.GetFormItem(6).(*tview.DropDown).SetCurrentOption(-1)
		}).
		AddButton("Quit", func() {
			ui.Stop()
		})
}

const generalHelp = `General information blah blah blah blah.`
