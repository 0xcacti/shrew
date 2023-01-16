package main

import (
	"fmt"
	"log"
	"math/rand"
	"sync/atomic"
	"time"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/app"
	"fyne.io/fyne/v2/driver/desktop"
	"fyne.io/fyne/v2/widget"
	"github.com/go-vgo/robotgo"
)

// TODO goals
// make it look pretty - I am not really sure what I can do here
// make it appear in a good location
// make it quit on red x

// use atomics

// add bench marks for cpu's to make sure it's not eating cycles unnecesarrily.

// do we need to draft up some legal agreement so they don't sue us if they lose their job
// do we need to handle returns or add non-refundability
//

var runBool atomic.Bool

func main() {
	// start the core functionality
	// TODO - should we add wait group for graceful shutdown - probably
	// add keyboard shortcut to quit

	a := app.New()
	w := a.NewWindow("Green Dot")

	deskTopIcon, err := fyne.LoadResourceFromPath("./assets/ethereum.jpg")
	if err != nil {
		log.Fatal(err)
	}
	iconResource, err := fyne.LoadResourceFromPath("./assets/ethereum.png")
	if err != nil {
		log.Fatal(err)
	}
	icon := widget.NewIcon(iconResource)

	w.SetContent(icon)
	a.SetIcon(deskTopIcon)

	// icon := widget.NewIcon(iconResource)

	if desk, ok := a.(desktop.App); ok {
		m := fyne.NewMenu("Green Dot",
			fyne.NewMenuItem("Show", func() {
				w.Show()
			}))
		desk.SetSystemTrayMenu(m)
		desk.SetSystemTrayIcon(iconResource)
		// r := fyne.NewStaticResource("meowtown", []byte(data))
		// // icon := widget.NewIcon(r)
		// desk.SetSystemTrayIcon(r)
	}

	w.SetContent(widget.NewLabel("Fyne System Tray"))
	w.SetCloseIntercept(func() {
		w.Close()
		a.Quit()
	})
	w.ShowAndRun()

	// go start()

	// a := app.New()
	// // TODO - create logging for not in desktop mode
	// if desk, ok := a.(desktop.App); ok {
	// 	m := fyne.NewMenu("MyApp",
	// 		fyne.NewMenuItem("Show", func() {
	// 			log.Println("Tapped show")
	// 		}))
	// 	desk.SetSystemTrayMenu(m)
	// }

	// w := a.NewWindow("Green Dot")
	// w.SetCloseIntercept(func() {
	// 	w.Hide()
	// })
	// var runButton *widget.Button

	// w.Resize(fyne.NewSize(200, 100))

	// runButton = widget.NewButton("Start", func() {

	// 	// switch text
	// 	toggleStartStopButton(runButton)

	// 	// define button behavior
	// 	if runButton.Text == "Stop" {
	// 		runBool.Store(true)
	// 	} else {
	// 		runBool.Store(false)
	// 	}
	// 	runButton.Refresh()
	// })

	// // run and display app
	// w.SetContent(runButton)
	// w.ShowAndRun()
}

func toggleStartStopButton(button *widget.Button) {
	if button.Text == "Start" {
		button.SetText("Stop")
	} else {
		button.SetText("Start")
	}

}

func start() {

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
