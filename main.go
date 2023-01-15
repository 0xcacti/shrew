package main

import (
	"fmt"
	"math/rand"
	"sync/atomic"
	"time"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/app"
	"fyne.io/fyne/v2/widget"
	"github.com/go-vgo/robotgo"
)

// goals
// make it look pretty

// make it appear in a good location
// make it behave properly
// make it quit on red x

// use atomics

// add bench marks for cpu's to make sure it's not eating cycles unnecesarrily.

// do we need to draft up some legal agreement so they don't sue us if they lose their job
// do we need to handle returns or add non-refundability
//

var runBool atomic.Bool

func main() {

	a := app.New()
	w := a.NewWindow("Green Dot")
	w.Resize(fyne.NewSize(500, 100))

	startStopChannel := make(chan bool)
	go startWithChan(startStopChannel)
	// startStopChannel <- true

	var runButton *widget.Button

	// create go func that does the movement with a channel to check if true or fales
	// start stop just tracks if we should be starting or stopping, on receipt of that value,
	// we send sigint to process
	runButton = widget.NewButton("Start", func() {

		toggleStartStopButton(runButton)

		if runButton.Text == "Stop" {
			runBool.Store(true)
			// startStopChannel <- true

		} else {
			runBool.Store(false)
			// startStopChannel <- false
		}

		runButton.Refresh()

	})

	w.SetContent(runButton)
	w.ShowAndRun()
}

func toggleStartStopButton(button *widget.Button) {
	if button.Text == "Start" {
		button.SetText("Stop")
	} else {
		button.SetText("Start")
	}

}

func startWithChan(c chan bool) {

	for {
		if runBool.Load() {
			fmt.Println("Moving cursor")
			x := rand.Intn(1000)
			y := rand.Intn(1000)
			robotgo.MoveSmooth(x, y)
			time.Sleep(2 * time.Second)
		} else {
			time.Sleep(1 * time.Second)
		}

	}

}
