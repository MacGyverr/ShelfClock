#include "Arduino.h"
#include "WString.h"
#include <FastLED.h>



struct WeatherAPI {
    char latitude[15];
    char longitude[15];
    char apikey[50];
};



void loadWebPageHandlers();
void loadSetupSettings();
void fakeClock(int);
void printLocalTime();
void initGreenMatrix();
void raininit();
void allBlank();
void GetBrightnessLevel();
void checkSleepTimer();
void displayTimeMode();
void displayCountdownMode();
void displayTemperatureMode();
void displayScoreboardMode();
void displayStopwatchMode();
void displayLightshowMode(); 
void displayDateMode();
void displayHumidityMode();
void displayScrollMode();
void ShelfDownLights();
void displayRealtimeMode();
void scroll(String);
void displayNumber(uint16_t number, byte segment, CRGB color);
void BlinkDots();
void happyNewYear();
void endCountdown();
void Chase();
void SpectrumAnalyzer();
void Twinkles();
void Rainbow();
void GreenMatrix();
void blueRain();
void Fire2021();
void Snake();
void Cylon();
CRGB colorWheel(int);
CRGB colorWheel2(int);
void updateMatrix();
void changeMatrixpattern();
void updaterain();
void Task1code(void * parameter);
void getRemoteWeather();
void playRTTTLsong(String goforit, int chimeNumber);
void writeFile(fs::FS &fs, const char * path, const char * message);
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void deleteFile(fs::FS &fs, const char * path);
void processSchedules(bool alarmType);
void createSchedulesArray();
void getListOfSongs();
void saveclockSettings(String fileType);
void getclockSettings(String fileType);