# ShelfClock v2.0 

Welcome to the ShelfClock project! A custom-built clock using an ESP32 microcontroller, enhanced by a purpose-designed PCB. ShelfClock v2.0 introduces several hardware and software upgrades over the previous version, focusing on expanding functionality and improving ease of use.

This is an update for the clock I made for my daughter that was a remix from an idea I saw on YouTube once: [DIY Machines](https://www.youtube.com/c/DIYMachines/videos).

My code here resembles nothing of what the idea was based on.

Original code at: [DIY-Machines/DigitalClockSmartShelving](https://github.com/DIY-Machines/DigitalClockSmartShelving)

A lot of feature ideas came from: [helpquick/7-Segment-WiFi-Clock](https://github.com/helpquick/7-Segment-WiFi-Clock)

---
***⚠️ I have done my best to maximize the circuit design to handle the current draw of 273 LEDs, but it's still high (12A high), so mistakes or shortcuts you make can still create a hazard.
I'm also not an electrical engineer, so use anything here at your own risk!***
---

## Features

It has lots of features, those include:

- **Clock Mode**
- **Date Mode**
- **Temperature Mode**
- **Humidity Mode**
- **Various Lightshow Modes** (like Snake and Matrix displays)
- **Various Music Visualizer Modes** (including FFT)
- **A Scoreboard**
- **A Countdown Timer**
- **A Count-up Timer**
- **New Year's Mode**
- **Text Scroller**
- **Web Interface**
- **OTA Updates**
- Can play RTTL files as alarms

## Hardware Used

It uses the following hardware:

- **ESP32 38-pin ESP-D Devkit (ESP-WROOM-32)** (microcontroller)
- **DS3231 AT24C32 IIC RTC Clock Module** (RTC module provides accurate timekeeping with battery backup)
- **DHT11 Temperature Sensor**
- **Photosensitive Resistance Sensor LDR** (Automatically adjusts display brightness based on ambient light levels)
- **INMP441 Sound Detector**
- **BTF-LIGHTING WS2812B RGB 5050SMD** (mine uses 273 of them, but can be changed; You can use whatever 5V LEDs you want, my test clock uses only 162 XGB1338 LEDs)
- I used a **5V Computer Buzzer** as the speaker.
- **NPN Transistor 2N3904** with either a 1k resistor as a simple 5V amplifier.
- **1N5817 Diode**

I made tons of settings and the ability to save them as presets in JSON files.

> **Note:** I don't write C code nor do I write JavaScript or CSS for that matter, but if there's something I want to do I can usually kludge something together by using the infinite examples made by much smarter people out there. This is not my code as much as it is me researching and mixing 50 examples together; none of it is written efficiently or well. I use global variables everywhere, I don't release memory, or even initialize variables correctly. I know enough to know that I don't do any of it correctly. I just keep hacking on it until it works, and just barely most times. If you like this code and actually know what you are doing, please feel free to remake it the right way. If you can show me the places I really made bad mistakes, feel free to show me the correct way and I'll try to fix it. It's close to 7000 lines long with all the web page code included, so cut an amateur with no training some slack. :)

I included the modified STLs I used to make mine, basically smaller versions of DIY-Machines' version and more of them, and I added separate diffusers instead of his multi-filament versions, so that they can be printed on my smaller single-color 3D printer (210x210mm). The 3D versions I made using TinkerCAD are here: [TinkerCAD Design](https://www.tinkercad.com/things/1DAPIPNRKK3-shelfclock-v20)

My shelf sleeves are printed in bamboo PLA; this stuff clogs continuously, so if you can live without it being real wood, it will save you a bunch of trouble.

The LED diffusers are printed in white PLA.

The scaffolding is printed in whatever PLA I had laying around.

### Print the following number of each:

- **1** - `Placement_template.stl` (will fit them in a 1055mm x 320mm space)
- **37** - `Lid-diffuser.STL`
- **37** - `LED Mount.STL`
- **36** - `Scaffolding.STL`
- **1** - `Scaffolding - Arduino Housing.STL`
- **10** - `Sleeve1-Bottom & Sides.STL`
- **7** - `Sleeve2-Top with Light.STL`
- **12** - `Sleeve3-Vertical Middle.STL`
- **7** - `Sleeve4-Middle with Light.STL`
- **1** - `Sleeve5-For Arduino.STL` or `Sleeve5-For Arduino with Mic-hole.STL`
- **1** - `Back-Cover.STL`
- **1** - `Back-Cover-plug.STL`
- **1** - `Corner Stand-off Legs.STL`

Printing this will take forever, something like 300+ hours, so make sure it is really what you want to spend the next month doing.

## Assembly Instructions

1. **Print all of the pieces.**
2. **Cut the LEDs into 37 strips of 7 LEDs, and 14 single LEDs**
3. **Tin all 6 leads on each strip** (this will make your life a lot easier later).
4. **Plan how you are going to mount the scaffolding elements to a board of some sort** (it should fit on a 1055mm x 320mm board, with no lip).
5. **Mount the special Arduino scaffolding, trace the opening, then remove it and remove the material to the back, mount the wire cover.**
6. **Mount the stand-off feet.**
7. **Mount half of the French cleat used to hang it on the wall.**
8. **Mount the scaffolding elements to the board with the drilling template. Don't forget to keep the indent for the drop lights pointing down.**
9. **Glue the LED mounting strip to the scaffolding with hot glue or super glue.**
10. **Mount the LED strip to the mounting strip with hot glue or super glue.**
11. **Wire the LED strips to each other** as outlined in the `LEDs.png` in the `diagrams.zip` file, making note to connect the 5V to 5V and GND to GND, and keep the arrow on the strip flowing in the right direction when connecting the Data from strip to strip.
12. **Add extra runs of 5V and GND to each segment** to stop the voltage drop (voltage drop will present as a dimming the further out you go until the last LED is yellow when everything is supposed to be white).
13a. **Use the attached `shelfclockv2-gerbers.zip` to have a custom PCB created** to make assembly easier (it should cost less than $5 to have it made). Solder in the 1N5817 diode, the 470 and 1k resistors, and the 2N3904 NPN transistor. Solder the various sensor modules and ESP32 to the circuit board. I use 90-degree 3-pin terminals to attach the LED strips to the circuit board from both ends of the LED strip.
13b. **Or if you aren't using the custom circuit board**, try and assemble the circuit as best you can to fit in the top-right most segment. I did it that way the first time; you can too, or split them between segments if you have to, but don't forget to route the wires correctly through the wire paths so your sleeve will seat all the way.
14. **Clamp your powercord into eyelets or spades and bolt them to the board with 3mm bolts or solder the wires directly to the terminals (it will take a lot of heat given the size of the copper traces)**

The distance the LED diffusers (printed at 100% infill) are away from the LEDs mostly fixes the hotspots from the LEDs. The trade-off is that you will get bleed-through on the shelves, so if you look at the image with 9 smaller images in it, it shows my solution to the bleed-through problem: black spray paint (but only about an inch down).

I made a jig out of cardboard so that I could paint 37 of them without too much trouble. I cut out a tight opening so that a shelf sleeve can just go through out of some cardboard, and then I put blue painters tape over the hole (and over the edge of the sleeve) and with a very sharp razor-knife I remove the inside of the tape leaving just the edge, then I spray paint them (making sure to cover the droplight holes too to prevent overspray from the inside out).

I was able to get about 3 or 4 uses of each tape cover before it started to transfer to the sleeve and I had to cut out another one. I tried a bunch of stuff first, like 3M metal duct tape, and in the end matte black spray paint was the easiest.

Programming the ESP32 is outside the scope of these instructions, but you can figure it out. I have included the custom CSV for the partition and the `platformio.ini`, just do some research.

I used VSCode / PlatformIO, and the libraries and versions listed in the `platformio.ini` file.

Don't forget to upload the `data` folder to the ESP32 via "Build Filesystem Image" and then "Upload Filesystem Image" via PlatformIO.

You should see the AutoConnect first run as its own Ad-Hoc device (`esp32ap`) in your phone's Wi-Fi connections. You will use this to set up the initial wireless settings (joining it to your Wi-Fi).

If your router is nice and allows MDNS, you should be able to reach the ESP32 at [http://shelfclock](http://shelfclock) after the AutoConnect setup.

The clock will attempt to scroll the IP twice on bootup.


### Circuit Board Images

![Circuit Board Front](https://github.com/MacGyverr/ShelfClock/blob/newer-vscode-wip/diagrams/circuitboard-front.PNG)

![Circuit Board Back](https://github.com/MacGyverr/ShelfClock/blob/newer-vscode-wip/diagrams/circuitboard-back.PNG)

![Circuit Board Assembled](https://github.com/MacGyverr/ShelfClock/blob/newer-vscode-wip/diagrams/20241115_173534.jpg)


## Links to Code Examples Used

Links to places I know I used code examples from:

- [DIY-Machines/DigitalClockSmartShelving](https://github.com/DIY-Machines/DigitalClockSmartShelving)
- [helpquick/7-Segment-WiFi-Clock](https://github.com/helpquick/7-Segment-WiFi-Clock)
- [FastLED Overview](https://github.com/FastLED/FastLED/wiki/Overview)
- [mattncsu/FastLED_examples](https://github.com/mattncsu/FastLED_examples)
- [ryanrk](https://github.com/ryanrk/ShelfClock)
- [rjackr](https://github.com/rjackr/ShelfClock)
- [All the RTTL songs](http://84.107.155.75/forum/index.php?action=view&id=300)

Its code is going to confuse people because I have fake segments for the scrolling and I basically "remap" the LEDs to accomplish different views and display modes. (Notice that there are really only 4 complete segments; to make it display 7 full segments would take about 100 LEDs more and make it look weird.)

I uploaded videos of it here:

- [YouTube Playlist](https://www.youtube.com/watch?v=FABxgoq68Fs&list=PLkV6jp60iXvjmrqkrMTHSjlMShLybMzkE)

## Key Differences from Previous Version

### Custom PCB

The latest version features a custom PCB layout, reducing the complexity of wiring and improving reliability, as well as a 1N5817 diode to protect against backflow. It has locations for all optional components: Buzzer, RTC, LDR, DHT11, and INMP441 sensor.

### Code Overhaul

The new firmware in the `newer-vscode-wip` branch is optimized for better configuration options and has the following:

- **FFT with 8 modes**
- **Ability to use an external website for outside temps via an API key**
- **Ability to create and delete alarms (schedules)**
- **Ability to upload and remove RTTL-based music files**
- **Ability to do OTA updates**
- **Improved debug page**
- **Will display info on bootup**
- **Will display the IP address on every boot for those without MDNS**
- **You can use from 1 to 10 LEDs per segment (default is 7)**
- **Every timezone on the planet**
- **Doesn't need a large custom partition for settings**
- **It uses LittleFS instead of SPIFFS and writes settings there as a JSON file (and does write balancing, sort of)**
- **It has the ability to compile without any extra hardware modules** (no DHT or RTL or INMP441 or light sensor, no problem). Just set what you have via `#DEFINE`.
- **Deals with the `millis()` 49-day issue**
- **All JSON settings to and from the webpages**

The custom PCB includes labeled sections for each component:

- **DHT11 Sensor**: For temperature and humidity.
- **DS3231 RTC Module**: For precise timekeeping.
- **ESP32 38-pin ESP-D Devkit**: Serving as the brain of the clock.
- **LDR for Ambient Light Detection**: Controls display brightness.
- **Buzzer Circuit with 2N3904 Transistorc and 1k resistor**: Enables alarms and notifications.
- **INMP441 Microphone**: A slot is available for sound and music detection.
- **470 Ohm resistor**: Inline with the LED output from the ESP32. Not needed in every use, just if you have flickering, bypass it if it casues corruption.

## Contributions

Contributions to the ShelfClock project are welcome! Please feel free to fork the repository, submit pull requests, or raise issues with ideas for improvement.
