// Demo code for the Grid primitive.
package main

import (
	"bytes"
	"fmt"
	"os/exec"

	"github.com/gdamore/tcell/v2"
	"github.com/rivo/tview"
)

const (
	cLang       = "C"
	goLang      = "Go"
	pwmModeBal  = "Balanced"
	pwmModeHelp = "PWM Mode"
)

// PWMEx contains the state of the application
type PWMEx struct {
	langs    *tview.DropDown
	pwmParms *tview.Form
	topics   *tview.Form
	msg      *tview.TextView
	helpView *tview.TextView
	codeView *tview.TextView
}

// SetPWMMode gets called when the PWM Mode is set/changed
func (p *PWMEx) SetPWMMode(option string, optionIdx int) {
	if p.topics == nil || p.pwmParms == nil || optionIdx == -1 {
		return // app is still initializing
	}

	_, lang := p.langs.GetCurrentOption()
	helpIdx, helpItem := p.topics.GetFormItem(0).(*tview.DropDown).GetCurrentOption()
	if lang == goLang && option == pwmModeBal {
		pwmModeDropdown := p.pwmParms.GetFormItem(3).(*tview.DropDown)
		pwmModeDropdown.SetCurrentOption(0)
		p.SetHelpTopic(helpItem, helpIdx)
		p.msg.SetText("Warning: PWM mode 'balanced' is not available in 'Go'.")
		return
	}

	p.msg.Clear()
	if helpItem == pwmModeHelp {
		p.SetHelpTopic(helpItem, helpIdx)
	}
}

// SetLanguage is called when a language option is selected or changed
func (p *PWMEx) SetLanguage(lang string, langIdx int) {
	if p.helpView == nil || p.pwmParms == nil {
		return // app is still initializing
	}

	pwmModeDropdown := p.pwmParms.GetFormItem(3).(*tview.DropDown)
	pwmIdx, pwmMode := pwmModeDropdown.GetCurrentOption()
	if pwmMode == pwmModeBal && lang == goLang {
		p.SetPWMMode(pwmMode, pwmIdx)
		p.msg.SetText("Warning: PWM mode 'balanced' is not available in 'Go'.")
	}

	helpTopic := p.topics.GetFormItem(0).(*tview.DropDown)
	helpIdx, hItem := helpTopic.GetCurrentOption()
	if hItem == pwmModeHelp {
		p.SetHelpTopic(hItem, helpIdx)
	}
}

var ()

func main() {
	msg := tview.NewTextView().SetTextColor(tcell.ColorRed)
	langDropDown := tview.NewDropDown().SetLabel("Language:").AddOption(cLang, nil).AddOption(goLang, nil)
	language := tview.NewForm().AddFormItem(langDropDown)

	pwmApp := PWMEx{msg: msg, langs: langDropDown}
	langDropDown.SetSelectedFunc(pwmApp.SetLanguage)

	ui := tview.NewApplication()

	helpView := tview.NewTextView().
		SetDynamicColors(true).
		SetRegions(true).
		SetWordWrap(true).
		SetChangedFunc(func() {
			ui.Draw()
		})

	codeView := tview.NewTextView().
		SetDynamicColors(true).
		SetRegions(true).
		SetWordWrap(true).
		SetChangedFunc(func() {
			ui.Draw()
		})

	helpView.SetTitle("Help").SetBorder(true).SetTitleColor(tcell.ColorYellow)
	codeView.SetTitle("Code").SetBorder(true)
	msg.SetText("Messages: Use mouse to navigate screen")

	pwmApp.helpView = helpView
	pwmApp.codeView = codeView

	// Text/Code Grid
	tcodeGrid := tview.NewGrid().
		SetRows(-1, -1).
		SetColumns(0).
		AddItem(helpView, 0, 0, 1, 1, 0, 0, false).
		AddItem(codeView, 1, 0, 1, 1, 0, 0, false)

	topics := tview.NewForm().
		AddDropDown("Help Topics:",
			[]string{"PWM Pin", "Non-PWM Pin", "Clock Divisor", "PWM Mode", "Range", "PulseWidth"},
			-1, pwmApp.SetHelpTopic)
	pwmApp.topics = topics

	parms := tview.NewForm().
		AddDropDown("PWM Pin:", []string{"12", "18", "13", "19"}, -1, nil).
		AddInputField("Non-PWM Pin:", "", 2, nil, nil).
		AddInputField("Clock Divisor:", "", 10, nil, nil).
		AddDropDown("PWM Mode:", []string{"Mark/Space", "Balanced"}, -1, pwmApp.SetPWMMode).
		AddInputField("Range:", "", 10, nil, nil).
		AddInputField("Pulse Width:", "", 10, nil, nil)

	pwmApp.pwmParms = parms

	stopTest := make(chan interface{})
	buttons := tview.NewForm().
		AddButton("Start", func() {
			go func() {
				_, pwmPin := pwmApp.pwmParms.GetFormItem(0).(*tview.DropDown).GetCurrentOption()
				divisor := pwmApp.pwmParms.GetFormItem(2).(*tview.InputField).GetText()
				cycle := pwmApp.pwmParms.GetFormItem(4).(*tview.InputField).GetText()
				pulseWidth := pwmApp.pwmParms.GetFormItem(5).(*tview.InputField).GetText()
				msg.SetText(fmt.Sprintf("PWM pin: %s, divisor: %s, cycle: %s, pulse width: %s", pwmPin, divisor, cycle, pulseWidth))
				msg.SetText(fmt.Sprintf("Command line: %v", buildCommand(pwmPin, divisor, cycle, pulseWidth)))

				var out bytes.Buffer
				cmd := exec.Command("sudo", buildCommand(pwmPin, divisor, cycle, pulseWidth)...)
				cmd.Stdout = &out

				if err := cmd.Start(); err != nil {
					msg.SetText(fmt.Sprintf("Error starting test: %s", err))
					return
				}

				<-stopTest
				if err := cmd.Process.Kill(); err != nil {
					msg.SetText(fmt.Sprintf("Error stopping test: %s", err))
					return
				}
				msg.SetText("Test stopped")
			}()
		}).
		AddButton("Stop", func() {
			go func() {
				close(stopTest)
			}()
		}).
		AddButton("Reset", nil).
		AddButton("Quit", func() {
			ui.Stop()
		})

	// Main Grid
	grid := tview.NewGrid().
		SetRows(2, 0, 2, 3).
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

// SetHelpTopic gets called when a help topic is chosen or changes
func (p *PWMEx) SetHelpTopic(option string, optionIndex int) {
	if p.pwmParms == nil {
		return // app is still initializing
	}

	p.helpView.Clear()
	p.codeView.Clear()

	_, lang := p.langs.GetCurrentOption()
	_, modeOpt := p.pwmParms.GetFormItem(3).(*tview.DropDown).GetCurrentOption()

	switch optionIndex {
	case 0:
		p.helpView.Write([]byte("The PWM Pin is blah blah blah."))
		p.codeView.Write([]byte("...\npin = rpio.Pin(18)\npin.Mode(rpio.PWM)\n\n..."))
	case 1:
		p.helpView.Write([]byte("The Non-PWM Pin is blah blah blah."))
		p.codeView.Write([]byte("...\npin = rpio.Pin(18)\npin.Mode(rpio.OUTPUT)\n\n..."))
	case 2:
		p.helpView.Write([]byte("The Clock Divisor is blah blah blah."))
		p.codeView.Write([]byte("...\npin.Freq(clockDivisor))\n\n..."))
	case 3:
		if lang == goLang && pwmModeBal == modeOpt {
			p.helpView.Write([]byte("[red]PWM Mode 'balanced' isn't available in Go.[white]\n"))
		}
		p.helpView.Write([]byte("PWM Mode is blah blah blah. "))
		p.codeView.Write([]byte("...\npin = pin.Mode(rpio.MarkSpace))\n...\n"))
	case 4:
		p.helpView.Write([]byte("Range is blah blah blah."))
		p.codeView.Write([]byte("...\npin.DutyCycle(pulseWidth, range))\n\n..."))
	case 5:
		p.helpView.Write([]byte("Pulse width is blah blah blah blah blah blah blah blah blah blah blah blah blah blah blah blah blah blah blah balh balh balh balh ablah blah blah."))
		p.codeView.Write([]byte("...\npin.DutyCycle(pulseWidth, range))\n\n..."))
	}

	p.msg.Clear()
}

func buildCommand(pwmPin, divisor, cycle, pulseWidth string) []string {
	return []string{"/usr/local/go/bin/go", "run", "./apps/freqtest.go",
		fmt.Sprintf("-pin=%s", pwmPin), fmt.Sprintf("-div=%s", divisor),
		fmt.Sprintf("-cycle=%s", cycle), fmt.Sprintf("-pulseWidth=%s", pulseWidth)}
}
