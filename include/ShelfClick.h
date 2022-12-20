#include "Arduino.h";
#include "WString.h";
#include <FastLED.h>;

struct Temperature {
    bool outdoor_enable; // = true;  
};

struct Humidity {
    bool outdoor_enable; // = true
};

struct WeatherAPI {
    char latitude[15];// = 47.698560;
    char longitude[15];// = -122.101970;
    char apikey[50];// = "680e541ac65981bd1835f7728a185c47";
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