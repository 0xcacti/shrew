package main

import (
	"fmt"
	"math/rand"
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
func main() {
	a := app.New()
	w := a.NewWindow("Green Dot")
	w.Resize(fyne.NewSize(500, 100))
	startStopChannel := make(chan bool)

	var runButton *widget.Button

	// create go func that does the movement with a channel to check if true or fales
	// start stop just tracks if we should be starting or stopping, on receipt of that value,
	// we send sigint to process
	runButton = widget.NewButton("Start", func() {
		toggleStartStopButton(runButton)
		if runButton.Text == "Start" {
			preStart(startStopChannel)
			fmt.Println("Have yer stopped lad?")
		} else {
			startStopChannel <- false
		}
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

func preStart(c chan bool) {
	fmt.Println("balls1")

	go start(c)

	fmt.Println("balls2")

}

func start(c chan bool) {

	for {
		select {
		case run := <-c:
			if !run {
				return
			}

		default:

			x := rand.Intn(1000)
			y := rand.Intn(1000)

			robotgo.MoveSmooth(x, y)

			time.Sleep(5 * time.Second)

		}

	}
}
