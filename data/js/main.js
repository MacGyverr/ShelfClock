const url = "http://shelfclock/";
document.addEventListener('DOMContentLoaded', function () {
    
    // toggle nav 
    document.querySelector("nav button").addEventListener('click', function() {
        let navDiv = document.querySelector("nav");
        if (navDiv.classList.contains("show")) {
            navDiv.classList.remove("show");
        } else {
            navDiv.classList.add("show");
        }
    });


    getpastelColors();

    
});

async function getpastelColors() {
    console.log(url + "getpastelColors")
    let response = await fetch(url + "getpastelColors");
    if (response.ok) {
        document.querySelector("input[name='colorPalette']").value = await response.text();
    }
}
   /* var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (this.readyState == 4 && this.status == 200) {
        if (this.responseText == 0) {document.getElementById("colorPalette1").checked = true;}
        if (this.responseText == 1) {document.getElementById("colorPalette2").checked = true;}
        }
    };
    xhttp.open("GET", "getpastelColors", true);
    xhttp.send();*/

