
// buttons
const mouseButton = document.getElementById('mouseButton');

// event listeners
mouseButton.addEventListener("click", Start);


// functions 
async function Start()  {
    console.log("Started");
    mouseButton.removeEventListener("click", Start);
    mouseButton.addEventListener("click", Stop);
    mouseButton.value = "Stop";
   await window.electronAPI.startMouse();

    console.log("test")
    

}

function Stop() {
    console.log("Stopped");
    mouseButton.removeEventListener("click", Stop);
    mouseButton.addEventListener("click", Start);
    mouseButton.value = "Start"
}


