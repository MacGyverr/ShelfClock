This is a clock that is a remix from an idea I saw on Youtube once. The code here resembles nothing of what the idea was based on. (at https://github.com/DIY-Machines/DigitalClockSmartShelving)
A lot of feature ideas came from https://github.com/helpquick/7-Segment-WiFi-Clock

It has lots of features, (most even work correctly) those include:
Clock Mode
Date Mode
Temperature Mode
Humidity Mode
Various Lightshow Modes
Various Music Visualizer Modes
A Scoreboard
A Countdown Timer
A Countup Timer
New Years Mode
Text Scroller
Web Interface

It uses the following hardware:
LOLIN32 featherboard (ESP-WROOM-32)
DS3231 AT24C32 IIC RTC Clock Module
DHT11 Temperature Sensor
Photosensitive Resistance Sensor
SparkFun Sound Detector
BTF-LIGHTING WS2812B RGB 5050SMD (271 of them)

I made tons of settings, and the ability to save them as presets in NVS.

Because it uses soooo much NVS space for Preferences and AutoConnect I had to create a newer partition scheme for the ESP32 that moves the NVS to just before SPIFFS and then takes away some space (this was easier then trying to get it to boot the App from a different memory location (other than 0x10000) after extending the original NVS size. I included the cvs and text for it in parttition-info.txt

Note:
I don't write C code nor do I write Javascript or CSS for that matter, but if there's something I want to do I can usually kludge something together by using the infinite examples made by much smarter people out there.
This is not my code as much as it is me researching and mixing 50 examples together, none of it is written efficiently or well. I use global variables everywhere, I don't release memory, or even initialize variables correctly. I know enough to know that I don't do any of it correctly.
I just keep hacking on it until it works, and just barely most times.
If you like this code and actually know what you are doing, please feel free to remake it the right way.
If you can show me the places I really made bad mistakes feel free to show me the correct way and I'll try to fix it.

I will try to add the list of libraries I use to compile this mess with soon.
I included the modified STLs I used to make mine, basically smaller versions of DIY-Machines and more of them, so that they can be printed on my smaller single-color 3D printer.
The 3D versions I made using TinkerCAD, here they are: https://www.tinkercad.com/things/fyxXr27PCph

Thanks to the work by rjackr, you should be able to use between 6 and 9 LEDs per segment without any trouble.
https://github.com/rjackr/ShelfClock/commit/4604793791f351333186262200bfa97d9bf8e52f

Links to places I know I used code examples from:
https://github.com/DIY-Machines/DigitalClockSmartShelving
https://github.com/helpquick/7-Segment-WiFi-Clock
https://github.com/FastLED/FastLED/wiki/Overview
https://github.com/mattncsu/FastLED_examples
http://84.107.155.75/forum/index.php?action=view&id=300  (all the RTTL songs)


I uploaded videos of it here.
https://www.youtube.com/watch?v=FABxgoq68Fs&list=PLkV6jp60iXvjmrqkrMTHSjlMShLybMzkE
