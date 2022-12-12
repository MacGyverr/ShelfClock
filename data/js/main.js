const url = "http://10.0.0.185";
const radioButtons = {
    'ColorPalette': 'colorPalette',
    'suspendType': 'suspendType',
    'spotlightsColorSettings': 'spotlightsColorSettings',
    'ClockDisplayType': 'ClockDisplayType',
    'ColonType': 'colonType',
    'ClockColorSettings': 'clockColorSettings',
    'DateDisplayType': 'dateSettings',
    'DateColorSettings': 'dateColorSettings', 
    'TempType': 'tempType',
    'TempDisplayType': 'tempSettings',
    'TempColorSettings': 'tempColorSettings',
    'HumiDisplayType': 'humiSettings',
    'HumiColorSettings': 'humiColorSettings',
    'spectrumBackgroundSettings': 'spectrumBackgroundSettings',
    'spectrumColorSettings': 'spectrumColorSettings',
    'scrollColorSettings': 'scrollColorSettings'
};
const dropDownSelectors = {
    'ColorChangeFrequency': 'colorChangeFrequency',
    'suspendFrequency': 'suspendFrequency',
    'TimezoneSetting': 'timezoneSelect',
    'CorrectionSelect': 'correctionSelect',
    'scrollFrequency': 'scrollFrequency'
}
const directValueSelectors = {
    'rangeBrightness': 'rangeBrightness',
    'spotlightcolor': 'spotlightsColor',
    'colorHour': 'colorHour',
    'colorMin': 'colorMins',
    'colorColon': 'colorColon',
    'dayColor': 'dayColor',
    'separatorColor': 'separatorColor',
    'monthColor': 'monthColor',
    'TempColor': 'tempColor',
    'DegreeColor': 'degreeColor',
    'TypeColor': 'typeColor',
    'HumiColor': 'humiColor',
    'HumiDecimalColor': 'humiDecimalColor',
    'HumiSymbolColor': 'symbolColor',
    'colorCD': 'colorCD',
    'scoreboardColorLeft': 'newscoreboardColorLeft',
    'scoreboardColorRight': 'newscoreboardColorRight',
    'spectrumBackgroundColor': 'spectrumBackground',
    'spectrumColor': 'spectrumColor',
    'scrollText': 'scrollText',
    'scrollColor': 'scrollColor',
}
const checkboxSelectors = {
    'useSpotlights': 'useSpotlights',
    'DSTime': 'DSTime',
    'useAudibleAlarm': 'alarmCD',
    'colorchangeCD': 'colorchangeCD',
    'randomSpectrumMode': 'randomSpectrumMode',
    'scrollOptions1': 'scrollOptions1',
    'scrollOptions2': 'scrollOptions2',
    'scrollOptions3': 'scrollOptions3',
    'scrollOptions4': 'scrollOptions4',
    'scrollOptions5': 'scrollOptions5',
    'scrollOptions6': 'scrollOptions6',
    'scrollOptions7': 'scrollOptions7',
    'scrollOptions8': 'scrollOptions8',
    'scrollOverride': 'scrollOverride'
}

document.addEventListener('DOMContentLoaded', function () {
    
    /* GLOBAL */
    document.querySelector("nav button").addEventListener('click', function() {
        let navDiv = document.querySelector("nav");
        if (navDiv.classList.contains("show")) {
            navDiv.classList.remove("show");
        } else {
            navDiv.classList.add("show");
        }
    });

    document.querySelectorAll("input[type='button'][data-url]").forEach((element) => {
        element.addEventListener('click', async event => {
            await fetch(`${url}/${event.target.dataset.url}`, {
                method: 'POST'
            });
        });
    });

    /* HOME STUFF */
    if (document.querySelectorAll("form[name='shelfclock-home']").length > 0) {

        /* load home */
        loadHome();
        
        document.querySelectorAll("[data-showspectrum]").forEach(element => {
            element.addEventListener('click', async event => {
                let formData = new FormData();
                formData.append("spectrumMode", event.target.dataset.showspectrum);
                await fetch(`${url}/goSpectrumMode`, {
                    method: 'POST',
                    body: formData
                });
            });
        });

        document.querySelectorAll("[data-lightshow]").forEach(element => {
            element.addEventListener('click', async event => {
                let formData = new FormData();
                formData.append("lightshowMode", event.target.dataset.lightshow);
                await fetch(`${url}/goLightshowMode`, {
                    method: 'POST',
                    body: formData
                });
            });
        });
        
        document.querySelectorAll("input[name='countdown'], input[name='stopwatch']").forEach(element => {
            element.addEventListener('click', async event => {
                let hours = document.querySelector("[name='hours']").value;
                let minutes = document.querySelector("[name='minutes']").value;
                let seconds = document.querySelector("[name='seconds']").value;
                let post = "goCountdownMode";
                var ms = (hours * 60 * 60 * 1000) + (minutes * 60 * 1000) + (seconds * 1000);

                if (event.target.name === 'stopwatch') {
                    post = "goStopwatchMode"
                }

                let formData = new FormData();
                formData.append("ms", ms);
                await fetch(`${url}/${post}`, {
                    method: 'POST',
                    body: formData
                });
            });
        });

        document.querySelectorAll("input[name='left'], input[name='right']").forEach(element => {
            element.addEventListener('change', async event => {
                let left = document.querySelector("input[name='left']").value;
                let right = document.querySelector("input[name='right']").value;
                let formData = new FormData();
                formData.append('left', left);
                formData.append('right', right)
                await fetch(`${url}/goScoreboardMode`, {
                    method: 'POST',
                    body: formData
                });           
            })
        });

        document.querySelectorAll("input[name='upleft'], input[name='upright'], input[name='downleft'], input[name='downright']").forEach(element => {
            element.addEventListener('click', async event => {
                let left = document.querySelector("input[name='left']");
                let right = document.querySelector("input[name='right']");
                switch (event.target.name) {
                    case 'upleft': left.value = (left.value === "99") ? "99" : parseInt(left.value) + 1; break;
                    case 'upright': right.value = (right.value === "99") ? "99" : parseInt(right.value) + 1; break;
                    case 'downleft': left.value = (left.value === "0") ? "0" : parseInt(left.value) - 1; break;
                    case 'downright': right.value = (right.value === "0") ? "0" : parseInt(right.value) - 1; break;
                }
                if (event.target.name === 'upleft' || event.target.name === 'downleft') {
                    left.dispatchEvent(new Event('change'));
                } else {
                    right.dispatchEvent(new Event('change'));
                }
            })
        });
    
    }

    /* SETTINGS STUFF */
    if (document.querySelectorAll("form[name='shelfclock-settings']").length > 0) {

        /* get all */
        loadSettings();

        for (let [key, value] of Object.entries({...radioButtons, ...dropDownSelectors, ...{'rangeBrightness': 'rangeBrightness'}})) {
            document.querySelectorAll(`[name='${value}`).forEach((element) => {
                element.addEventListener("change", async function(event) {
                    let body = {};
                    body[key] = event.target.value;
                    await fetch(`${url}/updateanything`, {
                        method: 'POST',
                        body: JSON.stringify(body)
                    });
                });
            });
        }

        for (let [key, value] of Object.entries({...directValueSelectors})) {
            if (key === 'rangeBrightness') continue;
            const debounced = debounce(async function (event) {
                let body = {};
                if (key === 'scrollText') {
                    body[key] = event.target.value;
                } else {
                    let rbg = hexToRgb(event.target.value);
                    body[key] = {
                        r: rbg['r'],
                        b: rbg['b'],
                        g: rbg['g']
                    }
                }
                await fetch(`${url}/updateanything`, {
                    method: 'POST',
                    body: JSON.stringify(body)
                });
            }, 500); // half a second debounce so we don't flood the server with requests. 
            document.querySelector(`[name='${value}']`).addEventListener("input", debounced);
        }

        /* shelf spotlights */
        updateCheckboxValueByUrl("useSpotlights", "useSpotlights", url + "/updateuseSpotlights");

        /* DSTtime */
        updateCheckboxValueByUrl("DSTime", "DSTime", url + "/updateDSTime");

        /* timer options */
        updateCheckboxValueByUrl("alarmCD", "alarmCD", url + "/updatealarmCD");
        updateCheckboxValueByUrl("colorchangeCD", "colorchangeCD", url + "/updatecolorchangeCD");

        /* spectrum analyzer options */
        updateCheckboxValueByUrl("randomSpectrumMode", "randomSpectrumMode", url + "/updaterandomSpectrumMode");

        /* scroll information */
        updateCheckboxValueByUrl("scrollOptions1", "scrollOptions1", url + "/updatescrollOptions1");
        updateCheckboxValueByUrl("scrollOptions2", "scrollOptions2", url + "/updatescrollOptions2");
        updateCheckboxValueByUrl("scrollOptions3", "scrollOptions3", url + "/updatescrollOptions3");
        updateCheckboxValueByUrl("scrollOptions4", "scrollOptions4", url + "/updatescrollOptions4");
        updateCheckboxValueByUrl("scrollOptions5", "scrollOptions5", url + "/updatescrollOptions5");
        updateCheckboxValueByUrl("scrollOptions6", "scrollOptions6", url + "/updatescrollOptions6");
        updateCheckboxValueByUrl("scrollOptions7", "scrollOptions7", url + "/updatescrollOptions7");
        updateCheckboxValueByUrl("scrollOptions8", "scrollOptions8", url + "/updatescrollOptions8");

        /* scroll options */
        updateCheckboxValueByUrl("scrollOverride", "scrollOverride", url + "/updatescrollOverride");

        document.querySelector("[name='setdatetime']").addEventListener('click', async (event) => {
            let date = new Date();
            await fetch(`${url}/setdate`, {
                method: 'POST',
                body: JSON.stringify({
                    year: date.getFullYear().toString(),
                    month: pad(date.getMonth() + 1),
                    day: pad(date.getDate()),
                    hour: pad(date.getHours()),
                    min: pad(date.getMinutes()),
                    sec: pad(date.getSeconds())
                })
            });
        });

        /* datetime */
        let datetime = document.querySelectorAll(".datetime");
        if (datetime.length > 0) {
            window.setInterval(() => {
                let date = new Date()
                datetime.forEach((element) => {
                    element.innerHTML = date.toLocaleDateString('en-us', {
                        month: 'long',
                        day: 'numeric',
                        year: 'numeric'
                    });
                    element.innerHTML += " ";
                    element.innerHTML += date.toLocaleTimeString('en-us', {
                        hour12: false
                    })
                });
            }, 1000);
        }
    }
});
/* load home */
async function loadHome() {
    let response = await fetch(`${url}/gethome`);
    if (response.ok) {
        let data = await response.json();
        document.querySelector("[name='left']").value = data.scoreboardLeft;
        document.querySelector("[name='right']").value = data.scoreboardRight;
    }
}

/* load settings */
async function loadSettings() {
    let response = await fetch(`${url}/getsettings`);
    if (response.ok) {
        let settings = await response.json();
        for (let [key, value] of Object.entries(settings)) {
            if (key in radioButtons) {
                setRadioButton(radioButtons[key], value);
            } else if (key in dropDownSelectors) {
                setDropDown(dropDownSelectors[key], value);
            } else if (key in directValueSelectors) {
                setValue(directValueSelectors[key], value);
            } else if (key in checkboxSelectors) {
                setCheckbox(checkboxSelectors[key], value)
            } else {
                console.log(`key is not found ${key}`);
            }
        }
    }
}

/* set form elements */
async function setCheckbox(selectorName, value) {
   document.querySelector(`[name='${selectorName}']`).checked = value;
}

async function setValue(selectorName, value) {
    document.querySelector(`[name='${selectorName}']`).value = value;
}

async function setDropDown(selectorName, value) {
    let select = document.querySelector(`select[name='${selectorName}']`);
    [...select.options].forEach((option, index) => {
        if(value.toString() === option.value) {
            select.selectedIndex = index;
        }
    });
}

async function setRadioButton(selectorName, value) {
    document.querySelectorAll(`input[name='${selectorName}']`).forEach((element) => {
        if (element.value === value.toString()) {
            element.checked = true;
        }
    });
}

/* update form elements */
async function updateTextFieldByUrl(selectorName, formDataName, updateUrl) {
    const debounced = debounce(async function (event) {
        let formData = new FormData();
        formData.append(formDataName, event.target.value);
        await fetch(updateUrl, {
            method: 'POST',
            body: formData
        });
    }, 500);
    document.querySelector(`[name='${selectorName}']`).addEventListener("input", debounced);
}

async function updateColorByUrl(selectorName, updateUrl) {
    const debounced = debounce(async function (event) {
        let rbg = hexToRgb(event.target.value);
        let formData = new FormData();
        if (rbg) {
            formData.append("r", rbg['r']);
            formData.append("b", rbg['b']);
            formData.append("g", rbg['g']);
            await fetch(updateUrl, {
                method: 'POST',
                body: formData
            });
        }
    }, 500); // half a second debounce so we don't flood the server with requests. 
    document.querySelector(`[name='${selectorName}']`).addEventListener("input", debounced);
}

async function updateElementByUrl(selectorName, formDataName, updateUrl, eventType) {
    document.querySelectorAll(`[name='${selectorName}']`).forEach((element) => {
        element.addEventListener(eventType, async function(event) {
            let formData = new FormData();
            formData.append(formDataName, event.target.value);
            await fetch(updateUrl, {
                method: 'POST',
                body: formData
            });
        });
    });
}

async function updateCheckboxValueByUrl(selectorName, formDataName, updateUrl) {
    document.querySelector(`[name='${selectorName}']`).addEventListener("change", async function (event) {
        let formData = new FormData();
        formData.append(formDataName, (event.target.checked === true) ? "true" : "false");
        await fetch(updateUrl, {
            method: 'POST',
            body: formData
        });
    });
}

async function getCheckboxValueByUrl(electorName, getURL) {
    let response = await fetch(getURL);
    if (response.ok) {
        document.querySelector(`[name='${electorName}']`).checked = (await response.text() === "1") ? true : false;
    }
}

/* utility functions */
function getKeyByValue (obj, value) {
    return Object.keys(obj).find(key => obj[key] === value);
}

function hexToRgb(hex) {
    // Expand shorthand form (e.g. "03F") to full form (e.g. "0033FF")
    var shorthandRegex = /^#?([a-f\d])([a-f\d])([a-f\d])$/i;
    hex = hex.replace(shorthandRegex, function(m, r, g, b) {
      return r + r + g + g + b + b;
    });
  
    var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
    return result ? {
      r: parseInt(result[1], 16),
      g: parseInt(result[2], 16),
      b: parseInt(result[3], 16)
    } : null;
}

function debounce(func, wait, immediate) {
    var timeout;
    return function() {
        var context = this, args = arguments;
        var later = function() {
            timeout = null;
            if (!immediate) func.apply(context, args);
        };
        var callNow = immediate && !timeout;
        clearTimeout(timeout);
        timeout = setTimeout(later, wait);
        if (callNow) func.apply(context, args);
    };
};

function pad(number) {
    return ( number < 10 ? '0' : '' ) + number
}