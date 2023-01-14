package main

import (
	"math/rand"
	"time"

	"fyne.io/fyne/v2/app"
	"fyne.io/fyne/v2/widget"
	"github.com/go-vgo/robotgo"
)

func main() {
	a := app.New()
	w := a.NewWindow("Clock")

	clock := widget.NewLabel("")
	updateTime(clock)

	w.SetContent(clock)
	go start()
	w.ShowAndRun()
}

func updateTime(clock *widget.Label) {
	formatted := time.Now().Format("Time: 03:04:05")
	clock.SetText(formatted)
}

func start() {
	for {
		x := rand.Intn(1000)
		y := rand.Intn(1000)

		robotgo.MoveSmooth(x, y)

		time.Sleep(2 * time.Second)
	}
}
