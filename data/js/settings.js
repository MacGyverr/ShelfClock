const url = "http://shelfclock";
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
    setpastelColors();
    getColorChangeFrequency();
    setColorChangeFrequency();
    getrangeBrightness();
    setrangeBrightness();
    getsuspendType();
    setsuspendType();
    getsuspendFrequency();
    setsuspendFrequency();
});

/* suspend frequency */
async function getsuspendFrequency() {
    let response = await fetch(url + "/getsuspendFrequency");
    let value = await response.text();
    if (response.ok) {
        // document.querySelector("select[name='suspendFrequency']").selectedIndex = parseInt(await response.text()) + 1;
        let select = document.querySelector("select[name='suspendFrequency']");
        [...select.options].forEach((option, index) => {
            if(value === option.value) {
                select.selectedIndex = index;
            }
        });
    }
}

async function setsuspendFrequency() {
    document.querySelector("select[name='suspendFrequency']").addEventListener("change", async function (event) {
        let formData = new FormData();
        formData.append("suspendFrequency", event.target.value);
        await fetch(url + "/updatesuspendFrequency", {
            method: 'POST',
            body: formData
        });
    });
}


/* suspend type */
async function getsuspendType() {
    let response = await fetch(url + "/getsuspendType");
    if (response.ok) {
        let value = await response.text();
        document.querySelectorAll("input[name='suspendType']").forEach((element) => {
            if (element.value === value) {
                element.checked = true;
            }
        });
    }
}

async function setsuspendType() {
    document.querySelectorAll("input[name='suspendType']").forEach((element) => {
        element.addEventListener('click', async function(event) {
            let formData = new FormData();
            formData.append("suspendType", event.target.value);
            await fetch(url + "/updatesuspendType", {
                method: 'POST',
                body: formData
            });
        });
    });
}

/* Brightness */
async function getrangeBrightness() {
    let response = await fetch(url + "/getrangeBrightness");
    if (response.ok) {
        document.querySelector("input[name='rangeBrightness']").value = await response.text();
    }
}

async function setrangeBrightness() {
    document.querySelector("input[name='rangeBrightness']").addEventListener("change", async function (event) {
        let formData = new FormData();
        formData.append("rangeBrightness", event.target.value);
        await fetch(url + "/updaterangeBrightness", {
            method: 'POST',
            body: formData
        });
    });
}

/* Color Change Frequency */
async function getColorChangeFrequency() {
    let response = await fetch(url + "/getColorChangeFrequency");
    if (response.ok) {
        document.querySelector("select[name='colorChangeFrequency']").selectedIndex = parseInt(await response.text()) + 1;
    }
}

async function setColorChangeFrequency() {
    document.querySelector("select[name='colorChangeFrequency']").addEventListener("change", async function (event) {
        let formData = new FormData();
        formData.append("ColorChangeFrequency", event.target.value);
        await fetch(url + "/updateColorChangeFrequency", {
            method: 'POST',
            body: formData
        });
    });
}

/* Pastel Colors */
async function getpastelColors() {
    let response = await fetch(url + "/getpastelColors");
    if (response.ok) {
        let value = await response.text();
        document.querySelectorAll("input[name='colorPalette']").forEach((element) => {
            if (element.value === value) {
                element.checked = true;
            }
        });
    }
}
async function setpastelColors() {
    document.querySelectorAll("input[name='colorPalette']").forEach((element) => {
        element.addEventListener('click', async function(event) {
            let formData = new FormData();
            formData.append("ColorPalette", event.target.value);
            await fetch(url + "/updatePastelColors", {
                method: 'POST',
                body: formData
            });
        });
    });
}
