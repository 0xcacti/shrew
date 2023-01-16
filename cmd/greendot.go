package main

import (
	"log"
	"math/rand"
	"sync/atomic"
	"time"

	"fyne.io/fyne/v2"
	"fyne.io/fyne/v2/app"
	"fyne.io/fyne/v2/driver/desktop"
	"fyne.io/fyne/v2/widget"
	"github.com/go-vgo/robotgo"
	"go.uber.org/zap"
	"go.uber.org/zap/zapcore"
)

// TODO - programming goals

// do we need to draft up some legal agreement so they don't sue us if they lose their job
// do we need to handle returns or add non-refundability

var runBool atomic.Bool
var sugar *zap.SugaredLogger

func init() {

	loggerConfig := zap.NewProductionConfig()
	loggerConfig.EncoderConfig.TimeKey = "timestamp"
	loggerConfig.EncoderConfig.EncodeTime = zapcore.TimeEncoderOfLayout(time.RFC1123)

	logger, err := loggerConfig.Build()
	if err != nil {
		log.Fatal(err)
	}

	sugar = logger.Sugar()
	sugar.Info("Warming up program")

}

func main() {

	// create app and window and init go routine for mouse behavior
	a := app.New()
	w := a.NewWindow("Green Dot")
	w.Resize(fyne.NewSize(200, 100))
	w.SetFixedSize(true) // should it be fixed size
	w.SetMaster()
	w.SetPadded(true)

	go start()

	// pull in resources for in app icons
	iconResource, err := fyne.LoadResourceFromPath("./assets/ethereum.png")
	if err != nil {
		sugar.Debug("Failed to load application icon")
	}

	// init keyboard shortcut

	// verify that we can add app to system tray
	if desk, ok := a.(desktop.App); ok {
		m := fyne.NewMenu("Green Dot",
			fyne.NewMenuItem("Show", func() {
				w.Show()
			}))
		desk.SetSystemTrayMenu(m)
		desk.SetSystemTrayIcon(iconResource)
	}

	// define shutdown behavior
	w.SetCloseIntercept(func() {
		w.Close()
		a.Quit()
	})

	var runButton *widget.Button

	runButton = widget.NewButton("Start", func() {

		// switch text
		toggleStartStopButton(runButton)

		// define button behavior
		if runButton.Text == "Stop" {
			runBool.Store(true)
			sugar.Info("Application started")
		} else {
			runBool.Store(false)
			sugar.Info("Application stopped")
		}
		runButton.Refresh()
	})

	ctrlTab := &desktop.CustomShortcut{KeyName: fyne.KeyG, Modifier: fyne.KeyModifierShortcutDefault}
	w.Canvas().AddShortcut(ctrlTab, func(shortcut fyne.Shortcut) {
		toggleStartStopButton(runButton)
		if runButton.Text == "Stop" {
			runBool.Store(true)
			sugar.Info("Application Started with shortcut ctrl-g")
		} else {
			runBool.Store(false)
			sugar.Info("Application Stopped with shortcut ctrl-g")
		}
	})

	// run and display app
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

func start() {

	for {
		if runBool.Load() {
			x := rand.Intn(1000)
			y := rand.Intn(1000)
			robotgo.MoveSmooth(x, y)
			time.Sleep(3 * time.Second)
		} else {
			time.Sleep(1 * time.Second)
		}

	}

}
