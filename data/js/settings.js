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

    /* color palette */
    getRadioButtonsByUrl("colorPalette", `${url}/getpastelColors`);
    updateElementByUrl("colorPalette", "ColorPalette", `${url}/updatePastelColors`, "click");

    /* color change frequency */
    getDropDownByUrl("colorChangeFrequency", url + "/getColorChangeFrequency");
    updateElementByUrl("colorChangeFrequency", "ColorChangeFrequency", url + "/updateColorChangeFrequency", "change");

    /* overall brightness */
    getDirectValueByUrl("rangeBrightness", url + "/getrangeBrightness");
    updateElementByUrl("rangeBrightness", "rangeBrightness", url + "/updaterangeBrightness", "change");

    /* suspend display */
    getRadioButtonsByUrl("suspendType", url + "/getsuspendType");
    updateElementByUrl("suspendType", "suspendType", url + "/updatesuspendType", "click");

    /* suspend frequency */
    getDropDownByUrl("suspendFrequency", url + "/getsuspendFrequency");
    updateElementByUrl("suspendFrequency", "suspendFrequency", url + "/updatesuspendFrequency", "change");

    /* shelf spotlights */
    getCheckboxValueByUrl("useSpotlights", url + "/getuseSpotlights");
    updateCheckboxValueByUrl("useSpotlights", "useSpotlights", url + "/updateuseSpotlights");
    
    /* shelf spotlight color */
    getDirectValueByUrl("spotlightsColor", url + "/getspotlightsColor");
    updateColorByUrl("spotlightsColor", url + "/updatespotlightsColor");
    
    /* shelf spotlight settings */
    getRadioButtonsByUrl("spotlightsColorSettings", url + "/getspotlightsColorSettings");
    updateElementByUrl("spotlightsColorSettings", "spotlightsColorSettings", url + "/updatespotlightsColorSettings", "click");
    
    /* clock display type */
    getRadioButtonsByUrl("ClockDisplayType", url + "/getClockDisplayType");
    updateElementByUrl("ClockDisplayType", "ClockDisplayType", url + "/updateClockDisplayType", "click");
    
    /* blicking center type */
    getRadioButtonsByUrl("colonType", url + "/getcolonType");
    updateElementByUrl("colonType", "ColonType", url + "/updateColonType", "click");
    
    /* timezone */
    getDropDownByUrl("timezoneSelect", url + "/getgmtOffset_sec");
    updateElementByUrl("timezoneSelect", "TimezoneSetting", url + "/updateTimezoneSettings", "change");

    /* DSTtime */
    getCheckboxValueByUrl("DSTime", url + "/getDSTime");
    updateCheckboxValueByUrl("DSTime", "DSTime", url + "/updateDSTime");

    /* color clock selection */
    getRadioButtonsByUrl("clockColorSettings", url + "/getClockColorSettings");
    updateElementByUrl("clockColorSettings", "ClockColorSettings", url + "/updateClockColorSettings", "click");

    /* color clock selection colors */
    getDirectValueByUrl("colorHour", url + "/getcolorHour");
    updateColorByUrl("colorHour", url + "/updateHourColor");
    getDirectValueByUrl("colorColon", url + "/getcolorColon");
    updateColorByUrl("colorColon", url + "/updateColonColor");
    getDirectValueByUrl("colorMins", url + "/getcolorMins");
    updateColorByUrl("colorMins", url + "/updateMinsColor");

    /* date display type */
    getRadioButtonsByUrl("dateSettings", url + "/getDateDisplayType");
    updateElementByUrl("dateSettings", "DateDisplayType", url + "/updateDateDisplayType", "click");

    /* color date selection */
    getRadioButtonsByUrl("dateColorSettings", url + "/getDateColorSettings");
    updateElementByUrl("dateColorSettings", "DateColorSettings", url + "/updateDateColorSettings", "click");
    
    /* color date settings colors */
    getDirectValueByUrl("dayColor", url + "/getdayColor");
    updateColorByUrl("dayColor", url + "/updatedayColor");
    getDirectValueByUrl("separatorColor", url + "/getseparatorColor");
    updateColorByUrl("separatorColor", url + "/updateseparatorColor");
    getDirectValueByUrl("monthColor", url + "/getmonthColor");
    updateColorByUrl("monthColor", url + "/updatemonthColor");

    /* temperature type */
    getRadioButtonsByUrl("tempType", url + "/gettemperatureSymbol");
    updateElementByUrl("tempType", "TempType", url + "/updateTempType", "click");

    /* temperature offset */
    getDropDownByUrl("correctionSelect", url + "/gettemperatureCorrection");
    updateElementByUrl("correctionSelect", "CorrectionSelect", url + "/updateCorrectionSelect", "change");

    /* temperature display type */
    getRadioButtonsByUrl("tempSettings", url + "/gettempDisplayType");
    updateElementByUrl("tempSettings", "TempDisplayType", url + "/updateTempDisplayType", "click");

    /* color temperature selection */
    getRadioButtonsByUrl("tempColorSettings", url + "/gettempColorSettings");
    updateElementByUrl("tempColorSettings", "TempColorSettings", url + "/updateTempColorSettings", "click");

    /* color temperature settings colors */
    getDirectValueByUrl("tempColor", url + "/gettempColor");
    updateColorByUrl("tempColor", url + "/updateTempColor");
    getDirectValueByUrl("degreeColor", url + "/getdegreeColor");
    updateColorByUrl("degreeColor", url + "/updateDegreeColor");
    getDirectValueByUrl("typeColor", url + "/gettypeColor");
    updateColorByUrl("typeColor", url + "/updateTypeColor");

    /* humidity settings */
    getRadioButtonsByUrl("humiSettings", url + "/gethumiDisplayType");
    updateElementByUrl("humiSettings", "HumiDisplayType", url + "/updateHumiDisplayType", "click");

    /* color humidity settings */
    getRadioButtonsByUrl("humiColorSettings", url + "/gethumiColorSettings");
    updateElementByUrl("humiColorSettings", "HumiColorSettings", url + "/updateHumiColorSettings", "click");

    /* color humidity settings colors */
    getDirectValueByUrl("humiColor", url + "/gethumiColor");
    updateColorByUrl("humiColor", url + "/updateHumiColor");
    getDirectValueByUrl("humiDecimalColor", url + "/gethumiDecimalColor");
    updateColorByUrl("humiDecimalColor", url + "/updateHumiDecimalColor");
    getDirectValueByUrl("symbolColor", url + "/getsymbolColor");
    updateColorByUrl("symbolColor", url + "/updateSymbolColor");

    /* timer options */
    getCheckboxValueByUrl("alarmCD", url + "/getalarmCD");
    updateCheckboxValueByUrl("alarmCD", "alarmCD", url + "/updatealarmCD");
    getCheckboxValueByUrl("colorchangeCD", url + "/getcolorchangeCD");
    updateCheckboxValueByUrl("colorchangeCD", "colorchangeCD", url + "/updatecolorchangeCD");

    /* timer color section */
    getDirectValueByUrl("colorCD", url + "/getcolorCD");
    updateColorByUrl("colorCD", url + "/updatecolorCD");

    /* scoreboard colors */
    getDirectValueByUrl("newscoreboardColorLeft", url + "/getscoreboardColorLeft");
    updateColorByUrl("newscoreboardColorLeft", url + "/updatescoreboardColorLeft");
    getDirectValueByUrl("newscoreboardColorRight", url + "/getscoreboardColorRight");
    updateColorByUrl("newscoreboardColorRight", url + "/updatescoreboardColorRight");

    /* spectrum analyzer options */
    getCheckboxValueByUrl("randomSpectrumMode", url + "/getrandomSpectrumMode");
    updateCheckboxValueByUrl("randomSpectrumMode", "randomSpectrumMode", url + "/updaterandomSpectrumMode");

    /* spectrum analyzer color selection */
    getDirectValueByUrl("spectrumBackground", url + "/getspectrumBackground");
    updateColorByUrl("spectrumBackground", url + "/updatespectrumBackground");
    getDirectValueByUrl("spectrumColor", url + "/getspectrumColor");
    updateColorByUrl("spectrumColor",  url + "/updatespectrumColor");

    /* spectrum analyzer background selection */
    getRadioButtonsByUrl("spectrumBackgroundSettings", url + "/getspectrumBackgroundSettings");
    updateElementByUrl("spectrumBackgroundSettings", "spectrumBackgroundSettings", url + "/updatespectrumBackgroundSettings", "click");

    /* spectrum analyzer segment selection */
    getRadioButtonsByUrl("spectrumColorSettings", url + "/getspectrumColorSettings");
    updateElementByUrl("spectrumColorSettings", "spectrumColorSettings", url + "/updatespectrumColorSettings", "click");

    /* scroll display frequency */
    getDropDownByUrl("scrollFrequency", url + "/getscrollFrequency");
    updateElementByUrl("scrollFrequency", "scrollFrequency", url + "/updatescrollFrequency", "change");

    /* scroll information */
    getCheckboxValueByUrl("scrollOptions1", url + "/getscrollOptions1");
    updateCheckboxValueByUrl("scrollOptions1", "scrollOptions1", url + "/updatescrollOptions1");
    getCheckboxValueByUrl("scrollOptions2", url + "/getscrollOptions2");
    updateCheckboxValueByUrl("scrollOptions2", "scrollOptions2", url + "/updatescrollOptions2");
    getCheckboxValueByUrl("scrollOptions3", url + "/getscrollOptions3");
    updateCheckboxValueByUrl("scrollOptions3", "scrollOptions3", url + "/updatescrollOptions3");
    getCheckboxValueByUrl("scrollOptions4", url + "/getscrollOptions4");
    updateCheckboxValueByUrl("scrollOptions4", "scrollOptions4", url + "/updatescrollOptions4");
    getCheckboxValueByUrl("scrollOptions5", url + "/getscrollOptions5");
    updateCheckboxValueByUrl("scrollOptions5", "scrollOptions5", url + "/updatescrollOptions5");
    getCheckboxValueByUrl("scrollOptions6", url + "/getscrollOptions6");
    updateCheckboxValueByUrl("scrollOptions6", "scrollOptions6", url + "/updatescrollOptions6");
    getCheckboxValueByUrl("scrollOptions7", url + "/getscrollOptions7");
    updateCheckboxValueByUrl("scrollOptions7", "scrollOptions7", url + "/updatescrollOptions7");
    getCheckboxValueByUrl("scrollOptions8", url + "/getscrollOptions8");
    updateCheckboxValueByUrl("scrollOptions8", "scrollOptions8", url + "/updatescrollOptions8");
    getDirectValueByUrl("scrollText", url + "/getscrollText");
    updateTextFieldByUrl("scrollText", "scrollText", url + "/updatescrollText");

    /* scroll options */
    getCheckboxValueByUrl("scrollOverride", url + "/getscrollOverride");
    updateCheckboxValueByUrl("scrollOverride", "scrollText", url + "/updatescrollOverride");

    /* scroll color options */
    getDirectValueByUrl("scrollColor", url + "/getscrollColor");
    updateColorByUrl("scrollColor", url + "/updatescrollColor");
    getRadioButtonsByUrl("scrollColorSettings", url + "/getscrollColorSettings");
    updateElementByUrl("scrollColorSettings", "scrollColorSettings", url + "/updatescrollColorSettings", "click");

    /* preset buttons */
    document.querySelectorAll(`[name^='setpreset']`).forEach((element) => {
        element.addEventListener("click", async function(event) {
            if (event.target.name === "setpreset1") {
                await fetch(`${url}/setpreset1`, {method: 'POST'});
            } else {
                await fetch(`${url}/setpreset2`, {method: 'POST'});
            }
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



    /*function setDateTime() {
        //tempyear, tempmonth, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
            var y = moment().format('YYYY');
            var n = moment().format('MM');
            var d = moment().format('DD');
            var h = moment().format('HH');
            var m = moment().format('mm');
            var s = moment().format('ss');
            var datetime = { year: y, month: n, day: d, hour: h, min: m, sec: s };
            console.log(datetime);
            $.post("/setdate", datetime);
        }*/
});

function pad(number) {
    return ( number < 10 ? '0' : '' ) + number
}


/* get set form elements */
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

async function getDirectValueByUrl(selectorName, getURL) {
    let response = await fetch(getURL);
    if (response.ok) {
        document.querySelector(`[name='${selectorName}']`).value = await response.text();
    }
}

async function getDropDownByUrl(selectorName, getURL) {
    let response = await fetch(getURL);
    let value = await response.text();
    if (response.ok) {
        let select = document.querySelector(`select[name='${selectorName}']`);
        [...select.options].forEach((option, index) => {
            if(value === option.value) {
                select.selectedIndex = index;
            }
        });
    }
}

async function getRadioButtonsByUrl(selectorName, getURL) {
    let response = await fetch(getURL);
    let value = await response.text();
    if (response.ok) {
        document.querySelectorAll(`input[name='${selectorName}']`).forEach((element) => {
            if (element.value === value) {
                element.checked = true;
            }
        });
    }
}

/* utility function */
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

