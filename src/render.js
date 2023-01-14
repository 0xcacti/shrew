



// buttons
const mouseButton = document.getElementById('mouseButton');

// event listeners
mouseButton.addEventListener("click", Start);


// functions 
function Start() {
    console.log("Started");
    mouseButton.removeEventListener("click", Start);
    mouseButton.addEventListener("click", Stop);
    mouseButton.value = "Stop"

    console.log("test")
    var robot = require("robotjs");

    var mouse=robot.getMousePos();
    console.log("Mouse is at x:" + mouse.x + " y:" + mouse.y);

    //Move the mouse down by 100 pixels.
    robot.moveMouse(mouse.x,mouse.y+100);


}

function Stop() {
    console.log("Stopped");
    mouseButton.removeEventListener("click", Stop);
    mouseButton.addEventListener("click", Start);
    mouseButton.value = "Start"
}


