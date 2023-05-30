
There is a working version of this code available but this is only a work in progress.

It is a testing ground for features and stuff, it works sometimes and doesn't need a special partition.

Stuff it does that is new (and still not ready):
You can upload and delete RTTTL files.
You can make and delete custom schedules, not just a countdown timer.
It should work with Wifi better
It uses FilesFS instead of SPIFFS and writes settings to there as a json file (and does write balancing sort of).
It has a real FFT.
Ability to set an alarm that can chime on the hour and even gong the hours.
Ability to set alarms that don't go off during the night.
Every timezone on the planet.
It has the ability to compile without any extra hardware modules, no DHT or RTL or SoundDetectoror light sensor, no problem.
it can deal with different light density in the segments (not more or less segments).
It has a massive debug page.
It will only show you web controls that you have compiled.
Online weather.
Hour minute dividers for 24hr clocks.
When it first gets plugged in, it defaults to scrolling the IP (in case people's routers don't do MDNS). 
The ability to set the timer countdown song, and play any song on the device.
Deals with the millis() 49 day issue.
All json settings to and from the webpages.
Losts of problems.
Hardly enough comments.
VERY badly writen code.