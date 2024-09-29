#define FASTLED_ESP32_FLASH_LOCK 1FS
//#include <MegunoLink.h>
#include <FastLED.h>
#include <WiFi.h>
#include "WebServer.h"
#include <FS.h>     
#include <HTTPUpdateServer.h>
#include <Preferences.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <AutoConnect.h>
#include <ArduinoJson.h>
#include <MultiMap.h>
#include "../include/ShelfClick.h"
//#include <LittleFS.h>       // https://github.com/espressif/arduino-esp32/tree/master/libraries/LittleFS
//#include <FS.h>    

#define HAS_RTC    true
#define HAS_DHT    true
#define HAS_SOUNDDETECTOR    true
#define HAS_BUZZER    true
#define HAS_PHOTOSENSOR    true
#define HAS_ONLINEWEATHER    false

#if HAS_RTC
  #include "RTClib.h"
#endif


#if HAS_SOUNDDETECTOR
  #include <arduinoFFT.h>				// Don't forget to change CPU Frequency to 240MHz in Arduino board settings
  #include <driver/i2s.h>
#endif

#if HAS_BUZZER
//  #include "../lib/Sounds/Sounds.h"
  #include <NonBlockingRtttl.h> 
#endif

#define USE_LITTLEFS    true
#define USE_SPIFFS      false

#if USE_LITTLEFS
  // Use LittleFS
  #include "FS.h"
  // The library has been merged into esp32 core from release 1.0.6
  #include <LittleFS.h>       // https://github.com/espressif/arduino-esp32/tree/master/libraries/LittleFS
  FS* filesystem =      &LittleFS;
  #define FileFS        LittleFS
  #define FS_Name       "LittleFS"
#elif USE_SPIFFS
  #include <SPIFFS.h>
  FS* filesystem =      &SPIFFS;
  #define FileFS        SPIFFS
  #define FS_Name       "SPIFFS"
#else
  // +Use FFat
  #include <FFat.h>
  FS* filesystem =      &FFat;
  #define FileFS        FFat
  #define FS_Name       "FFat"
#endif


#define FORMAT_SPIFFS_IF_FAILED false

#define LED_TYPE  WS2812B
#define COLOR_ORDER GRB
#define SEGMENTS_PER_NUMBER 7 // this can never change unless you redesign all display routines
#define NUMBER_OF_DIGITS 7    // 7 = 4 real + 3 fake,  this should be always 7 unless you redesign all display routines
#define SPECTRUM_PIXELS 37    // 7 digits = 37 (5 unshared segments for every digit (7) and 2 more on the last from the side)
#define LED_PIN 2             // led control pin
#define MILLI_AMPS 2400 
#define LEDS_PER_SEGMENT 7    // can be 1 to 10 LEDS per segment (4 for test display, 7 for full)
#define LEDS_PER_DIGIT (LEDS_PER_SEGMENT * SEGMENTS_PER_NUMBER)
#define FAKE_NUM_LEDS (NUMBER_OF_DIGITS * LEDS_PER_DIGIT)
#define PHOTO_SAMPLES 10      //number of samples to take from the photoresister
#define PHOTO_SIZE 5
#define SEGMENTS_LEDS (SPECTRUM_PIXELS * LEDS_PER_SEGMENT)  // Number leds in all segments
#define SPOT_LEDS (NUMBER_OF_DIGITS * 2)        // Number of Spotlight leds
#define NUM_LEDS  (SEGMENTS_LEDS + SPOT_LEDS)   // Number of all leds
#define WiFi_MAX_RETRIES 100
#define WiFi_MAX_RETRY_DURATION 600000 // 10 minutes in milliseconds

#if HAS_DHT
  #include "DHT.h"
  #define DHTTYPE DHT11         // DHT 11 tempsensor
  #define DHT_PIN 33            // temp sensor pin
#endif
  #if HAS_SOUNDDETECTOR
//  #define SOUNDDETECTOR_ENVELOPE_IN_PIN 34    // Use 34 for envelope pin input
//  #define SOUNDDETECTOR_AUDIO_GATE_PIN 35     // for sound gate input trigger
//  #define AUDIO_IN_PIN    32    // Analog audio in (audio pin)
  #define SOUNDDETECTOR_I2S_WS 15
#define SOUNDDETECTOR_I2S_SD 32
#define SOUNDDETECTOR_I2S_SCK 14
#define SOUNDDETECTOR_I2S_PORT I2S_NUM_0
#define SOUNDDETECTOR_SAMPLING_FREQ 8000    // Lower sampling rate
#define SOUNDDETECTOR_BITS_PER_SAMPLE 16
#define SOUNDDETECTOR_SAMPLES 128           // Smaller FFT size
  //#define SOUNDDETECTOR_SAMPLES         256          // Must be a power of 2
 // #define SOUNDDETECTOR_SAMPLING_FREQ   40000         // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
  #define SOUNDDETECTOR_BANDS_WIDTH       8            // To change this, you will need to change the bunch of if statements describing the mapping from bins to bands
//  #define NOISE           500           // Used as a crude noise filter, values below this are ignored
  #define SOUNDDETECTOR_BANDS_HEIGHT            (LEDS_PER_SEGMENT * 2)                // Don't allow the bars to go offscreen
  const int ANALYZER_SIZE = SOUNDDETECTOR_BANDS_WIDTH * LEDS_PER_SEGMENT * 2;
  #endif
#if HAS_BUZZER
  #define BUZZER_PIN 16         // peizo speaker
//  #define BUZZER_PIN 25         // peizo speaker
#endif
#if HAS_PHOTOSENSOR
  #define PHOTORESISTER_PIN 36  // select the analog input pin for the photoresistor
#endif
#define digit0 seg(0), seg(1), seg(2), seg(3), seg(4), seg(5), seg(6)
#define fdigit1 seg(2), seg(7), seg(10), seg(15), seg(8), seg(3), seg(9)
#define digit2 seg(10), seg(11), seg(12), seg(13), seg(14), seg(15), seg(16)
#define fdigit3 seg(12), seg(17), seg(20), seg(25), seg(18), seg(13), seg(19)
#define digit4 seg(20), seg(21), seg(22), seg(23), seg(24), seg(25), seg(26)
#define fdigit5 seg(22), seg(27), seg(30), seg(35), seg(28), seg(23), seg(29)
#define digit6 seg(30), seg(31), seg(32), seg(33), seg(34), seg(35), seg(36)


#if LEDS_PER_SEGMENT == 1
 #define seg(n) n*LEDS_PER_SEGMENT
 #define nseg(n) n*LEDS_PER_SEGMENT
#elif LEDS_PER_SEGMENT == 2
 #define seg(n) n*LEDS_PER_SEGMENT, n*LEDS_PER_SEGMENT+1
 #define nseg(n) n*LEDS_PER_SEGMENT+1, n*LEDS_PER_SEGMENT
#elif LEDS_PER_SEGMENT == 3
 #define seg(n) n*LEDS_PER_SEGMENT, n*LEDS_PER_SEGMENT+1, n*LEDS_PER_SEGMENT+2
 #define nseg(n) n*LEDS_PER_SEGMENT+2, n*LEDS_PER_SEGMENT+1, n*LEDS_PER_SEGMENT
#elif LEDS_PER_SEGMENT == 4
 #define seg(n) n*LEDS_PER_SEGMENT, n*LEDS_PER_SEGMENT+1, n*LEDS_PER_SEGMENT+2, n*LEDS_PER_SEGMENT+3
 #define nseg(n) n*LEDS_PER_SEGMENT+3, n*LEDS_PER_SEGMENT+2, n*LEDS_PER_SEGMENT+1, n*LEDS_PER_SEGMENT
#elif LEDS_PER_SEGMENT == 5
 #define seg(n) n*LEDS_PER_SEGMENT, n*LEDS_PER_SEGMENT+1, n*LEDS_PER_SEGMENT+2, n*LEDS_PER_SEGMENT+3, n*LEDS_PER_SEGMENT+4
 #define nseg(n) n*LEDS_PER_SEGMENT+4, n*LEDS_PER_SEGMENT+3, n*LEDS_PER_SEGMENT+2, n*LEDS_PER_SEGMENT+1, n*LEDS_PER_SEGMENT
#elif LEDS_PER_SEGMENT == 6
 #define seg(n) n*LEDS_PER_SEGMENT, n*LEDS_PER_SEGMENT+1, n*LEDS_PER_SEGMENT+2, n*LEDS_PER_SEGMENT+3, n*LEDS_PER_SEGMENT+4, n*LEDS_PER_SEGMENT+5
 #define nseg(n) n*LEDS_PER_SEGMENT+5, n*LEDS_PER_SEGMENT+4, n*LEDS_PER_SEGMENT+3, n*LEDS_PER_SEGMENT+2, n*LEDS_PER_SEGMENT+1, n*LEDS_PER_SEGMENT
#elif LEDS_PER_SEGMENT == 7
 #define seg(n) n*LEDS_PER_SEGMENT, n*LEDS_PER_SEGMENT+1, n*LEDS_PER_SEGMENT+2, n*LEDS_PER_SEGMENT+3, n*LEDS_PER_SEGMENT+4, n*LEDS_PER_SEGMENT+5, n*LEDS_PER_SEGMENT+6
 #define nseg(n) n*LEDS_PER_SEGMENT+6, n*LEDS_PER_SEGMENT+5, n*LEDS_PER_SEGMENT+4, n*LEDS_PER_SEGMENT+3, n*LEDS_PER_SEGMENT+2, n*LEDS_PER_SEGMENT+1, n*LEDS_PER_SEGMENT
#elif LEDS_PER_SEGMENT == 8
 #define seg(n) n*LEDS_PER_SEGMENT, n*LEDS_PER_SEGMENT+1, n*LEDS_PER_SEGMENT+2, n*LEDS_PER_SEGMENT+3, n*LEDS_PER_SEGMENT+4, n*LEDS_PER_SEGMENT+5, n*LEDS_PER_SEGMENT+6, n*LEDS_PER_SEGMENT+7
 #define nseg(n) n*LEDS_PER_SEGMENT+7, n*LEDS_PER_SEGMENT+6, n*LEDS_PER_SEGMENT+5, n*LEDS_PER_SEGMENT+4, n*LEDS_PER_SEGMENT+3, n*LEDS_PER_SEGMENT+2, n*LEDS_PER_SEGMENT+1, n*LEDS_PER_SEGMENT
#elif LEDS_PER_SEGMENT == 9
 #define seg(n) n*LEDS_PER_SEGMENT, n*LEDS_PER_SEGMENT+1, n*LEDS_PER_SEGMENT+2, n*LEDS_PER_SEGMENT+3, n*LEDS_PER_SEGMENT+4, n*LEDS_PER_SEGMENT+5, n*LEDS_PER_SEGMENT+6, n*LEDS_PER_SEGMENT+7, n*LEDS_PER_SEGMENT+8
 #define nseg(n) n*LEDS_PER_SEGMENT+8, n*LEDS_PER_SEGMENT+7, n*LEDS_PER_SEGMENT+6, n*LEDS_PER_SEGMENT+5, n*LEDS_PER_SEGMENT+4, n*LEDS_PER_SEGMENT+3, n*LEDS_PER_SEGMENT+2, n*LEDS_PER_SEGMENT+1, n*LEDS_PER_SEGMENT
#elif LEDS_PER_SEGMENT == 10
 #define seg(n) n*LEDS_PER_SEGMENT, n*LEDS_PER_SEGMENT+1, n*LEDS_PER_SEGMENT+2, n*LEDS_PER_SEGMENT+3, n*LEDS_PER_SEGMENT+4, n*LEDS_PER_SEGMENT+5, n*LEDS_PER_SEGMENT+6, n*LEDS_PER_SEGMENT+7, n*LEDS_PER_SEGMENT+8, n*LEDS_PER_SEGMENT+9
 #define nseg(n) n*LEDS_PER_SEGMENT+9, n*LEDS_PER_SEGMENT+8, n*LEDS_PER_SEGMENT+7, n*LEDS_PER_SEGMENT+6, n*LEDS_PER_SEGMENT+5, n*LEDS_PER_SEGMENT+4, n*LEDS_PER_SEGMENT+3, n*LEDS_PER_SEGMENT+2, n*LEDS_PER_SEGMENT+1, n*LEDS_PER_SEGMENT
#else
 #error "Not supported Leds per segment. You need to add definition of seg(n) with needed number of elements according to formula above"
#endif



#if HAS_SOUNDDETECTOR
  /*
  b     b     b     b     b     b     b     b
  a     a     a     a     a     a     a     a
  r     r     r     r     r     r     r     r
  0     1     2     3     4     5     6     7
    34    28    24    18    14     8     4
  →→→ →→→  →→→ →→→ →→→ →→→  →→→
  ↑     ↓     ↑     ↓     ↑     ↓     ↑     ↓
  ↑33   ↓35   ↑23   ↓25   ↑13   ↓15   ↑3    ↓5
  ↑     ↓     ↑     ↓     ↑     ↓     ↑     ↓
  ↑ 36  ↓ 29  ↑ 26  ↓ 19  ↑ 16  ↓   9 ↑  6  ↓
  ←←← ←←←  ←←← ←←← ←←← ←←←  ←←←
  ↑     ↓     ↑     ↓     ↑     ↓     ↑     ↓
  ↑32   ↓30   ↑22   ↓20   ↑12   ↓10   ↑2    ↓0
  ↑     ↓     ↑     ↓     ↑     ↓     ↑     ↓
  ↑ 31  ↓ 27  ↑ 21  ↓ 17  ↑ 11  ↓  7  ↑  1  ↓
  ←←← ←←←  ←←← ←←← ←←← ←←←  ←←←
  */
  #define bar0 seg(32), seg(33)
  #define bar1 nseg(30), nseg(35)
  #define bar2 seg(22), seg(23)
  #define bar3 nseg(20), nseg(25)
  #define bar4 seg(12), seg(13)
  #define bar5 nseg(10), nseg(15)
  #define bar6 seg(2), seg(3)
  #define bar7 nseg(0), nseg(5)
  const uint16_t ANALYZER[(NUMBER_OF_DIGITS+1)*LEDS_PER_SEGMENT*2] = {bar0, bar1, bar2, bar3, bar4, bar5, bar6, bar7};


  // Sampling and FFT stuff
  unsigned int sampling_period_us;
  byte peak[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};              // The length of these arrays must be >= SOUNDDETECTOR_BANDS_WIDTH
  int oldBarHeights[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  int SOUNDDETECTOR_bandValues[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
  int SOUNDDETECTOR_noiseThresholds[8] = {800, 300, 200, 200, 200, 200, 200, 200}; // Example thresholds
  double vReal[SOUNDDETECTOR_SAMPLES];
  double vImag[SOUNDDETECTOR_SAMPLES];
  unsigned long newTime;
  long SOUNDDETECTOR_Amplitude = 1000;
  arduinoFFT FFT = arduinoFFT(vReal, vImag, SOUNDDETECTOR_SAMPLES, SOUNDDETECTOR_SAMPLING_FREQ);
  DEFINE_GRADIENT_PALETTE( purple_gp ) {
    0,   0, 212, 255,   //blue
  255, 179,   0, 255 }; //purple
  DEFINE_GRADIENT_PALETTE( outrun_gp ) {
    0, 141,   0, 100,   //purple
  127, 255, 192,   0,   //yellow
  255,   0,   5, 255 };  //blue
  DEFINE_GRADIENT_PALETTE( greenblue_gp ) {
    0,   0, 255,  60,   //green
  64,   0, 236, 255,   //cyan
  128,   0,   5, 255,   //blue
  192,   0, 236, 255,   //cyan
  255,   0, 255,  60 }; //green
  DEFINE_GRADIENT_PALETTE( redyellow_gp ) {
    0,   200, 200,  200,   //white
  64,   255, 218,    0,   //yellow
  128,   231,   0,    0,   //red
  192,   255, 218,    0,   //yellow
  255,   200, 200,  200 }; //white
  DEFINE_GRADIENT_PALETTE( fire_gp ) {
    0,   255,   0,    0,   //red
  64,   255, 64, 0,   //orange
  128,   255, 128,    0,   //yellow
  192,   255, 192,    0,   //light yellow
  255,   255, 255,  255 }; //white
  CRGBPalette16 purplePal = purple_gp;
  CRGBPalette16 outrunPal = outrun_gp;
  CRGBPalette16 greenbluePal = greenblue_gp;
  CRGBPalette16 heatPal = redyellow_gp;
  CRGBPalette16 heaterPal = fire_gp;
  uint8_t colorTimer = 0;
  int buttonPushCounter = 0;
  int SOUNDDETECTOR_averageAudioInput = 0;
  int SOUNDDETECTOR_decay = 0; // HOW MANY MS BEFORE ONE LIGHT DECAY
  int SOUNDDETECTOR_decay_check = 0;
  int SOUNDDETECTOR_pre_react = 0; // NEW SPIKE CONVERSION
  int SOUNDDETECTOR_react = 0; // NUMBER OF LEDs BEING LIT
  int SOUNDDETECTOR_post_react = 0; // OLD SPIKE CONVERSION
#endif

String softwareVersion = "version-3.0.1-alpha";
const char* host = "shelfclock";
const int   daylightOffset_sec = 3600;
const char* ntpServer = "pool.ntp.org";
const int valid_durations[] = {1, 2, 4, 8, 16, 32};
const int valid_octaves[] = {4, 5, 6, 7};
const int valid_beats[] = {25, 28, 31, 35, 40, 45, 50, 56, 63, 70, 80, 90, 100, 112, 125, 140, 160, 180, 200, 225, 250, 285, 320, 355, 400, 450, 500, 565, 635, 715, 800, 900};
unsigned long WiFi_startTime = 0;
unsigned long WiFi_elapsedTime = 0;
int WiFi_retryCount = 0;
int WiFi_totalReconnections = 0;
int breakOutSet = 0; //jump out of count
int colorWheelPosition = 255; // COLOR WHEEL POSITION
int colorWheelPositionTwo = 255; // 2nd COLOR WHEEL POSITION
const int colorWheelSpeed = 3;
int sleepTimerCurrent = 0;
int isAsleep = 0;
int photo_in[PHOTO_SIZE] = {   0, 150, 1180, 2170, 4095};
int photo_out[PHOTO_SIZE] = {255, 160,   40,   14,    4};
int photoresisterReadings[PHOTO_SAMPLES];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int lightSensorValue = 255;
int previousTimeMin = 0;
int previousTimeHour = 0;
int previousTimeDay = 0;
int previousTimeWeek = 0;
int previousTimeMonth = 0;
byte randomMinPassed = 1;
byte randomHourPassed = 1;
byte randomDayPassed = 1;
byte randomWeekPassed = 1;
byte randomMonthPassed = 1;
bool dotsOn = true;
unsigned long prevTime = 0;
unsigned long prevTime2 = 0;
unsigned long countdownMilliSeconds;
unsigned long endCountDownMillis;
byte  fakeclockrunning = 0;    
unsigned long countupMilliSeconds;
unsigned long CountUpMillis;
int scoreboardLeft = 0;
int scoreboardRight = 0; 
int currentMode = 0;
int currentReal = 0;
int cylonPosition = 0;
int clearOldLeds = 0; 
byte rain[SPECTRUM_PIXELS];
byte greenMatrix[SPECTRUM_PIXELS];
int lightshowSpeed = 1;
int snakeLastDirection = 0;  //snake's last dirction
int snakePosition = 0;  //snake's position
int foodSpot = random(SPECTRUM_PIXELS-1);  //food spot
int snakeWaiting = 0;  //waiting
int getSlower = 180;
int daysUptime = 0;
int hoursUptime = 0;
int minutesUptime = 0;
float outdoorTemp = -500;
float outdoorHumidity = -1;
bool humidity_outdoor_enable = 0;
bool temperature_outdoor_enable = 0;
const size_t songTaskbufferSize = 128;  // Adjust the buffer size as per your requirements
char songTaskbuffer[songTaskbufferSize];

struct WeatherAPI weatherapi;
#if HAS_BUZZER
  bool useAudibleAlarm = 0;
  char defaultAudibleAlarm[100] = "Final Countdown";
  char specialAudibleAlarm[100] = "Final Countdown";
  DynamicJsonDocument SONGS(8192);
char filesArray[64][64];
char songsArray[64][64];
#endif

const int NUM_ROWS = 7;
const int NUM_COLS = 21;

bool updateSettingsRequired = 0;
int totalSongs = 0;

DynamicJsonDocument jsonDoc(8192); // create a JSON document to store the data
DynamicJsonDocument jsonScheduleData(8192);  //stores schedules
File fsUploadFile;

struct tm timeinfo; 
CRGB LEDs[NUM_LEDS];
Preferences preferences;
#if HAS_DHT
  DHT dht(DHT_PIN, DHTTYPE);
  int altitudeLocal = 81;  //in meters
#endif
WebServer server(80);
HTTPUpdateServer httpUpdateServer;
#if HAS_RTC
  RTC_DS3231 rtc;
#endif
AutoConnect      Portal(server);
AutoConnectConfig  Config;

// global settings that get saved to flash via preffs
byte cd_r_val = 0;
byte cd_g_val = 255;
byte cd_b_val = 0;
byte r0_val = 255;  //spotlight
byte g0_val = 255;
byte b0_val = 255;
byte r1_val = 255;  //colorHour
byte g1_val = 0;
byte b1_val = 0;
byte r2_val = 255;  //colorMins
byte g2_val = 0;
byte b2_val = 0;
byte r3_val = 255;   //colorColon
byte g3_val = 0;
byte b3_val = 0;
byte r4_val = 255;   //dayColor
byte g4_val = 0;
byte b4_val = 0;
byte r5_val = 255;   //monthColor
byte g5_val = 0;
byte b5_val = 0;
byte r6_val = 255;    //separatorColor
byte g6_val = 0;
byte b6_val = 0;
byte r7_val = 255;  //TempColor
byte g7_val = 0;
byte b7_val = 0;
byte r8_val = 255;  //TypeColor
byte g8_val = 0;
byte b8_val = 0;
byte r9_val = 255;  //DegreeColor
byte g9_val = 0;
byte b9_val = 0;
byte r10_val = 255;  //HumiColor
byte g10_val = 0;
byte b10_val = 0;
byte r11_val = 255;  //SymbolColor
byte g11_val = 0;
byte b11_val = 0;
byte r12_val = 255;  //HumiDecimalColor
byte g12_val = 0;
byte b12_val = 0;
byte r13_val = 255;  //scoreboard left
byte g13_val = 0;
byte b13_val = 0;
byte r14_val = 255;  //scoreboard right
byte g14_val = 0;
byte b14_val = 0;
byte r15_val = 255;  //spectrum analyzer
byte g15_val = 0;
byte b15_val = 0;
byte r16_val = 255;  //scrolling text analyzer
byte g16_val = 255;
byte b16_val = 255;
byte r17_val = 0;  //spectrum analyzer background
byte g17_val = 0;
byte b17_val = 0;
byte clockMode = 11;   // Clock modes: 0=Clock, 1=Countdown, 2=Temperature, 3=Scoreboard, 4=Stopwatch, 5=Lightshow, 6=Rainbows/Scroll, 7=Date, 8=Humidity, 9=Spectrum, 10=Display Off
byte clockDisplayType = 3; //0-Center Times, 1-24-hour Military Time, 2-12-hour Space-Padded, 3-Blinking Center Light
byte dateDisplayType = 5; //0-Zero-Padded (MMDD), 1-Space-Padded (MMDD), 2-Center Dates (1MDD), 3-Just Day of Week (Sun), 4-Just Numeric Day (DD), 5-With "." Separator (MM.DD), 6-Just Year (YYYY)
byte tempDisplayType = 0; //0-Temperature with Degree and Type (79°F), 1-Temperature with just Type (79 F), 2-Temperature with just Degree (79°), 3-Temperature with Decimal (79.9), 4-Just Temperature (79)
byte humiDisplayType = 0; //0-Humidity with Symbol (34 H), 1-Humidity with Decimal (34.9), 2-Just Humidity (79)
byte pastelColors = 0;
byte temperatureSymbol = 39;   // 36=Celcius, 39=Fahrenheit check 'numbers'
bool DSTime = 0;
long gmtOffset_sec = -28800;
byte ClockColorSettings = 0;
byte DateColorSettings = 0;
byte tempColorSettings = 0;
byte humiColorSettings = 0;
int temperatureCorrection = 0;
int colonType = 0;
byte ColorChangeFrequency = 0;
byte brightness = 10;  //set to 10 for photoresister control at startup
String scrollText = "dAdS ArE tHE bESt";
bool colorchangeCD = 1;
int spectrumMode = 0;
int spectrumColorSettings = 2;
int spectrumBackgroundSettings = 0;
int realtimeMode = 0;
int spotlightsColorSettings = 0;
bool useSpotlights = 1;
int scrollColorSettings = 0;
bool scrollOverride = 0;
bool scrollOptions1 = 0;   //Military Time (HHMM)
bool scrollOptions2 = 0;   //Day of Week (DOW)
bool scrollOptions3 = 0;   //Today's Date (DD-MM)
bool scrollOptions4 = 0;   //Year (YYYY)
bool scrollOptions5 = 0;   //Temperature (70 °F)
bool scrollOptions6 = 0;   //Humidity (47 H)
bool scrollOptions7 = 0;   //Text Message
bool scrollOptions8 = 1;   //IP Address of Clock
int scrollFrequency = 1;
int lightshowMode = 0;
byte randomSpectrumMode = 0;
int suspendFrequency = 1;  //in minutes
byte suspendType = 0; //0-off, 1-digits-only, 2-everything
static int prevPeak[SOUNDDETECTOR_BANDS_WIDTH] = {0}; // Initialize all elements to 0


CRGB spectrumColor = CRGB(r15_val, g15_val, b15_val);
CRGB spectrumBackground = CRGB(r17_val, g17_val, b17_val);
CRGB hourColor = CRGB(r1_val, g1_val, b1_val); 
CRGB colonColor = CRGB(r3_val, g3_val, b3_val); 
CRGB spotlightsColor = CRGB(r0_val, g0_val, b0_val);
CRGB alternateColor = CRGB::Black; 
CRGB scrollColor = CRGB(r16_val, g16_val, b16_val);
CRGB humiColor = CRGB(r10_val, g10_val, b10_val);
CRGB tinyhumiColor = CRGB(r10_val, g10_val, b10_val);
CRGB symbolColor = CRGB(r11_val, g11_val, b11_val);
CRGB humiDecimalColor = CRGB(r12_val, g12_val, b12_val);
CRGB tinytempColor = CRGB(r7_val, g7_val, b7_val);
CRGB tempColor = CRGB(r7_val, g7_val, b7_val);
CRGB typeColor = CRGB(r8_val, g8_val, b8_val);
CRGB degreeColor = CRGB(r9_val, g9_val, b9_val);
CRGB dayColor = CRGB(r4_val, g4_val, b4_val);
CRGB monthColor = CRGB(r5_val, g5_val, b5_val);
CRGB tinymonthColor = CRGB(r5_val, g5_val, b5_val);
CRGB separatorColor = CRGB(r6_val, g6_val, b6_val);
CRGB minColor = CRGB(r2_val, g2_val, b2_val);
CRGB tinyhourColor = CRGB(r3_val, g3_val, b3_val);
CRGB lightchaseColorOne = CRGB::Blue;
CRGB lightchaseColorTwo = CRGB::Red;
CRGB oldsnakecolor = CRGB::Green;
CRGB spotcolor = CHSV(random(0, 255), 255, 255);

const uint16_t FAKE_LEDs[FAKE_NUM_LEDS] = {digit0, fdigit1, digit2, fdigit3, digit4, fdigit5, digit6};
//fake LED layout for spectrum (from the middle out)
const uint16_t FAKE_LEDs_C_BMUP[SEGMENTS_LEDS] = {seg(17), seg(11), seg(21), seg(12), seg(20), seg(19), seg(27), seg(7), seg(22), seg(10), seg(26), seg(16), seg(25), seg(13), seg(18), seg(1), seg(31), seg(2), seg(30), seg(9), seg(29), seg(15), seg(23), seg(14), seg(24), seg(0), seg(32), seg(6), seg(36), seg(3), seg(35), seg(8), seg(28), seg(5), seg(33), seg(4), seg(34)};
//fake LED layout for spectrum (bfrom the outside in)
const uint16_t FAKE_LEDs_C_CMOT[SEGMENTS_LEDS] = {seg(19), seg(26), seg(16), seg(20), seg(13), seg(25), seg(12), seg(17), seg(18), seg(21), seg(14), seg(24), seg(11), seg(29), seg(9), seg(22), seg(15), seg(23), seg(10), seg(28), seg(7), seg(27), seg(8), seg(36), seg(6), seg(30), seg(3), seg(35), seg(2), seg(34), seg(1), seg(31), seg(4), seg(32), seg(5), seg(33), seg(0)};
//fake LED layout for spectrum (bottom-left to right-top)
const uint16_t FAKE_LEDs_C_BLTR[SEGMENTS_LEDS] = {seg(32), seg(31), seg(27), seg(30), seg(36), seg(33), seg(34), seg(35), seg(29), seg(22), seg(21), seg(17), seg(20), seg(26), seg(23), seg(28), seg(24), seg(25), seg(19), seg(12), seg(11), seg(7), seg(10), seg(16), seg(13), seg(18), seg(14), seg(15), seg(9), seg(2), seg(1), seg(0), seg(6), seg(3), seg(8), seg(4), seg(5)};
//fake LED layout for spectrum (top-left to bottom-right) 
const uint16_t FAKE_LEDs_C_TLBR[SEGMENTS_LEDS] = {seg(34), seg(33), seg(32), seg(36), seg(35), seg(28), seg(24), seg(23), seg(29), seg(30), seg(31), seg(27), seg(22), seg(26), seg(25), seg(18), seg(14), seg(13), seg(19), seg(20), seg(21), seg(17), seg(12), seg(16), seg(15), seg(8), seg(4), seg(3), seg(9), seg(10), seg(11), seg(7), seg(2), seg(6), seg(5), seg(0), seg(1)};
//fake LED layout for spectrum (top-middle down)  
const uint16_t FAKE_LEDs_C_TMDN[SEGMENTS_LEDS] = {seg(18), seg(14), seg(24), seg(13), seg(25), seg(19), seg(8), seg(28), seg(15), seg(23), seg(16), seg(26), seg(12), seg(20), seg(17), seg(4), seg(34), seg(3), seg(35), seg(9), seg(29), seg(10), seg(22), seg(11), seg(21), seg(5), seg(33), seg(6), seg(36), seg(2), seg(30), seg(7), seg(27), seg(0), seg(32), seg(1), seg(31)};
//fake LED layout for spectrum (center-sides in)  
const uint16_t FAKE_LEDs_C_CSIN[SEGMENTS_LEDS] = {seg(5), seg(32), seg(0), seg(33), seg(6), seg(36), seg(4), seg(31), seg(1), seg(34), seg(3), seg(30), seg(2), seg(35), seg(9), seg(29), seg(8), seg(27), seg(7), seg(28), seg(15), seg(22), seg(10), seg(23), seg(16), seg(26), seg(14), seg(21), seg(11), seg(24), seg(13), seg(20), seg(12), seg(25), seg(19), seg(18), seg(17)};
//fake LED layout for spectrum (bottom-right to left-top)
const uint16_t FAKE_LEDs_C_BRTL[SEGMENTS_LEDS] = {seg(0), seg(1), seg(7), seg(2), seg(6), seg(5), seg(4), seg(3), seg(9), seg(10), seg(11), seg(17), seg(12), seg(16), seg(15), seg(8), seg(14), seg(13), seg(19), seg(20), seg(21), seg(27), seg(22), seg(26), seg(25), seg(18), seg(24), seg(23), seg(29), seg(30), seg(31), seg(32), seg(36), seg(35), seg(28), seg(34), seg(33)};
//fake LED layout for spectrum (top-right to bottom-left)
const uint16_t FAKE_LEDs_C_TRBL[SEGMENTS_LEDS] = {seg(4), seg(5), seg(0), seg(6), seg(3), seg(8), seg(14), seg(15), seg(9), seg(2), seg(1), seg(7), seg(10), seg(16), seg(13), seg(18), seg(24), seg(25), seg(19), seg(12), seg(11), seg(17), seg(20), seg(26), seg(23), seg(28), seg(34), seg(35), seg(29), seg(22), seg(21), seg(27), seg(30), seg(36), seg(33), seg(32), seg(31)};
//fake LED layout for spectrum (horizontal parts)   
const uint16_t FAKE_LEDs_C_OUTS[SEGMENTS_LEDS] = {seg(31), seg(39), seg(36), seg(39), seg(34), seg(39), seg(27), seg(39), seg(29), seg(39), seg(28), seg(39), seg(21), seg(39), seg(26), seg(39), seg(24), seg(39), seg(17), seg(39), seg(19), seg(39), seg(18), seg(39), seg(11), seg(39), seg(16), seg(39), seg(14), seg(39), seg(7), seg(39),seg(9), seg(8), seg(1), seg(6), seg(4)};
const uint16_t FAKE_LEDs_C_OUTS2[SEGMENTS_LEDS] = {seg(1), seg(39), seg(6), seg(39), seg(4), seg(39), seg(7), seg(39), seg(9), seg(39), seg(8), seg(39), seg(11), seg(39), seg(16), seg(39), seg(14), seg(39), seg(17), seg(39), seg(19), seg(39), seg(18), seg(39), seg(21), seg(39), seg(26), seg(39), seg(24), seg(39), seg(27), seg(39),seg(29), seg(28), seg(31), seg(36), seg(34)};
//fake LED layout for spectrum (vertical parts)  
const uint16_t FAKE_LEDs_C_VERT[SEGMENTS_LEDS] = {seg(32), seg(39), seg(33), seg(39), seg(30), seg(39), seg(35), seg(39), seg(39), seg(22), seg(39), seg(23), seg(39), seg(39), seg(20), seg(39), seg(25), seg(39), seg(39), seg(12), seg(39), seg(13), seg(39), seg(39), seg(10), seg(39), seg(15), seg(39), seg(39), seg(2), seg(39), seg(3), seg(39), seg(0), seg(39), seg(5), seg(39)};
const uint16_t FAKE_LEDs_C_VERT2[SEGMENTS_LEDS] = {seg(0), seg(39), seg(5), seg(39), seg(39), seg(2), seg(39), seg(3), seg(39), seg(39), seg(10), seg(39), seg(15), seg(39), seg(39), seg(12), seg(39), seg(13), seg(39), seg(39), seg(20), seg(39), seg(25), seg(39), seg(39), seg(22), seg(39), seg(23), seg(39), seg(30), seg(39), seg(35), seg(39), seg(32), seg(39), seg(33), seg(39)};

//fake LED layout for fire display  
const uint16_t FAKE_LEDs_C_FIRE[SEGMENTS_LEDS] = {seg(17), seg(11), seg(21), seg(12), seg(20), seg(19), seg(27), seg(7), seg(22), seg(10), seg(26), seg(16), seg(25), seg(13), seg(18), seg(1), seg(31), seg(2), seg(30), seg(9), seg(29), seg(15), seg(23), seg(14), seg(24), seg(0), seg(32), seg(6), seg(36), seg(3), seg(35), seg(8), seg(28), seg(5), seg(33), seg(4), seg(34)};
//fake LED layout for Rain display  
const uint16_t FAKE_LEDs_C_RAIN[SEGMENTS_LEDS] = {seg(30), seg(35), seg(1), seg(6), seg(4), seg(22), seg(23), seg(31), seg(36), seg(34), seg(12), seg(13), seg(7), seg(9), seg(8), seg(0), seg(5), seg(17), seg(19), seg(18), seg(10), seg(15), seg(21), seg(26), seg(24), seg(2), seg(3), seg(27), seg(29), seg(28), seg(20), seg(25), seg(11), seg(16), seg(14), seg(32), seg(33)};
//fake LED layout for Snake display 
const uint16_t FAKE_LEDs_SNAKE[SEGMENTS_LEDS] = {seg(0), seg(1), seg(7), seg(2), seg(6), seg(5), seg(4), seg(3), seg(9), seg(10), seg(11), seg(17), seg(12), seg(16), seg(15), seg(8), seg(14), seg(13), seg(19), seg(20), seg(21), seg(27), seg(22), seg(26), seg(25), seg(18), seg(24), seg(23), seg(29), seg(30), seg(31), seg(32), seg(36), seg(35), seg(28), seg(34), seg(33)};

//  real6     fake5     real4     fake3     real2     fake1     real0
// RRRRRRR   RRRRRRR   RRRRRRR   RRRRRRR   RRRRRRR   RRRRRRR   RRRRRRR 
//R       R F       F R       R F       F R       R F       F R       R
//R   S   R F   S   F R   S   R F   S   F R   S   R F   S   F R   S   R
//R       R F       F R       R F       F R       R F       F R       R
//R       R F       F R       R F       F R       R F       F R       R
//R       R F       F R       R F       F R       R F       F R       R
//R       R F       F R       R F       F R       R F       F R       R
//R       R F       F R       R F       F R       R F       F R       R
// RRRRRRR   RRRRRRR   RRRRRRR   RRRRRRR   RRRRRRR   RRRRRRR   RRRRRRR 
//R       R F       F R       R F       F R       R F       F R       R
//R   S   R F   S   F R   S   R F   S   F R   S   R F   S   F R   S   R
//R       R F       F R       R F       F R       R F       F R       R
//R       R F       F R       R F       F R       R F       F R       R
//R       R F       F R       R F       F R       R F       F R       R
//R       R F       F R       R F       F R       R F       F R       R
//R       R F       F R       R F       F R       R F       F R       R
// RRRRRRR   RRRRRRR   RRRRRRR   RRRRRRR   RRRRRRR   RRRRRRR   RRRRRRR 

//digit wiring order
//  4444444 
// 3       5
// 3       5
// 3       5
// 3       5
// 3       5
// 3       5
// 3       5
//  6666666 
// 2       0
// 2       0
// 2       0
// 2       0
// 2       0
// 2       0
// 2       0
//  1111111 
// byte numbers digit order 0b6543210
byte numbers[] = { 
   0b0111111,   //  [0] 0
   0b0100001,   //  [1] 1
   0b1110110,   //  [2] 2
   0b1110011,   //  [3] 3
   0b1101001,   //  [4] 4
   0b1011011,   //  [5] 5
   0b1011111,   //  [6] 6
   0b0110001,   //  [7] 7
   0b1111111,   //  [8] 8
   0b1111011,   //  [9] 9
   0b0000000,   //  [10] all off, space
   0b0100001,   //  [11] !
   0b0101000,   //  [12] "
   0b1101111,   //  [13] #
   0b1011011,   //  [14] $
   0b1100100,   //  [15] %
   0b1100001,   //  [16] &
   0b0001000,   //  [17] '
   0b0011010,   //  [18] (
   0b0110010,   //  [19] )
   0b0011000,   //  [20] *
   0b1001100,   //  [21] +
   0b0000100,   //  [22] ,
   0b1000000,   //  [23] -
   0b0000010,   //  [24] .
   0b1100100,   //  [25] /
   0b1111000,   //  [26] degrees symbol
   0b0010010,   //  [27] :
   0b0010011,   //  [28] ;
   0b1011000,   //  [29] <
   0b1000010,   //  [30] =
   0b1110000,   //  [31] >
   0b1110100,   //  [32] ?
   0b1110111,   //  [33] @
   0b1111101,   //  [34] A
   0b1001111,   //  [35] B
   0b0011110,   //  [36] C(elsius)
   0b1100111,   //  [37] D
   0b1011110,   //  [38] E
   0b1011100,   //  [39] F(ahrenheit)
   0b0011111,   //  [40] G
   0b1101101,   //  [41] H(umidity)
   0b0001100,   //  [42] I
   0b0100111,   //  [43] J
   0b1011101,   //  [44] K
   0b0001110,   //  [45] L
   0b0010101,   //  [46] M
   0b0111101,   //  [47] N
   0b0111111,   //  [48] O
   0b1111100,   //  [49] P
   0b1111010,   //  [50] Q
   0b0111100,   //  [51] R
   0b1011011,   //  [52] S
   0b1001110,   //  [53] T
   0b0101111,   //  [54] U
   0b0101111,   //  [55] V
   0b0101010,   //  [56] W
   0b1101101,   //  [57] X
   0b1101011,   //  [58] Y
   0b1110110,   //  [59] Z
   0b0011110,   //  [60] left bracket
   0b1001001,   //  [61] backslash
   0b0110011,   //  [62] right bracket
   0b0111000,   //  [63] carrot
   0b0000010,   //  [64] underscore
   0b0100000,   //  [65] aposhtophe
   0b1110111,   //  [66] a
   0b1001111,   //  [67] b
   0b1000110,   //  [68] c
   0b1100111,   //  [69] d
   0b1111110,   //  [70] e
   0b1011100,   //  [71] f
   0b1111011,   //  [72] g
   0b1001101,   //  [73] h
   0b0000100,   //  [74] i
   0b0000011,   //  [75] j
   0b1011101,   //  [76] k
   0b0001100,   //  [77] l
   0b0000101,   //  [78] m
   0b1000101,   //  [79] n
   0b1000111,   //  [80] o
   0b1111100,   //  [81] p
   0b1111001,   //  [82] q
   0b1000100,   //  [83] r
   0b1011011,   //  [84] s
   0b1001110,   //  [85] t
   0b0000111,   //  [86] u
   0b0000111,   //  [87] v
   0b0000101,   //  [88] w
   0b1101101,   //  [89] x
   0b1101011,   //  [90] y
   0b1110110,   //  [91] z
   0b1100001,   //  [92] left squigly bracket
   0b0001100,   //  [93] pipe
   0b1001100,   //  [94] right squigly bracket
   0b0010000,   //  [95] tilde
   0b1010010    //  [96] 3-lines (special pad)
   };

TaskHandle_t Task1;
QueueHandle_t jobQueue;

void setup() {
  Serial.begin(115200);
  #if HAS_SOUNDDETECTOR
    sampling_period_us = round(1000000 * (1.0 / SOUNDDETECTOR_SAMPLING_FREQ));
  #endif
  loadWebPageHandlers();  //load about 900 webpage handlers from the bottom of this sketch

  // Initialize FileFS 
  Serial.println(F("Inizializing FS..."));
  if (FileFS.begin()){
      Serial.println(F("FileFS mounted correctly."));
      Serial.println("Total Bytes");
      Serial.println(FileFS.totalBytes());
      Serial.println("Used Bytes");
      Serial.println(FileFS.usedBytes());
  }else{
      Serial.println(F("!An error occurred during FileFS mounting"));
  }

    Serial.println("list setting folder files");
  listDir(FileFS, "/settings/", 0);
  //load settings from nvram and flash
      Serial.println("load all settings from json file");
  getclockSettings("generic"); //load all settings from json file
      Serial.println("load all previously saved settings from Preferences");
  loadSetupSettings();  //load all previously saved settings from Preferences
  
  #if HAS_RTC
    //init DS3231 RTC
    if (! rtc.begin()) {
      Serial.println("Couldn't find DS3231 RTC");
      Serial.flush();
    //  abort();
    }
  #endif




  #if HAS_SOUNDDETECTOR
    // Initialize peaks to zero
    for (byte band = 0; band < SOUNDDETECTOR_BANDS_WIDTH; band++) {
        peak[band] = 0;
        prevPeak[band] = 0;
    }


    i2sConfig();
    i2sPins();
  #endif

  // init temp & humidity sensor
  #if HAS_DHT
    Serial.println(F("DHTxx test!"));
    dht.begin();
  #endif

  //setup LEDs
  FastLED.addLeds<LED_TYPE,LED_PIN,COLOR_ORDER>(LEDs,NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(255);
  fill_solid(LEDs, NUM_LEDS, CRGB::Black);
  FastLED.show();
  
  fakeClock(2);  // blink 12:00 like old clocks once did
  
  Config.autoReconnect = true; // Enable auto-reconnect. 
  Config.portalTimeout = 20000; // Sets timeout value for the captive portal 
  Config.retainPortal = true; // Retains the portal function after timed-out 
  Config.autoRise = true; // False for disabling the captive portal
  Config.reconnectInterval = 6;
  Portal.config(Config);      

  Serial.println("Wifi Starting");
  WiFi.hostname(host); //set hostname

  // setup AutoConnect to control WiFi
  if (Portal.begin()) {
    Serial.println("WiFi Connected: " + WiFi.localIP().toString());
    WiFi_startTime = millis();
    WiFi_retryCount = 0;
   }  else {Serial.println("Wifi Failed");}

  //use mdns for host name resolution
  if (!MDNS.begin(host)) { //http://shelfclock
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");

  //init and set the time of the internal RTC from NTP server
      Serial.println("set the time of the internal RTC from NTP server");
  configTime(gmtOffset_sec, (daylightOffset_sec * DSTime), ntpServer);
  
  #if HAS_RTC
    //was the internal RTC time set by the NTP server?, if not set it via the RTC DS3231 stored time, will be wrong if daylight savings time is active
    if(!getLocalTime(&timeinfo)){ 
      struct tm tm;
      DateTime now = rtc.now();
      tm.tm_year = now.year() - 1900;
      tm.tm_mon = now.month() - 1;
      tm.tm_mday = now.day();
      tm.tm_hour = now.hour();
      tm.tm_min = now.minute();
      tm.tm_sec = now.second();
      time_t t = mktime(&tm);
      printf("NTP server not found, setting localtime from DS3231: %s", asctime(&tm));
      struct timeval now1 = { .tv_sec = t };
      settimeofday(&now1, NULL);
    }
    //did the DS3231 lose power (battery dead/changed), if so, set from time recieved from the NTP above
    if (rtc.lostPower()) {
      Serial.println("DS3231's RTC lost power, setting the time via NTP!");
      if(!getLocalTime(&timeinfo)){Serial.println("Error, no NTP Server found!");}
      int tempyear = (timeinfo.tm_year +1900);
      int tempmonth = (timeinfo.tm_mon + 1);
      rtc.adjust(DateTime(tempyear, tempmonth, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
    }
  #endif

      Serial.println("print time");
  printLocalTime(); 

  //create something to know if now is not then
  previousTimeMin = timeinfo.tm_min;
  previousTimeHour = timeinfo.tm_hour;
  previousTimeDay = timeinfo.tm_mday;
  previousTimeWeek = ((timeinfo.tm_yday + 7 - (timeinfo.tm_wday ? (timeinfo.tm_wday - 1) : 6)) / 7);
  previousTimeMonth = timeinfo.tm_mon;
  
  // I assume this starts the OTA stuff
  httpUpdateServer.setup(&server);

  initGreenMatrix();   //setup lightshow functions
  raininit();          //setup lightshow functions
  
  allBlank();   //clear everything off the leds
  
  server.enableCrossOrigin(true);
  server.enableCORS(true);

  //Webpage Handlers for FileFS access to flash
  server.serveStatic("/", FileFS, "/index.html");  //send default webpage from root request
  server.serveStatic("/", FileFS, "/", "max-age=86400");

  //OTA firmware Upgrade Webpage Handlers
  Serial.println("OTA Available");


  #if HAS_BUZZER
    //init rtttl (functions that play the alarms)
    pinMode(BUZZER_PIN, OUTPUT);
   rtttl::begin(BUZZER_PIN, "Intel:d=4,o=5,b=400:32p,d,g,d,2a");  //play mario sound and set initial brightness level
    while( !rtttl::done() ){GetBrightnessLevel(); rtttl::play();}
        Serial.println("get list of songs");
    getListOfSongs();
  #endif

  
        Serial.println("load array of schedule files on drive ");
  createSchedulesArray();  //load array of schedule files on drive
        Serial.println("print out schedule array");
  processSchedules(0);  //print out schedule array

        Serial.println("start of job queue");
    jobQueue = xQueueCreate(30, sizeof(String *));
        Serial.println("start of xTaskCreatePinnedToCore");
    xTaskCreatePinnedToCore(Task1code, "Task1", 10000, NULL, 0, &Task1, 0);

        Serial.println("end of Setup");
}    //end of Setup()

void Task1code(void * parameter) {
  unsigned long lastTime = 0;
  unsigned long weatherTimerDelay = 600000;  //600000 = 10 min
  getRemoteWeather();
  while(true) {
    #if HAS_BUZZER

        if (xQueueReceive(jobQueue, songTaskbuffer, (TickType_t)10)) {
          String job = String(songTaskbuffer);
          Serial.print("job ");
          Serial.println(job);
          playRTTTLsong(job, 1);
        } 
    #endif
    if ((millis() - lastTime) > weatherTimerDelay) {
      getRemoteWeather();
      lastTime = millis();
    }  
  }
}

void getRemoteWeather() {
  #if HAS_ONLINEWEATHER
    if (WiFi.status() == WL_CONNECTED && weatherapi.apikey && weatherapi.latitude && weatherapi.longitude) {
      HTTPClient http;
      String serverPath = String("http://api.openweathermap.org/data/2.5/weather?lat=") + weatherapi.latitude + String("&lon=") + weatherapi.longitude + String("&APPID=") + weatherapi.apikey + String("&units=imperial");
      Serial.println(serverPath);
      http.begin(serverPath.c_str());
      int httpResponseCode = http.GET();
      if (httpResponseCode > 0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        DynamicJsonDocument payload(2048);
        deserializeJson(payload, http.getStream());
        JsonObject main = payload["main"].as<JsonObject>();
        outdoorTemp = main["temp"].as<String>().toFloat();
        outdoorHumidity = main["humidity"].as<String>().toFloat();
      } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
  }
  #endif
}

void loop(){
  
  server.handleClient(); 
  Portal.handleRequest(); 
  if (WiFi.status() != WL_CONNECTED) {
    WiFi_elapsedTime = millis() - WiFi_startTime;
    if (WiFi_elapsedTime >= WiFi_MAX_RETRY_DURATION) {
      // Reboot if maximum retry duration exceeded
        Serial.println("maximum retry duration exceeded ");
      ESP.restart();
    } else {
      if (WiFi_retryCount >= WiFi_MAX_RETRIES) {
        // Reboot if maximum retry attempts exceeded
        Serial.println("maximum retry attemps exceeded ");
        ESP.restart();
      } else if (WiFi.status() == WL_IDLE_STATUS) {
        // Attempt to reconnect to Wi-Fi
        WiFi.disconnect();
        delay(1000);
        WiFi.begin();
        WiFi_retryCount++;
        WiFi_totalReconnections++;
        Serial.print("Retry ");
        Serial.print(WiFi_retryCount);
        Serial.println(" - Reconnecting...");
      }
    }
  } else {
    // Wi-Fi connection is successful
    //Serial.println("Connected!");
    // Reset retry count and start time
    WiFi_retryCount = 0;
    WiFi_startTime = millis();
     //   Serial.println("Wi-Fi connection is successful ");
    // Your other code logic can go here
  }

  //Change Frequency so as to not use hard-coded delays
  unsigned long currentMillis = millis();  
  //run everything inside here every second
  if (((unsigned long)(currentMillis - prevTime) >= 1000) || ((unsigned long)(currentMillis - prevTime) <= 0)) {  //deals with millis() having a rollover period after approximately 49.7 days
    prevTime = currentMillis;
    if(!getLocalTime(&timeinfo)){ 
      Serial.println("Failed to obtain time");
    }

    //setup time-passage trackers
    int secs = timeinfo.tm_sec;
    int currentTimeMin = timeinfo.tm_min;
    byte m1 = currentTimeMin / 10;
    byte m2 = currentTimeMin % 10;
    int currentTimeHour = timeinfo.tm_hour;
    int currentTimeDay = timeinfo.tm_mday;
    int currentTimeWeek = ((timeinfo.tm_yday + 7 - (timeinfo.tm_wday ? (timeinfo.tm_wday - 1) : 6)) / 7);
    int currentTimeMonth = timeinfo.tm_mon;

    checkSleepTimer();  //time to sleep?

    if ((currentTimeHour == 23 && currentTimeMin == 11 && timeinfo.tm_sec == 0 && clockDisplayType != 1) || (currentTimeHour == 11 && currentTimeMin == 11 && timeinfo.tm_sec == 0)) { scroll("MAkE A WISH"); }  //at 1111 make a wish
 
    if (abs(currentTimeMin - previousTimeMin) >= 1) { //run every minute
      previousTimeMin = currentTimeMin; 
      randomMinPassed = 1;
      minutesUptime += 1; 
      GetBrightnessLevel(); 
      processSchedules(0);  //see if there's a timer
      if (scrollFrequency == 1 && (suspendType == 0 || isAsleep == 0) && scrollOverride == 1 && ((clockMode != 11) && (clockMode != 1) && (clockMode != 4))) {displayScrollMode();}
      if (scrollFrequency == 1 && randomSpectrumMode == 1 && clockMode == 9) {allBlank(); spectrumMode = random(11);}
      } //end of run every minute

    if ((m2 == 0 || m2 == 5) && (secs == 0)) { //run every 5 minutes
      if (scrollFrequency == 2 && (suspendType == 0 || isAsleep == 0) && scrollOverride == 1 && ((clockMode != 11) && (clockMode != 1) && (clockMode != 4))) {displayScrollMode();}
      if (scrollFrequency == 2 && randomSpectrumMode == 1 && clockMode == 9) {allBlank(); spectrumMode = random(11);}
      } //end of run every 5 minutes

    if ((m2 == 0) && (secs == 0)) { //run every 10 minutes
      if (scrollFrequency == 3 && (suspendType == 0 || isAsleep == 0) && scrollOverride == 1 && ((clockMode != 11) && (clockMode != 1) && (clockMode != 4))) {displayScrollMode();}
      if (scrollFrequency == 3 && randomSpectrumMode == 1 && clockMode == 9) {allBlank(); spectrumMode = random(11);}
      } //end of run every 10 minutes

    if (((m1 == 0 && m2 == 0) || (m1 == 1 && m2 == 5) || (m1 == 3 && m2 == 0) || (m1 == 4 && m2 == 5)) && (secs == 0)) { //run every 15 minutes
      if (scrollFrequency == 4 && (suspendType == 0 || isAsleep == 0) && scrollOverride == 1 && ((clockMode != 11) && (clockMode != 1) && (clockMode != 4))) {displayScrollMode();}
      if (scrollFrequency == 4 && randomSpectrumMode == 1 && clockMode == 9) {allBlank(); spectrumMode = random(11);}
      } //end of run every 15 minutes

    if (((m1 == 0 && m2 == 0) || (m1 == 3 && m2 == 0)) && (secs == 0)) { //run every 30 minutes
      if (scrollFrequency == 5 && (suspendType == 0 || isAsleep == 0) && scrollOverride == 1 && ((clockMode != 11) && (clockMode != 1) && (clockMode != 4))) {displayScrollMode();}
      if (scrollFrequency == 5 && randomSpectrumMode == 1 && clockMode == 9) {allBlank(); spectrumMode = random(11);}
      } //end of run every 30 minutes
      
    if (abs(currentTimeHour - previousTimeHour) >= 1) { //run every hour
      if (updateSettingsRequired == 1) { saveclockSettings("generic"); updateSettingsRequired = 0; }  //if something was changed in settings, write those changes when the home page is loaded.
      previousTimeHour = currentTimeHour; 
      randomHourPassed = 1;
      hoursUptime += 1;
      if (scrollFrequency == 6 && (suspendType == 0 || isAsleep == 0) && scrollOverride == 1 && ((clockMode != 11) && (clockMode != 1) && (clockMode != 4))) {displayScrollMode();}
      if (scrollFrequency == 6 && randomSpectrumMode == 1 && clockMode == 9) {allBlank(); spectrumMode = random(11);}
      } //end of run every hour
      
    if (abs(currentTimeDay - previousTimeDay) >= 1) { 
        previousTimeDay = currentTimeDay; 
        randomDayPassed = 1; 
        configTime(gmtOffset_sec, (daylightOffset_sec * DSTime), ntpServer);
        #if HAS_RTC
          int tempyear = (timeinfo.tm_year +1900);
          int tempmonth = (timeinfo.tm_mon + 1);
          rtc.adjust(DateTime(tempyear, tempmonth, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
        #endif
        daysUptime = daysUptime + 1;
     }
    if (abs(currentTimeWeek - previousTimeWeek) >= 1) { previousTimeWeek = currentTimeWeek; randomWeekPassed = 1;}
    
    if (abs(currentTimeMonth - previousTimeMonth) >= 1) { previousTimeMonth = currentTimeMonth; randomMonthPassed = 1;}

    if (realtimeMode == 0) {    //give autodim sensors some CPU time, update display
       //GetBrightnessLevel();        
       //FastLED.show();
    }
    //give the various clock modes CPU time every 1 seconds
    if ((suspendType == 0 || isAsleep == 0) && clockMode == 0) {
       displayTimeMode();
    }  else if (clockMode == 1) {
       displayCountdownMode();
    }  else if ((suspendType == 0 || isAsleep == 0) && clockMode == 2) {
       displayTemperatureMode();  
    }  else if ((suspendType == 0 || isAsleep == 0) && clockMode == 3) {
       displayScoreboardMode();  
    }  else if (clockMode == 4) {
       displayStopwatchMode();           
    }  else if ((suspendType == 0 || isAsleep == 0) && clockMode == 5) {
       displayLightshowMode();             
    }  else if ((suspendType == 0 || isAsleep == 0) && clockMode == 6) {
       //notused
    }  else if ((suspendType == 0 || isAsleep == 0) && clockMode == 7) {
       displayDateMode();         
    }  else if ((suspendType == 0 || isAsleep == 0) && clockMode == 8) {
       displayHumidityMode();    
    }  else if (clockMode == 9) {
      //spectrum mode
    }  else if (clockMode == 10) {
      //display off
    }  else if ((suspendType == 0 || isAsleep == 0) && clockMode == 11) {
       displayScrollMode();  
    } 
    ShelfDownLights(); 
    randomMinPassed = 0; 
    randomHourPassed = 0; 
    randomDayPassed = 0; 
    randomWeekPassed = 0; 
    randomMonthPassed = 0;
    
    if (brightness != 10) {  //if not set to auto-dim just use user set brightness
      FastLED.setBrightness(brightness);
    } else if (brightness == 10) {  //auto-dim use the value from above
      FastLED.setBrightness(lightSensorValue);     
    } 
  }

  displayRealtimeMode();  //always run outside time loop for speed, but only really show when it's needed
}  // end of main loop


void displayTimeMode() {  //main clock function
  currentMode = 0;
	if(!getLocalTime(&timeinfo)){ 
	  Serial.println("Failed to obtain time");
	}
 // printLocalTime();  //display time in monitor window
	int hour = timeinfo.tm_hour;
	int mins = timeinfo.tm_min;
	int secs = timeinfo.tm_sec;

  if (clockDisplayType != 1 && hour > 12){hour = hour - 12; }
  if (clockDisplayType != 1 && hour < 1){hour = hour + 12; }
	byte h1 = hour / 10;
	byte h2 = hour % 10;
	byte m1 = mins / 10;
	byte m2 = mins % 10;  
	byte s1 = secs / 10;
	byte s2 = secs % 10;
  
  if (ClockColorSettings == 0) {	hourColor = CRGB(r1_val, g1_val, b1_val);  minColor = CRGB(r2_val, g2_val, b2_val); }
  if (ClockColorSettings == 1) { hourColor = CRGB(r1_val, g1_val, b1_val);  minColor = hourColor; }
  if ((ClockColorSettings == 2 && pastelColors == 0)  && ( (ColorChangeFrequency == 0 ) || (ColorChangeFrequency == 1 && randomMinPassed == 1) || (ColorChangeFrequency == 2 && randomHourPassed == 1) || (ColorChangeFrequency == 3 && randomDayPassed == 1) || (ColorChangeFrequency == 4 && randomWeekPassed == 1) || (ColorChangeFrequency == 5 && randomMonthPassed == 1) )) { hourColor = CHSV(random(0, 255), 255, 255);  minColor = CHSV(random(0, 255), 255, 255);}
  if ((ClockColorSettings == 2 && pastelColors == 1)  && ( (ColorChangeFrequency == 0 ) || (ColorChangeFrequency == 1 && randomMinPassed == 1) || (ColorChangeFrequency == 2 && randomHourPassed == 1) || (ColorChangeFrequency == 3 && randomDayPassed == 1) || (ColorChangeFrequency == 4 && randomWeekPassed == 1) || (ColorChangeFrequency == 5 && randomMonthPassed == 1) )) { hourColor = CRGB(random(0, 255), random(0, 255), random(0, 255));  minColor = CRGB(random(0, 255), random(0, 255), random(0, 255));}
  if ((ClockColorSettings == 3 && pastelColors == 0) && ( (ColorChangeFrequency == 0 ) || (ColorChangeFrequency == 1 && randomMinPassed == 1) || (ColorChangeFrequency == 2 && randomHourPassed == 1) || (ColorChangeFrequency == 3 && randomDayPassed == 1) || (ColorChangeFrequency == 4 && randomWeekPassed == 1) || (ColorChangeFrequency == 5 && randomMonthPassed == 1) )) { hourColor = CHSV(random(0, 255), 255, 255);  minColor = hourColor;}
  if ((ClockColorSettings == 3 && pastelColors == 1) && ( (ColorChangeFrequency == 0 ) || (ColorChangeFrequency == 1 && randomMinPassed == 1) || (ColorChangeFrequency == 2 && randomHourPassed == 1) || (ColorChangeFrequency == 3 && randomDayPassed == 1) || (ColorChangeFrequency == 4 && randomWeekPassed == 1) || (ColorChangeFrequency == 5 && randomMonthPassed == 1) )) { hourColor = CRGB(random(0, 255), random(0, 255), random(0, 255));  minColor = hourColor;}
  if ((clockDisplayType == 3)) {          //Blinking Center Light
  	if (h1 > 0) {
      tinyhourColor = hourColor;
      if (ClockColorSettings == 4 && pastelColors == 0){ tinyhourColor = CHSV(random(0, 255), 255, 255); }
      if (ClockColorSettings == 4 && pastelColors == 1){ tinyhourColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
        for (int i=(32*LEDS_PER_SEGMENT); i<(33*LEDS_PER_SEGMENT); i++) { LEDs[i] = tinyhourColor;}
      if (ClockColorSettings == 4 && pastelColors == 0){ tinyhourColor = CHSV(random(0, 255), 255, 255); }
      if (ClockColorSettings == 4 && pastelColors == 1){ tinyhourColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
        for (int i=(33*LEDS_PER_SEGMENT); i<(34*LEDS_PER_SEGMENT); i++) { LEDs[i] = tinyhourColor;}
  	} else {
  	    for (int i=(32*LEDS_PER_SEGMENT); i<(34*LEDS_PER_SEGMENT); i++) { LEDs[i] = CRGB::Black;}
  	  }
      displayNumber(h2,5,hourColor);
      displayNumber(m1,2,minColor);
      displayNumber(m2,0,minColor); 
      BlinkDots();    
  }
  
  if (clockDisplayType == 2) {  //12-hour Space-Padded
    if (h1 < 1) { displayNumber(10,6,hourColor); }
    else { displayNumber(h1,6,hourColor); }
      displayNumber(h2,4,hourColor);
      displayNumber(m1,2,minColor);
      displayNumber(m2,0,minColor); 
	  BlinkDots();
  }
  if (clockDisplayType == 0) {  //center set and hour is less than 1 and no 0 is set, default
  if (h1 > 0) {
    tinyhourColor = hourColor;
    if (ClockColorSettings == 4 && pastelColors == 0){ tinyhourColor = CHSV(random(0, 255), 255, 255); }
    if (ClockColorSettings == 4 && pastelColors == 1){ tinyhourColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
      for (int i=(32*LEDS_PER_SEGMENT); i<(33*LEDS_PER_SEGMENT); i++) { LEDs[i] = tinyhourColor;}
    if (ClockColorSettings == 4 && pastelColors == 0){ tinyhourColor = CHSV(random(0, 255), 255, 255); }
    if (ClockColorSettings == 4 && pastelColors == 1){ tinyhourColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
      for (int i=(33*LEDS_PER_SEGMENT); i<(34*LEDS_PER_SEGMENT); i++) { LEDs[i] = tinyhourColor;}
  } else {for (int i=(32*LEDS_PER_SEGMENT); i<(34*LEDS_PER_SEGMENT); i++) { LEDs[i] = CRGB::Black;}
   }
    	displayNumber(h2,5,hourColor);
    	displayNumber(m1,3,minColor);
    	displayNumber(m2,1,minColor); 
  }
  if (clockDisplayType == 1) {     //24-hour Military Time
    if (h1 < 1) { displayNumber(0,6,hourColor);}
    else  { displayNumber(h1,6,hourColor);}
     
      displayNumber(h2,4,hourColor);
      displayNumber(m1,2,minColor);
      displayNumber(m2,0,minColor); 
	  BlinkDots();
  }

  if (clockDisplayType == 4) {     //New Year Countdown
    int hour = timeinfo.tm_hour;
    int mins = timeinfo.tm_min;
    int secs = timeinfo.tm_sec;
    int mday = timeinfo.tm_mday;
    int mont = timeinfo.tm_mon + 1;
    int year = (timeinfo.tm_year +1900);
    int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int daysLeft = 0;
    if (((year % 4 == 0) && (year % 100 != 0)) || ((year % 100 != 0) && (year % 400 == 0))){daysLeft = daysLeft + 1;}  //leapyear?
    if (mont !=12) {for (int i=(mont+1); i<13; i++) { daysLeft = daysLeft + daysInMonth[i-1];}}
    daysLeft = daysLeft + (daysInMonth[mont-1]-mday);
    int hoursLeft = (daysLeft * 24) + (23 - hour);
    bool DST;  //start of DST adjustment algorithm
    int y = year-2000; // uses two digit year 
    int x = (y + y/4 + 2) % 7; // remainder will identify which day of month
    if(mont == 3 && mday == (14 - x) && hour >= 2){DST = 1;} //DST begins on 2nd Sunday of March @ 2:00 AM
    if((mont == 3 && mday > (14 - x)) || mont > 3){DST = 1;}
    if(mont == 11 && mday == (7 - x) && hour >= 2){DST = 0; }  //DST ends on 1st Sunday of Nov @ 2:00 AM
    if((mont == 11 && mday > (7 - x)) || mont > 11 || mont < 3){DST = 0;}
    if(DST == 1) {hoursLeft = hoursLeft + 1; }  //adjust for DST
    int minutesLeft = (hoursLeft * 60) + (59 - mins);
    int secondsLeft = (minutesLeft * 60) + (60 - secs);
    int inputNums = hoursLeft;
    if ((minutesLeft <= 9999) && (secondsLeft > 9999)) {inputNums = minutesLeft; hourColor = minColor;}
    if ((secondsLeft <= 9999) && (secondsLeft > 10)) {inputNums = secondsLeft; hourColor = colonColor;}
    if (secondsLeft <= 10) {inputNums = secondsLeft;FastLED.setBrightness(255);hourColor = CRGB::Red;}
    byte ledNum1 = inputNums / 1000;
    byte ledNum2 = (inputNums - (ledNum1 * 1000)) / 100;
    byte ledNum3 = ((inputNums - (ledNum1 * 1000)) - (ledNum2 * 100)) / 10;
    byte ledNum4 = inputNums % 10;
    
    if (inputNums >= 1000) {
      if (clearOldLeds != 1000){clearOldLeds = 1000; allBlank();}
      displayNumber(ledNum1,6,hourColor);
      displayNumber(ledNum2,4,hourColor);
      displayNumber(ledNum3,2,hourColor);
      displayNumber(ledNum4,0,hourColor); 
    }
    if ((inputNums >= 100) && (inputNums < 1000)) {
      if (clearOldLeds != 100){clearOldLeds = 100; allBlank();}
      displayNumber(ledNum2,5,hourColor);
      displayNumber(ledNum3,3,hourColor);
      displayNumber(ledNum4,1,hourColor); 
    }
    if ((inputNums > 0) && (inputNums < 100)) {
      if (clearOldLeds != 10){clearOldLeds = 10; allBlank();}
      displayNumber(ledNum3,4,hourColor);
      displayNumber(ledNum4,2,hourColor); 
    }
  //  if (mday == 1 && mont == 1 && hour == 0 && mins == 0 && secs <= 3) { happyNewYear();}
  //  if (mday == 7 && mont == 6 && hour == 16 && mins == 10 && secs <= 3) { happyNewYear();}  //for testing
    }
} // end of update clock


void displayDateMode() {  //main date function
  currentMode = 0;
  if(!getLocalTime(&timeinfo)){ 
    Serial.println("Failed to obtain time");
  }
  int mday = timeinfo.tm_mday;
  int mont = timeinfo.tm_mon + 1;
  int year = (timeinfo.tm_year +1900)-2000;
  byte d1 = mday / 10;
  byte d2 = mday % 10;
  byte m1 = mont / 10;
  byte m2 = mont % 10;  
  byte y1 = year / 10;
  byte y2 = year % 10;
  if (DateColorSettings == 0) { monthColor = CRGB(r5_val, g5_val, b5_val);  dayColor = CRGB(r4_val, g4_val, b4_val); separatorColor = CRGB(r6_val, g6_val, b6_val);}
  if (DateColorSettings == 1) { monthColor = CRGB(r5_val, g5_val, b5_val);  dayColor = monthColor; separatorColor = CRGB(r6_val, g6_val, b6_val);}
  if ((DateColorSettings == 2 && pastelColors == 0)  && ( (ColorChangeFrequency == 0 ) || (ColorChangeFrequency == 1 && randomMinPassed == 1) || (ColorChangeFrequency == 2 && randomHourPassed == 1) || (ColorChangeFrequency == 3 && randomDayPassed == 1) || (ColorChangeFrequency == 4 && randomWeekPassed == 1) || (ColorChangeFrequency == 5 && randomMonthPassed == 1) )) { monthColor = CHSV(random(0, 255), 255, 255);  dayColor = CHSV(random(0, 255), 255, 255);separatorColor = CHSV(random(0, 255), 255, 255);}
  if ((DateColorSettings == 2 && pastelColors == 1)  && ( (ColorChangeFrequency == 0 ) || (ColorChangeFrequency == 1 && randomMinPassed == 1) || (ColorChangeFrequency == 2 && randomHourPassed == 1) || (ColorChangeFrequency == 3 && randomDayPassed == 1) || (ColorChangeFrequency == 4 && randomWeekPassed == 1) || (ColorChangeFrequency == 5 && randomMonthPassed == 1) )) { monthColor = CRGB(random(0, 255), random(0, 255), random(0, 255));  dayColor = CRGB(random(0, 255), random(0, 255), random(0, 255));separatorColor = CRGB(random(0, 255), random(0, 255), random(0, 255));}
  if ((DateColorSettings == 3 && pastelColors == 0) && ( (ColorChangeFrequency == 0 ) || (ColorChangeFrequency == 1 && randomMinPassed == 1) || (ColorChangeFrequency == 2 && randomHourPassed == 1) || (ColorChangeFrequency == 3 && randomDayPassed == 1) || (ColorChangeFrequency == 4 && randomWeekPassed == 1) || (ColorChangeFrequency == 5 && randomMonthPassed == 1) )) { monthColor = CHSV(random(0, 255), 255, 255);  dayColor = monthColor; separatorColor = monthColor;}
  if ((DateColorSettings == 3 && pastelColors == 1) && ( (ColorChangeFrequency == 0 ) || (ColorChangeFrequency == 1 && randomMinPassed == 1) || (ColorChangeFrequency == 2 && randomHourPassed == 1) || (ColorChangeFrequency == 3 && randomDayPassed == 1) || (ColorChangeFrequency == 4 && randomWeekPassed == 1) || (ColorChangeFrequency == 5 && randomMonthPassed == 1) )) { monthColor = CRGB(random(0, 255), random(0, 255), random(0, 255));  dayColor = monthColor; separatorColor = monthColor;}

 if ((dateDisplayType == 5)) {    //With "." Separator (MM.DD)
  if (m1 > 0) {
    tinymonthColor = monthColor;
    if (DateColorSettings == 4 && pastelColors == 0){ tinymonthColor = CHSV(random(0, 255), 255, 255); }
    if (DateColorSettings == 4 && pastelColors == 1){ tinymonthColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
      for (int i=(32*LEDS_PER_SEGMENT); i<(33*LEDS_PER_SEGMENT); i++) { LEDs[i] = tinymonthColor;}
    if (DateColorSettings == 4 && pastelColors == 0){ tinymonthColor = CHSV(random(0, 255), 255, 255); }
    if (DateColorSettings == 4 && pastelColors == 1){ tinymonthColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
      for (int i=(33*LEDS_PER_SEGMENT); i<(34*LEDS_PER_SEGMENT); i++) { LEDs[i] = tinymonthColor;}
  } else {for (int i=(32*LEDS_PER_SEGMENT); i<(34*LEDS_PER_SEGMENT); i++) { LEDs[i] = CRGB::Black;}
   }
    displayNumber(m2,5,monthColor);
    displayNumber(d1,2,dayColor);
    displayNumber(d2,0,dayColor); 
    if ((DateColorSettings == 4 ) && pastelColors == 0){ separatorColor = CHSV(random(0, 255), 255, 255); }
    if ((DateColorSettings == 4 ) && pastelColors == 1){ separatorColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
      for (int i=(20*LEDS_PER_SEGMENT); i<(21*LEDS_PER_SEGMENT); i++) { LEDs[i] = separatorColor;}  //separator
  }
  
  if (dateDisplayType == 1) {    //Space-Padded (MMDD)
    if (m1 < 1) { displayNumber(10,6,monthColor); }
    else { displayNumber(m1,6,monthColor); }
      displayNumber(m2,4,monthColor);
      displayNumber(d1,2,dayColor);
      displayNumber(d2,0,dayColor); 
  }
  
  if (dateDisplayType == 2) {    //Center Dates (1MDD)
  if (m1 > 0) {
    tinymonthColor = monthColor;
    if (DateColorSettings == 4 && pastelColors == 0){ tinymonthColor = CHSV(random(0, 255), 255, 255); }
    if (DateColorSettings == 4 && pastelColors == 1){ tinymonthColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
      for (int i=(32*LEDS_PER_SEGMENT); i<(33*LEDS_PER_SEGMENT); i++) { LEDs[i] = tinymonthColor;}
    if (DateColorSettings == 4 && pastelColors == 0){ tinymonthColor = CHSV(random(0, 255), 255, 255); }
    if (DateColorSettings == 4 && pastelColors == 1){ tinymonthColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
      for (int i=(33*LEDS_PER_SEGMENT); i<(34*LEDS_PER_SEGMENT); i++) { LEDs[i] = tinymonthColor;}
  } else {for (int i=(32*LEDS_PER_SEGMENT); i<(34*LEDS_PER_SEGMENT); i++) { LEDs[i] = CRGB::Black;}
   }
      displayNumber(m2,5,monthColor);
      displayNumber(d1,3,dayColor);
      displayNumber(d2,1,dayColor); 
  }
  
  if (dateDisplayType == 0) {    //Zero-Padded (MMDD)
    if (m1 < 1) { displayNumber(0,6,monthColor);}
    else  { displayNumber(m1,6,monthColor);}
     
      displayNumber(m2,4,monthColor);
      displayNumber(d1,2,dayColor);
      displayNumber(d2,0,dayColor); 
  }
  
  if (dateDisplayType == 4) {    //Just Numeric Day (DD)
  if (d1 < 1) {
      displayNumber(d2,3,dayColor);
  } else {
      displayNumber(d1,4,dayColor);
      displayNumber(d2,2,dayColor); 
  }
  }
  
  if (dateDisplayType == 3) {    //Just Day of Week (Sun)
  if (timeinfo.tm_wday == 1)    {displayNumber(78,5,dayColor);displayNumber(80,3,dayColor); displayNumber(79,1,dayColor);}  //mon
  if (timeinfo.tm_wday == 2)  {displayNumber(85,6,dayColor);displayNumber(54,4,dayColor); displayNumber(38,2,dayColor); displayNumber(52,0,dayColor);} //tUES
  if (timeinfo.tm_wday == 3) {displayNumber(88,5,dayColor);displayNumber(38,3,dayColor); displayNumber(69,1,dayColor);}  //wEd
  if (timeinfo.tm_wday == 4)  {displayNumber(85,6,dayColor);displayNumber(73,4,dayColor); displayNumber(86,2,dayColor); displayNumber(83,0,dayColor);}  //thur
  if (timeinfo.tm_wday == 5)    {displayNumber(39,5,dayColor);displayNumber(83,3,dayColor); displayNumber(42,1,dayColor);}  //FrI
  if (timeinfo.tm_wday == 6)  {displayNumber(52,5,dayColor);displayNumber(34,3,dayColor); displayNumber(85,1,dayColor);}  //SAt
  if (timeinfo.tm_wday == 0)    {displayNumber(52,5,dayColor);displayNumber(86,3,dayColor); displayNumber(79,1,dayColor);} //Sun
  }
  
  if (dateDisplayType == 6) {    //Just Year (YYYY)
  displayNumber(2,6,monthColor);
  displayNumber(0,4,monthColor);
  displayNumber(y1,2,dayColor);
  displayNumber(y2,0,dayColor); 
  } 
} // end of update date

void displayTemperatureMode() {   //miain temp function
  static int countFlip = 1;
  currentMode = 0;
  #if HAS_DHT
    float humidTemp = dht.readHumidity();        // read humidity
    float sensorTemp = dht.readTemperature();     // read temperature
    float f = dht.readTemperature(true);
    // float saturationVaporPressure = 6.1078 * pow(10, (7.5 * sensorTemp / (237.3 + sensorTemp)));  // Calculate saturation vapor pressure
    // float vaporPressure = saturationVaporPressure * (humidTemp / 100.0);  // Calculate vapor pressure
    // float absoluteHumidity = 217 * vaporPressure / (273.15 + sensorTemp);  // Calculate absolute humidity
    // float humidityRatio = 0.622 * vaporPressure / (101325 - vaporPressure);
    float heatIndex = -8.784695 + 1.61139411 * sensorTemp + 2.338549 * humidTemp + -0.14611605 * sensorTemp * humidTemp + -0.01230809 * pow(sensorTemp, 2) + -0.01642482 * pow(humidTemp, 2) + 0.00221173 * pow(sensorTemp, 2) * humidTemp + 0.00072546 * sensorTemp * pow(humidTemp, 2) + -0.00000358 * pow(sensorTemp, 2) * pow(humidTemp, 2);
    heatIndex += altitudeLocal * 0.0065;  // Adjust heat index based on altitude
    sensorTemp = heatIndex;
    if (isnan(humidTemp) || isnan(sensorTemp) || isnan(f)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
  #else
    float h = 00.00;        // fake humidity
    float sensorTemp = 00.00;     // fake temperature
  #endif
  float correctedTemp = sensorTemp + temperatureCorrection;
  if (temperatureSymbol == 39) {  correctedTemp = ((sensorTemp * 1.8000) + 32) + temperatureCorrection; }
  byte t1 = 0;
  byte t2 = 0;

  if (temperature_outdoor_enable == true) {
    if (countFlip > 5) {
      correctedTemp = outdoorTemp;
    }
    if (countFlip > 9) {
      countFlip = 0;
    }
    countFlip++;
  }

  int tempDecimal = correctedTemp * 10;
  if (correctedTemp >= 100) {
    int tempHundred = correctedTemp / 10;
    t1 = tempHundred / 10;
    t2 = tempHundred % 10;
    } else {
    t2 = int(correctedTemp) / 10;
    }
  byte t3 = int(correctedTemp) % 10;
  byte t4 = tempDecimal % 10;
  if (tempColorSettings == 0) {tempColor = CRGB(r7_val, g7_val, b7_val);  typeColor = CRGB(r8_val, g8_val, b8_val); degreeColor = CRGB(r9_val, g9_val, b9_val);}
  if (tempColorSettings == 1) {tempColor = CRGB(r7_val, g7_val, b7_val);  typeColor = tempColor; degreeColor = CRGB(r9_val, g9_val, b9_val);}
  if ((tempColorSettings == 2 && pastelColors == 0) && ( (ColorChangeFrequency == 0 ) || (ColorChangeFrequency == 1 && randomMinPassed == 1) || (ColorChangeFrequency == 2 && randomHourPassed == 1) || (ColorChangeFrequency == 3 && randomDayPassed == 1) || (ColorChangeFrequency == 4 && randomWeekPassed == 1) || (ColorChangeFrequency == 5 && randomMonthPassed == 1) )) { tempColor = CHSV(random(0, 255), 255, 255);  typeColor = CHSV(random(0, 255), 255, 255);degreeColor = CHSV(random(0, 255), 255, 255);}
  if ((tempColorSettings == 2 && pastelColors == 1) && ( (ColorChangeFrequency == 0 ) || (ColorChangeFrequency == 1 && randomMinPassed == 1) || (ColorChangeFrequency == 2 && randomHourPassed == 1) || (ColorChangeFrequency == 3 && randomDayPassed == 1) || (ColorChangeFrequency == 4 && randomWeekPassed == 1) || (ColorChangeFrequency == 5 && randomMonthPassed == 1) )) { tempColor = CRGB(random(0, 255), random(0, 255), random(0, 255));  typeColor = CRGB(random(0, 255), random(0, 255), random(0, 255));degreeColor = CRGB(random(0, 255), random(0, 255), random(0, 255));}
  if ((tempColorSettings == 3 && pastelColors == 0) && ( (ColorChangeFrequency == 0 ) || (ColorChangeFrequency == 1 && randomMinPassed == 1) || (ColorChangeFrequency == 2 && randomHourPassed == 1) || (ColorChangeFrequency == 3 && randomDayPassed == 1) || (ColorChangeFrequency == 4 && randomWeekPassed == 1) || (ColorChangeFrequency == 5 && randomMonthPassed == 1) )) { tempColor = CHSV(random(0, 255), 255, 255);  typeColor = tempColor; degreeColor = tempColor;}
  if ((tempColorSettings == 3 && pastelColors == 1) && ( (ColorChangeFrequency == 0 ) || (ColorChangeFrequency == 1 && randomMinPassed == 1) || (ColorChangeFrequency == 2 && randomHourPassed == 1) || (ColorChangeFrequency == 3 && randomDayPassed == 1) || (ColorChangeFrequency == 4 && randomWeekPassed == 1) || (ColorChangeFrequency == 5 && randomMonthPassed == 1) )) { tempColor = CRGB(random(0, 255), random(0, 255), random(0, 255));  typeColor = tempColor; degreeColor = tempColor;}
    
//0-Temperature with Degree and Type (79°F), 1-Temperature with just Type (79 F), 2-Temperature with just Degree (79°), 3-Temperature with Decimal (79.9), 4-Just Temperature (79)
  if ((tempDisplayType == 0) && (correctedTemp < 100)) {  //0-Temperature with Degree and Type (79°F) under 100 only
    displayNumber(t2,6,tempColor);
    displayNumber(t3,4,tempColor);
    displayNumber(26,2,degreeColor);
    displayNumber(temperatureSymbol,0,typeColor);
  }
  if ((tempDisplayType == 1) && (correctedTemp < 100)) {   // 1-Temperature with just Type (79 F) under 100
    displayNumber(t2,5,tempColor);
    displayNumber(t3,3,tempColor);
    displayNumber(temperatureSymbol,1,typeColor);
  }
  if (((tempDisplayType == 1) || (tempDisplayType == 0)) && (correctedTemp >= 100)) {   // 1-Temperature with just Type (79 F) over 100
    displayNumber(t1,6,tempColor);
    displayNumber(t2,4,tempColor);
    displayNumber(t3,2,tempColor);
    displayNumber(temperatureSymbol,0,typeColor);
  }
  if ((tempDisplayType == 2) && (correctedTemp < 100)) {  //2-Temperature with just Degree (79°) under 100
    displayNumber(t2,5,tempColor);
    displayNumber(t3,3,tempColor);
    displayNumber(26,1,degreeColor);
  }
  if ((tempDisplayType == 2) && (correctedTemp >= 100)) {  //2-Temperature with just Degree (79°) over 100
    displayNumber(t1,6,tempColor);
    displayNumber(t2,4,tempColor);
    displayNumber(t3,2,tempColor);
    displayNumber(26,0,degreeColor);
  }
  if ((tempDisplayType == 3) && (correctedTemp < 100)) {   //3-Temperature with Decimal (79.9) under 100
    displayNumber(t2,6,tempColor);
    displayNumber(t3,4,tempColor);
    displayNumber(t4,1,typeColor); 
    if (tempColorSettings == 4 && pastelColors == 0){ degreeColor = CHSV(random(0, 255), 255, 255); }
    if (tempColorSettings == 4 && pastelColors == 1){ degreeColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
    for (int i=(12*LEDS_PER_SEGMENT); i<(13*LEDS_PER_SEGMENT); i++) { LEDs[i] = degreeColor;}  //period goes here
  }
  if ((tempDisplayType == 3) && (correctedTemp >= 100)) {   //3-Temperature with Decimal (79.9) over 100
    displayNumber(t2,5,tempColor);
    displayNumber(t3,3,tempColor);
    displayNumber(t4,0,typeColor);
    tinytempColor = tempColor;
    if (tempColorSettings == 4 && pastelColors == 0){ tinytempColor = CHSV(random(0, 255), 255, 255); }
    if (tempColorSettings == 4 && pastelColors == 1){ tinytempColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
    for (int i=(32*LEDS_PER_SEGMENT); i<(33*LEDS_PER_SEGMENT); i++) { LEDs[i] = tinytempColor;}  //1xx split across 2 for color reasons
    if (tempColorSettings == 4 && pastelColors == 0){ tinytempColor = CHSV(random(0, 255), 255, 255); }
    if (tempColorSettings == 4 && pastelColors == 1){ tinytempColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
    for (int i=(33*LEDS_PER_SEGMENT); i<(34*LEDS_PER_SEGMENT); i++) { LEDs[i] = tinytempColor;}  //1xx split across 2 for color reasons
    if (tempColorSettings == 4 && pastelColors == 0){ degreeColor = CHSV(random(0, 255), 255, 255); }
    if (tempColorSettings == 4 && pastelColors == 1){ degreeColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
    for (int i=(10*LEDS_PER_SEGMENT); i<(11*LEDS_PER_SEGMENT); i++) { LEDs[i] = degreeColor;}  //period goes here
  }
  if ((tempDisplayType == 4) && (correctedTemp < 100)) {  //4-Just Temperature (79) under 100
    displayNumber(t2,4,tempColor);
    displayNumber(t3,2,typeColor);
  }
  if ((tempDisplayType == 4) && (correctedTemp >= 100)) {  //4-Just Temperature (79) over 100
    displayNumber(t1,5,degreeColor);
    displayNumber(t2,3,tempColor);
    displayNumber(t3,1,typeColor);
  } 
}//end of temp settings


void displayHumidityMode() {   //main humidity function
  static int countFlip = 1;
  currentMode = 0;
  #if HAS_DHT
    float sensorHumi = dht.readHumidity();        // read humidity
    float t = dht.readTemperature();     // read temperature
    float f = dht.readTemperature(true);

    if (isnan(sensorHumi) || isnan(t) || isnan(f)) {
      Serial.println(F("Failed to read from DHT sensor!"));
      return;
    }
  #else
    float sensorHumi = 00.00;        // fake humidity
    float t = 00.00;     // fake temperature
  #endif
  byte t1 = 0;
  byte t2 = 0;

  if (humidity_outdoor_enable == true) {
    if (countFlip > 5) {
      sensorHumi = outdoorHumidity;
    }
    if (countFlip > 9) {
      countFlip = 0;
    }
    countFlip++;
  }

  int humiDecimal = sensorHumi * 10;
  if (sensorHumi >= 100) {
    int humiHundred = sensorHumi / 10;
    t1 = humiHundred / 10;
    t2 = humiHundred % 10;
    } else {
    t2 = int(sensorHumi) / 10;
    }
  byte t3 = int(sensorHumi) % 10;
  byte t4 = humiDecimal % 10;
  if (humiColorSettings == 0) {humiColor = CRGB(r10_val, g10_val, b10_val);  symbolColor = CRGB(r11_val, g11_val, b11_val); humiDecimalColor = CRGB(r12_val, g12_val, b12_val);}
  if (humiColorSettings == 1) {humiColor = CRGB(r10_val, g10_val, b10_val);  symbolColor = humiColor; humiDecimalColor = CRGB(r12_val, g12_val, b12_val);}
  if ((humiColorSettings == 2 && pastelColors == 0) && ( (ColorChangeFrequency == 0 ) || (ColorChangeFrequency == 1 && randomMinPassed == 1) || (ColorChangeFrequency == 2 && randomHourPassed == 1) || (ColorChangeFrequency == 3 && randomDayPassed == 1) || (ColorChangeFrequency == 4 && randomWeekPassed == 1) || (ColorChangeFrequency == 5 && randomMonthPassed == 1) )) { humiColor = CHSV(random(0, 255), 255, 255);  symbolColor = CHSV(random(0, 255), 255, 255);humiDecimalColor = CHSV(random(0, 255), 255, 255); }
  if ((humiColorSettings == 2 && pastelColors == 1) && ( (ColorChangeFrequency == 0 ) || (ColorChangeFrequency == 1 && randomMinPassed == 1) || (ColorChangeFrequency == 2 && randomHourPassed == 1) || (ColorChangeFrequency == 3 && randomDayPassed == 1) || (ColorChangeFrequency == 4 && randomWeekPassed == 1) || (ColorChangeFrequency == 5 && randomMonthPassed == 1) )) { humiColor = CRGB(random(0, 255), random(0, 255), random(0, 255));  symbolColor = CRGB(random(0, 255), random(0, 255), random(0, 255));humiDecimalColor = CRGB(random(0, 255), random(0, 255), random(0, 255));}
  if ((humiColorSettings == 3 && pastelColors == 0) && ( (ColorChangeFrequency == 0 ) || (ColorChangeFrequency == 1 && randomMinPassed == 1) || (ColorChangeFrequency == 2 && randomHourPassed == 1) || (ColorChangeFrequency == 3 && randomDayPassed == 1) || (ColorChangeFrequency == 4 && randomWeekPassed == 1) || (ColorChangeFrequency == 5 && randomMonthPassed == 1) )) { humiColor = CHSV(random(0, 255), 255, 255);  symbolColor = humiColor; humiDecimalColor = humiColor;}
  if ((humiColorSettings == 3 && pastelColors == 1) && ( (ColorChangeFrequency == 0 ) || (ColorChangeFrequency == 1 && randomMinPassed == 1) || (ColorChangeFrequency == 2 && randomHourPassed == 1) || (ColorChangeFrequency == 3 && randomDayPassed == 1) || (ColorChangeFrequency == 4 && randomWeekPassed == 1) || (ColorChangeFrequency == 5 && randomMonthPassed == 1) )) { humiColor = CRGB(random(0, 255), random(0, 255), random(0, 255));  symbolColor = humiColor; humiDecimalColor = humiColor;}
  if ((humiDisplayType == 0) && (sensorHumi < 100)) {   //0-Humidity with Symbol (34 H) under 100
    displayNumber(t2,5,humiColor);
    displayNumber(t3,3,humiColor);
    displayNumber(41,1,symbolColor); //H(umitidy
  }
  if ((humiDisplayType == 0) && (sensorHumi >= 100)) {   //0-Humidity with Symbol (34 H) over 100
    displayNumber(t1,6,humiColor);
    displayNumber(t2,4,humiColor);
    displayNumber(t3,2,humiColor);
    displayNumber(41,0,symbolColor); //H(umitidy
  }
  if ((humiDisplayType == 1) && (sensorHumi < 100)) {   //1-Humidity with Decimal (34.9) under 100
    displayNumber(t2,6,humiColor);
    displayNumber(t3,4,humiColor);
    displayNumber(t4,1,symbolColor);
    if (humiColorSettings == 4 && pastelColors == 0){ humiDecimalColor = CHSV(random(0, 255), 255, 255); }
    if (humiColorSettings == 4 && pastelColors == 1){ humiDecimalColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
    for (int i=(12*LEDS_PER_SEGMENT); i<(13*LEDS_PER_SEGMENT); i++) { LEDs[i] = humiDecimalColor;}  //period goes here
  }
  if ((humiDisplayType == 1) && (sensorHumi >= 100)) {   //1-Humidity with Decimal (34.9) over 100
    displayNumber(t2,5,humiColor);
    displayNumber(t3,3,humiColor);
    displayNumber(t4,0,symbolColor);
    tinyhumiColor = humiColor;
    if (humiColorSettings == 4 && pastelColors == 0){ tinyhumiColor = CHSV(random(0, 255), 255, 255); }
    if (humiColorSettings == 4 && pastelColors == 1){ tinyhumiColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
    for (int i=(32*LEDS_PER_SEGMENT); i<(33*LEDS_PER_SEGMENT); i++) { LEDs[i] = tinyhumiColor;}  //1xx split across 2 for color reasons
    if (humiColorSettings == 4 && pastelColors == 0){ tinyhumiColor = CHSV(random(0, 255), 255, 255); }
    if (humiColorSettings == 4 && pastelColors == 1){ tinyhumiColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
    for (int i=(33*LEDS_PER_SEGMENT); i<(34*LEDS_PER_SEGMENT); i++) { LEDs[i] = tinyhumiColor;} //1xx
    if (humiColorSettings == 4 && pastelColors == 0){ humiDecimalColor = CHSV(random(0, 255), 255, 255); }
    if (humiColorSettings == 4 && pastelColors == 1){ humiDecimalColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
    for (int i=(10*LEDS_PER_SEGMENT); i<(11*LEDS_PER_SEGMENT); i++) { LEDs[i] = humiDecimalColor;}   //period goes here
  }
  if ((humiDisplayType == 2) && (sensorHumi < 100)) {  //2-Just Humidity (79) under 100
    displayNumber(t2,4,humiColor);
    displayNumber(t3,2,symbolColor);
  }
  if ((humiDisplayType == 2) && (sensorHumi >= 100)) {  //2-Just Humidity (79) over 100
    displayNumber(t1,5,humiDecimalColor);
    displayNumber(t2,3,humiColor);
    displayNumber(t3,1,symbolColor);
  }
}//end of update humidity


void displayScrollMode(){   //scrollmode for displaying clock things not just text
  currentMode = 0;
  if (realtimeMode == 0) {
    if (!getLocalTime(&timeinfo)){ Serial.println("Failed to obtain time");  }
    char strTime[10];
    char strDate[10];
    char strYear[10];
    char strTemp[25];
    char strOutdoorTemp[25];
    char strHumitidy[10];
    char strIPaddy[20];
    char processedText[255];
    char DOW[10]; 
    int hour = timeinfo.tm_hour;
    int mins = timeinfo.tm_min;
    int mday = timeinfo.tm_mday;
    int mont = timeinfo.tm_mon + 1;
    int year = timeinfo.tm_year +1900;
    #if HAS_DHT
      float h = dht.readHumidity();        // read humidity
      float sensorTemp = dht.readTemperature();     // read temperature
      float f = dht.readTemperature(true);
      if (isnan(h) || isnan(sensorTemp) || isnan(f)) {  Serial.println(F("Failed to read from DHT sensor!"));  return;  }
    #else
      float h = 00.00;        // fake humidity
      float sensorTemp = 00.00;     // fake temperature
    #endif
    float correctedTemp = sensorTemp + temperatureCorrection;

    if (temperatureSymbol == 39) {  correctedTemp = ((sensorTemp * 1.8000) + 32) + temperatureCorrection; }
    if (timeinfo.tm_wday == 1)    {sprintf(DOW,"%s","Mon    ");}
    if (timeinfo.tm_wday == 2)    {sprintf(DOW,"%s","tUES    ");}
    if (timeinfo.tm_wday == 3)    {sprintf(DOW,"%s","WEd    ");}
    if (timeinfo.tm_wday == 4)    {sprintf(DOW,"%s","thur    ");}
    if (timeinfo.tm_wday == 5)    {sprintf(DOW,"%s","FrI    ");}
    if (timeinfo.tm_wday == 6)    {sprintf(DOW,"%s","SAt    ");}
    if (timeinfo.tm_wday == 0)    {sprintf(DOW,"%s","Sun    ");}
    sprintf(strTime, "%.2d%.2d    ", hour, mins);  //1111
    sprintf(strDate, "%.2d-%.2d    ", mont, mday);  //10-22
    sprintf(strYear, "%d    ", year);  //2021
    if (temperature_outdoor_enable == true && outdoorTemp != -500) {
      sprintf(strTemp, "%.1f : %.1f^F     ", correctedTemp, outdoorTemp);
    } else {
      sprintf(strTemp, "%.1f^F    ", correctedTemp );  //98_6 ^F
    }
    if (humidity_outdoor_enable == true && outdoorHumidity != -1) {
      sprintf(strHumitidy, "%.0f : %.0fH    ", h, outdoorHumidity);  //48_6 H
    } else {
      sprintf(strHumitidy, "%.0fH    ", h);  //48_6 H
    }
    sprintf(strIPaddy, "%s", WiFi.localIP().toString().c_str());  //192_168_0_10
    strcpy(processedText, " ");
    if (scrollOptions1 == 1)    {strcat(processedText, strTime);}
    if (scrollOptions2 == 1)    {strcat(processedText, DOW);}
    if (scrollOptions3 == 1)    {strcat(processedText, strDate);}
    if (scrollOptions4 == 1)    {strcat(processedText, strYear);}
    if (scrollOptions5 == 1)    {strcat(processedText, strTemp);}
    if (scrollOptions6 == 1)    {strcat(processedText, strHumitidy);}
    if (scrollOptions7 == 1)    {strcat(processedText, scrollText.c_str()); strcat(processedText, "    ");}
    if (scrollOptions8 == 1)    {strcat(processedText, strIPaddy);}
    if (scrollOptions1 == 0 && scrollOptions2 == 0 && scrollOptions3 == 0 && scrollOptions4 == 0 && scrollOptions5 == 0 && scrollOptions6 == 0 && scrollOptions7 == 0 && scrollOptions8 == 0)    {strcat(processedText, scrollText.c_str());}
    scroll(processedText);  
    }
}

void displayCountdownMode() {     //main countdown function
  if (countdownMilliSeconds == 0 && endCountDownMillis == 0) 
    return;
  unsigned long restMillis = endCountDownMillis - millis();
  unsigned long hours   = ((restMillis / 1000) / 60) / 60;
  unsigned long minutes = (restMillis / 1000) / 60;
  unsigned long seconds = restMillis / 1000;
  int remSeconds = seconds - (minutes * 60);
  int remMinutes = minutes - (hours * 60); 
  byte h1 = hours / 10;
  byte h2 = hours % 10;
  byte m1 = remMinutes / 10;
  byte m2 = remMinutes % 10;  
  byte s1 = remSeconds / 10;
  byte s2 = remSeconds % 10;
  CRGB color = CRGB(cd_r_val, cd_g_val, cd_b_val); 
  if (restMillis <= 10000 && colorchangeCD == 1) {  //red mode last 10 seconds
    color = CRGB::Red;
  }
  if (hours > 0) {   // hh:mm
    displayNumber(h1,6,color); 
    displayNumber(h2,4,color);
    displayNumber(m1,2,color);
    displayNumber(m2,0,color);  
  } else {   // mm:ss   
    displayNumber(m1,6,color);
    displayNumber(m2,4,color);
    displayNumber(s1,2,color);
    displayNumber(s2,0,color);  
  }
  if (hours <= 0 && remMinutes <= 0 && remSeconds <= 0) { //timer ended
    endCountdown();
    countdownMilliSeconds = 0;
    endCountDownMillis = 0;
    return;
  }   
}

void displayStopwatchMode() {     //main stopwatch timer function
  if (millis() >= endCountDownMillis) {  //timer ended
    CountUpMillis = 0;
    endCountDownMillis = 0;
    endCountdown();
    return;
  }
  unsigned long restMillis = millis() - CountUpMillis;
  unsigned long hours   = ((restMillis / 1000) / 60) / 60;
  unsigned long minutes = (restMillis / 1000) / 60;
  unsigned long seconds = restMillis / 1000;
  int remSeconds = seconds - (minutes * 60);
  int remMinutes = minutes - (hours * 60); 
  byte h1 = hours / 10;
  byte h2 = hours % 10;
  byte m1 = remMinutes / 10;
  byte m2 = remMinutes % 10;  
  byte s1 = remSeconds / 10;
  byte s2 = remSeconds % 10;
  CRGB color = CRGB(cd_r_val, cd_g_val, cd_b_val); 
  if (millis() >= (endCountDownMillis - 10000) && colorchangeCD == 1) {  //red mode at last 10 seconds
    color = CRGB::Red;
  } 
  if (hours > 0) { //show hours and minutes
    displayNumber(h1,6,color); 
    displayNumber(h2,4,color);
    displayNumber(m1,2,color);
    displayNumber(m2,0,color); 
  } else { //or show minutes and seconds  
    displayNumber(m1,6,color);
    displayNumber(m2,4,color);
    displayNumber(s1,2,color);
    displayNumber(s2,0,color);  
  }
}

void displayScoreboardMode() {  //main scoreboard function
  currentMode = 0;
  byte sl1 = scoreboardLeft / 10;
  byte sl2 = scoreboardLeft % 10;
  byte sr1 = scoreboardRight / 10;
  byte sr2 = scoreboardRight % 10;
  CRGB scoreboardColorLeft = CRGB(r13_val, g13_val, b13_val);
  CRGB scoreboardColorRight = CRGB(r14_val, g14_val, b14_val);
  displayNumber(sl1,6,scoreboardColorLeft);
  displayNumber(sl2,4,scoreboardColorLeft);
  displayNumber(sr1,2,scoreboardColorRight);
  displayNumber(sr2,0,scoreboardColorRight);
}//end of update scoreboard

void displayLightshowMode() {
  currentMode = 0;
  if (lightshowMode == 0) {Chase();}
  //if (lightshowMode == 1) {Twinkles();}
  //if (lightshowMode == 2) {Rainbow();}
  //if (lightshowMode == 3) {GreenMatrix();}
  //if (lightshowMode == 4) {Chase();}
  //if (lightshowMode == 5) {Fire2021();}
  //if (lightshowMode == 6) {Snake();}
  //if (lightshowMode == 7) {Cylon();}
}

void displayRealtimeMode(){   //main RealtimeModes function, always is running
  #if HAS_SOUNDDETECTOR
    if ( (suspendType == 0 || isAsleep == 0) && clockMode == 9 && realtimeMode == 1) {SpectrumAnalyzer(); }
  #endif
  if ( (suspendType == 0 || isAsleep == 0) && clockMode == 5 && lightshowMode == 1) {EVERY_N_MILLISECONDS(30) {Twinkles();FastLED.show();}}
  if ( (suspendType == 0 || isAsleep == 0) && clockMode == 5 && lightshowMode == 2) {Rainbow();FastLED.show();}
  if ( (suspendType == 0 || isAsleep == 0) && clockMode == 5 && lightshowMode == 3) {EVERY_N_MILLISECONDS(100) {GreenMatrix();}}
  if ( (suspendType == 0 || isAsleep == 0) && clockMode == 5 && lightshowMode == 4) {blueRain();FastLED.show();}
  if ( (suspendType == 0 || isAsleep == 0) && clockMode == 5 && lightshowMode == 5) {EVERY_N_MILLISECONDS(60) {Fire2021();FastLED.show();}}
  if ( (suspendType == 0 || isAsleep == 0) && clockMode == 5 && lightshowMode == 6) {EVERY_N_MILLISECONDS(getSlower) {Snake();FastLED.show();}}
  if ( (suspendType == 0 || isAsleep == 0) && clockMode == 5 && lightshowMode == 7) {EVERY_N_MILLISECONDS(150) {Cylon(); FastLED.show();}}
}//end of RealtimeModes



#if HAS_SOUNDDETECTOR
void rainbowBars(int band, int barHeight) {
  int xStart = LEDS_PER_SEGMENT * 2 * band;
    for (int y = 0; y < SOUNDDETECTOR_BANDS_HEIGHT; y++) {
      if ( barHeight >= y)  
        LEDs[ANALYZER[xStart + y]] = CHSV(band * (255 / SOUNDDETECTOR_BANDS_WIDTH), 255, 255);
      else
        LEDs[ANALYZER[xStart + y]] = CRGB::Black;  
    }
}

void purpleBars(int band, int barHeight) {
  int xStart = LEDS_PER_SEGMENT * 2 * band;
    for (int y = 0; y < SOUNDDETECTOR_BANDS_HEIGHT; y++) {
      if ( barHeight >= y)  
        LEDs[ANALYZER[xStart + y]] = ColorFromPalette(purplePal, y * (255 / (barHeight + 1)));
      else
        LEDs[ANALYZER[xStart + y]] = CRGB::Black;  
    }
}

void fireBars(int band, int barHeight) {
  int xStart = LEDS_PER_SEGMENT * 2 * band;
    for (int y = 0; y < SOUNDDETECTOR_BANDS_HEIGHT; y++) {
      if ( barHeight >= y)  
        LEDs[ANALYZER[xStart + y]] = ColorFromPalette(heaterPal, y * (255 / (barHeight + 1)));
      else
        LEDs[ANALYZER[xStart + y]] = CRGB::Black;  
    }
}

void changingBars(int band, int barHeight) {
  int xStart = LEDS_PER_SEGMENT * 2 * band;
    for (int y = 0; y < SOUNDDETECTOR_BANDS_HEIGHT; y++) {
      if ( barHeight >= y)  
        LEDs[ANALYZER[xStart + y]] = CHSV(y * (255 / (LEDS_PER_SEGMENT*2)) + colorTimer, 255, 255);
      else
        LEDs[ANALYZER[xStart + y]] = CRGB::Black;  
    }
}

void centerBars(int band, int barHeight) {
    int xStart = LEDS_PER_SEGMENT * 2 * band;
    int totalHeight = LEDS_PER_SEGMENT * 2;
    // Clear the entire bar area first
    for (int y = 0; y < totalHeight; y++) {
        LEDs[ANALYZER[xStart + y]] = CRGB::Black;
    }
    if (barHeight % 2 == 0) barHeight--;
    int yStart = (totalHeight - barHeight) / 2;
    // Draw the bar
    for (int y = yStart; y < yStart + barHeight; y++) {
        int colorIndex = constrain((y - yStart) * (255 / barHeight), 0, 255);
        LEDs[ANALYZER[xStart + y]] = ColorFromPalette(heatPal, colorIndex);
    }
}

void whitePeak(int band) {
    int xStart = LEDS_PER_SEGMENT * 2 * band;
    int peakHeight = peak[band];
    // Clear the previous peak LED
    LEDs[ANALYZER[xStart + prevPeak[band]]] = CRGB::Black;
    // Draw the new peak
    LEDs[ANALYZER[xStart + peakHeight]] = CHSV(0, 0, 255);
    // Update the previous peak position
    prevPeak[band] = peakHeight;
}

void outrunPeak(int band) {
    int xStart = LEDS_PER_SEGMENT * 2 * band;
    int peakHeight = peak[band];
    // Clear the previous peak LED
    LEDs[ANALYZER[xStart + prevPeak[band]]] = CRGB::Black;
    // Draw the new peak
    LEDs[ANALYZER[xStart + peakHeight]] = ColorFromPalette(outrunPal, peakHeight * (255 / (LEDS_PER_SEGMENT * 2)));
    // Update the previous peak position
    prevPeak[band] = peakHeight;
}

void waterfall(int band) {
    int totalHeight = LEDS_PER_SEGMENT * 2;
    int xStart = totalHeight * band;
    // Move bar up
    for (int y = totalHeight - 1; y > 0; y--) {
        int indexTo = xStart + y;
        int indexFrom = xStart + y - 1;
        // Check for out-of-bounds access
        if (indexTo >= ANALYZER_SIZE || indexFrom >= ANALYZER_SIZE) {
            Serial.println("Index out of bounds!");
            continue; // Skip to avoid crashing
        }
        LEDs[ANALYZER[indexTo]] = LEDs[ANALYZER[indexFrom]];
    }
    // Map band value to hue
    int hue = constrain(map(SOUNDDETECTOR_bandValues[band], 0, SOUNDDETECTOR_BANDS_HEIGHT, 160, 0), 0, 160);
    // Draw bottom point
    LEDs[ANALYZER[xStart]] = CHSV(hue, 255, 255);
}

void i2sConfig() {
  const i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = SOUNDDETECTOR_SAMPLING_FREQ,
    .bits_per_sample = (i2s_bits_per_sample_t)SOUNDDETECTOR_BITS_PER_SAMPLE,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = SOUNDDETECTOR_SAMPLES,
    .use_apll = false
  };
  i2s_driver_install(SOUNDDETECTOR_I2S_PORT, &i2s_config, 0, NULL);
  Serial.println("I2S Config Setup");
}

void i2sPins() {
  const i2s_pin_config_t pin_config = {
    .bck_io_num = SOUNDDETECTOR_I2S_SCK,
    .ws_io_num = SOUNDDETECTOR_I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = SOUNDDETECTOR_I2S_SD
  };
  i2s_set_pin(SOUNDDETECTOR_I2S_PORT, &pin_config);
  Serial.println("I2S Pins Setup");
}

int i2sWaveformRead() {
    static int16_t sBuffer[8]; // Buffer for I2S data
    size_t bytesIn = 0;
    float mean = 0;
    // Perform I2S read
    esp_err_t result = i2s_read(SOUNDDETECTOR_I2S_PORT, &sBuffer, sizeof(sBuffer), &bytesIn, portMAX_DELAY);
    if (result != ESP_OK) {
        Serial.println("I2S read failed");
        return 0; // Return 0 if read fails
    }
    // Calculate mean value of the samples
    int samples_read = bytesIn / sizeof(int16_t); // Calculate number of samples read
    if (samples_read > 0) {
        for (int i = 0; i < samples_read; ++i) {
         if (sBuffer[i] < 0) { mean += (sBuffer[i]*-1); } else { mean += sBuffer[i]; }
        }
        mean /= samples_read; // Compute average
    }
    // Map the mean value from its range to 30-1023
    // Assuming the mean ranges from -4096 to 4096 (16-bit signed)
    if (mean < 30) mean = 0;
    int scaledValue = map(mean, 30, 4096, 0, 1023);
    scaledValue = constrain(mean, 0, 1023); // Ensure the value is within 0-4095
   //   Serial.println(scaledValue);
    return scaledValue;
}

void readAndProcessAudio() {
    static double vReal[SOUNDDETECTOR_SAMPLES];
    static double vImag[SOUNDDETECTOR_SAMPLES];
    int16_t buffer[SOUNDDETECTOR_SAMPLES];
    size_t bytesIn = 0;
    static double AGC_amp = 1.0; // Initialize AGC amplitude
    const double agcSpeed = 0.1;  // AGC adaptation speed (adjust as needed)
    const double noiseFloor = 500.0; // Set this value to your noise floor threshold

    // Reset SOUNDDETECTOR_bandValues for each loop iteration
    memset(SOUNDDETECTOR_bandValues, 0, sizeof(SOUNDDETECTOR_bandValues));

    // Read audio data (non-blocking)
    i2s_read(SOUNDDETECTOR_I2S_PORT, &buffer, sizeof(buffer), &bytesIn, 0);
    if (bytesIn < sizeof(buffer)) {
        // Not enough data read, fill the rest with zeros
        memset(&buffer[bytesIn / sizeof(int16_t)], 0, sizeof(buffer) - bytesIn);
    }

    // Convert samples to double
    for (int i = 0; i < SOUNDDETECTOR_SAMPLES; i++) {
        vReal[i] = (double)buffer[i];
        vImag[i] = 0.0;
    }

    // Perform FFT
    FFT.Windowing(vReal, SOUNDDETECTOR_SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, SOUNDDETECTOR_SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, SOUNDDETECTOR_SAMPLES);

    // Apply dynamic range compression and map FFT bins to frequency bands
    double maxCompressedMagnitude = 0.0;

    for (int i = 1; i < (SOUNDDETECTOR_SAMPLES / 2); i++) {
        int bandIndex = -1;

        if (i >= 1 && i <= 2) bandIndex = 0;          // 62.5 Hz - 125 Hz
        else if (i >= 3 && i <= 4) bandIndex = 1;     // 187.5 Hz - 250 Hz
        else if (i >= 5 && i <= 6) bandIndex = 2;     // 312.5 Hz - 375 Hz
        else if (i >= 7 && i <= 8) bandIndex = 3;     // 437.5 Hz - 500 Hz
        else if (i >= 9 && i <= 16) bandIndex = 4;    // 562.5 Hz - 1,000 Hz
        else if (i >= 17 && i <= 52) bandIndex = 5;   // 1,062.5 Hz - 2,000 Hz
        else if (i >= 53 && i <= 56) bandIndex = 6;   // 2,062.5 Hz - 3,000 Hz
        else if (i >= 57 && i <= 100) bandIndex = 7;   // 3,062.5 Hz - 4,000 Hz

        if (bandIndex != -1) {
            // Apply noise floor threshold
            if (vReal[i] < noiseFloor) {
                vReal[i] = 0.0; // Ignore values below the noise floor
            } else {
                // Apply dynamic range compression using logarithmic scaling
                double magnitude = vReal[i];
                double compressedMagnitude = log10(magnitude - noiseFloor + 1.0); // +1 to avoid log(0)

                // Sum the compressed magnitudes into the band
                SOUNDDETECTOR_bandValues[bandIndex] += compressedMagnitude;

                // Update maxCompressedMagnitude
                if (compressedMagnitude > maxCompressedMagnitude) {
                    maxCompressedMagnitude = compressedMagnitude;
                }
            }
        }
    }

    // Update AGC amplitude towards the current maximum compressed magnitude
    AGC_amp = (1.0 - agcSpeed) * AGC_amp + agcSpeed * maxCompressedMagnitude;

    // Map compressed magnitudes to bar heights
    for (int band = 0; band < SOUNDDETECTOR_BANDS_WIDTH; band++) {
        double magnitude = SOUNDDETECTOR_bandValues[band];

        // Scale using AGC amplitude
        double scaledMagnitude = magnitude / (AGC_amp + 1e-6); // Avoid division by zero

        // Map to bar height
        int barHeight = (int)(scaledMagnitude * SOUNDDETECTOR_BANDS_HEIGHT);

        // Constrain barHeight
        barHeight = constrain(barHeight, 0, SOUNDDETECTOR_BANDS_HEIGHT);

        // Store barHeight in SOUNDDETECTOR_bandValues[band]
        SOUNDDETECTOR_bandValues[band] = barHeight;
    }
}

void SpectrumAnalyzer() {    //mostly from github.com/justcallmekoko/Arduino-FastLED-Music-Visualizer/blob/master/music_visualizer.ino
  currentMode = 0;
  if (spectrumMode < 12) {
	  const TProgmemRGBPalette16 FireColors = {0xFFFFCC, 0xFFFF99, 0xFFFF66, 0xFFFF33, 0xFFFF00, 0xFFCC00, 0xFF9900, 0xFF6600, 0xFF3300, 0xFF3300, 0xFF0000, 0xCC0000, 0x990000, 0x660000, 0x330000, 0x110000};
	  const TProgmemRGBPalette16 FireColors2 = {0xFFFF99, 0xFFFF66, 0xFFFF33, 0xFFFF00, 0xFFCC00, 0xFF9900, 0xFF6600, 0xFF3300, 0xFF3300, 0xFF0000, 0xCC0000, 0x990000, 0x660000, 0x330000, 0x110000};
	  const TProgmemRGBPalette16 FireColors3 = {0xFFFF66, 0xFFFF33, 0xFFFF00, 0xFFCC00, 0xFF9900, 0xFF6600, 0xFF3300, 0xFF3300, 0xFF0000, 0xCC0000, 0x990000, 0x660000, 0x330000, 0x110000};
	  int audio_input = i2sWaveformRead();
	  if (audio_input > 0) {
   //   Serial.println("spectrum running");
		SOUNDDETECTOR_pre_react = (SPECTRUM_PIXELS * audio_input) / 1023L; // TRANSLATE AUDIO LEVEL TO NUMBER OF LEDs
		if (SOUNDDETECTOR_pre_react > SOUNDDETECTOR_react) // ONLY ADJUST LEVEL OF LED IF LEVEL HIGHER THAN CURRENT LEVEL
		  SOUNDDETECTOR_react = SOUNDDETECTOR_pre_react;
	   }
	  for(int i = SPECTRUM_PIXELS - 1; i >= 0; i--) {
      int fake =  i * LEDS_PER_SEGMENT;
      int fireChoice = random(4);
      if (i < SOUNDDETECTOR_react)
      for (byte s=0; s<LEDS_PER_SEGMENT; s++ ){              // 7 LEDs per segment
        if (spectrumColorSettings == 0) { spectrumColor = CRGB(r15_val, g15_val, b15_val); }
        if (spectrumColorSettings == 1) { spectrumColor = CHSV((255/SPECTRUM_PIXELS)*i, 255, 255);}
        if (spectrumColorSettings == 2) { spectrumColor = colorWheel((i * 256 / 50 + colorWheelPosition) % 256);}
        if (spectrumColorSettings == 3 && fireChoice >= 2) { spectrumColor = ColorFromPalette( FireColors, (255/SPECTRUM_PIXELS)*i, 255, LINEARBLEND);}
        if (spectrumColorSettings == 3 && fireChoice == 0) { spectrumColor = ColorFromPalette( FireColors2, (255/SPECTRUM_PIXELS)*i, 255, LINEARBLEND);}
        if (spectrumColorSettings == 3 && fireChoice == 1) { spectrumColor = ColorFromPalette( FireColors3, (255/SPECTRUM_PIXELS)*i, 255, LINEARBLEND);}
        if (spectrumColorSettings == 4) { spectrumColor = ColorFromPalette( OceanColors_p, (255/SPECTRUM_PIXELS)*i, 255, LINEARBLEND);}
        if (spectrumColorSettings == 5) { spectrumColor = ColorFromPalette( ForestColors_p, (255/SPECTRUM_PIXELS)*i, 255, LINEARBLEND);}
        if (spectrumColorSettings == 6) { spectrumColor = colorWheel2(((255) + colorWheelPositionTwo) % 256);}
        if (spectrumMode == 0) {LEDs[FAKE_LEDs_C_BMUP[s+((fake))]] = spectrumColor;}
        if (spectrumMode == 1) {LEDs[FAKE_LEDs_C_CMOT[s+((fake))]] = spectrumColor;}
        if (spectrumMode == 2) {LEDs[FAKE_LEDs_C_BLTR[s+((fake))]] = spectrumColor;}
        if (spectrumMode == 3) {LEDs[FAKE_LEDs_C_TLBR[s+((fake))]] = spectrumColor;}
        if (spectrumMode == 4) {LEDs[FAKE_LEDs_C_VERT[s+((fake))]] = spectrumColor;}
        if (spectrumMode == 5) {LEDs[FAKE_LEDs_C_TMDN[s+((fake))]] = spectrumColor;}
        if (spectrumMode == 6) {LEDs[FAKE_LEDs_C_CSIN[s+((fake))]] = spectrumColor;}
        if (spectrumMode == 7) {LEDs[FAKE_LEDs_C_BRTL[s+((fake))]] = spectrumColor;}
        if (spectrumMode == 8) {LEDs[FAKE_LEDs_C_TRBL[s+((fake))]] = spectrumColor;}
        if (spectrumMode == 9) {LEDs[FAKE_LEDs_C_OUTS[s+((fake))]] = spectrumColor;}
        if (spectrumMode == 10) {LEDs[FAKE_LEDs_C_VERT2[s+((fake))]] = spectrumColor;}
        if (spectrumMode == 11) {LEDs[FAKE_LEDs_C_OUTS2[s+((fake))]] = spectrumColor;}
      }
      else
      for (byte s=0; s<LEDS_PER_SEGMENT; s++ ){              // 7 LEDs per segment
        if (spectrumBackgroundSettings == 0) { spectrumBackground = CRGB(r17_val, g17_val, b17_val); }
        if (spectrumBackgroundSettings == 1) { spectrumBackground = colorWheel((i * 256 / 50 + colorWheelPosition) % 256);}
        if (spectrumBackgroundSettings == 2) { spectrumBackground = colorWheel2(((255) + colorWheelPositionTwo) % 256);}
        if (spectrumMode == 0) {LEDs[FAKE_LEDs_C_BMUP[s+((fake))]] = spectrumBackground;}
        if (spectrumMode == 1) {LEDs[FAKE_LEDs_C_CMOT[s+((fake))]] = spectrumBackground;}
        if (spectrumMode == 2) {LEDs[FAKE_LEDs_C_BLTR[s+((fake))]] = spectrumBackground;}
        if (spectrumMode == 3) {LEDs[FAKE_LEDs_C_TLBR[s+((fake))]] = spectrumBackground;}
        if (spectrumMode == 4) {LEDs[FAKE_LEDs_C_VERT[s+((fake))]] = spectrumBackground;}
        if (spectrumMode == 5) {LEDs[FAKE_LEDs_C_TMDN[s+((fake))]] = spectrumBackground;}
        if (spectrumMode == 6) {LEDs[FAKE_LEDs_C_CSIN[s+((fake))]] = spectrumBackground;}
        if (spectrumMode == 7) {LEDs[FAKE_LEDs_C_BRTL[s+((fake))]] = spectrumBackground;}
        if (spectrumMode == 8) {LEDs[FAKE_LEDs_C_TRBL[s+((fake))]] = spectrumBackground;}
        if (spectrumMode == 9) {LEDs[FAKE_LEDs_C_OUTS[s+((fake))]] = spectrumBackground;}
        if (spectrumMode == 10) {LEDs[FAKE_LEDs_C_VERT2[s+((fake))]] = spectrumBackground;}
        if (spectrumMode == 11) {LEDs[FAKE_LEDs_C_OUTS2[s+((fake))]] = spectrumBackground;}
      }    
	  }
	  FastLED.show();                              // Increment the Hue to get the Rainbow
	  colorWheelPosition = colorWheelPosition - colorWheelSpeed; // SPEED OF COLOR WHEEL
	  if (colorWheelPosition < 0) // RESET COLOR WHEEL
		colorWheelPosition = 255;
	  SOUNDDETECTOR_decay_check++;
	  if (SOUNDDETECTOR_decay_check > SOUNDDETECTOR_decay) {
      SOUNDDETECTOR_decay_check = 0;
      if (SOUNDDETECTOR_react > 0)
        SOUNDDETECTOR_react--;
	  }
	}// end spectrumMode < 12
  else {

    // Reset SOUNDDETECTOR_bandValues[]
    for (int i = 0; i<SOUNDDETECTOR_BANDS_WIDTH; i++){
      SOUNDDETECTOR_bandValues[i] = 0;
    }

    readAndProcessAudio();

    // Process the FFT data into bar heights
    for (byte band = 0; band < SOUNDDETECTOR_BANDS_WIDTH; band++) {

      // Scale the bars for the display
      int barHeight = SOUNDDETECTOR_bandValues[band] ;     if (barHeight > SOUNDDETECTOR_BANDS_HEIGHT) {
        SOUNDDETECTOR_Amplitude += ((barHeight - SOUNDDETECTOR_BANDS_HEIGHT) /8);
        barHeight = SOUNDDETECTOR_BANDS_HEIGHT;
      }

      // Small amount of averaging between frames
      barHeight = ((oldBarHeights[band] * 1) + barHeight) / 2;

      // Move peak up
      if (barHeight > peak[band]) {
        peak[band] = min(SOUNDDETECTOR_BANDS_HEIGHT, barHeight);
      }

      if (spectrumMode < 12 + 7)
		    buttonPushCounter = spectrumMode - 12;

      // Draw bars
      switch (buttonPushCounter) {
        case 0:
          rainbowBars(band, barHeight);
          break;
        case 1:
          // No bars on this one
          break;
        case 2:
          purpleBars(band, barHeight);
          break;
        case 3:
          centerBars(band, barHeight);
          break;
        case 4:
          changingBars(band, barHeight);
          break;
        case 5:
          fireBars(band, barHeight);
          break;
        case 6:
          waterfall(band);
          break;
      }

      // Draw peaks
      switch (buttonPushCounter) {
        case 0:
          whitePeak(band);
          break;
        case 1:
          outrunPeak(band);
          break;
        case 2:
          whitePeak(band);
          break;
        case 3:
          // No peaks
          break;
        case 4:
          // No peaks
          break;
        case 5:
          // No peaks
          break;
        case 6:
          // No peaks
          break;
      }

      // Save oldBarHeights for averaging later
      oldBarHeights[band] = barHeight;
    }

	// Decay peak
	EVERY_N_MILLISECONDS(60) {
		for (byte band = 0; band < SOUNDDETECTOR_BANDS_WIDTH; band++) {
			if (peak[band] > 0) {
				peak[band] -= 1;
			} else {
				peak[band] = 0; // Ensure it doesn't go negative
			}
		}
	}
/*
	Serial.print("Peak values: ");
	for (byte band = 0; band < SOUNDDETECTOR_BANDS_WIDTH; band++) {
		Serial.print(peak[band]);
		Serial.print(" ");
	}
	Serial.println();
*/
    // Used in some of the patterns
    EVERY_N_MILLISECONDS(10) {
      colorTimer++;
    }

    EVERY_N_SECONDS(1) {
      if (SOUNDDETECTOR_Amplitude > 500) {
        SOUNDDETECTOR_Amplitude -= (SOUNDDETECTOR_Amplitude /150);
      }
    }

    EVERY_N_SECONDS(10) {
		// Auto Switch mode
      if (spectrumMode >= 12 + 7) buttonPushCounter = (buttonPushCounter + 1) % 7;
    }

    FastLED.show();
  } // end else spectrumMode > 12
}
#endif

void endCountdown() {  //countdown timer has reached 0, sound alarm and flash End for 30 seconds
  //  Serial.println("endcountdown function");
          Serial.println("timer ends");
  breakOutSet = 0;
  FastLED.setBrightness(255);
  CRGB color = CRGB::Red; 
  allBlank();
  #if HAS_BUZZER
  if (useAudibleAlarm == 1) { //alarm, ok
    if(!getLocalTime(&timeinfo)){ Serial.println("Failed to obtain time"); }
    int mday = timeinfo.tm_mday;
    int mont = timeinfo.tm_mon + 1;
    String songName = SONGS[specialAudibleAlarm];  // Get the song name as a String
    Serial.print("songName ");
    Serial.println(songName);
    songName.toCharArray(songTaskbuffer, songTaskbufferSize);
    xQueueSend(jobQueue, songTaskbuffer, (TickType_t)0);
    color = colorWheel((205 / 50 + colorWheelPosition) % 256);
    displayNumber(38,5,color);  //E 38
    displayNumber(79,3,color);  //n 79
    displayNumber(69,1,color);  //d 69
    FastLED.show();
    colorWheelPosition++;
    server.handleClient();   
    } else {  //no alarm, ok, flash lights
    for (int i=0; i<1000 && !breakOutSet; i++) {
      color = colorWheel((205 / 50 + colorWheelPosition) % 256);
      displayNumber(38,5,color);  //E 38
      displayNumber(79,3,color);  //n 79
      displayNumber(69,1,color);  //d 69
      FastLED.show();
      colorWheelPosition++;
      server.handleClient();   
    }
      }
  #else
    for (int i=0; i<1000 && !breakOutSet; i++) {
      color = colorWheel((205 / 50 + colorWheelPosition) % 256);
      displayNumber(38,5,color);  //E 38
      displayNumber(79,3,color);  //n 79
      displayNumber(69,1,color);  //d 69
      FastLED.show();
      colorWheelPosition++;
      server.handleClient();   
    }
  #endif
  clockMode = currentMode; 
  realtimeMode = currentReal;
  if (!breakOutSet) {scroll("tIMEr Ended      tIMEr Ended");}
  allBlank(); 
}

CRGB colorWheel(int pos) {   //color wheel for mostly spectrum analyzer
  CRGB color (0,0,0);
  if(pos < 85) {
    color.g = 0;
    color.r = ((float)pos / 85.0f) * 255.0f;
    color.b = 255 - color.r;
  } else if(pos < 170) {
    color.g = ((float)(pos - 85) / 85.0f) * 255.0f;
    color.r = 255 - color.g;
    color.b = 0;
  } else if(pos < 256) {
    color.b = ((float)(pos - 170) / 85.0f) * 255.0f;
    color.g = 255 - color.b;
    color.r = 1;
  }
  return color;
}

CRGB colorWheel2(int pos) {   //color wheel for things not the spectrum analyzer
  CRGB color (0,0,0);
  if(pos < 85) {
    color.g = 0;
    color.r = ((float)pos / 85.0f) * 255.0f;
    color.b = 255 - color.r;
  } else if(pos < 170) {
    color.g = ((float)(pos - 85) / 85.0f) * 255.0f;
    color.r = 255 - color.g;
    color.b = 0;
  } else if(pos < 256) {
    color.b = ((float)(pos - 170) / 85.0f) * 255.0f;
    color.g = 255 - color.b;
    color.r = 1;
  }
  return color;
}

void scroll(String IncomingString) {    //main scrolling function
  breakOutSet = 0;
  scrollColor = CRGB(r16_val, g16_val, b16_val);
  if (IncomingString.length() > 256 ) { IncomingString = "ArE U A HAckEr"; }   //too big?
  char SentenceArray[IncomingString.length() + 1];
  IncomingString.toCharArray(SentenceArray, IncomingString.length()+1);
  uint16_t TranslatedSentence[(((IncomingString.length()*2)+6)+6)+1];
  TranslatedSentence[0] = 96;    //pad first 6 at front with a marker
  TranslatedSentence[1] = 96;    
  TranslatedSentence[2] = 96;    
  TranslatedSentence[3] = 96;    
  TranslatedSentence[4] = 96;    
  TranslatedSentence[5] = 96;    
  TranslatedSentence[(((IncomingString.length()*2)+6)+0)] = 96;  //pad last 6 at back with a marker
  TranslatedSentence[(((IncomingString.length()*2)+6)+1)] = 96;
  TranslatedSentence[(((IncomingString.length()*2)+6)+2)] = 96;
  TranslatedSentence[(((IncomingString.length()*2)+6)+3)] = 96;
  TranslatedSentence[(((IncomingString.length()*2)+6)+4)] = 96;
  TranslatedSentence[(((IncomingString.length()*2)+6)+5)] = 96;
  for (uint16_t realposition=0; realposition<(IncomingString.length()); realposition++){   //run string through translation
    char SentenceLetter = SentenceArray[realposition];  
    uint16_t LetterNumber = 10;  //for all unknown characters
    if( SentenceLetter == '0') { LetterNumber = 0; }
    if( SentenceLetter == '1') { LetterNumber = 1; }
    if( SentenceLetter == '2') { LetterNumber = 2; }
    if( SentenceLetter == '3') { LetterNumber = 3; }
    if( SentenceLetter == '4') { LetterNumber = 4; }
    if( SentenceLetter == '5') { LetterNumber = 5; }
    if( SentenceLetter == '6') { LetterNumber = 6; }
    if( SentenceLetter == '7') { LetterNumber = 7; }
    if( SentenceLetter == '8') { LetterNumber = 8; }
    if( SentenceLetter == '9') { LetterNumber = 9; }
    if( SentenceLetter == ' ') { LetterNumber = 10; }
    if( SentenceLetter == '`') { LetterNumber = 17; }
    if( SentenceLetter == 'A') { LetterNumber = 34; }
    if( SentenceLetter == 'B') { LetterNumber = 35; }
    if( SentenceLetter == 'C') { LetterNumber = 36; }
    if( SentenceLetter == 'D') { LetterNumber = 37; }
    if( SentenceLetter == 'E') { LetterNumber = 38; }
    if( SentenceLetter == 'F') { LetterNumber = 39; }
    if( SentenceLetter == 'G') { LetterNumber = 40; }
    if( SentenceLetter == 'H') { LetterNumber = 41; }
    if( SentenceLetter == 'I') { LetterNumber = 42; }
    if( SentenceLetter == 'J') { LetterNumber = 43; }
    if( SentenceLetter == 'K') { LetterNumber = 44; }
    if( SentenceLetter == 'L') { LetterNumber = 45; }
    if( SentenceLetter == 'M') { LetterNumber = 46; }
    if( SentenceLetter == 'N') { LetterNumber = 47; }
    if( SentenceLetter == 'O') { LetterNumber = 48; }
    if( SentenceLetter == 'P') { LetterNumber = 49; }
    if( SentenceLetter == 'Q') { LetterNumber = 50; }
    if( SentenceLetter == 'R') { LetterNumber = 51; }
    if( SentenceLetter == 'S') { LetterNumber = 52; }
    if( SentenceLetter == 'T') { LetterNumber = 53; }
    if( SentenceLetter == 'U') { LetterNumber = 54; }
    if( SentenceLetter == 'V') { LetterNumber = 55; }
    if( SentenceLetter == 'W') { LetterNumber = 56; }
    if( SentenceLetter == 'X') { LetterNumber = 57; }
    if( SentenceLetter == 'Y') { LetterNumber = 58; }
    if( SentenceLetter == 'Z') { LetterNumber = 59; }
    if( SentenceLetter == 'a') { LetterNumber = 66; }
    if( SentenceLetter == 'b') { LetterNumber = 67; }
    if( SentenceLetter == 'c') { LetterNumber = 68; }
    if( SentenceLetter == 'd') { LetterNumber = 69; }
    if( SentenceLetter == 'e') { LetterNumber = 70; }
    if( SentenceLetter == 'f') { LetterNumber = 71; }
    if( SentenceLetter == 'g') { LetterNumber = 72; }
    if( SentenceLetter == 'h') { LetterNumber = 73; }
    if( SentenceLetter == 'i') { LetterNumber = 74; }
    if( SentenceLetter == 'j') { LetterNumber = 75; }
    if( SentenceLetter == 'k') { LetterNumber = 76; }
    if( SentenceLetter == 'l') { LetterNumber = 77; }
    if( SentenceLetter == 'm') { LetterNumber = 78; }
    if( SentenceLetter == 'n') { LetterNumber = 79; }
    if( SentenceLetter == 'o') { LetterNumber = 80; }
    if( SentenceLetter == 'p') { LetterNumber = 81; }
    if( SentenceLetter == 'q') { LetterNumber = 82; }
    if( SentenceLetter == 'r') { LetterNumber = 83; }
    if( SentenceLetter == 's') { LetterNumber = 84; }
    if( SentenceLetter == 't') { LetterNumber = 85; }
    if( SentenceLetter == 'u') { LetterNumber = 86; }
    if( SentenceLetter == 'v') { LetterNumber = 87; }
    if( SentenceLetter == 'w') { LetterNumber = 88; }
    if( SentenceLetter == 'x') { LetterNumber = 89; }
    if( SentenceLetter == 'y') { LetterNumber = 90; }
    if( SentenceLetter == 'z') { LetterNumber = 91; }
    if( SentenceLetter == ',') { LetterNumber = 22; }
    if( SentenceLetter == '-') { LetterNumber = 23; }
    if( SentenceLetter == '.') { LetterNumber = 24; }
    if( SentenceLetter == ':') { LetterNumber = 27; }
    if( SentenceLetter == '^') { LetterNumber = 26; }
    if( SentenceLetter == '\'') { LetterNumber = 17; }
    if( SentenceLetter == '%') { LetterNumber = 15; }
    TranslatedSentence[(realposition*2)+6] = LetterNumber;  //letter starting at position 7 
    TranslatedSentence[(realposition*2)+7] = 96; //add padding to next position because fake digit shares a leg with adjacent real ones and can't be on at the same time
  }  
  if (scrollColorSettings == 0){ scrollColor = CRGB(r16_val, g16_val, b16_val); }
  if (scrollColorSettings == 1 && pastelColors == 0){ scrollColor = CHSV(random(0, 255), 255, 255); }
  if (scrollColorSettings == 1 && pastelColors == 1){ scrollColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
  for (uint16_t finalposition=0; finalposition<((IncomingString.length()*2)+6); finalposition++){  //count to end of padded array
    for (int i=0; i<SEGMENTS_LEDS; i++) { LEDs[i] = CRGB::Black;  }    //clear 
    if( TranslatedSentence[finalposition] != 96) { displayNumber(TranslatedSentence[finalposition],6,scrollColor); }
    if( TranslatedSentence[finalposition+1] != 96) { displayNumber(TranslatedSentence[finalposition+1],5,scrollColor); }
    if( TranslatedSentence[finalposition+2] != 96) { displayNumber(TranslatedSentence[finalposition+2],4,scrollColor); }
    if( TranslatedSentence[finalposition+3] != 96) { displayNumber(TranslatedSentence[finalposition+3],3,scrollColor); }
    if( TranslatedSentence[finalposition+4] != 96) { displayNumber(TranslatedSentence[finalposition+4],2,scrollColor); }
    if( TranslatedSentence[finalposition+5] != 96) { displayNumber(TranslatedSentence[finalposition+5],1,scrollColor); }
    if( TranslatedSentence[finalposition+6] != 96) { displayNumber(TranslatedSentence[finalposition+6],0,scrollColor); }
    FastLED.show();
    if ( finalposition < ((IncomingString.length()*2)+6)) {
      for (int i=0; i<400 && !breakOutSet; i++) {
        server.handleClient();   
      }    //slow down on non-padded parts with web server polls
    } 
  }
  allBlank();
  allBlank();
} //end of scroll function


void printLocalTime() {  //what could this do
  #if HAS_RTC
    DateTime now = rtc.now();
    char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    char monthsOfTheYear[12][12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    if (!rtc.lostPower()) {
      Serial.print("DS3231 Time: ");
      Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
      Serial.print(", ");
      Serial.print(monthsOfTheYear[now.month()-1]);
      Serial.print(" ");
      Serial.print(now.day(), DEC);
      Serial.print(" ");
      Serial.print(now.year(), DEC);
      Serial.print(' ');
      Serial.print(now.hour(), DEC);
      Serial.print(':');
      Serial.print(now.minute(), DEC);
      Serial.print(':');
      Serial.print(now.second(), DEC);
      Serial.println();
    }
  #endif
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "Local Time: %A, %B %d %Y %H:%M:%S");
}


void checkSleepTimer(){  //controls suspend mode
  #if HAS_BUZZER
  if(!rtttl::isPlaying()) {  // don't allow chimes to keep it awake. 
    if (suspendType != 0) {sleepTimerCurrent++;}  //sleep enabled? add one to timer
      #if HAS_SOUNDDETECTOR
 /*       int audio_input1 = i2sWaveformRead(); 
        if (audio_input1 > 100) {sleepTimerCurrent = 0; isAsleep = 0;} //try it with the real sensor, the digital one was tripping false positives just as much
        int audio_input2 = i2sWaveformRead(); 
        if (audio_input2 > 100) {sleepTimerCurrent = 0; isAsleep = 0;} //try it with the real sensor, the digital one was tripping false positives just as much
        int audio_input3 = i2sWaveformRead(); 
        if (audio_input3 > 100) {sleepTimerCurrent = 0; isAsleep = 0;} //try it with the real sensor, the digital one was tripping false positives just as much
        int audio_input4 = i2sWaveformRead(); 
        if (audio_input4 > 100) {sleepTimerCurrent = 0; isAsleep = 0;} //try it with the real sensor, the digital one was tripping false positives just as much
        int audio_input5 = i2sWaveformRead();
        if (audio_input5 > 100) {sleepTimerCurrent = 0; isAsleep = 0;} //try it with the real sensor, the digital one was tripping false positives just as much
        */
        SOUNDDETECTOR_averageAudioInput = i2sWaveformRead();
        if (SOUNDDETECTOR_averageAudioInput > 50) {sleepTimerCurrent = 0; isAsleep = 0;} //try it with the real sensor, the digital one was tripping false positives just as much
      #endif
    if ((suspendType != 0) && sleepTimerCurrent >= (suspendFrequency * 60)) {sleepTimerCurrent = 0; isAsleep = 1; allBlank(); }  //sleep enabled, been some amount of time, go to sleep
  }
  #else
    #if HAS_SOUNDDETECTOR
      if (suspendType != 0) {sleepTimerCurrent++;}  //sleep enabled? add one to timer
      int audio_input1 = i2sWaveformRead();
      if (audio_input1 > 100) {sleepTimerCurrent = 0; isAsleep = 0;} //try it with the real sensor, the digital one was tripping false positives just as much
      int audio_input2 = i2sWaveformRead();
      if (audio_input2 > 100) {sleepTimerCurrent = 0; isAsleep = 0;} //try it with the real sensor, the digital one was tripping false positives just as much
      int audio_input3 = i2sWaveformRead(); 
      if (audio_input3 > 100) {sleepTimerCurrent = 0; isAsleep = 0;} //try it with the real sensor, the digital one was tripping false positives just as much
      int audio_input4 = i2sWaveformRead();
      if (audio_input4 > 100) {sleepTimerCurrent = 0; isAsleep = 0;} //try it with the real sensor, the digital one was tripping false positives just as much
      int audio_input5 = i2sWaveformRead(); 
      if (audio_input5 > 100) {sleepTimerCurrent = 0; isAsleep = 0;} //try it with the real sensor, the digital one was tripping false positives just as much
      SOUNDDETECTOR_averageAudioInput = (audio_input1 + audio_input2 + audio_input3 + audio_input4 + audio_input5) / 5;
      if (SOUNDDETECTOR_averageAudioInput > 50) {sleepTimerCurrent = 0; isAsleep = 0;} //try it with the real sensor, the digital one was tripping false positives just as much
      if ((suspendType != 0) && sleepTimerCurrent >= (suspendFrequency * 60)) {sleepTimerCurrent = 0; isAsleep = 1; allBlank(); }  //sleep enabled, been some amount of time, go to sleep
    #endif
  #endif
}




void GetBrightnessLevel() {   //samples the photoresister and set brightness
#if HAS_PHOTOSENSOR
  photoresisterReadings[readIndex] = analogRead(PHOTORESISTER_PIN); //get an average light level from previouse set of samples
  readIndex = readIndex + 1; // advance to the next position in the array:
  if (readIndex >= PHOTO_SAMPLES) {   //reset count of readings even N times around
    readIndex = 0;
  }
  int sumBrightness = 0;
  for (int i=0; i < PHOTO_SAMPLES; i++)
    {
     sumBrightness += photoresisterReadings[i];  // add all the current readings together
    }
 //   Serial.println(analogRead(PHOTORESISTER_PIN));
  // lightSensorValue = multiMap<int>(sumBrightness / PHOTO_SAMPLES, photo_in, photo_out, PHOTO_SIZE);
  //lightSensorValue = 255 - (((sumBrightness / PHOTO_SAMPLES) * (254)) / 4095);  //linear conversion of 0-4095 to 255 to 40, after getting the average of the readings
  lightSensorValue = 275 - (((sumBrightness / PHOTO_SAMPLES) * (245)) / 4095);  //linear conversion of 0-4095 to 305 to 10 (a little brighter), after getting the average of the readings
  if (lightSensorValue > 255) {lightSensorValue = 255;} //constrain brightness
    if (brightness != 10) {  //if not set to auto-dim just use user set brightness
      FastLED.setBrightness(brightness);
    } else if (brightness == 10) {  //auto-dim use the value from above
      FastLED.setBrightness(lightSensorValue);     
    } 
#else
  FastLED.setBrightness(10); //SET TO MIN LEVEL IF NO SENSOR
#endif
}  // end of auto-brightness




void displayNumber(uint16_t number, byte segment, CRGB color) {   //main digit rendering (except when scrolling)
  // segments from left to right: 6, 5, 4, 3, 2, 1, 0
  uint16_t startindex = 0;
 //   Serial.println(startindex);
  switch (segment) {
    case 0:
      startindex = 0;
      break;
    case 1:
      startindex = (LEDS_PER_DIGIT * 1);
      break;
    case 2:
      startindex = (LEDS_PER_DIGIT * 2);
      break;
    case 3:
      startindex = (LEDS_PER_DIGIT * 3);
      break;    
    case 4:
      startindex = (LEDS_PER_DIGIT * 4);
      break;    
    case 5:
      startindex = (LEDS_PER_DIGIT * 5);
      break;    
    case 6:
      startindex = (LEDS_PER_DIGIT * 6);
      break;    
  }

  for (byte i=0; i<SEGMENTS_PER_NUMBER; i++){                // 7 segments
    if (fakeclockrunning == 0 && (((ClockColorSettings == 4 && clockMode == 0) || (DateColorSettings == 4 && clockMode == 7) || (tempColorSettings == 4 && clockMode == 2) || (humiColorSettings == 4 && clockMode == 8)) && pastelColors == 0))  { color = CHSV(random(0, 255), 255, 255);}
    if (fakeclockrunning == 0 && (((ClockColorSettings == 4 && clockMode == 0) || (DateColorSettings == 4 && clockMode == 7) || (tempColorSettings == 4 && clockMode == 2) || (humiColorSettings == 4 && clockMode == 8)) && pastelColors == 1)) { color = CRGB(random(0, 255), random(0, 255), random(0, 255));}
    for (byte j=0; j<LEDS_PER_SEGMENT; j++ ){              // 7 LEDs per segment
      yield();
      LEDs[FAKE_LEDs[i * LEDS_PER_SEGMENT + j + startindex]] = ((numbers[number] & 1 << i) == 1 << i) ? color : alternateColor;
    }
  }
} //end of displayNumber


void allBlank() {   //clears all non-shelf LEDs to black
  //  Serial.println("allblank function");
  for (int i=0; i<SEGMENTS_LEDS; i++) {
    LEDs[i] = CRGB::Black;
  }
  FastLED.show();
  randomMinPassed = 1;
  randomHourPassed = 1;
  randomDayPassed = 1;
  randomWeekPassed = 1;
  randomMonthPassed = 1;
  GetBrightnessLevel();
}  // end of all-blank


void fakeClock(int loopy) {  //flashes 12:00 like all old clocks did
  fakeclockrunning = 1;
  for (int i=0; i<loopy; i++) {
      for (int i=(32*LEDS_PER_SEGMENT); i<(34*LEDS_PER_SEGMENT); i++) { LEDs[i] = CRGB::Red;}
      displayNumber(2,5,CRGB::Red);
      for (int i=(25*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2-1); i<(25*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2+1); i++) { LEDs[i] = CRGB::Black; }
      for (int i=(20*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2-1); i<(20*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2+1); i++) { LEDs[i] = CRGB::Black; }
      displayNumber(0,2,CRGB::Red);
      displayNumber(0,0,CRGB::Red); 
      FastLED.show();
      delay(500);
      for (int i=(32*LEDS_PER_SEGMENT); i<(34*LEDS_PER_SEGMENT); i++) { LEDs[i] = CRGB::Black;}
      displayNumber(2,5,CRGB::Black);
      for (int i=(25*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2-1); i<(25*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2+1); i++) { LEDs[i] = CRGB::Red; }
      for (int i=(20*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2-1); i<(20*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2+1); i++) { LEDs[i] = CRGB::Red; }
      displayNumber(0,2,CRGB::Black);
      displayNumber(0,0,CRGB::Black); 
      FastLED.show();
      delay(500);
   }
  for (int i=(25*LEDS_PER_SEGMENT); i<(26*LEDS_PER_SEGMENT); i++) { LEDs[i] = CRGB::Black; }
  for (int i=(20*LEDS_PER_SEGMENT); i<(21*LEDS_PER_SEGMENT); i++) { LEDs[i] = CRGB::Black; }
  fakeclockrunning = 0;  // so the digit render knows not to apply the rainbow colors
}  //end of fakeClock




void ShelfDownLights() {  //turns on the drop lights on the underside of each shelf
 //   Serial.println("ShelfDownLights function");
 if ((suspendType != 2 || isAsleep == 0) && useSpotlights == 1) {  //not sleeping? suposed to be running?
  unsigned long currentMillis = millis();  
  if (currentMillis - prevTime2 >= 250) {  //run everything inside here every quarter second
    for (int i=SEGMENTS_LEDS; i<NUM_LEDS; i++) {
        if (spotlightsColorSettings == 0){ spotlightsColor = CRGB(r0_val, g0_val, b0_val);  LEDs[i] = spotlightsColor;}
        if ((spotlightsColorSettings == 1 && pastelColors == 0)  && ( (ColorChangeFrequency == 0 ) || (ColorChangeFrequency == 1 && randomMinPassed == 1) || (ColorChangeFrequency == 2 && randomHourPassed == 1) || (ColorChangeFrequency == 3 && randomDayPassed == 1) || (ColorChangeFrequency == 4 && randomWeekPassed == 1) || (ColorChangeFrequency == 5 && randomMonthPassed == 1) )) { spotlightsColor = CHSV(random(0, 255), 255, 255);  LEDs[i] = spotlightsColor;}
        if ((spotlightsColorSettings == 1 && pastelColors == 1)  && ( (ColorChangeFrequency == 0 ) || (ColorChangeFrequency == 1 && randomMinPassed == 1) || (ColorChangeFrequency == 2 && randomHourPassed == 1) || (ColorChangeFrequency == 3 && randomDayPassed == 1) || (ColorChangeFrequency == 4 && randomWeekPassed == 1) || (ColorChangeFrequency == 5 && randomMonthPassed == 1) )) { spotlightsColor = CRGB(random(0, 255), random(0, 255), random(0, 255));  LEDs[i] = spotlightsColor;}
        if (spotlightsColorSettings == 2 ){ LEDs[i] = colorWheel2(((i-SEGMENTS_LEDS)  * 18 + colorWheelPositionTwo) % 256); }
        if (spotlightsColorSettings == 3 ){ LEDs[i] = colorWheel2(((255) + colorWheelPositionTwo) % 256); }
        if (spotlightsColorSettings == 4 ){
    int seed = random(2500);         // A random number. Higher number => fewer twinkles. Use random16() for values >255.
    if (seed < 30) {
      CRGB color = CRGB::Black;
      if (pastelColors == 0){ color = CHSV(random(0, 255), 255, 255); }
      if (pastelColors == 1){ color = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
            {              
              LEDs[SEGMENTS_LEDS+random((NUM_LEDS-SEGMENTS_LEDS))] = color;
            }
      }
    
      for (int j=SEGMENTS_LEDS; j<NUM_LEDS; j++) {
        LEDs[j].fadeToBlackBy(1);
        }
     }
    }
    colorWheelPositionTwo = colorWheelPositionTwo - 1; // SPEED OF 2nd COLOR WHEEL
    if (colorWheelPositionTwo < 0) {colorWheelPositionTwo = 255;} // RESET 2nd COLOR WHEEL 
    if ((spotlightsColorSettings == 2 || spotlightsColorSettings == 3) && clockMode != 11){FastLED.show();}
    prevTime2 = currentMillis;
  FastLED.show();		 
  }
 } else if (useSpotlights == 0) {  //or turn them all off
  for (int i=SEGMENTS_LEDS; i<NUM_LEDS; i++) {
    LEDs[i] = CRGB::Black;
  }
  FastLED.show();
 }
 
}//end of ShelfDownLights 




void BlinkDots() {  //displays the 2 dots in the middle of the time (colon)
  if (dotsOn) {
  if (ClockColorSettings == 0 || ClockColorSettings == 1) {colonColor = CRGB(r3_val, g3_val, b3_val);}
    if (((ClockColorSettings == 2) || (ClockColorSettings == 4)) && ( (ColorChangeFrequency == 0 ) || (ColorChangeFrequency == 1 && randomMinPassed == 1) || (ColorChangeFrequency == 2 && randomHourPassed == 1) || (ColorChangeFrequency == 3 && randomDayPassed == 1) || (ColorChangeFrequency == 4 && randomWeekPassed == 1) || (ColorChangeFrequency == 5 && randomMonthPassed == 1) )) { colonColor = CRGB(random(0, 255), random(0, 255), random(0, 255));}
    if (ClockColorSettings == 3 && ( (ColorChangeFrequency == 0 ) || (ColorChangeFrequency == 1 && randomMinPassed == 1) || (ColorChangeFrequency == 2 && randomHourPassed == 1) || (ColorChangeFrequency == 3 && randomDayPassed == 1) || (ColorChangeFrequency == 4 && randomWeekPassed == 1) || (ColorChangeFrequency == 5 && randomMonthPassed == 1) )) { colonColor = hourColor;}
    if (colonType == 0) {
      if (clockDisplayType == 1 || clockDisplayType == 2) {     //12 24-hour Military Time
        if (ClockColorSettings == 4 && pastelColors == 0){ colonColor = CHSV(random(0, 255), 255, 255); }
        if (ClockColorSettings == 4 && pastelColors == 1){ colonColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
        for (int i=17*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2; i<17*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2+1; i++) { LEDs[i] = colonColor;}
        if (ClockColorSettings == 4 && pastelColors == 0){ colonColor = CHSV(random(0, 255), 255, 255); }
        if (ClockColorSettings == 4 && pastelColors == 1){ colonColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
        for (int i=19*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2; i<19*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2+1; i++) { LEDs[i] = colonColor;}        
      }
      else {
        if (ClockColorSettings == 4 && pastelColors == 0){ colonColor = CHSV(random(0, 255), 255, 255); }
        if (ClockColorSettings == 4 && pastelColors == 1){ colonColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
        for (int i=25*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2-1; i<25*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2+1; i++) { LEDs[i] = colonColor;}
        if (ClockColorSettings == 4 && pastelColors == 0){ colonColor = CHSV(random(0, 255), 255, 255); }
        if (ClockColorSettings == 4 && pastelColors == 1){ colonColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
        for (int i=20*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2-1; i<20*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2+1; i++) { LEDs[i] = colonColor;}
      }
    }
    if (colonType == 1) {
      if (clockDisplayType == 1 || clockDisplayType == 2) {     //12 24-hour Military Time
        if (ClockColorSettings == 4 && pastelColors == 0){ colonColor = CHSV(random(0, 255), 255, 255); }
        if (ClockColorSettings == 4 && pastelColors == 1){ colonColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
        for (int i=17*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2-1; i<17*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2+2; i++) { LEDs[i] = colonColor;}
        if (ClockColorSettings == 4 && pastelColors == 0){ colonColor = CHSV(random(0, 255), 255, 255); }
        if (ClockColorSettings == 4 && pastelColors == 1){ colonColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
        for (int i=19*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2-1; i<19*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2+2; i++) { LEDs[i] = colonColor;}        
      }
      else {
        if (ClockColorSettings == 4 && pastelColors == 0){ colonColor = CHSV(random(0, 255), 255, 255); }
        if (ClockColorSettings == 4 && pastelColors == 1){ colonColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
        for (int i=25*LEDS_PER_SEGMENT; i<26*LEDS_PER_SEGMENT; i++) { LEDs[i] = colonColor;}
        if (ClockColorSettings == 4 && pastelColors == 0){ colonColor = CHSV(random(0, 255), 255, 255); }
        if (ClockColorSettings == 4 && pastelColors == 1){ colonColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
        for (int i=20*LEDS_PER_SEGMENT; i<21*LEDS_PER_SEGMENT; i++) { LEDs[i] = colonColor;}
      }
    }
    if (colonType == 2) {
      if (ClockColorSettings == 4 && pastelColors == 0){ colonColor = CHSV(random(0, 255), 255, 255); }
      if (ClockColorSettings == 4 && pastelColors == 1){ colonColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
      if (clockDisplayType == 1 || clockDisplayType == 2) {     //12 24-hour Military Time
        for (int i=17*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2; i<17*LEDS_PER_SEGMENT+LEDS_PER_SEGMENT/2+1; i++) { LEDs[i] = colonColor;}
      }
      else {
	      for (int i=20*LEDS_PER_SEGMENT; i<21*LEDS_PER_SEGMENT; i++) { LEDs[i] = colonColor;}
      }
    }
  } else {
    if (clockDisplayType == 1 || clockDisplayType == 2) {     //12 24-hour Military Time
      for (int i=17*LEDS_PER_SEGMENT; i<18*LEDS_PER_SEGMENT; i++) { LEDs[i] = CRGB::Black;}
      for (int i=19*LEDS_PER_SEGMENT; i<20*LEDS_PER_SEGMENT; i++) { LEDs[i] = CRGB::Black;}
    }
    else {
	    for (int i=25*LEDS_PER_SEGMENT; i<26*LEDS_PER_SEGMENT; i++) { LEDs[i] = CRGB::Black;}
	    for (int i=20*LEDS_PER_SEGMENT; i<21*LEDS_PER_SEGMENT; i++) { LEDs[i] = CRGB::Black;}
    }
  }
  dotsOn = !dotsOn;  
}//end of shelf and gaps

/* void happyNewYear() {  
  CRGB color = CRGB::Red; 
  allBlank();
  breakOutSet = 0;
  int year = (timeinfo.tm_year +1900)-2000;
  byte y1 = year / 10;
  byte y2 = year % 10;
//  #if HAS_BUZZER
 // rtttl::begin(BUZZER_PIN, sounds.getSongByName("auldlang"));
//  while( !rtttl::done() && !breakOutSet )
 // {
 //   rtttl::play();
 //   color = colorWheel((205 / 50 + colorWheelPosition) % 256);
////    displayNumber(2,6,color);  //2
//    displayNumber(0,4,color);  //0
//    displayNumber(y1,2,color);  //Y
////    displayNumber(y2,0,color);  //Y
//    FastLED.show();
//    colorWheelPosition++;
//    server.handleClient();   
//  }
//  #else
    color = colorWheel((205 / 50 + colorWheelPosition) % 256);
    displayNumber(2,6,color);  //2
    displayNumber(0,4,color);  //0
    displayNumber(y1,2,color);  //Y
    displayNumber(y2,0,color);  //Y
    FastLED.show();
    colorWheelPosition++;
    server.handleClient();  
 // #endif
  if (!breakOutSet) {scroll("HAPPy nEW yEAr");}
  allBlank();
}
*/
void Chase() {   //lightshow chase mode
  int chaseMode = random(0, 7);
  if (pastelColors == 0){ lightchaseColorOne = CHSV(random(0, 255), 255, 255); }
  if (pastelColors == 0){ lightchaseColorTwo = CHSV(random(0, 255), 255, 255); }
  if (pastelColors == 1){ lightchaseColorOne = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
  if (pastelColors == 1){ lightchaseColorTwo = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
  for (int i=0; i<SPECTRUM_PIXELS; i++) {  //draw forward
    for (byte s=0; s<LEDS_PER_SEGMENT; s++ ){              // 7 LEDs per segment
   int fake =  i * LEDS_PER_SEGMENT;
      if (chaseMode == 0) {LEDs[FAKE_LEDs_C_BLTR[s+((fake))]] = lightchaseColorOne;}
      if (chaseMode == 1) {LEDs[FAKE_LEDs_C_BRTL[s+((fake))]] = lightchaseColorOne;}
      if (chaseMode == 2) {LEDs[FAKE_LEDs_C_BMUP[s+((fake))]] = lightchaseColorOne;}
      if (chaseMode == 3) {LEDs[FAKE_LEDs_C_TMDN[s+((fake))]] = lightchaseColorOne;}
      if (chaseMode == 4) {LEDs[FAKE_LEDs_C_CMOT[s+((fake))]] = lightchaseColorOne;}
      if (chaseMode == 5) {LEDs[FAKE_LEDs_C_TLBR[s+((fake))]] = lightchaseColorOne;}
      if (chaseMode == 6) {LEDs[FAKE_LEDs_C_CSIN[s+((fake))]] = lightchaseColorOne;}
      if (chaseMode == 7) {LEDs[FAKE_LEDs_C_TRBL[s+((fake))]] = lightchaseColorOne;}
      if (chaseMode == 8) {LEDs[FAKE_LEDs_C_OUTS[s+((fake))]] = lightchaseColorOne;}
      if (chaseMode == 9) {LEDs[FAKE_LEDs_C_VERT[s+((fake))]] = lightchaseColorOne;}
      if (chaseMode == 10) {LEDs[FAKE_LEDs_C_OUTS2[s+((fake))]] = lightchaseColorOne;}
      if (chaseMode == 11) {LEDs[FAKE_LEDs_C_VERT2[s+((fake))]] = lightchaseColorOne;}
    }
    FastLED.show();
  for( int d = 0; d < 40; d++) {server.handleClient(); }  //delay to speed, but so the web buttons still work
  // delay(1);
  }  
  for (int i = SPECTRUM_PIXELS-1; i > -1; --i) {   //draw backwards
    for (byte s=0; s<LEDS_PER_SEGMENT; s++ ){              // 7 LEDs per segment
   int fake =  i * LEDS_PER_SEGMENT;
      if (chaseMode == 0) {LEDs[FAKE_LEDs_C_BLTR[s+((fake))]] = lightchaseColorTwo;}
      if (chaseMode == 1) {LEDs[FAKE_LEDs_C_BRTL[s+((fake))]] = lightchaseColorTwo;}
      if (chaseMode == 2) {LEDs[FAKE_LEDs_C_BMUP[s+((fake))]] = lightchaseColorTwo;}
      if (chaseMode == 3) {LEDs[FAKE_LEDs_C_TMDN[s+((fake))]] = lightchaseColorTwo;}
      if (chaseMode == 4) {LEDs[FAKE_LEDs_C_CMOT[s+((fake))]] = lightchaseColorTwo;}
      if (chaseMode == 5) {LEDs[FAKE_LEDs_C_TLBR[s+((fake))]] = lightchaseColorTwo;}
      if (chaseMode == 6) {LEDs[FAKE_LEDs_C_CSIN[s+((fake))]] = lightchaseColorTwo;}
      if (chaseMode == 7) {LEDs[FAKE_LEDs_C_TRBL[s+((fake))]] = lightchaseColorTwo;}
      if (chaseMode == 8) {LEDs[FAKE_LEDs_C_OUTS[s+((fake))]] = lightchaseColorTwo;}
      if (chaseMode == 9) {LEDs[FAKE_LEDs_C_VERT[s+((fake))]] = lightchaseColorTwo;}
      if (chaseMode == 10) {LEDs[FAKE_LEDs_C_OUTS2[s+((fake))]] = lightchaseColorTwo;}
      if (chaseMode == 11) {LEDs[FAKE_LEDs_C_VERT2[s+((fake))]] = lightchaseColorTwo;}
    }
    FastLED.show();
  for( int d = 0; d < 40; d++) {server.handleClient(); }  //delay to speed, but so the web buttons still work
  //  delay(1);
  }  
  if (clockMode != 5) { allBlank(); }
} //end of chase


 
void Twinkles() {
#if HAS_SOUNDDETECTOR
  int audio_input = i2sWaveformRead();
  int Level = map(audio_input, 0, 1023, 50, 210);
  if (audio_input < 100){  Level = 50;  }
  if (audio_input > 1023){  Level = 210;  }
#else
  int Level = 100;  //set to default if no audio board
#endif
  int seed = random(Level);         // A random number. Higher number => fewer twinkles. Use random16() for values >255.
  int i = random(SPECTRUM_PIXELS);         // A random number. Higher number => fewer twinkles. Use random16() for values >255.
  if (seed > 46) {
    CRGB color = CRGB::Black;
    if (pastelColors == 0){ color = CHSV(random(0, 255), 255, 255); }
    if (pastelColors == 1){ color = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
          int fake =  i * LEDS_PER_SEGMENT;
          for (byte s=0; s<LEDS_PER_SEGMENT; s++ ){              // 7 LEDs per segment
            LEDs[FAKE_LEDs_C_BLTR[s+((fake))]] = color;
          }
  }
  
  for (int j = 0; j < SPECTRUM_PIXELS; j++) {
          int fake =  j * LEDS_PER_SEGMENT;
          for (byte s=0; s<LEDS_PER_SEGMENT; s++ ){              // 7 LEDs per segment
            LEDs[FAKE_LEDs_C_BLTR[s+((fake))]].fadeToBlackBy(20);
          }
  }
  if (clockMode != 5) { allBlank(); }
} // twinkles



void Rainbow() {
  //fill_gradient_RGB(LEDs, NUM_LEDS, CRGB::Red, CRGB::Yellow, CRGB::Green, CRGB::Blue);
    for (byte j = 0; j < SPECTRUM_PIXELS; j++) {
      int fake =  j * LEDS_PER_SEGMENT;
      for (byte s=0; s<LEDS_PER_SEGMENT; s++ ){              // 7 LEDs per segment
        LEDs[FAKE_LEDs_C_BMUP[s+((fake))]] = colorWheel(((255) + colorWheelPosition) % 256);
      }
    }
  EVERY_N_MILLISECONDS(150) {
    colorWheelPosition = colorWheelPosition - colorWheelSpeed; // SPEED OF COLOR WHEEL
  if (colorWheelPosition < 0) // RESET COLOR WHEEL
    colorWheelPosition = 255;
  }
  if (clockMode != 5) { allBlank(); }
} //end of Rainbow



void GreenMatrix() {
  EVERY_N_MILLISECONDS(65) {
    updateMatrix();
    FastLED.show();
 }
  EVERY_N_MILLISECONDS(30) {
    changeMatrixpattern();
  }
} //loop

void changeMatrixpattern () {
  int rand1 = random16 (SPECTRUM_PIXELS);
  int rand2 = random16 (SPECTRUM_PIXELS);
  if ((greenMatrix[rand1] == 1) && (greenMatrix[rand2] == 0) )   //simple get two random dot 1 and 0 and swap it,
  {
    greenMatrix[rand1] = 0;  //this will not change total number of dots
    greenMatrix[rand2] = 1;
  }
  if (clockMode != 5) { allBlank(); }
} //changeMatrixpattern

void initGreenMatrix() {                               //init array of dots. run once
  for (int i = 0; i < SPECTRUM_PIXELS; i++) {
    if (random8(20) == 0) {
      greenMatrix[i] = 1;  //random8(20) number of dots. decrease for more dots
    }
    else {
      greenMatrix[i] = 0;
    }
  }
} //initGreenMatrix

void updateMatrix() {
    for (byte j = 0; j < SPECTRUM_PIXELS; j++) {
      byte layer = greenMatrix[((j + lightshowSpeed + random8(2) + SPECTRUM_PIXELS) % SPECTRUM_PIXELS)];   //fake scroll based on shift coordinate
      // random8(2) add glitchy look
      if (layer) {
          int fake =  j * LEDS_PER_SEGMENT;
          for (byte s=0; s<LEDS_PER_SEGMENT; s++ ){              // 7 LEDs per segment
            //LEDs[FAKE_LEDs_C_RAIN[s+((fake))]] = CHSV(110, 255, 255);
            LEDs[FAKE_LEDs_C_RAIN[s+((fake))]] = CRGB::Green;
          }
      }
    }
  lightshowSpeed ++;
    for (byte j = 0; j < SPECTRUM_PIXELS; j++) {
      int fake =  j * LEDS_PER_SEGMENT;
      for (byte s=0; s<LEDS_PER_SEGMENT; s++ ){              // 7 LEDs per segment
        LEDs[FAKE_LEDs_C_RAIN[s+((fake))]].fadeToBlackBy( 60 );
      }
    }
} //updateMatrix



void blueRain() {
#if HAS_SOUNDDETECTOR
    int audio_input = i2sWaveformRead(); 
  int Level = map(audio_input, 0, 1023, 0, 400);
  if (audio_input < 100){  Level = 0;  }
  if (audio_input > 1023){  Level = 400;  }
#else
  int Level = 400;  //set to default if no sound board
#endif
  //  Serial.println(Level);
    int delayRain = 400-Level;
  EVERY_N_MILLIS_I( thistimer, 400 ) { // initial period = 100ms
thistimer.setPeriod(delayRain);
    updaterain();
 //   FastLED.show();
  }
  if (clockMode != 5) { allBlank(); }
} //loop

void raininit() {                               //init array of dots. run once
  for (int i = 0; i < SPECTRUM_PIXELS; i++) {
    if (random8(24) == 0) {  //30?
      rain[i] = 1;  //random8(20) number of dots. decrease for more dots
    }
    else {
      rain[i] = 0;
    }
  }
} //raininit

void updaterain() {
    for (byte j = 0; j < SPECTRUM_PIXELS; j++) {
      byte layer = rain[((j + lightshowSpeed + 2 + SPECTRUM_PIXELS) % SPECTRUM_PIXELS)];   //fake scroll based on shift coordinate
      // random8(2) add glitchy look
      if (layer) {
          int fake =  j * LEDS_PER_SEGMENT;
          for (byte s=0; s<LEDS_PER_SEGMENT; s++ ){              // 7 LEDs per segment
            //LEDs[FAKE_LEDs_C_RAIN[s+((fake))]] = CHSV(110, 255, 255);
            LEDs[FAKE_LEDs_C_RAIN[s+((fake))]] = CRGB::Blue;
          }
      }
    }
  lightshowSpeed ++;
  for (byte j = 0; j < SPECTRUM_PIXELS; j++) {
      int fake =  j * LEDS_PER_SEGMENT;
      for (byte s=0; s<LEDS_PER_SEGMENT; s++ ){              // 7 LEDs per segment
        LEDs[FAKE_LEDs_C_RAIN[s+((fake))]].fadeToBlackBy( 128 );
      }
  }

}

void Fire2021() {
// Array of temperature readings at each simulation cell
  static byte heat[SPECTRUM_PIXELS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < SPECTRUM_PIXELS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, 30));  //55-cooling
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= SPECTRUM_PIXELS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat to the bottom center 3
    if( random8() < 128 ) {  //128-sparkling
      int y = random8(0,2);
      heat[y] = qadd8( heat[y], random8(155,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < SPECTRUM_PIXELS; j++) {
      CRGB color = HeatColor( heat[j]);
      int fake =  j * LEDS_PER_SEGMENT;
      for (byte s=0; s<LEDS_PER_SEGMENT; s++ ){              // 7 LEDs per segment
        LEDs[FAKE_LEDs_C_FIRE[s+((fake))]] = color;
      }

    }
  if (clockMode != 5) { allBlank(); }
}



void Snake() {  //real random snake mode with random food changing its color
  int move = 0;
  int fadeby = 130;
  int pickOne = random(3);
  //just write out every possible move, that's all, oh and make it random
  if (snakePosition == 0 && snakeLastDirection == 0 && move == 0)  {if (pickOne == 3) {snakeLastDirection = 3; snakePosition = 4; move = 1;} else {snakePosition = 5; move = 1;}}
  if (snakePosition == 0 && snakeLastDirection == 2 && move == 0)  {snakeLastDirection = 3; snakePosition = 1; move = 1;}
  if (snakePosition == 1 && snakeLastDirection == 3 && move == 0)  {if (pickOne == 0) {snakeLastDirection = 0; snakePosition = 3; move = 1;} else {snakePosition = 2; move = 1;}}
  if (snakePosition == 1 && snakeLastDirection == 1 && move == 0)  {snakeLastDirection = 0; snakePosition = 0; move = 1;}
  if (snakePosition == 2 && snakeLastDirection == 3 && move == 0)  {if (pickOne == 0) {snakeLastDirection = 0; snakePosition = 9; move = 1;} else {snakePosition = 10; move = 1;}}
  if (snakePosition == 2 && snakeLastDirection == 1 && move == 0)  {if (pickOne == 0) {snakeLastDirection = 0; snakePosition = 3; move = 1;} else {snakePosition = 1; move = 1;}}
  if (snakePosition == 3 && snakeLastDirection == 0 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 4; move = 1;} else if (pickOne == 3) {snakeLastDirection = 3; snakePosition = 8; move = 1;} else {snakePosition = 7; move = 1;}}
  if (snakePosition == 3 && snakeLastDirection == 2 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 1; move = 1;} else {snakeLastDirection = 3; snakePosition = 2; move = 1;}}
  if (snakePosition == 4 && snakeLastDirection == 1 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 2; snakePosition = 0; move = 1;} else {snakeLastDirection = 0; snakePosition = 5; move = 1;}}
  if (snakePosition == 4 && snakeLastDirection == 3 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 0; snakePosition = 7; move = 1;} else if (pickOne == 3) {snakeLastDirection = 2; snakePosition = 3; move = 1;} else {snakePosition = 8; move = 1;}}
  if (snakePosition == 5 && snakeLastDirection == 0 && move == 0)  {snakeLastDirection = 3; snakePosition = 6; move = 1;}
  if (snakePosition == 5 && snakeLastDirection == 2 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 3; snakePosition = 4; move = 1;} else {snakeLastDirection = 2; snakePosition = 0; move = 1;}}
  if (snakePosition == 6 && snakeLastDirection == 3 && move == 0)  {if (pickOne == 3) {snakeLastDirection = 2; snakePosition = 7; move = 1;} else {snakePosition = 15; move = 1;}}
  if (snakePosition == 6 && snakeLastDirection == 1 && move == 0)  {snakeLastDirection = 2; snakePosition = 5; move = 1;}
  if (snakePosition == 7 && snakeLastDirection == 0 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 6; move = 1;} else {snakeLastDirection = 3; snakePosition = 15; move = 1;}} 
  if (snakePosition == 7 && snakeLastDirection == 2 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 4; move = 1;} else if (pickOne == 3) {snakeLastDirection = 3; snakePosition = 8; move = 1;} else {snakePosition = 3; move = 1;}}
  if (snakePosition == 8 && snakeLastDirection == 1 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 2; snakePosition = 3; move = 1;} else if (pickOne == 3) {snakeLastDirection = 0; snakePosition = 7; move = 1;} else {snakePosition = 4; move = 1;}}
  if (snakePosition == 8 && snakeLastDirection == 3 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 0; snakePosition = 14; move = 1;} else if (pickOne == 3) {snakeLastDirection = 2; snakePosition = 9; move = 1;} else {snakePosition = 13; move = 1;}}
  if (snakePosition == 9 && snakeLastDirection == 0 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 8; move = 1;} else if (pickOne == 3) {snakeLastDirection = 3; snakePosition = 13; move = 1;} else {snakePosition = 14; move = 1;}}
  if (snakePosition == 9 && snakeLastDirection == 2 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 2; move = 1;} else {snakeLastDirection = 3; snakePosition = 10; move = 1;}}
  if (snakePosition == 10 && snakeLastDirection == 3 && move == 0)  {if (pickOne == 0) {snakeLastDirection = 0; snakePosition = 12; move = 1;} else {snakePosition = 11; move = 1;}}
  if (snakePosition == 10 && snakeLastDirection == 1 && move == 0)  {if (pickOne == 0) {snakeLastDirection = 0; snakePosition = 9; move = 1;} else {snakePosition = 2; move = 1;}}
  if (snakePosition == 11 && snakeLastDirection == 3 && move == 0)  {if (pickOne == 0) {snakeLastDirection = 0; snakePosition = 19; move = 1;} else {snakePosition = 20; move = 1;}}
  if (snakePosition == 11 && snakeLastDirection == 1 && move == 0)  {if (pickOne == 0) {snakeLastDirection = 0; snakePosition = 12; move = 1;} else {snakePosition = 10; move = 1;}}
  if (snakePosition == 12 && snakeLastDirection == 0 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 13; move = 1;} else if (pickOne == 3) {snakeLastDirection = 3; snakePosition = 18; move = 1;} else {snakePosition = 17; move = 1;}}
  if (snakePosition == 12 && snakeLastDirection == 2 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 10; move = 1;} else {snakeLastDirection = 3; snakePosition = 11; move = 1;}}
  if (snakePosition == 13 && snakeLastDirection == 1 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 2; snakePosition = 9; move = 1;} else if (pickOne == 3) {snakeLastDirection = 0; snakePosition = 14; move = 1;} else {snakePosition = 8; move = 1;}}
  if (snakePosition == 13 && snakeLastDirection == 3 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 0; snakePosition = 17; move = 1;} else if (pickOne == 3) {snakeLastDirection = 2; snakePosition = 12; move = 1;} else {snakePosition = 18; move = 1;}}
  if (snakePosition == 14 && snakeLastDirection == 0 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 15; move = 1;} else {snakeLastDirection = 3; snakePosition = 16; move = 1;}} 
  if (snakePosition == 14 && snakeLastDirection == 2 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 8; move = 1;} else if (pickOne == 3) {snakeLastDirection = 3; snakePosition = 13; move = 1;} else {snakePosition = 9; move = 1;}}
  if (snakePosition == 15 && snakeLastDirection == 3 && move == 0)  {if (pickOne == 3) {snakeLastDirection = 2; snakePosition = 14; move = 1;} else {snakePosition = 16; move = 1;}}
  if (snakePosition == 15 && snakeLastDirection == 1 && move == 0)  {if (pickOne == 3) {snakeLastDirection = 2; snakePosition = 7; move = 1;} else {snakeLastDirection = 1; snakePosition = 6; move = 1;}}
  if (snakePosition == 16 && snakeLastDirection == 3 && move == 0)  {if (pickOne == 3) {snakeLastDirection = 2; snakePosition = 17; move = 1;} else {snakePosition = 25; move = 1;}}
  if (snakePosition == 16 && snakeLastDirection == 1 && move == 0)  {if (pickOne == 3) {snakeLastDirection = 2; snakePosition = 14; move = 1;} else {snakeLastDirection = 1; snakePosition = 15; move = 1;}}
  if (snakePosition == 17 && snakeLastDirection == 0 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 16; move = 1;} else {snakeLastDirection = 3; snakePosition = 25; move = 1;}} 
  if (snakePosition == 17 && snakeLastDirection == 2 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 13; move = 1;} else if (pickOne == 3) {snakeLastDirection = 3; snakePosition = 18; move = 1;} else {snakePosition = 12; move = 1;}}
  if (snakePosition == 18 && snakeLastDirection == 1 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 2; snakePosition = 12; move = 1;} else if (pickOne == 3) {snakeLastDirection = 0; snakePosition = 17; move = 1;} else {snakePosition = 13; move = 1;}}
  if (snakePosition == 18 && snakeLastDirection == 3 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 0; snakePosition = 24; move = 1;} else if (pickOne == 3) {snakeLastDirection = 2; snakePosition = 19; move = 1;} else {snakePosition = 23; move = 1;}}
  if (snakePosition == 19 && snakeLastDirection == 0 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 18; move = 1;} else if (pickOne == 3) {snakeLastDirection = 3; snakePosition = 23; move = 1;} else {snakePosition = 24; move = 1;}}
  if (snakePosition == 19 && snakeLastDirection == 2 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 11; move = 1;} else {snakeLastDirection = 3; snakePosition = 20; move = 1;}}
  if (snakePosition == 20 && snakeLastDirection == 3 && move == 0)  {if (pickOne == 0) {snakeLastDirection = 0; snakePosition = 22; move = 1;} else {snakePosition = 21; move = 1;}}
  if (snakePosition == 20 && snakeLastDirection == 1 && move == 0)  {if (pickOne == 0) {snakeLastDirection = 0; snakePosition = 19; move = 1;} else {snakePosition = 11; move = 1;}}
  if (snakePosition == 21 && snakeLastDirection == 3 && move == 0)  {if (pickOne == 0) {snakeLastDirection = 0; snakePosition = 29; move = 1;} else {snakePosition = 30; move = 1;}}
  if (snakePosition == 21 && snakeLastDirection == 1 && move == 0)  {if (pickOne == 0) {snakeLastDirection = 0; snakePosition = 22; move = 1;} else {snakePosition = 20; move = 1;}}
  if (snakePosition == 22 && snakeLastDirection == 0 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 23; move = 1;} else if (pickOne == 3) {snakeLastDirection = 3; snakePosition = 28; move = 1;} else {snakePosition = 27; move = 1;}}
  if (snakePosition == 22 && snakeLastDirection == 2 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 20; move = 1;} else {snakeLastDirection = 3; snakePosition = 21; move = 1;}}
  if (snakePosition == 23 && snakeLastDirection == 1 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 2; snakePosition = 19; move = 1;} else if (pickOne == 3) {snakeLastDirection = 0; snakePosition = 24; move = 1;} else {snakePosition = 18; move = 1;}}
  if (snakePosition == 23 && snakeLastDirection == 3 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 0; snakePosition = 27; move = 1;} else if (pickOne == 3) {snakeLastDirection = 2; snakePosition = 22; move = 1;} else {snakePosition = 28; move = 1;}}
  if (snakePosition == 24 && snakeLastDirection == 0 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 25; move = 1;} else {snakeLastDirection = 3; snakePosition = 26; move = 1;}} 
  if (snakePosition == 24 && snakeLastDirection == 2 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 18; move = 1;} else if (pickOne == 3) {snakeLastDirection = 3; snakePosition = 23; move = 1;} else {snakePosition = 19; move = 1;}}
  if (snakePosition == 25 && snakeLastDirection == 3 && move == 0)  {if (pickOne == 3) {snakeLastDirection = 2; snakePosition = 24; move = 1;} else {snakePosition = 26; move = 1;}}
  if (snakePosition == 25 && snakeLastDirection == 1 && move == 0)  {if (pickOne == 3) {snakeLastDirection = 2; snakePosition = 17; move = 1;} else {snakeLastDirection = 1; snakePosition = 16; move = 1;}}
  if (snakePosition == 26 && snakeLastDirection == 3 && move == 0)  {if (pickOne == 3) {snakeLastDirection = 2; snakePosition = 27; move = 1;} else {snakePosition = 34; move = 1;}}
  if (snakePosition == 26 && snakeLastDirection == 1 && move == 0)  {if (pickOne == 3) {snakeLastDirection = 2; snakePosition = 24; move = 1;} else {snakeLastDirection = 1; snakePosition = 25; move = 1;}}
  if (snakePosition == 27 && snakeLastDirection == 0 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 26; move = 1;} else {snakeLastDirection = 3; snakePosition = 34; move = 1;}} 
  if (snakePosition == 27 && snakeLastDirection == 2 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 23; move = 1;} else if (pickOne == 3) {snakeLastDirection = 3; snakePosition = 28; move = 1;} else {snakePosition = 22; move = 1;}}
  if (snakePosition == 28 && snakeLastDirection == 1 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 2; snakePosition = 22; move = 1;} else if (pickOne == 3) {snakeLastDirection = 0; snakePosition = 27; move = 1;} else {snakePosition = 23; move = 1;}}
  if (snakePosition == 28 && snakeLastDirection == 3 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 0; snakePosition = 33; move = 1;} else if (pickOne == 3) {snakeLastDirection = 2; snakePosition = 29; move = 1;} else {snakePosition = 32; move = 1;}}
  if (snakePosition == 29 && snakeLastDirection == 0 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 28; move = 1;} else if (pickOne == 3) {snakeLastDirection = 3; snakePosition = 32; move = 1;} else {snakePosition = 33; move = 1;}}
  if (snakePosition == 29 && snakeLastDirection == 2 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 21; move = 1;} else {snakeLastDirection = 3; snakePosition = 30; move = 1;}}
  if (snakePosition == 30 && snakeLastDirection == 3 && move == 0)  {snakeLastDirection = 0; snakePosition = 31; move = 1;}
  if (snakePosition == 30 && snakeLastDirection == 1 && move == 0)  {if (pickOne == 0) {snakeLastDirection = 0; snakePosition = 29; move = 1;} else {snakePosition = 21; move = 1;}}
  if (snakePosition == 31 && snakeLastDirection == 0 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 32; move = 1;} else {snakePosition = 36; move = 1;}}
  if (snakePosition == 31 && snakeLastDirection == 2 && move == 0)  {snakeLastDirection = 1; snakePosition = 30; move = 1;}
  if (snakePosition == 32 && snakeLastDirection == 1 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 2; snakePosition = 29; move = 1;} else if (pickOne == 3) {snakeLastDirection = 0; snakePosition = 33; move = 1;} else {snakePosition = 28; move = 1;}}
  if (snakePosition == 32 && snakeLastDirection == 3 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 0; snakePosition = 36; move = 1;} else if (pickOne == 3) {snakeLastDirection = 2; snakePosition = 31; move = 1;}}
  if (snakePosition == 33 && snakeLastDirection == 0 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 34; move = 1;} else {snakeLastDirection = 3; snakePosition = 35; move = 1;}} 
  if (snakePosition == 33 && snakeLastDirection == 2 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 28; move = 1;} else if (pickOne == 3) {snakeLastDirection = 3; snakePosition = 32; move = 1;} else {snakePosition = 29; move = 1;}}
  if (snakePosition == 34 && snakeLastDirection == 3 && move == 0)  {if (pickOne == 3) {snakeLastDirection = 2; snakePosition = 33; move = 1;} else {snakePosition = 35; move = 1;}}
  if (snakePosition == 34 && snakeLastDirection == 1 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 2; snakePosition = 27; move = 1;} else {snakeLastDirection = 1; snakePosition = 26; move = 1;}}
  if (snakePosition == 35 && snakeLastDirection == 3 && move == 0)  {snakeLastDirection = 2; snakePosition = 36; move = 1;}
  if (snakePosition == 35 && snakeLastDirection == 1 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 2; snakePosition = 33; move = 1;} else {snakeLastDirection = 1; snakePosition = 34; move = 1;}}
  if (snakePosition == 36 && snakeLastDirection == 0 && move == 0)  {snakeLastDirection = 1; snakePosition = 35; move = 1;}
  if (snakePosition == 36 && snakeLastDirection == 2 && move == 0)  {if (pickOne == 1) {snakeLastDirection = 1; snakePosition = 32; move = 1;} else {snakePosition = 31; move = 1;}}
  if (snakePosition == foodSpot) { oldsnakecolor = spotcolor;  snakeWaiting = 1; foodSpot = 40;}  //did snake find the food, change snake color
  if (snakeWaiting > 0) {snakeWaiting = snakeWaiting + 1;} //counting while waiting
#if HAS_SOUNDDETECTOR
  int audio_input = i2sWaveformRead(); 
  int Level = map(audio_input, 0, 1023, 1, 10);
  if (audio_input < 100){  Level = 1;  }
  if (audio_input > 1023){  Level = 10;  }
#else
  int Level = 10;
#endif

  if (snakeWaiting > random((30/Level),(600/Level))) {snakeWaiting = 0; foodSpot = random(SPECTRUM_PIXELS-1); spotcolor = CHSV(random(0, 255), 255, 255); if (getSlower > 3){  getSlower =  getSlower / 3;  }}  //waiting time is up, pick new spot, new color, reset speed
  if (getSlower > 1000){  getSlower = 1000;  }
  int fake =  foodSpot * LEDS_PER_SEGMENT;   //draw food
  for (byte s=0; s<LEDS_PER_SEGMENT; s++ ){              // 7 LEDs per segment
    if (foodSpot != 40)     {  LEDs[FAKE_LEDs_C_BRTL[s+((fake))]] = spotcolor; }  //draw food, but not while waiting
  }
  int fake2 =  snakePosition * LEDS_PER_SEGMENT;   //draw snake
  for (byte s=0; s<LEDS_PER_SEGMENT; s++ ){              // 7 LEDs per segment
    LEDs[FAKE_LEDs_C_BRTL[s+((fake2))]] = oldsnakecolor;
  } 
  for (byte j = 0; j < SPECTRUM_PIXELS; j++) {  //slowly erase snake
    int fake3 =  j * LEDS_PER_SEGMENT;
    for (byte s=0; s<LEDS_PER_SEGMENT; s++ ){              // 7 LEDs per segment
      LEDs[FAKE_LEDs_C_BRTL[s+((fake3))]].fadeToBlackBy( fadeby );
     }
  }
  getSlower = getSlower + 40;
  if (clockMode != 5) { allBlank(); }
} //snake

void Cylon() {
  int fake = 0;
  const uint8_t CYLON[12] = {4,8,13,18,23,28,32,28,23,18,13,8};
  if (cylonPosition >=12) { cylonPosition = 0;}
  fake = CYLON[cylonPosition] * LEDS_PER_SEGMENT;
  for (byte s=0; s<LEDS_PER_SEGMENT; s++ ){LEDs[FAKE_LEDs_SNAKE[s+((fake))]] = CRGB::Red;}
  cylonPosition++;
  if (cylonPosition >=12) { cylonPosition = 0;}
  for (byte j = 0; j < SPECTRUM_PIXELS; j++) {
    int fake =  j * LEDS_PER_SEGMENT;
    for (byte s=0; s<LEDS_PER_SEGMENT; s++ ){              // 7 LEDs per segment
      LEDs[FAKE_LEDs_SNAKE[s+((fake))]].fadeToBlackBy( 120 );
    }
  }
  if (clockMode != 5) { allBlank(); }
} //Cylon


void loadSetupSettings(){  //setting stored in preffs and loaded at boot
  preferences.begin("shelfclock", false);
  preferences.getBytes("weatherapi", &weatherapi, preferences.getBytesLength("weatherapi"));
  //   ssid = preferences.getChar("ssid");
  //    password = preferences.getChar("password");
}


#if HAS_BUZZER
void getListOfSongs() {
  totalSongs = 0;
  bool fileBad = 0;
  SONGS.clear();
  File dir = FileFS.open("/songs");    //open root of sound directory
  String allowedChars = "0123456789 _.-=#,:ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqwerstuvwxyz\n\r";
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) { break; }
    if (!entry.isDirectory()) {   //make sure its a file and not a directory
      char processedText[255];
      strcpy(processedText, "/songs/");
      strcat(processedText, entry.name());  //build full pathname
      entry.close();
     File file = FileFS.open(processedText, "r");  //open each song
      if(!file){Serial.println("No Saved Data!"); return;}
      String output;
      while (file.available()) {     //read song file contents
        char intRead = file.read();
        if (allowedChars.indexOf(intRead) != -1){ output += intRead; } else { Serial.print(processedText); Serial.print(" contains non-RTTTL characters! "); Serial.println(intRead); fileBad = 1; break;}
      }
      file.close();
      if (!fileBad) {
            char *token;
            char SongName[512];
            const char *delimiter =":";
            if (output.length() > 511) {Serial.print(processedText); Serial.println(" file is too big!"); deleteFile(FileFS, processedText); break;}
            sprintf(SongName, "%s", output.c_str());    //throw whole contents into variable
            token = strtok(SongName, delimiter);    //keep only the contents before first :
            if (strlen(token) > 32) {Serial.print(processedText); Serial.println(" Song name too long!"); deleteFile(FileFS, processedText); break;}
            if (strlen(processedText) > 40) {Serial.print(processedText); Serial.println(" Filename too long!");Serial.print(strlen(processedText)); deleteFile(FileFS, processedText); break;}
            File file = FileFS.open(processedText, "r");  //open each song
            char buffer[1024]; // Adjust the buffer size as per your requirement
            memset(buffer, 0, sizeof(buffer));
            size_t bytesRead = file.readBytes(buffer, sizeof(buffer));
            file.close();  // Close the file
            if (bytesRead == 0) {   // Check if any bytes were read
              Serial.print(processedText);
              Serial.println("Empty RTTTL file!");
              deleteFile(FileFS, processedText);
              break;
            }
            if (validate_rtttl(buffer)) { // Validate the RTTTL contents
              Serial.print(processedText);
              Serial.println(" is valid!");
              sprintf(songsArray[totalSongs], "%s",token);         //throw song title from file into array
              sprintf(filesArray[totalSongs], "%s", processedText);  //throw full path and name into array
              SONGS[songsArray[totalSongs]] = filesArray[totalSongs]; 
              totalSongs++;
            } else {
              Serial.print(processedText); 
              Serial.println(" Invalid RTTTL format!");
              deleteFile(FileFS, processedText); 
              //   break;
            }
      } else {
        FileFS.remove(processedText);
        fileBad = 0;
      }
    }
  }
  totalSongs--;  //strip off last file count
  Serial.print("Total Songs found: ");
  Serial.println(totalSongs+1);  //tell the human a human file count
  serializeJson(SONGS, Serial);
  Serial.println(" ");
  Serial.println(sizeof(SONGS));
  listDir(FileFS, "/songs/", 0);
  return;
}

bool is_valid_attribute(int value, const int* valid_values, int array_size) {
  for (int i = 0; i < array_size; i++) {
    if (valid_values[i] == value) return true;
  }
  return false;
}

bool validate_rtttl(char* rtttl) {
  // Parsing variables
  int duration, octave, beats;
  char delimiter;
  // Check format and extract metadata
  if (sscanf(rtttl, "%*[^:]:d=%d,o=%d,b=%d%c", &duration, &octave, &beats, &delimiter) != 4 ||
      !is_valid_attribute(duration, valid_durations, sizeof(valid_durations) / sizeof(valid_durations[0])) ||
      !is_valid_attribute(octave, valid_octaves, sizeof(valid_octaves) / sizeof(valid_octaves[0])) ||
      !is_valid_attribute(beats, valid_beats, sizeof(valid_beats) / sizeof(valid_beats[0])) ||
      delimiter != ':') {
    Serial.println(rtttl);
    Serial.println(" Invalid metadata");
    return false; // Invalid format or metadata
  }
  // Locate the start of notes section
  const char* pos = strchr(strchr(rtttl, ':') + 1, ':') + 1;
  // Validate notes
while (*pos && *pos != '\0') {
  char token[7];
  sscanf(pos, "%6[^,]%c", token, &delimiter);
  // Validate the token against the allowed patterns
  bool isValidToken = false;
  if (strlen(token) == 1) {
    if (strchr("abcdefghp", token[0])) {
      isValidToken = true;
    }
  } else if (strlen(token) == 2) {
    if ((strchr("abcdefghp", token[0]) && token[1] == '#') ||
        (strchr("abcdefghp", token[0]) && token[1] == '.') ||
        (strchr("abcdefghp", token[0]) && strchr("12345678", token[1])) ||
        (strchr("1248", token[0]) && strchr("abcdefghp", token[1]))) {
      isValidToken = true;
    }
  } else if (strlen(token) == 3) {
    if ((strchr("abcdefghp", token[0]) && strchr("12345678", token[1]) && token[2] == '.') ||
        (strchr("abcdefghp", token[0]) && token[1] == '.'&& strchr("12345678", token[2])) ||
        (strchr("abcdefghp", token[0]) && token[1] == '#' && token[2] == '.') ||
        (strchr("abcdefghp", token[0]) && token[1] == '#' && strchr("12345678", token[2])) ||
        (strchr("13", token[0]) && strchr("26", token[1]) && strchr("abcdefghp", token[2])) ||
        (strchr("1248", token[0]) && strchr("abcdefghp", token[1]) && token[2] == '#') ||
        (strchr("1248", token[0]) && strchr("abcdefghp", token[1]) && token[2] == '.') ||
        (strchr("1248", token[0]) && strchr("abcdefghp", token[1]) && strchr("12345678", token[2]))) {
      isValidToken = true;
    }
  } else if (strlen(token) == 4) {
    if ((strchr("1248", token[0]) && strchr("abcdefghp", token[1]) && token[2] == '#' && strchr("12345678", token[3])) ||
        (strchr("1248", token[0]) && strchr("abcdefghp", token[1]) && token[2] == '#' && token[3] == '.') ||
        (strchr("13", token[0]) && strchr("26", token[1]) && strchr("abcdefghp", token[2]) && token[3] == '.') ||
        (strchr("13", token[0]) && strchr("26", token[1]) && strchr("abcdefghp", token[2]) && strchr("12345678", token[3])) ||
        (strchr("13", token[0]) && strchr("26", token[1]) && strchr("abcdefghp", token[2]) && token[3] == '#') ||
        (strchr("abcdefghp", token[0]) && token[1] == '#' && strchr("12345678", token[2]) && token[3] == '.') ||
        (strchr("abcdefghp", token[0]) && token[1] == '#' && token[2] == '.' && strchr("12345678", token[3])) ||
        (strchr("1248", token[0]) && strchr("abcdefghp", token[1]) && token[2] == '.' && strchr("12345678", token[3])) ||
        (strchr("1248", token[0]) && strchr("abcdefghp", token[1]) && strchr("12345678", token[2]) && token[3] == '.')) {
      isValidToken = true;
    }
  } else if (strlen(token) == 5) {
    if ((strchr("13", token[0]) && strchr("26", token[1]) && strchr("abcdefghp", token[2]) && token[3] == '#' && token[4] == '.') ||
        (strchr("1248", token[0]) && strchr("abcdefghp", token[1]) && token[2] == '#'  && token[3] == '.'&& strchr("12345678", token[4])) ||
        (strchr("1248", token[0]) && strchr("abcdefghp", token[1]) && token[2] == '#' && strchr("12345678", token[3]) && token[4] == '.') ||
        (strchr("13", token[0]) && strchr("26", token[1]) && strchr("abcdefghp", token[2]) && token[3] == '.' && strchr("12345678", token[4])) ||
        (strchr("13", token[0]) && strchr("26", token[1]) && strchr("abcdefghp", token[2]) && strchr("12345678", token[3]) && token[4] == '.') ||
        (strchr("13", token[0]) && strchr("26", token[1]) && strchr("abcdefghp", token[2]) && token[3] == '#' && strchr("12345678", token[4]))) {
      isValidToken = true;
    }
  } else if (strlen(token) == 6) {
    if ((strchr("13", token[0]) && strchr("26", token[1]) && strchr("abcdefghp", token[2]) && token[3] == '#' && strchr("12345678", token[4]) && token[5] == '.') ||
    (strchr("13", token[0]) && strchr("26", token[1]) && strchr("abcdefghp", token[2]) && token[3] == '#' && token[4] == '.' && strchr("12345678", token[5]))) {
      isValidToken = true;
    }
  } 
  if (isValidToken) {
  //  Serial.println(token);
  } else {
    Serial.print(token);
    Serial.println(" Invalid note format");
    return false; // Invalid note format
  }
  pos = strchr(pos, ',');
  if (pos == NULL) {
    return true; // RTTTL file is valid
    break; // Exit the loop if no more comma found
  }
  pos += 1; // Move to the next note after the comma
}
  return true; // RTTTL file is valid
}
#endif


void writeFile(fs::FS &fs, const char * path, const char * message){
   Serial.printf("Writing file: %s\r\n", path);
   File file = fs.open(path, FILE_WRITE);
   if(!file){
      Serial.println("− failed to open file for writing");
      return;
   }
   if(file.print(message)){
      Serial.println("− file written");
   }else {
      Serial.println("− frite failed");
   }
}

#if HAS_BUZZER
void playRTTTLsong(String goforit, int chimeNumber)
{
  File file = FileFS.open(goforit, "r");
          Serial.print("goforit ");
          Serial.println(goforit);
  //Check if the file exists
  if(!file){
    Serial.println("No Saved Data!"); 
    return;
  }
  String output;
  while (file.available()) {
    char intRead = file.read();
    output += intRead;
  }
  file.close();
  pinMode(BUZZER_PIN, OUTPUT);
  rtttl::begin(BUZZER_PIN, output.c_str());  
  while( !rtttl::done() ){ rtttl::play();}
}
#endif

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
   Serial.printf("Listing directory: %s\r\n", dirname);
   File root = fs.open(dirname);
   if(!root){
      Serial.println("− failed to open directory");
      return;
   }
   if(!root.isDirectory()){
      Serial.println(" − not a directory");
      return;
   }
   File file = root.openNextFile();
   while(file){
      if(file.isDirectory()){
         Serial.print("  DIR : ");
         Serial.println(file.name());
         if(levels){
            listDir(fs, file.name(), levels -1);
         }
      } else {
         Serial.print("  FILE: ");
         Serial.print(file.name());
         Serial.print("\tSIZE: ");
         Serial.println(file.size());
      }
      file = root.openNextFile();
   }
}


void deleteFile(fs::FS &fs, const char * path){
   Serial.printf("Deleting file: %s\r\n", path);
   if(fs.remove(path)){
      Serial.println("− file deleted");
   } else {
      Serial.println("− delete failed");
   }
}





void processSchedules(bool alarmType) {
    int fileChanged = 0;
    int hour = timeinfo.tm_hour;
    int mins = timeinfo.tm_min;
    int minsSixty = 101;  //prime number
    if (mins == 0) {minsSixty = 60;}
    else if (mins == 45) {minsSixty = 45;}
    else if (mins == 30) {minsSixty = 30;}
    else if (mins == 15) {minsSixty = 15;}
    int dayOfweek = timeinfo.tm_wday;
    int mday = timeinfo.tm_mday;
    int mont = timeinfo.tm_mon + 1;
    int years = (timeinfo.tm_year +1900);
    int currentEOM = 0;
    //  Serial.println("testing...................");
    int object_count = jsonScheduleData.size();
    for (int i = 0; i < (object_count); i++) {
      JsonObject jsonObj = jsonScheduleData[i];
      int scheduleType = (int)jsonObj["scheduleType"];
      int mon = (int)jsonObj["mon"];
      int tue = (int)jsonObj["tue"];
      int wed = (int)jsonObj["wed"];
      int thu = (int)jsonObj["thu"];
      int fri = (int)jsonObj["fri"];
      int sat = (int)jsonObj["sat"];
      int sun = (int)jsonObj["sun"];
      int dailyType = (int)jsonObj["dailyType"];
      int dom = (int)jsonObj["dom"];
      int hours = (int)jsonObj["hours"];
      int minutes = (int)jsonObj["minutes"];
      String song = jsonObj["song"].as<String>();
      String title = jsonObj["title"].as<String>();
      int hourlyType = (int)jsonObj["hourlyType"];
      int gongmode = (int)jsonObj["gongmode"];
      int daymode = (int)jsonObj["daymode"];
      int year = (int)jsonObj["year"];
      int month = (int)jsonObj["month"];
      int day = (int)jsonObj["day"];
      int recurring = (int)jsonObj["recurring"];
      String uid = jsonObj["uid"].as<String>();
/*
    "scheduleType": "3",
    "dom": "00",
    "date": "2023-05-18",
    "hours": "99",
    "minutes": "99",
    "song": "MacGyver",
    "title": "Scott's Birthday",
    "mon": "0",
    "tue": "0",
    "wed": "0",
    "thu": "0",
    "fri": "0",
    "sat": "0",
    "sun": "0",
    "dailyType": "9",
    "hourlyType": "99",
    "gongmode": "0",
    "daymode": "0",
    "year": "9999",
    "month": "05",
    "day": "18",
    "recurring": "0",
    "uid": "00000009999000099990518"
  

      Serial.println("-----local--------");
      Serial.print("record: ");
      Serial.print(i);
      Serial.print(" of ");
      Serial.println(object_count - 1);
      Serial.print("dayOfweek: ");
      Serial.println(dayOfweek);
      Serial.print("mont: ");
      Serial.println(mont);
      Serial.print("mday: ");
      Serial.println(mday);
      Serial.print("hour: ");
      Serial.println(hour);
      Serial.print("mins: ");
      Serial.println(mins);
      Serial.print("minsSixty: ");
      Serial.println(minsSixty);
      Serial.println("-----timers---------");
      Serial.print("scheduleType: ");
      Serial.println(scheduleType);
      Serial.print("dom: ");
      Serial.println(dom);
      Serial.print("hours: ");
      Serial.println(hours);
      Serial.print("minutes: ");
      Serial.println(minutes);
      Serial.print("song: ");
      Serial.println(song);
      Serial.print("title: ");
      Serial.println(title);
      Serial.println("----dow-------");
      Serial.println(mon);
      Serial.println(tue);
      Serial.println(wed);
      Serial.println(thu);
      Serial.println(fri);
      Serial.println(sat);
      Serial.println(sun);
      Serial.print("dailyType: ");
      Serial.println(dailyType);
      Serial.print("hourlyType: ");
      Serial.println(hourlyType);
      Serial.print("gongmode: ");
      Serial.println(gongmode);
      Serial.print("daymode: ");
      Serial.println(daymode);
      Serial.print("year: ");
      Serial.println(year);
      Serial.print("month: ");
      Serial.println(month);
      Serial.print("day: ");
      Serial.println(day);
      Serial.print("recurring: ");
      Serial.println(recurring);
      Serial.println("----------------");
      Serial.println("");
*/

   if (scheduleType == 0)  {   //0 = daily schedule type
      if ((dayOfweek == 0 && sun == 1) || (dayOfweek == 1 && mon == 1) || (dayOfweek == 2 && tue == 1) || (dayOfweek == 3 && wed == 1) || (dayOfweek == 4 && thu == 1) || (dayOfweek == 5 && fri == 1) || (dayOfweek == 6 && sat == 1)) {   //day of week match?
        if (((daymode == 1) && ((hour >= 8) && (hour <=19 ))) || (daymode == 0)) {  //datlight mode is right or not on?
          if (((dailyType == 1) && ((hours == hour && minutes == mins) || (hours == 99 && minutes == mins) || (hours == hour && mins == 99 && minutes == 0))) || (((hourlyType == 15) && (minsSixty % 15 == 0)) || ((hourlyType == 30) && (minsSixty % 30 == 0)) || ((hourlyType == 60) && (minsSixty % 60 == 0))) || ((dailyType == 0) && ((hourlyType == 99) && (minsSixty % 60 == 0)))) {  //specific time or hourly type is right, or hourlytype 15,30,60?        
            #if HAS_BUZZER
              if (gongmode && (dailyType == 0) && (hourlyType == 99) && (minsSixty % 60 == 0)){  //hourly bongs should always run as secondary because of the uid file name
                // Makes sure the bongs are between 1 and 12. 
                int numberofChimes = hour;
                if (numberofChimes == 0) numberofChimes = 12;
                else if (numberofChimes > 12) numberofChimes -= 12;
                for(int i = 0; i < numberofChimes; i++) {
                  playRTTTLsong(SONGS[song], 1);
                 }
                }  else {
                playRTTTLsong(SONGS[song], 1);
                }
            #endif
           // if (!breakOutSet) {scroll(title);}
          //  allBlank();
          }
        }
      }
   }  //end of daily schedule type


   if (scheduleType == 1)  {   //1 = Monthly schedule type
    if (mont == 2) {    // Function to calculate the number of days in a given month and year
      if (years % 4 == 0) {
        if (years % 100 == 0) {
          if (years % 400 == 0) {
            currentEOM = 29;
          } else {
            currentEOM = 28;
          }
        } else {
          currentEOM = 29;
        }
      } else {
        currentEOM = 28;
      }
    } else if (mont == 4 || mont == 6 || mont == 9 || mont == 11) {
      currentEOM = 30;
    } else {
      currentEOM = 31;
    }

    if ((dom == mday) || (dom == 34 && (mday == currentEOM)) || (dom == 33 && (mday == (currentEOM-1))) || (dom == 32 && (mday == (currentEOM-2))))  {    // Check DOM and EOM
      if (((hours == hour && minutes == mins) || (hours == 99 && minutes == mins) || (hours == hour && mins == 99 && minutes == 0))) {  //time, some minute all day, sloppy top of hour
            #if HAS_BUZZER
              playRTTTLsong(SONGS[song], 1);
            #endif
              if (!breakOutSet) {scroll(title);}
              allBlank(); 
      } else {
            #if HAS_BUZZER
        if (!jsonObj["song"].isNull()) {
            strcpy(specialAudibleAlarm, jsonObj["song"]);
           
      //    Serial.println(defaultAudibleAlarm);
      //    Serial.println(specialAudibleAlarm);
        }
            #endif
        }
    } else {
            #if HAS_BUZZER
          sprintf(specialAudibleAlarm, "%s", defaultAudibleAlarm);
            #endif
      }
   }  //end of Monthly schedule type

   if (scheduleType == 2)  {   //2 = Date schedule type
        if (mont == month && mday == day && hours == hour && minutes == mins) {  //check month/day/hour/min
          if (!recurring && year == years) { //no recurring? check year 
            char processedName[40];
            char processedText[64];
            strcpy(processedText, "/scheduler/");
            strcpy(processedName, jsonObj["uid"]);
            strcat(processedText, processedName);
            strcat(processedText, ".json");
            Serial.println(uid);
            Serial.println(processedText);
            //FileFS.remove(processedText);
            deleteFile(FileFS, processedText);  //delete non-recurring schedule
            fileChanged = 1;
          }
            #if HAS_BUZZER
              playRTTTLsong(SONGS[song], 1);
            #endif
          if (!breakOutSet) {scroll(title);}
          allBlank(); 
        }
   }  //end of Date schedule type


   if (scheduleType == 3)  {   //3 = Special Date schedule type
     if (mont == month && mday == day) {
          Serial.println("special alarm");
          //fix in future, play song
        if (hour == 12 && mins == 0) {
            #if HAS_BUZZER
              playRTTTLsong(SONGS[song], 1);
            #endif
          if (!breakOutSet) {scroll(title);}
          allBlank(); 
        } 
            #if HAS_BUZZER
        if (!jsonObj["song"].isNull()) {
            strcpy(specialAudibleAlarm, jsonObj["song"]);
          Serial.println(defaultAudibleAlarm);
          Serial.println(specialAudibleAlarm);
        }
            #endif
     } else {
            #if HAS_BUZZER
          sprintf(specialAudibleAlarm, "%s", defaultAudibleAlarm);
            #endif
     }
   }  //end of Special Date schedule type

   if (scheduleType == 4)  {   //4 = New Years schedule type
    if (mont == 01 && mday == 01 && hour == 0 && mins == 0) {
            #if HAS_BUZZER
              playRTTTLsong("/settings/auldlang.rttl", 1);
            #endif
          if (!breakOutSet) {scroll(title);}
          allBlank(); 
    }
   }  //end of New Years schedule type

//  Serial.println("testing...................");
}
  if (fileChanged) { createSchedulesArray(); }
}


void createSchedulesArray() {
  JsonArray dataArray = jsonScheduleData.to<JsonArray>(); // create a JSON array to store the data
  File dir = FileFS.open("/scheduler"); // open the directory containing the text files
  while (true) { // loop through each file in the directory
    File file = dir.openNextFile();
    if (!file) break; // end the loop when there are no more files
    String fileName = file.name();
    if (fileName.endsWith(".json")) { // only read files that have a .txt extension
      String fileContent = file.readString(); // read the contents of the file
      JsonObject fileData = dataArray.createNestedObject(); // create a JSON object to store the file data
      DynamicJsonDocument jsonScheduleDataTemp(1024);
      DeserializationError error = deserializeJson(jsonScheduleDataTemp, fileContent);
      if (error) {
        Serial.println("Failed to parse JSON file");
        return;
      }
      // Get the values from the JSON object
      JsonObject jsonObj = jsonScheduleDataTemp.as<JsonObject>();
      int scheduleType = jsonObj["scheduleType"];
      int mon = jsonObj["mon"];
      int tue = jsonObj["tue"];
      int wed = jsonObj["wed"];
      int thu = jsonObj["thu"];
      int fri = jsonObj["fri"];
      int sat = jsonObj["sat"];
      int sun = jsonObj["sun"];
      int dailyType = jsonObj["dailyType"];
      int dom = jsonObj["dom"];
      int hours = jsonObj["hours"];
      int minutes = jsonObj["minutes"];
      String song = jsonObj["song"];
      String title = jsonObj["title"];
      int hourlyType = jsonObj["hourlyType"];
      int gongmode = jsonObj["gongmode"];
      int daymode = jsonObj["daymode"];
      int year = jsonObj["year"];
      int month = jsonObj["month"];
      int day = jsonObj["day"];
      int recurring = jsonObj["recurring"];
      String uid = jsonObj["uid"];
      /* Display the values for testing
      Serial.print("scheduleType = ");
      Serial.println(scheduleType);
      Serial.print("mon = ");
      Serial.println(mon);
      Serial.print("tue = ");
      Serial.println(tue);
      Serial.print("wed = ");
      Serial.println(wed);
      Serial.print("thu = ");
      Serial.println(thu);
      Serial.print("fri = ");
      Serial.println(fri);
      Serial.print("sat = ");
      Serial.println(sat);
      Serial.print("sun = ");
      Serial.println(sun);
      Serial.print("dailyType = ");
      Serial.println(dailyType);
      Serial.print("dom = ");
      Serial.println(dom);
      Serial.print("hours = ");
      Serial.println(hours);
      Serial.print("minutes = ");
      Serial.println(minutes);
      Serial.print("song = ");
      Serial.println(song);
      Serial.print("title = ");
      Serial.println(title);
      Serial.print("hourlyType = ");
      Serial.println(hourlyType);
      Serial.print("gongmode = ");
      Serial.println(gongmode);
      Serial.print("daymode = ");
      Serial.println(daymode);
      Serial.print("year = ");
      Serial.println(year);
      Serial.print("month = ");
      Serial.println(month);
      Serial.print("day = ");
      Serial.println(day);
      Serial.print("recurring = ");
      Serial.println(recurring);
      Serial.print("uid = ");
      Serial.println(uid);
      */
      fileData["scheduleType"] = scheduleType;
      fileData["mon"] = mon;
      fileData["tue"] = tue;
      fileData["wed"] = wed;
      fileData["thu"] = thu;
      fileData["fri"] = fri;
      fileData["sat"] = sat;
      fileData["sun"] = sun;
      fileData["dailyType"] = dailyType;
      fileData["dom"] = dom;
      fileData["hours"] = hours;
      fileData["minutes"] = minutes;
      fileData["song"] = song;
      fileData["title"] = title;
      fileData["hourlyType"] = hourlyType;
      fileData["gongmode"] = gongmode;
      fileData["daymode"] = daymode;
      fileData["year"] = year;
      fileData["month"] = month;
      fileData["day"] = day;
      fileData["recurring"] = recurring;
      fileData["uid"] = uid;
    }
    file.close();
  }
  serializeJson(jsonScheduleData, Serial); // output the entire JSON array to the console
  Serial.println("");
}






void getclockSettings(String fileType) {
  char new_filename[50];
  sprintf(new_filename, "/settings/clockSettings-%s.json", fileType); //build filename and path from argument
  Serial.print("Opening ");
  Serial.println(new_filename);
  JsonArray dataArray = jsonDoc.to<JsonArray>(); // create a JSON array to store the data
  File file = FileFS.open(new_filename); // open the directory containing the text files
  String fileContent = file.readString(); // read the contents of the file
  JsonObject fileData = dataArray.createNestedObject(); // create a JSON object to store the file data
  Serial.println(fileContent);
  //Parse the JSON string into a JSON object
  DynamicJsonDocument jsonDocu(8192);
  DeserializationError error = deserializeJson(jsonDocu, fileContent);
  if (error) {
    Serial.println("Failed to parse JSON file");
    return;
  }
  // Set the values from the JSON object
  JsonObject jsonObj = jsonDocu.as<JsonObject>();
  ClockColorSettings = jsonObj["ClockColorSettings"].as<byte>();
  ColorChangeFrequency = jsonObj["ColorChangeFrequency"].as<byte>();
  DSTime = jsonObj["DSTime"].as<bool>();
  DateColorSettings = jsonObj["DateColorSettings"].as<byte>();
  b0_val = jsonObj["b0_val"].as<byte>();
  b10_val = jsonObj["b10_val"].as<byte>();
  b11_val = jsonObj["b11_val"].as<byte>();
  b12_val = jsonObj["b12_val"].as<byte>();
  b13_val = jsonObj["b13_val"].as<byte>();
  b14_val = jsonObj["b14_val"].as<byte>();
  b15_val = jsonObj["b15_val"].as<byte>();
  b16_val = jsonObj["b16_val"].as<byte>();
  b17_val = jsonObj["b17_val"].as<byte>();
  b1_val = jsonObj["b1_val"].as<byte>();
  b2_val = jsonObj["b2_val"].as<byte>();
  b3_val = jsonObj["b3_val"].as<byte>();
  b4_val = jsonObj["b4_val"].as<byte>();
  b5_val = jsonObj["b5_val"].as<byte>();
  b6_val = jsonObj["b6_val"].as<byte>();
  b7_val = jsonObj["b7_val"].as<byte>();
  b8_val = jsonObj["b8_val"].as<byte>();
  b9_val = jsonObj["b9_val"].as<byte>();
  brightness = jsonObj["brightness"].as<byte>();
  cd_b_val = jsonObj["cd_b_val"].as<byte>();
  cd_g_val = jsonObj["cd_g_val"].as<byte>();
  cd_r_val = jsonObj["cd_r_val"].as<byte>();
  clockDisplayType = jsonObj["clockDisplayType"].as<byte>();
  clockMode = jsonObj["clockMode"].as<byte>();
  colonType = jsonObj["colonType"].as<int>();
  colorchangeCD = jsonObj["colorchangeCD"].as<bool>();
  dateDisplayType = jsonObj["dateDisplayType"].as<byte>();
  g0_val = jsonObj["g0_val"].as<byte>();
  g10_val = jsonObj["g10_val"].as<byte>();
  g11_val = jsonObj["g11_val"].as<byte>();
  g12_val = jsonObj["g12_val"].as<byte>();
  g13_val = jsonObj["g13_val"].as<byte>();
  g14_val = jsonObj["g14_val"].as<byte>();
  g15_val = jsonObj["g15_val"].as<byte>();
  g16_val = jsonObj["g16_val"].as<byte>();
  g17_val = jsonObj["g17_val"].as<byte>();
  g1_val = jsonObj["g1_val"].as<byte>();
  g2_val = jsonObj["g2_val"].as<byte>();
  g3_val = jsonObj["g3_val"].as<byte>();
  g4_val = jsonObj["g4_val"].as<byte>();
  g5_val = jsonObj["g5_val"].as<byte>();
  g6_val = jsonObj["g6_val"].as<byte>();
  g7_val = jsonObj["g7_val"].as<byte>();
  g8_val = jsonObj["g8_val"].as<byte>();
  g9_val = jsonObj["g9_val"].as<byte>();
  gmtOffset_sec = jsonObj["gmtOffset_sec"].as<long>();
  humiColorSettings = jsonObj["humiColorSettings"].as<byte>();
  humiDisplayType = jsonObj["humiDisplayType"].as<byte>();
  lightshowMode = jsonObj["lightshowMode"].as<int>();
  pastelColors = jsonObj["pastelColors"].as<byte>();
  r0_val = jsonObj["r0_val"].as<byte>();
  r10_val = jsonObj["r10_val"].as<byte>();
  r11_val = jsonObj["r11_val"].as<byte>();
  r12_val = jsonObj["r12_val"].as<byte>();
  r13_val = jsonObj["r13_val"].as<byte>();
  r14_val = jsonObj["r14_val"].as<byte>();
  r15_val = jsonObj["r15_val"].as<byte>();
  r16_val = jsonObj["r16_val"].as<byte>();
  r17_val = jsonObj["r17_val"].as<byte>();
  r1_val = jsonObj["r1_val"].as<byte>();
  r2_val = jsonObj["r2_val"].as<byte>();
  r3_val = jsonObj["r3_val"].as<byte>();
  r4_val = jsonObj["r4_val"].as<byte>();
  r5_val = jsonObj["r5_val"].as<byte>();
  r6_val = jsonObj["r6_val"].as<byte>();
  r7_val = jsonObj["r7_val"].as<byte>();
  r8_val = jsonObj["r8_val"].as<byte>();
  r9_val = jsonObj["r9_val"].as<byte>();
  randomSpectrumMode = jsonObj["randomSpectrumMode"].as<byte>();
  realtimeMode = jsonObj["realtimeMode"].as<int>();
  scrollColorSettings = jsonObj["scrollColorSettings"].as<int>();
  scrollFrequency = jsonObj["scrollFrequency"].as<int>();
  scrollOptions1 = jsonObj["scrollOptions1"].as<bool>();
  scrollOptions2 = jsonObj["scrollOptions2"].as<bool>();
  scrollOptions3 = jsonObj["scrollOptions3"].as<bool>();
  scrollOptions4 = jsonObj["scrollOptions4"].as<bool>();
  scrollOptions5 = jsonObj["scrollOptions5"].as<bool>();
  scrollOptions6 = jsonObj["scrollOptions6"].as<bool>();
  scrollOptions7 = jsonObj["scrollOptions7"].as<bool>();
  scrollOptions8 = jsonObj["scrollOptions8"].as<bool>();
  scrollOverride = jsonObj["scrollOverride"].as<bool>();
  scrollText = jsonObj["scrollText"].as<String>();
  spectrumBackgroundSettings = jsonObj["spectrumBackgroundSettings"].as<byte>();
  spectrumColorSettings = jsonObj["spectrumColorSettings"].as<byte>();
  spectrumMode = jsonObj["spectrumMode"].as<int>();
  spotlightsColorSettings = jsonObj["spotlightsColorSettings"].as<int>();
  suspendFrequency = jsonObj["suspendFrequency"].as<int>();
  suspendType = jsonObj["suspendType"].as<byte>();
  tempColorSettings = jsonObj["tempColorSettings"].as<byte>();
  tempDisplayType = jsonObj["tempDisplayType"].as<byte>();
  temperatureCorrection = jsonObj["temperatureCorrection"].as<int>();
  temperatureSymbol = jsonObj["temperatureSymbol"].as<byte>();
  #if HAS_BUZZER
  useAudibleAlarm = jsonObj["useAudibleAlarm"].as<bool>();
  #endif
  useSpotlights = jsonObj["useSpotlights"].as<bool>();
  humidity_outdoor_enable = jsonObj["humidity_outdoor_enable"].as<bool>();
  temperature_outdoor_enable = jsonObj["temperature_outdoor_enable"].as<bool>();
  //weatherapi = jsonObj["weatherapi"];weatherapi.latitude

  // Close the file.
  file.close();
}  



void saveclockSettings(String fileType) {
  char new_filename[50];
  char old_filename[50];
  sprintf(new_filename, "/settings/clockSettings-%s.json.%05d", fileType, random(0,65535));  //rename old settings to new name with random number at end
  sprintf(old_filename, "/settings/clockSettings-%s.json", fileType);  //build filename and path from argument
  Serial.print("Saving ");
  Serial.println(new_filename);
  //renameFile(FileFS, old_filename, new_filename);//rename old settings to new name to ensure the new settings are writen to a new flash location (balencing)
  FileFS.rename(old_filename, new_filename);//rename old settings to new name to ensure the new settings are writen to a new flash location (balencing)
  DynamicJsonDocument doc(8192);
  JsonObject clockSettings = doc.to<JsonObject>();
  // Add the values to the JSON object
  /*
  clockSettings["gmtOffset_sec"] = -28800;
  clockSettings["DSTime"] = 0;
  clockSettings["cd_r_val"] = 0;
  clockSettings["cd_g_val"] = 255;
  clockSettings["cd_b_val"] = 0;
  clockSettings["r0_val"] = 193;
  clockSettings["g0_val"] = 204;
  clockSettings["b0_val"] = 78;
  clockSettings["r1_val"] = 255;
  clockSettings["g1_val"] = 0;
  clockSettings["b1_val"] = 0;
  clockSettings["r2_val"] = 255;
  clockSettings["g2_val"] = 0;
  clockSettings["b2_val"] = 0;
  clockSettings["r3_val"] = 255;
  clockSettings["g3_val"] = 0;
  clockSettings["b3_val"] = 0;
  clockSettings["r4_val"] = 255;
  clockSettings["g4_val"] = 0;
  clockSettings["b4_val"] = 0;
  clockSettings["r5_val"] = 255;
  clockSettings["g5_val"] = 0;
  clockSettings["b5_val"] = 0;
  clockSettings["r6_val"] = 255;
  clockSettings["g6_val"] = 0;
  clockSettings["b6_val"] = 0;
  clockSettings["r7_val"] = 255;
  clockSettings["g7_val"] = 0;
  clockSettings["b7_val"] = 0;
  clockSettings["r8_val"] = 255;
  clockSettings["g8_val"] = 0;
  clockSettings["b8_val"] = 0;
  clockSettings["r9_val"] = 255;
  clockSettings["g9_val"] = 0;
  clockSettings["b9_val"] = 0;
  clockSettings["r10_val"] = 255;
  clockSettings["g10_val"] = 0;
  clockSettings["b10_val"] = 0;
  clockSettings["r11_val"] = 255;
  clockSettings["g11_val"] = 0;
  clockSettings["b11_val"] = 0;
  clockSettings["r12_val"] = 255;
  clockSettings["g12_val"] = 0;
  clockSettings["b12_val"] = 0;
  clockSettings["r13_val"] = 255;
  clockSettings["g13_val"] = 0;
  clockSettings["b13_val"] = 0;
  clockSettings["r14_val"] = 255;
  clockSettings["g14_val"] = 0;
  clockSettings["b14_val"] = 0;
  clockSettings["r15_val"] = 255;
  clockSettings["g15_val"] = 0;
  clockSettings["b15_val"] = 0;
  clockSettings["r16_val"] = 255;
  clockSettings["g16_val"] = 255;
  clockSettings["b16_val"] = 255;
  clockSettings["r17_val"] = 0;
  clockSettings["g17_val"] = 0;
  clockSettings["b17_val"] = 0;
  clockSettings["clockMode"] = 11;
  clockSettings["pastelColors"] = 0;
  clockSettings["temperatureSymbol"] = 39;
  clockSettings["ClockColorSettings"] = 0;
  clockSettings["DateColorSettings"] = 0;
  clockSettings["tempColorSettings"] = 0;
  clockSettings["humiColorSettings"] = 0;
  clockSettings["tempDisplayType"] = 0;
  clockSettings["humiDisplayType"] = 0;
  clockSettings["temperatureCorrection"] = 0;
  clockSettings["colonType"] = 0;
  clockSettings["ColorChangeFrequency"] = 0;
  clockSettings["scrollText"] = "dAdS ArE tHE bESt";
  clockSettings["clockDisplayType"] = 3;
  clockSettings["dateDisplayType"] = 5;
  clockSettings["colorchangeCD"] = 1;
  clockSettings["useAudibleAlarm"] = 0;
  clockSettings["spectrumMode"] = 0;
  clockSettings["realtimeMode"] = 0;
  clockSettings["spectrumColorSettings"] = 2;
  clockSettings["spectrumBackgroundSettings"] = 0;
  clockSettings["spotlightsColorSettings"] = 0;
  clockSettings["brightness"] = 10;
  clockSettings["useSpotlights"] = 1;
  clockSettings["scrollColorSettings"] = 0;
  clockSettings["scrollFrequency"] = 1;
  clockSettings["randomSpectrumMode"] = 0;
  clockSettings["scrollOverride"] = 0;
  clockSettings["scrollOptions1"] = 0;
  clockSettings["scrollOptions2"] = 0;
  clockSettings["scrollOptions3"] = 0;
  clockSettings["scrollOptions4"] = 0;
  clockSettings["scrollOptions5"] = 0;
  clockSettings["scrollOptions6"] = 0;
  clockSettings["scrollOptions7"] = 0;
  clockSettings["scrollOptions8"] = 1;
  clockSettings["lightshowMode"] = 0;
  clockSettings["suspendFrequency"] = 1;
  clockSettings["suspendType"] = 0;
  clockSettings["temperature"] = "";
  clockSettings["humidity"] = "";
  clockSettings["weatherapi"] = "";
  clockSettings["humidity_outdoor_enable"] = 0;
  clockSettings["temperature_outdoor_enable"] = 0;
  */
  clockSettings["gmtOffset_sec"] = gmtOffset_sec;
  clockSettings["DSTime"] = DSTime;
  clockSettings["cd_r_val"] = cd_r_val;
  clockSettings["cd_g_val"] = cd_g_val;
  clockSettings["cd_b_val"] = cd_b_val;
  clockSettings["r0_val"] = r0_val;
  clockSettings["g0_val"] = g0_val;
  clockSettings["b0_val"] = b0_val;
  clockSettings["r1_val"] = r1_val;
  clockSettings["g1_val"] = g1_val;
  clockSettings["b1_val"] = b1_val;
  clockSettings["r2_val"] = r2_val;
  clockSettings["g2_val"] = g2_val;
  clockSettings["b2_val"] = b2_val;
  clockSettings["r3_val"] = r3_val;
  clockSettings["g3_val"] = g3_val;
  clockSettings["b3_val"] = b3_val;
  clockSettings["r4_val"] = r4_val;
  clockSettings["g4_val"] = g4_val;
  clockSettings["b4_val"] = b4_val;
  clockSettings["r5_val"] = r5_val;
  clockSettings["g5_val"] = g5_val;
  clockSettings["b5_val"] = b5_val;
  clockSettings["r6_val"] = r6_val;
  clockSettings["g6_val"] = g6_val;
  clockSettings["b6_val"] = b6_val;
  clockSettings["r7_val"] = r7_val;
  clockSettings["g7_val"] = g7_val;
  clockSettings["b7_val"] = b7_val;
  clockSettings["r8_val"] = r8_val;
  clockSettings["g8_val"] = g8_val;
  clockSettings["b8_val"] = b8_val;
  clockSettings["r9_val"] = r9_val;
  clockSettings["g9_val"] = g9_val;
  clockSettings["b9_val"] = b9_val;
  clockSettings["r10_val"] = r10_val;
  clockSettings["g10_val"] = g10_val;
  clockSettings["b10_val"] = b10_val;
  clockSettings["r11_val"] = r11_val;
  clockSettings["g11_val"] = g11_val;
  clockSettings["b11_val"] = b11_val;
  clockSettings["r12_val"] = r12_val;
  clockSettings["g12_val"] = g12_val;
  clockSettings["b12_val"] = b12_val;
  clockSettings["r13_val"] = r13_val;
  clockSettings["g13_val"] = g13_val;
  clockSettings["b13_val"] = b13_val;
  clockSettings["r14_val"] = r14_val;
  clockSettings["g14_val"] = g14_val;
  clockSettings["b14_val"] = b14_val;
  clockSettings["r15_val"] = r15_val;
  clockSettings["g15_val"] = g15_val;
  clockSettings["b15_val"] = b15_val;
  clockSettings["r16_val"] = r16_val;
  clockSettings["g16_val"] = g16_val;
  clockSettings["b16_val"] = b16_val;
  clockSettings["r17_val"] = r17_val;
  clockSettings["g17_val"] = g17_val;
  clockSettings["b17_val"] = b17_val;
  clockSettings["clockMode"] = clockMode;
  clockSettings["pastelColors"] = pastelColors;
  clockSettings["temperatureSymbol"] = temperatureSymbol;
  clockSettings["ClockColorSettings"] = ClockColorSettings;
  clockSettings["DateColorSettings"] = DateColorSettings;
  clockSettings["tempColorSettings"] = tempColorSettings;
  clockSettings["humiColorSettings"] = humiColorSettings;
  clockSettings["tempDisplayType"] = tempDisplayType;
  clockSettings["humiDisplayType"] = humiDisplayType;
  clockSettings["temperatureCorrection"] = temperatureCorrection;
  clockSettings["colonType"] = colonType;
  clockSettings["ColorChangeFrequency"] = ColorChangeFrequency;
  clockSettings["scrollText"] = scrollText;
  clockSettings["clockDisplayType"] = clockDisplayType;
  clockSettings["dateDisplayType"] = dateDisplayType;
  clockSettings["colorchangeCD"] = colorchangeCD;
  #if HAS_BUZZER
  clockSettings["useAudibleAlarm"] = useAudibleAlarm;
  #endif
  clockSettings["spectrumMode"] = spectrumMode;
  clockSettings["realtimeMode"] = realtimeMode;
  clockSettings["spectrumColorSettings"] = spectrumColorSettings;
  clockSettings["spectrumBackgroundSettings"] = spectrumBackgroundSettings;
  clockSettings["spotlightsColorSettings"] = spotlightsColorSettings;
  clockSettings["brightness"] = brightness;
  clockSettings["useSpotlights"] = useSpotlights;
  clockSettings["scrollColorSettings"] = scrollColorSettings;
  clockSettings["scrollFrequency"] = scrollFrequency;
  clockSettings["randomSpectrumMode"] = randomSpectrumMode;
  clockSettings["scrollOverride"] = scrollOverride;
  clockSettings["scrollOptions1"] = scrollOptions1;
  clockSettings["scrollOptions2"] = scrollOptions2;
  clockSettings["scrollOptions3"] = scrollOptions3;
  clockSettings["scrollOptions4"] = scrollOptions4;
  clockSettings["scrollOptions5"] = scrollOptions5;
  clockSettings["scrollOptions6"] = scrollOptions6;
  clockSettings["scrollOptions7"] = scrollOptions7;
  clockSettings["scrollOptions8"] = scrollOptions8;
  clockSettings["lightshowMode"] = lightshowMode;
  clockSettings["suspendFrequency"] = suspendFrequency;
  clockSettings["suspendType"] = suspendType;
  clockSettings["humidity_outdoor_enable"] = humidity_outdoor_enable;
  clockSettings["temperature_outdoor_enable"] = temperature_outdoor_enable;
  //clockSettings["weatherapi"] = weatherapi;
  // Serialize the JSON document to a string
  String jsonStr;
  serializeJson(doc, jsonStr);
  // Write the JSON string to a file
  File file = FileFS.open(old_filename, FILE_WRITE);
  if (file) {
    file.print(jsonStr);
    file.close();
    Serial.println("JSON data written to file");
  } else {
    Serial.println("Failed to open file for writing");
  }
}




















void loadWebPageHandlers() {
 
  server.on("/gethome", []() {
    DynamicJsonDocument json(1500);
    String output;
    if (updateSettingsRequired == 1) { saveclockSettings("generic"); updateSettingsRequired = 0; }  //if something was changed in settings, write those changes when the home page is loaded.
    json["scoreboardLeft"] = scoreboardLeft;
    json["scoreboardRight"] = scoreboardRight;
    json["HAS_DHT"] = HAS_DHT;
    json["HAS_RTC"] = HAS_RTC;
    json["HAS_SOUNDDETECTOR"] = HAS_SOUNDDETECTOR;
    json["HAS_BUZZER"] = HAS_BUZZER;
    json["HAS_PHOTOSENSOR"] = HAS_PHOTOSENSOR;
    json["HAS_ONLINEWEATHER"] = HAS_ONLINEWEATHER;
    serializeJson(json, output);
    server.send(200, "application/json", output);
  });

  server.on("/getsettings", []() {
    DynamicJsonDocument json(8192);
    String output;
    char spotlightcolor[8];
    char colorHour[8], colorMin[8], colorColon[8];
    char dayColor[8], monthColor[8], separatorColor[8];
    char tempColor[8], typeColor[8], degreeColor[8];
    char humiColor[8], humiDecimalColor[8], humiSymbolColor[8];
    char colorCD[8], scoreboardColorLeft[8], scoreboardColorRight[8];
    char spectrumColor[8], spectrumBackgroundColor[8], scrollingColor[8];
    json["ColorPalette"] = pastelColors;
    json["ColorChangeFrequency"] = ColorChangeFrequency;
    json["rangeBrightness"] = brightness;
    json["suspendType"] = suspendType;
    json["suspendFrequency"] = suspendFrequency;
    json["useSpotlights"] = useSpotlights;
    sprintf(spotlightcolor, "#%02X%02X%02X", r0_val, g0_val, b0_val);
    json["spotlightcolor"] = spotlightcolor;
    json["spotlightsColorSettings"] = spotlightsColorSettings;
    json["ClockDisplayType"] = clockDisplayType;
    json["ColonType"] = colonType;
    json["TimezoneSetting"] = gmtOffset_sec;
    json["DSTime"] = DSTime;
    json["ClockColorSettings"] = ClockColorSettings;
    sprintf(colorHour, "#%02X%02X%02X", r1_val, g1_val, b1_val);
    json["colorHour"] = colorHour;
    sprintf(colorMin, "#%02X%02X%02X", r2_val, g2_val, b2_val);
    json["colorMin"] = colorMin;
    sprintf(colorColon, "#%02X%02X%02X", r3_val, g3_val, b3_val);
    json["colorColon"] = colorColon;
    json["DateDisplayType"] = dateDisplayType;
    json["DateColorSettings"] = DateColorSettings;
    sprintf(dayColor, "#%02X%02X%02X", r4_val, g4_val, b4_val);
    json["dayColor"] = dayColor;
    sprintf(monthColor, "#%02X%02X%02X", r5_val, g5_val, b5_val);
    json["monthColor"] = monthColor;
    sprintf(separatorColor, "#%02X%02X%02X", r6_val, g6_val, b6_val);
    json["separatorColor"] = separatorColor;
    json["TempType"] = temperatureSymbol;
    json["CorrectionSelect"] = temperatureCorrection;
    json["TempDisplayType"] = tempDisplayType;
    json["TempColorSettings"] = tempColorSettings;
    sprintf(tempColor, "#%02X%02X%02X", r7_val, g7_val, b7_val);
    json["TempColor"] = tempColor;
    sprintf(typeColor, "#%02X%02X%02X", r8_val, g8_val, b8_val);
    json["TypeColor"] = typeColor;
    sprintf(degreeColor, "#%02X%02X%02X", r9_val, g9_val, b9_val);
    json["DegreeColor"] = degreeColor;
    json["HumiDisplayType"] = humiDisplayType;
    json["HumiColorSettings"] = humiColorSettings;
    sprintf(humiColor, "#%02X%02X%02X", r10_val, g10_val, b10_val);
    json["HumiColor"] = humiColor;
    sprintf(humiDecimalColor, "#%02X%02X%02X", r11_val, g11_val, b11_val);
    json["HumiDecimalColor"] = humiDecimalColor;
    sprintf(humiSymbolColor, "#%02X%02X%02X", r12_val, g12_val, b12_val);
    json["HumiSymbolColor"] = humiSymbolColor;
#if HAS_BUZZER
    json["useAudibleAlarm"] = useAudibleAlarm;
#endif
    json["colorchangeCD"] = colorchangeCD;
    sprintf(colorCD, "#%02X%02X%02X", cd_r_val, cd_g_val, cd_b_val);
    json["colorCD"] = colorCD;
    sprintf(scoreboardColorLeft, "#%02X%02X%02X", r13_val, g13_val, b13_val);
    json["scoreboardColorLeft"] = scoreboardColorLeft;
    sprintf(scoreboardColorRight, "#%02X%02X%02X", r14_val, g14_val, b14_val);
    json["scoreboardColorRight"] = scoreboardColorRight;
    json["randomSpectrumMode"] = randomSpectrumMode;
    sprintf(spectrumColor, "#%02X%02X%02X", r15_val, g15_val, b15_val);
    json["spectrumColor"] = spectrumColor;
    sprintf(spectrumBackgroundColor, "#%02X%02X%02X", r17_val, g17_val, b17_val);
    json["spectrumBackgroundColor"] = spectrumBackgroundColor;
    json["spectrumBackgroundSettings"] = spectrumBackgroundSettings;
    json["spectrumColorSettings"] = spectrumColorSettings;
    json["scrollFrequency"] = scrollFrequency;
    json["scrollOptions1"] = scrollOptions1;
    json["scrollOptions2"] = scrollOptions2;
    json["scrollOptions3"] = scrollOptions3;
    json["scrollOptions4"] = scrollOptions4;
    json["scrollOptions5"] = scrollOptions5;
    json["scrollOptions6"] = scrollOptions6;
    json["scrollOptions7"] = scrollOptions7;
    json["scrollOptions8"] = scrollOptions8;
    json["scrollText"] = scrollText;
    json["scrollOverride"] = scrollOverride;
    sprintf(scrollingColor, "#%02X%02X%02X", r16_val, g16_val, b16_val);
    json["scrollColor"] = scrollingColor;
    json["scrollColorSettings"] = scrollColorSettings;
    json["weatherapi"]["latitude"] = weatherapi.latitude;
    json["weatherapi"]["longitude"] = weatherapi.longitude;
    json["weatherapi"]["apikey"] = weatherapi.apikey;
    json["humidity_outdoor_enable"] = humidity_outdoor_enable;
    json["temperature_outdoor_enable"] = temperature_outdoor_enable;
    json["HAS_DHT"] = HAS_DHT;
    json["HAS_RTC"] = HAS_RTC;
    json["HAS_SOUNDDETECTOR"] = HAS_SOUNDDETECTOR;
    json["HAS_BUZZER"] = HAS_BUZZER;
    json["HAS_PHOTOSENSOR"] = HAS_PHOTOSENSOR;
    json["HAS_ONLINEWEATHER"] = HAS_ONLINEWEATHER;
  #if HAS_BUZZER
    json["listOfSong"] = SONGS;
  #endif
    serializeJson(json, output);
    server.send(200, "application/json", output);
  });

  server.on("/updateanything", HTTP_POST, []() {
    DynamicJsonDocument json(1500);
    if(server.args() > 0) {
      String body = server.arg(0);
      Serial.println("----");
      Serial.println(body);
      Serial.println("----");
      deserializeJson(json, server.arg(0));

      // ColorPalette
      if (!json["ColorPalette"].isNull()){
        pastelColors = (int)json["ColorPalette"];
        updateSettingsRequired = 1;
        allBlank();
      }

      // SuspendType
      if (!json["suspendType"].isNull()) {
        suspendType = (int)json["suspendType"];
        updateSettingsRequired = 1;
      }

      // spotlightsColorSettings
      if (!json["spotlightsColorSettings"].isNull()) {
        spotlightsColorSettings = (int)json["spotlightsColorSettings"];
        updateSettingsRequired = 1;
        ShelfDownLights(); 
      }

      // ClockDisplayType
      if (!json["ClockDisplayType"].isNull()) {
        clockDisplayType = (int)json["ClockDisplayType"];
        updateSettingsRequired = 1;
        if (clockMode == 0) { allBlank(); }  
      }

      // ColonType
      if (!json["ColonType"].isNull()) {
        colonType = (int)json["ColonType"];
        updateSettingsRequired = 1;
        if (clockMode == 0) { allBlank(); } 
      }

      // ClockColorSettings
      if (!json["ClockColorSettings"].isNull()) {
        ClockColorSettings = (int)json["ClockColorSettings"];
        updateSettingsRequired = 1;
        if (clockMode == 0) { allBlank(); } 
      }

      // DateDisplayType
      if (!json["DateDisplayType"].isNull()) {
        ClockColorSettings = (int)json["DateDisplayType"];
        updateSettingsRequired = 1;
        if (clockMode == 7) { allBlank(); }
      }

      // DateColorSettings
      if (!json["DateColorSettings"].isNull()) {
        DateColorSettings = (int)json["DateColorSettings"];
        updateSettingsRequired = 1;
        if (clockMode == 7) { allBlank(); } 
      }

      // TempType
      if (!json["TempType"].isNull()) {
        temperatureSymbol = (int)json["TempType"];
        updateSettingsRequired = 1;
        if (clockMode == 2) { allBlank(); } 
      }

      // TempDisplayType
      if (!json["TempDisplayType"].isNull()) {
        tempDisplayType = (int)json["TempDisplayType"];
        updateSettingsRequired = 1;
        if (clockMode == 2) { allBlank(); } 
      }

      // TempColorSettings
      if (!json["TempColorSettings"].isNull()) {
        tempColorSettings = (int)json["TempColorSettings"];
        updateSettingsRequired = 1;
        if (clockMode == 2) { allBlank(); }
      }

      // HumiDisplayType
      if (!json["HumiDisplayType"].isNull()) {
        humiDisplayType = (int)json["HumiDisplayType"];
        updateSettingsRequired = 1;
        if (clockMode == 8) { allBlank(); } 
      }

      // HumiColorSettings
      if (!json["HumiColorSettings"].isNull()) {
        humiColorSettings = (int)json["HumiColorSettings"];
        updateSettingsRequired = 1;
        if (clockMode == 8) { allBlank(); } 
      }

      // spectrumBackgroundSettings
      if (!json["spectrumBackgroundSettings"].isNull()) {
        spectrumBackgroundSettings = (int)json["spectrumBackgroundSettings"];
        updateSettingsRequired = 1;
        if (clockMode == 9) { allBlank(); } 
      }

      // spectrumColorSettings
      if (!json["spectrumColorSettings"].isNull()) {
        spectrumColorSettings = (int)json["spectrumColorSettings"];
        updateSettingsRequired = 1;
        if (clockMode == 9) { allBlank(); } 
      }

      // scrollColorSettings
      if (!json["scrollColorSettings"].isNull()) {
        scrollColorSettings = (int)json["scrollColorSettings"];
        updateSettingsRequired = 1;
        if (clockMode == 11) { allBlank(); } 
      }

      // ColorChangeFrequency
      if (!json["ColorChangeFrequency"].isNull()) {
        ColorChangeFrequency = (int)json["ColorChangeFrequency"];
        updateSettingsRequired = 1;
        allBlank(); 
      }

      // suspendFrequency
      if (!json["suspendFrequency"].isNull()) {
        suspendFrequency = (int)json["suspendFrequency"];
        updateSettingsRequired = 1;
      }

      // TimezoneSetting
      if (!json["TimezoneSetting"].isNull()) {
        gmtOffset_sec = (long)json["TimezoneSetting"];
        configTime(gmtOffset_sec, (daylightOffset_sec * DSTime), ntpServer);
        if(!getLocalTime(&timeinfo)){Serial.println("Error, no NTP Server found!");}
        #if HAS_RTC
          int tempyear = (timeinfo.tm_year +1900);
          int tempmonth = (timeinfo.tm_mon + 1);
          rtc.adjust(DateTime(tempyear, tempmonth, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
        #endif
        updateSettingsRequired = 1;
        if (clockMode == 0) { allBlank(); } 
        printLocalTime(); 
      }

      // CorrectionSelect
      if (!json["CorrectionSelect"].isNull()) {
        temperatureCorrection = (int)json["CorrectionSelect"];
        updateSettingsRequired = 1;
        if (clockMode == 2) { allBlank(); } 
      }

      // scrollFrequency
      if (!json["scrollFrequency"].isNull()) {
        scrollFrequency = (int)json["scrollFrequency"];
        updateSettingsRequired = 1;
        if (clockMode == 11) { allBlank(); } 
      }

      // rangeBrightness
      if (!json["rangeBrightness"].isNull()) {
        brightness = (int)json["rangeBrightness"];
        updateSettingsRequired = 1;
        ShelfDownLights();
      }
      
      // spotlightcolor
      if (!json["spotlightcolor"].isNull()) {
        r0_val = (int)json["spotlightcolor"]["r"];
        g0_val = (int)json["spotlightcolor"]["g"];
        b0_val = (int)json["spotlightcolor"]["b"];
        updateSettingsRequired = 1;
        ShelfDownLights();
      }

      // colorHour
      if (!json["colorHour"].isNull()) {
        r1_val = (int)json["colorHour"]["r"];
        g1_val = (int)json["colorHour"]["g"];
        b1_val = (int)json["colorHour"]["b"];
        updateSettingsRequired = 1;
        if (clockMode == 0) { allBlank(); } 
      }

      // colorMin
      if (!json["colorMin"].isNull()) {
        r2_val = (int)json["colorMin"]["r"];
        g2_val = (int)json["colorMin"]["g"];
        b2_val = (int)json["colorMin"]["b"];
        updateSettingsRequired = 1;
        if (clockMode == 0) { allBlank(); } 
      }

      // colorColon
      if (!json["colorColon"].isNull()) {
        r3_val = (int)json["colorColon"]["r"];
        g3_val = (int)json["colorColon"]["g"];
        b3_val = (int)json["colorColon"]["b"];
        updateSettingsRequired = 1;
        if (clockMode == 0) { allBlank(); } 
      }

      // dayColor
      if (!json["dayColor"].isNull()) {
        r4_val = (int)json["dayColor"]["r"];
        g4_val = (int)json["dayColor"]["g"];
        b4_val = (int)json["dayColor"]["b"];
        updateSettingsRequired = 1;
        if (clockMode == 7) { allBlank(); } 
      }

      // monthColor
      if (!json["monthColor"].isNull()) {
        r5_val = (int)json["monthColor"]["r"];
        g5_val = (int)json["monthColor"]["g"];
        b5_val = (int)json["monthColor"]["b"];
        updateSettingsRequired = 1;
        if (clockMode == 7) { allBlank(); } 
      }

      // separatorColor
      if (!json["separatorColor"].isNull()) {
        r6_val = (int)json["separatorColor"]["r"];
        g6_val = (int)json["separatorColor"]["g"];
        b6_val = (int)json["separatorColor"]["b"];
        updateSettingsRequired = 1;
        if (clockMode == 7) { allBlank(); } 
      }
      
      // TempColor
      if (!json["TempColor"].isNull()) {
        r7_val = (int)json["TempColor"]["r"];
        g7_val = (int)json["TempColor"]["g"];
        b7_val = (int)json["TempColor"]["b"];
        updateSettingsRequired = 1;
        if (clockMode == 2) { allBlank(); } 
      }

      // TypeColor
      if (!json["TypeColor"].isNull()) {
        r8_val = (int)json["TypeColor"]["r"];
        g8_val = (int)json["TypeColor"]["g"];
        b8_val = (int)json["TypeColor"]["b"];
        updateSettingsRequired = 1;
        if (clockMode == 2) { allBlank(); } 
      }

      // DegreeColor
      if (!json["DegreeColor"].isNull()) {
        r9_val = (int)json["DegreeColor"]["r"];
        g9_val = (int)json["DegreeColor"]["g"];
        b9_val = (int)json["DegreeColor"]["b"];
        updateSettingsRequired = 1;
        if (clockMode == 2) { allBlank(); } 
      }

      // HumiColor
      if (!json["HumiColor"].isNull()) {
        r10_val = (int)json["HumiColor"]["r"];
        g10_val = (int)json["HumiColor"]["g"];
        b10_val = (int)json["HumiColor"]["b"];
        updateSettingsRequired = 1;
        if (clockMode == 8) { allBlank(); } 
      }

       // HumiDecimalColor
      if (!json["HumiDecimalColor"].isNull()) {
        r11_val = (int)json["HumiDecimalColor"]["r"];
        g11_val = (int)json["HumiDecimalColor"]["g"];
        b11_val = (int)json["HumiDecimalColor"]["b"];
        updateSettingsRequired = 1;
        if (clockMode == 8) { allBlank(); } 
      }

      // HumiSymbolColor
      if (!json["HumiSymbolColor"].isNull()) {
        r12_val = (int)json["HumiSymbolColor"]["r"];
        g12_val = (int)json["HumiSymbolColor"]["g"];
        b12_val = (int)json["HumiSymbolColor"]["b"];
        updateSettingsRequired = 1;
        if (clockMode == 8) { allBlank(); } 
      }

      // colorCD
      if (!json["colorCD"].isNull()) {
        cd_r_val = (int)json["colorCD"]["r"];
        cd_g_val = (int)json["colorCD"]["g"];
        cd_b_val = (int)json["colorCD"]["b"];
        updateSettingsRequired = 1;
        if ((clockMode == 1) || (clockMode == 4)) { allBlank(); } 
      }

      // scoreboardColorLeft
      if (!json["scoreboardColorLeft"].isNull()) {
        r13_val = (int)json["scoreboardColorLeft"]["r"];
        g13_val = (int)json["scoreboardColorLeft"]["g"];
        b13_val = (int)json["scoreboardColorLeft"]["b"];
        updateSettingsRequired = 1;
        if (clockMode == 3) { allBlank(); } 
      }

      // scoreboardColorRight
      if (!json["scoreboardColorRight"].isNull()) {
        r14_val = (int)json["scoreboardColorRight"]["r"];
        g14_val = (int)json["scoreboardColorRight"]["g"];
        b14_val = (int)json["scoreboardColorRight"]["b"];
        updateSettingsRequired = 1;
        if (clockMode == 3) { allBlank(); } 
      }

      // spectrumColor
      if (!json["spectrumColor"].isNull()) {
        r15_val = (int)json["spectrumColor"]["r"];
        g15_val = (int)json["spectrumColor"]["g"];
        b15_val = (int)json["spectrumColor"]["b"];
        updateSettingsRequired = 1;
        if (clockMode == 9) { allBlank(); }
      }

      // scrollColor
      if (!json["scrollColor"].isNull()) {
        r16_val = (int)json["scrollColor"]["r"];
        g16_val = (int)json["scrollColor"]["g"];
        b16_val = (int)json["scrollColor"]["b"];
        updateSettingsRequired = 1;
        if (clockMode == 11) { allBlank(); } 
      }

      // spectrumBackgroundColor
      if (!json["spectrumBackgroundColor"].isNull()) {
        r17_val = (int)json["spectrumBackgroundColor"]["r"];
        g17_val = (int)json["spectrumBackgroundColor"]["g"];
        b17_val = (int)json["spectrumBackgroundColor"]["b"];
        updateSettingsRequired = 1;
        if (clockMode == 9) { allBlank(); } 
      }

      // useSpotlights
      if (!json["useSpotlights"].isNull()) {
        if ( json["useSpotlights"] == true) {useSpotlights = 1;}
        if ( json["useSpotlights"] == false) {useSpotlights = 0;}
        updateSettingsRequired = 1;
        ShelfDownLights(); 
      }

      // DSTime
      if (!json["DSTime"].isNull()) {
        if ( json["DSTime"] == true) {DSTime = 1;}
        if ( json["DSTime"] == false) {DSTime = 0;}
        configTime(gmtOffset_sec, (daylightOffset_sec * DSTime), ntpServer);
        if(!getLocalTime(&timeinfo)){Serial.println("Error, no NTP Server found!");}
        #if HAS_RTC
          int tempyear = (timeinfo.tm_year +1900);
          int tempmonth = (timeinfo.tm_mon + 1);
          rtc.adjust(DateTime(tempyear, tempmonth, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
        #endif
        updateSettingsRequired = 1;
        if (clockMode == 0) { allBlank(); } 
        printLocalTime(); 
      }

  #if HAS_BUZZER
      // useAudibleAlarm
      if (!json["useAudibleAlarm"].isNull()) {
        if ( json["useAudibleAlarm"] == true) {useAudibleAlarm = 1;}
        if ( json["useAudibleAlarm"] == false) {useAudibleAlarm = 0;}
        updateSettingsRequired = 1;
        if ((clockMode == 1) || (clockMode == 4)) { allBlank(); } 
      }
  #endif

      // colorchangeCD
      if (!json["colorchangeCD"].isNull()) {
        if ( json["colorchangeCD"] == true) {colorchangeCD = 1;}
        if ( json["colorchangeCD"] == false) {colorchangeCD = 0;}
        updateSettingsRequired = 1;
        if ((clockMode == 1) || (clockMode == 4)) { allBlank(); } 
      }

      // randomSpectrumMode
      if (!json["randomSpectrumMode"].isNull()) {
        if ( json["randomSpectrumMode"] == true) {randomSpectrumMode = 1;}
        if ( json["randomSpectrumMode"] == false) {randomSpectrumMode = 0;}
        updateSettingsRequired = 1;
        if (clockMode == 9) { allBlank(); } 
      }

      // scrollOptions1
      if (!json["scrollOptions1"].isNull()) {
        if ( json["scrollOptions1"] == true) {scrollOptions1 = 1;}
        if ( json["scrollOptions1"] == false) {scrollOptions1 = 0;}
        updateSettingsRequired = 1;
        if (clockMode == 11) { allBlank(); } 
      }

      // scrollOptions2
      if (!json["scrollOptions2"].isNull()) {
        if ( json["scrollOptions2"] == true) {scrollOptions2 = 1;}
        if ( json["scrollOptions2"] == false) {scrollOptions2 = 0;}
        updateSettingsRequired = 1;
        if (clockMode == 11) { allBlank(); } 
      }

      // scrollOptions3
      if (!json["scrollOptions3"].isNull()) {
        if ( json["scrollOptions3"] == true) {scrollOptions3 = 1;}
        if ( json["scrollOptions3"] == false) {scrollOptions3 = 0;}
        updateSettingsRequired = 1;
        if (clockMode == 11) { allBlank(); } 
      }

      // scrollOptions4
      if (!json["scrollOptions4"].isNull()) {
        if ( json["scrollOptions4"] == true) {scrollOptions4 = 1;}
        if ( json["scrollOptions4"] == false) {scrollOptions4 = 0;}
        updateSettingsRequired = 1;
        if (clockMode == 11) { allBlank(); } 
      }

      // scrollOptions5
      if (!json["scrollOptions5"].isNull()) {
        if ( json["scrollOptions5"] == true) {scrollOptions5 = 1;}
        if ( json["scrollOptions5"] == false) {scrollOptions5 = 0;}
        updateSettingsRequired = 1;
        if (clockMode == 11) { allBlank(); } 
      }

      // scrollOptions6
      if (!json["scrollOptions6"].isNull()) {
        if ( json["scrollOptions6"] == true) {scrollOptions6 = 1;}
        if ( json["scrollOptions6"] == false) {scrollOptions6 = 0;}
        updateSettingsRequired = 1;
        if (clockMode == 11) { allBlank(); } 
      }

      // scrollOptions7
      if (!json["scrollOptions7"].isNull()) {
        if ( json["scrollOptions7"] == true) {scrollOptions7 = 1;}
        if ( json["scrollOptions7"] == false) {scrollOptions7 = 0;}
        updateSettingsRequired = 1;
        if (clockMode == 11) { allBlank(); } 
      }

      // scrollOptions8
      if (!json["scrollOptions8"].isNull()) {
        if ( json["scrollOptions8"] == true) {scrollOptions8 = 1;}
        if ( json["scrollOptions8"] == false) {scrollOptions8 = 0;}
        updateSettingsRequired = 1;
        if (clockMode == 11) { allBlank(); } 
      }

      // scrollOverride
      if (!json["scrollOverride"].isNull()) {
        if ( json["scrollOverride"] == true) {scrollOverride = 1;}
        if ( json["scrollOverride"] == false) {scrollOverride = 0;}
        updateSettingsRequired = 1;
        if (clockMode == 11) { allBlank(); } 
      }

      // scrollText
      if (!json["scrollText"].isNull()) {
        scrollText = json["scrollText"].as<String>();
        if ( scrollText == "") {scrollText = "dAdS ArE tHE bESt";}
        updateSettingsRequired = 1;
        if (clockMode == 11) { allBlank(); } 
      }

      // setpreset1
      if (!json["setpreset1"].isNull()) {
         saveclockSettings("preset1");
      }

      // setpreset2
      if (!json["setpreset2"].isNull()) {
         saveclockSettings("preset2");
      }

      // setdate
      if (!json["setdate"].isNull()) {
          int yeararg = json["setdate"]["year"].as<int>();
          int montharg = json["setdate"]["month"].as<int>();
          int dayarg = json["setdate"]["day"].as<int>();
          int hourarg = json["setdate"]["hour"].as<int>();
          int minarg = json["setdate"]["min"].as<int>();
          int secarg = json["setdate"]["sec"].as<int>();
          #if HAS_RTC
            rtc.adjust(DateTime(yeararg, montharg, dayarg, hourarg, minarg, secarg));   //set time on the RTC of the DS3231
          #endif
          struct tm tm;
          tm.tm_year = yeararg - 1900;
          tm.tm_mon = montharg - 1;
          tm.tm_mday = dayarg;
          tm.tm_hour = hourarg;
          tm.tm_min = minarg;
          tm.tm_sec = secarg;
          time_t t = mktime(&tm);
          struct timeval now1 = { .tv_sec = t };
          settimeofday(&now1, NULL);    //set time on the RTC of the ESP32
          printLocalTime(); 
      }

      // ClockMode
      if (!json["ClockMode"].isNull()) {
        allBlank();  
        clockMode = 0; 
        updateSettingsRequired = 1;
        realtimeMode = 0;   
        printLocalTime(); 
        breakOutSet = 1;
      }

      // DateMode
      if (!json["DateMode"].isNull()) {
        allBlank();   
        clockMode = 7;     
        updateSettingsRequired = 1;
        realtimeMode = 0;   
      }

      // TemperatureMode
      if (!json["TemperatureMode"].isNull()) {
        allBlank();
        clockMode = 2;    
        realtimeMode = 0;   
        updateSettingsRequired = 1;
      }

      // HumidityMode
      if (!json["HumidityMode"].isNull()) {
        allBlank();   
        clockMode = 8;     
        realtimeMode = 0;   
        updateSettingsRequired = 1;
      }

      // ScrollingMode
      if (!json["ScrollingMode"].isNull()) {  
          allBlank();   
          clockMode = 11;     
          realtimeMode = 0;   
        updateSettingsRequired = 1;
      }; 

      // DisplayOffMode
      if (!json["DisplayOffMode"].isNull()) {
        allBlank();   
        clockMode = 10;     
        realtimeMode = 0;   
        updateSettingsRequired = 1;
        #if HAS_BUZZER
          rtttl::stop();
        #endif
        breakOutSet = 1;   
      }

      // loadpreset1
      if (!json["loadPreset1"].isNull()) {
        getclockSettings("preset1");
        //GetBrightnessLevel();        
        allBlank(); 
      }

      // loadpreset2
      if (!json["loadPreset2"].isNull()) {
        getclockSettings("preset2");
        //GetBrightnessLevel();        
        allBlank();  
      }

      // CountdownMode
      if (!json["CountdownMode"].isNull() || !json["StopwatchMode"].isNull()) {
        CountUpMillis = !json["CountdownMode"].isNull() ? CountUpMillis : millis();
        countdownMilliSeconds = !json["CountdownMode"].isNull() ? json["CountdownMode"].as<int>() : json["StopwatchMode"].as<int>();
        if (countdownMilliSeconds < 1000) {countdownMilliSeconds = 1000;}
        if (countdownMilliSeconds > 86400000) {countdownMilliSeconds = 86400000;} 
        endCountDownMillis = millis() + countdownMilliSeconds;
        if (currentMode == 0) {currentMode = clockMode; currentReal = realtimeMode;}
        allBlank();
        clockMode = !json["CountdownMode"].isNull() ? 1 : 4;
        realtimeMode = 0; 
      }

      // ScoreboardMode
      if (!json["ScoreboardMode"].isNull()) {
        scoreboardLeft = json["ScoreboardMode"]["left"].as<int>();
        if (scoreboardLeft < 0) {scoreboardLeft = 0;}
        if (scoreboardLeft > 99) {scoreboardLeft = 99;}
        scoreboardRight = json["ScoreboardMode"]["right"].as<int>();
        if (scoreboardRight < 0) {scoreboardRight = 0;}
        if (scoreboardRight > 99) {scoreboardRight = 99;}
        allBlank();
        clockMode = 3;   
        realtimeMode = 0;   
        updateSettingsRequired = 1;
      }

      // lightshowMode
      if (!json["lightshowMode"].isNull()) {
        allBlank(); 
        lightshowMode = json["lightshowMode"].as<int>();
        oldsnakecolor = CRGB::Green;
        getSlower = 180;
        clockMode = 5;    
        realtimeMode = 1;   
        updateSettingsRequired = 1;
      }

      // spectrumMode
      if (!json["spectrumMode"].isNull()) {
        allBlank(); 
        spectrumMode = json["spectrumMode"].as<int>();
        clockMode = 9;    
        realtimeMode = 1;   
        updateSettingsRequired = 1;
      }

  #if HAS_BUZZER
      // set default music
      if (!json["defaultAudibleAlarm"].isNull()) {
      //  playRTTTLsong(SONGS[json["defaultAudibleAlarm"].as<String>()], 1);
        sprintf(defaultAudibleAlarm, "%s", json["defaultAudibleAlarm"].as<String>());
        updateSettingsRequired = 1;
      }
  #endif

      // outdoor weather api
      if (!json["weatherapi"].isNull()) {       
        if (!json["weatherapi"]["latitude"].isNull()) strncpy(weatherapi.latitude, json["weatherapi"]["latitude"], sizeof(weatherapi.latitude));
        if (!json["weatherapi"]["longitude"].isNull()) strncpy(weatherapi.longitude, json["weatherapi"]["longitude"], sizeof(weatherapi.longitude));
        if (!json["weatherapi"]["apikey"].isNull()) strncpy(weatherapi.apikey, json["weatherapi"]["apikey"], sizeof(weatherapi.apikey));
        preferences.putBytes("weatherapi", &weatherapi, sizeof(weatherapi));
      }

      // humidity
      if (!json["humidity_outdoor_enable"].isNull()) {
        if (!json["humidity_outdoor_enable"].isNull()) humidity_outdoor_enable = json["humidity_outdoor_enable"];
        if ( json["humidity_outdoor_enable"] == true) {humidity_outdoor_enable = 1;}
        if ( json["humidity_outdoor_enable"] == false) {humidity_outdoor_enable = 0;}
      Serial.println(humidity_outdoor_enable);
        updateSettingsRequired = 1;
      }

      // temperature
      if (!json["temperature_outdoor_enable"].isNull()) {
        if (!json["temperature_outdoor_enable"].isNull()) temperature_outdoor_enable = json["temperature_outdoor_enable"];
        if ( json["temperature_outdoor_enable"] == true) {temperature_outdoor_enable = 1;}
        if ( json["temperature_outdoor_enable"] == false) {temperature_outdoor_enable = 0;}
      Serial.println(temperature_outdoor_enable);
        updateSettingsRequired = 1;
      }

      server.send(200, "text/json", "{\"result\":\"ok\"}");
    } 
    server.send(401);
  });

  server.on("/playsong", HTTP_POST, []() {
    DynamicJsonDocument json(1500);
    if(server.args() > 0) {
      String body = server.arg(0);
      Serial.println("----");
      Serial.println(body);
      Serial.println("----");
      deserializeJson(json, server.arg(0));
  #if HAS_BUZZER
      // play music
      if (!json["song"].isNull()) {
          String songName = SONGS[json["song"].as<String>()];  // Get the song name as a String
          Serial.print("songName ");
          Serial.println(songName);
          songName.toCharArray(songTaskbuffer, songTaskbufferSize);
          xQueueSend(jobQueue, songTaskbuffer, (TickType_t)0);
          Serial.println("play song only");
      }
  #endif
    } 
    server.send(204);
  });

   server.on("/getscheduler", []() {
    DynamicJsonDocument json(16192);
    String output;
    Serial.println("getscheduler running");
    json["HAS_BUZZER"] = HAS_BUZZER;
#if HAS_BUZZER
    json["listOfSong"] = SONGS;
#endif
    json["jsonScheduleData"] = jsonScheduleData;
    serializeJson(json, output);
    server.send(200, "application/json", output);
  });

  server.on("/schedulerDelete", HTTP_POST, []() {   //catch post from the scheduler page save function
    DynamicJsonDocument json(1500);
    if(server.args() > 0) {
      deserializeJson(json, server.arg(0));
      Serial.println(server.arg(0));
      if (!json["deleteData"].isNull()) {     //if there was data
        String deleteData = json["deleteData"].as<String>();	  
        char deleteData_array[(deleteData.length() + 1)];
        deleteData.toCharArray(deleteData_array, (deleteData.length() + 1));  //make UID filename
        char XprocessedText[255];
        strcpy(XprocessedText, "/scheduler/");       //build full pathname
        strcat(XprocessedText, deleteData_array);  //build full pathname
        strcat(XprocessedText, ".json");
      deleteFile(FileFS, XprocessedText);              //build full pathname
      Serial.println("Deleting Schedule..");
      Serial.println(deleteData);
      }
      server.send(200, "text/json", "{\"result\":\"ok\"}");
    } 
  server.send(401);
  createSchedulesArray();   //update the running schedule json array with the new file contents
  });

  server.on("/scheduler", HTTP_POST, []() {   //catch post from the scheduler page save function
    DynamicJsonDocument json(1500);
    if(server.args() > 0) {
      String body = server.arg(0); //assign data from post
      Serial.println(server.arg(0));
      deserializeJson(json, server.arg(0));
      if (!json["scheduleType"].isNull()) {     //if there was data
        int schedulerScheduleType = (int)json["scheduleType"];
        int schedulerDom = (int)json["dom"];
        int schedulerHours = (int)json["hours"];
        int schedulerMinutes = (int)json["minutes"];
        String schedulerSong = json["song"].as<String>();
        String schedulerTitle = json["title"].as<String>();
        int schedulerMon = (int)json["mon"];
        int schedulerTue = (int)json["tue"];
        int schedulerWed = (int)json["wed"];
        int schedulerThu = (int)json["thu"];
        int schedulerFri = (int)json["fri"];
        int schedulerSat = (int)json["sat"];
        int schedulerSun = (int)json["sun"];
        int schedulerDailyType = (int)json["dailyType"];
        int schedulerHourlyType = (int)json["hourlyType"];
        int schedulerGongmode = (int)json["gongmode"];
        int schedulerDaymode = (int)json["daymode"];
        int schedulerYear = (int)json["year"];
        int schedulerMonth = (int)json["month"];
        int schedulerDay = (int)json["day"];
        int schedulerRecurring = (int)json["recurring"];
        String schedulerUid = json["uid"].as<String>();	  
        char schedulerUid_array[(schedulerUid.length() + 1)];
        schedulerUid.toCharArray(schedulerUid_array, (schedulerUid.length() + 1));  //make UID filename
        char body_array[(body.length() + 1)];   //make data blerb
        body.toCharArray(body_array, (body.length() + 1));
        char XprocessedText[255];
        strcpy(XprocessedText, "/scheduler/");       //build full pathname
        strcat(XprocessedText, schedulerUid_array);  //build full pathname
        strcat(XprocessedText, ".json");              //build full pathname
        writeFile(FileFS, XprocessedText, body_array);   //write json data from post to a file named for the UID in the post
      }
      server.send(200, "text/json", "{\"result\":\"ok\"}");
    } 
  server.send(401);
  createSchedulesArray();   //update the running schedule json array with the new UID file contents
  });

  server.on("/getdebug", []() {
    char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    char monthsOfTheYear[12][12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    #if HAS_DHT
      int humidTemp = dht.readHumidity();        // read humidity
      float sensorTemp = dht.readTemperature();     // read temperature
      float saturationVaporPressure = 6.1078 * pow(10, (7.5 * sensorTemp / (237.3 + sensorTemp)));  // Calculate saturation vapor pressure
      float vaporPressure = saturationVaporPressure * (humidTemp / 100.0);  // Calculate vapor pressure
      float absoluteHumidity = 217 * vaporPressure / (273.15 + sensorTemp);  // Calculate absolute humidity
      float humidityRatio = 0.622 * vaporPressure / (101325 - vaporPressure);
      float heatIndex = -8.784695 + 1.61139411 * sensorTemp + 2.338549 * humidTemp + -0.14611605 * sensorTemp * humidTemp + -0.01230809 * pow(sensorTemp, 2) + -0.01642482 * pow(humidTemp, 2) + 0.00221173 * pow(sensorTemp, 2) * humidTemp + 0.00072546 * sensorTemp * pow(humidTemp, 2) + -0.00000358 * pow(sensorTemp, 2) * pow(humidTemp, 2);
      heatIndex += altitudeLocal * 0.0065;  // Adjust heat index based on altitude
    #endif
    char tempRTC[64]="";
    char tempRTCE[64]="";
    String output;
    DynamicJsonDocument json(3000);
    #if HAS_RTC
      DateTime now = rtc.now();
      //DS3231 RTC    
      sprintf(tempRTC, "%s, ", daysOfTheWeek[now.dayOfTheWeek()]);
      sprintf(tempRTC + strlen(tempRTC), "%s ", monthsOfTheYear[now.month()-1]);
      sprintf(tempRTC + strlen(tempRTC), "%02d ", now.day());
      sprintf(tempRTC + strlen(tempRTC), "%d ", now.year());
      sprintf(tempRTC + strlen(tempRTC), "%02d:", now.hour());
      sprintf(tempRTC + strlen(tempRTC), "%02d:", now.minute());
      sprintf(tempRTC + strlen(tempRTC), "%02d", now.second());
      json["DS-3231"] = tempRTC;
    #endif
    //ESP32 RTC
    sprintf(tempRTCE, "%s, ", daysOfTheWeek[timeinfo.tm_wday]);
    sprintf(tempRTCE + strlen(tempRTCE), "%s ", monthsOfTheYear[timeinfo.tm_mon]);
    sprintf(tempRTCE + strlen(tempRTCE), "%02d ", timeinfo.tm_mday);
    sprintf(tempRTCE + strlen(tempRTCE), "%d ", timeinfo.tm_year+1900);
    sprintf(tempRTCE + strlen(tempRTCE), "%02d:", timeinfo.tm_hour);
    sprintf(tempRTCE + strlen(tempRTCE), "%02d:", timeinfo.tm_min);
    sprintf(tempRTCE + strlen(tempRTCE), "%02d", timeinfo.tm_sec);
      json["softwareVersion"] = softwareVersion;
    #if HAS_BUZZER
      json["BUZZER_PIN"] = BUZZER_PIN;
    #endif
    json["ClockColorSettings"] = ClockColorSettings;
    json["ColorChangeFrequency"] = ColorChangeFrequency;
    json["CountUpMillis"] = CountUpMillis;
    json["DateColorSettings"] = DateColorSettings;
    #if HAS_DHT
      json["DHT_PIN"] = DHT_PIN;
      json["DHT11 C Temp"] = sensorTemp;
      json["DHT11 F Temp"] = (sensorTemp * 1.8000) + 32;
      json["DHT11 Humidity (Relative)"] = humidTemp;
      json["DHT11 Humidity (Absolute)"] = absoluteHumidity;
      json["DHT11 Humidity (Ratio)"] = humidityRatio;
      json["DHT11 (Heat Index F)"] = (heatIndex * 1.8000) + 32;
      json["DHTTYPE"] = DHTTYPE;
    #endif
    json["DSTime"] = DSTime;
    json["ESP32-NTP"] = tempRTCE;
    json["FAKE_NUM_LEDS"] = FAKE_NUM_LEDS;
    json["HAS_DHT"] = HAS_DHT;
    json["HAS_BUZZER"] = HAS_BUZZER;
    json["HAS_ONLINEWEATHER"] = HAS_ONLINEWEATHER;
    json["HAS_PHOTOSENSOR"] = HAS_PHOTOSENSOR;
    json["HAS_RTC"] = HAS_RTC;
    json["HAS_SOUNDDETECTOR"] = HAS_SOUNDDETECTOR;
    json["LEDS_PER_DIGIT"] = LEDS_PER_DIGIT;
    json["LEDS_PER_SEGMENT"] = LEDS_PER_SEGMENT;
    json["LED_PIN"] = LED_PIN;
    json["MILLI_AMPS"] = MILLI_AMPS;
    json["NUMBER_OF_DIGITS"] = NUMBER_OF_DIGITS;
    json["NUM_LEDS"] = NUM_LEDS;
    #if HAS_PHOTOSENSOR
      json["PHOTORESISTER_PIN"] = PHOTORESISTER_PIN;
      json["PHOTO_SAMPLES"] = PHOTO_SAMPLES;
    #endif
    json["SEGMENTS_LEDS"] = SEGMENTS_LEDS;
    json["SEGMENTS_PER_NUMBER"] = SEGMENTS_PER_NUMBER;
    #if HAS_SOUNDDETECTOR
      json["SOUNDDETECTOR_I2S_WS"] = SOUNDDETECTOR_I2S_WS;
      json["SOUNDDETECTOR_I2S_SD"] = SOUNDDETECTOR_I2S_SD;
      json["SOUNDDETECTOR_I2S_SCK"] = SOUNDDETECTOR_I2S_SCK;
      json["SOUNDDETECTOR_I2S_PORT"] = SOUNDDETECTOR_I2S_PORT;
      json["SOUNDDETECTOR_SAMPLING_FREQ"] = SOUNDDETECTOR_SAMPLING_FREQ;
      json["SOUNDDETECTOR_SAMPLES"] = SOUNDDETECTOR_SAMPLES;
      json["SOUNDDETECTOR_BITS_PER_SAMPLE"] = SOUNDDETECTOR_BITS_PER_SAMPLE;
      json["SOUNDDETECTOR_BANDS_WIDTH"] = SOUNDDETECTOR_BANDS_WIDTH;
      json["SOUNDDETECTOR_BANDS_HEIGHT"] = SOUNDDETECTOR_BANDS_HEIGHT;
      json["SOUNDDETECTOR_averageAudioInput"] = SOUNDDETECTOR_averageAudioInput;
      json["SOUNDDETECTOR_decay"] = SOUNDDETECTOR_decay;
      json["SOUNDDETECTOR_decay_check"] = SOUNDDETECTOR_decay_check;
      json["SOUNDDETECTOR_post_react"] = SOUNDDETECTOR_post_react;
      json["SOUNDDETECTOR_pre_react"] = SOUNDDETECTOR_pre_react;
      json["SOUNDDETECTOR_react"] = SOUNDDETECTOR_react;
      json["SOUNDDETECTOR_Amplitude"] = SOUNDDETECTOR_Amplitude;
      json["SOUNDDETECTOR_noiseThresholds[0]"] = SOUNDDETECTOR_noiseThresholds[0];
      json["SOUNDDETECTOR_noiseThresholds[1]"] = SOUNDDETECTOR_noiseThresholds[1];
      json["SOUNDDETECTOR_noiseThresholds[2]"] = SOUNDDETECTOR_noiseThresholds[2];
      json["SOUNDDETECTOR_noiseThresholds[3]"] = SOUNDDETECTOR_noiseThresholds[3];
      json["SOUNDDETECTOR_noiseThresholds[4]"] = SOUNDDETECTOR_noiseThresholds[4];
      json["SOUNDDETECTOR_noiseThresholds[5]"] = SOUNDDETECTOR_noiseThresholds[5];
      json["SOUNDDETECTOR_noiseThresholds[6]"] = SOUNDDETECTOR_noiseThresholds[6];
      json["SOUNDDETECTOR_noiseThresholds[7]"] = SOUNDDETECTOR_noiseThresholds[7];
/*      json["SOUNDDETECTOR_bandWeightingFactors[0]"] = SOUNDDETECTOR_bandWeightingFactors[0];
      json["SOUNDDETECTOR_bandWeightingFactors[1]"] = SOUNDDETECTOR_bandWeightingFactors[1];
      json["SOUNDDETECTOR_bandWeightingFactors[2]"] = SOUNDDETECTOR_bandWeightingFactors[2];
      json["SOUNDDETECTOR_bandWeightingFactors[3]"] = SOUNDDETECTOR_bandWeightingFactors[3];
      json["SOUNDDETECTOR_bandWeightingFactors[4]"] = SOUNDDETECTOR_bandWeightingFactors[4];
      json["SOUNDDETECTOR_bandWeightingFactors[5]"] = SOUNDDETECTOR_bandWeightingFactors[5];
      json["SOUNDDETECTOR_bandWeightingFactors[6]"] = SOUNDDETECTOR_bandWeightingFactors[6];
      json["SOUNDDETECTOR_bandWeightingFactors[7]"] = SOUNDDETECTOR_bandWeightingFactors[7];*/
      json["SOUNDDETECTOR_bandValues[0]"] = SOUNDDETECTOR_bandValues[0];
      json["SOUNDDETECTOR_bandValues[1]"] = SOUNDDETECTOR_bandValues[1];
      json["SOUNDDETECTOR_bandValues[2]"] = SOUNDDETECTOR_bandValues[2];
      json["SOUNDDETECTOR_bandValues[3]"] = SOUNDDETECTOR_bandValues[3];
      json["SOUNDDETECTOR_bandValues[4]"] = SOUNDDETECTOR_bandValues[4];
      json["SOUNDDETECTOR_bandValues[5]"] = SOUNDDETECTOR_bandValues[5];
      json["SOUNDDETECTOR_bandValues[6]"] = SOUNDDETECTOR_bandValues[6];
      json["SOUNDDETECTOR_bandValues[7]"] = SOUNDDETECTOR_bandValues[7];
    #endif
    json["SPECTRUM_PIXELS"] = SPECTRUM_PIXELS;
    json["SPOT_LEDS"] = SPOT_LEDS;
    json["WiFi IP"] = WiFi.localIP().toString();
    json["WiFi_MAX_RETRIES"] = WiFi_MAX_RETRIES;
    json["WiFi_MAX_RETRY_DURATION"] = WiFi_MAX_RETRY_DURATION;
    json["WiFi_elapsedTime"] = WiFi_elapsedTime;
    json["WiFi_retryCount"] = WiFi_retryCount;
    json["WiFi_startTime"] = WiFi_startTime;
    json["WiFi_totalReconnections"] = WiFi_totalReconnections;
    #if HAS_PHOTOSENSOR
      json["analogRead(PHOTORESISTER_PIN)"] = analogRead(PHOTORESISTER_PIN);
    #endif
    #if HAS_DHT
      json["altitudeLocal"] = altitudeLocal;
    #endif
    #if HAS_SOUNDDETECTOR
   //   json["analogRead(SOUNDDETECTOR_ENVELOPE_IN_PIN)"] = analogRead(SOUNDDETECTOR_ENVELOPE_IN_PIN);
    #endif
    json["breakOutSet"] = breakOutSet;
    json["brightness"] = brightness;
    json["clearOldLeds"] = clearOldLeds;
    json["clockDisplayType"] = clockDisplayType;
    json["clockMode"] = clockMode;
    json["colonType"] = colonType;
    json["colorWheelPosition"] = colorWheelPosition;
    json["colorWheelPositionTwo"] = colorWheelPositionTwo;
    json["colorWheelSpeed"] = colorWheelSpeed;
    json["colorchangeCD"] = colorchangeCD;
    json["countdownMilliSeconds"] = countdownMilliSeconds;
    json["countupMilliSeconds"] = countupMilliSeconds;
    json["currentMode"] = currentMode;
    json["currentReal"] = currentReal;
    json["cylonPosition"] = cylonPosition;
    json["dateDisplayType"] = dateDisplayType;
    json["daylightOffset_sec"] = daylightOffset_sec;
    json["daysUptime"] = daysUptime;
    #if HAS_BUZZER
      json["defaultAudibleAlarm"] = SONGS[defaultAudibleAlarm];
    #endif
    #if HAS_SOUNDDETECTOR
    //  json["digitalRead(SOUNDDETECTOR_AUDIO_GATE_PIN)"] = digitalRead(SOUNDDETECTOR_AUDIO_GATE_PIN);
    #endif
    json["dotsOn"] = dotsOn;
    json["endCountDownMillis"] = endCountDownMillis;
    json["fakeclockrunning"] = fakeclockrunning;
    json["foodSpot"] = foodSpot;
    json["getSlower"] = getSlower;
    json["gmtOffset_sec"] = gmtOffset_sec;
    json["host"] = host;
    json["hoursUptime"] = hoursUptime;
    #if HAS_ONLINEWEATHER || HAS_DHT
      json["humiColorSettings"] = humiColorSettings;
      json["humiDisplayType"] = humiDisplayType;
      json["humidity_outdoor_enable"] = humidity_outdoor_enable;
    #endif
    json["isAsleep"] = isAsleep;
    json["lightSensorValue"] = lightSensorValue;
    json["lightshowMode"] = lightshowMode;
    json["lightshowSpeed"] = lightshowSpeed;
    json["millis"] = millis();
  	json["minutesUptime"] = minutesUptime;
    json["ntpServer"] = ntpServer;
    #if HAS_ONLINEWEATHER
      json["outdoortemp"] = outdoorTemp;
      json["outdoorHumidity"] = outdoorTemp;
    #endif
    json["pastelColors"] = pastelColors;
    #if HAS_PHOTOSENSOR
      json["photoresisterReadings[0]"] = photoresisterReadings[0];
      json["photoresisterReadings[1]"] = photoresisterReadings[1];
      //json["photoresisterReadings[2]"] = photoresisterReadings[2];
      //json["photoresisterReadings[3]"] = photoresisterReadings[3];
      //json["photoresisterReadings[4]"] = photoresisterReadings[4];
      //json["photoresisterReadings[5]"] = photoresisterReadings[5];
      //json["photoresisterReadings[6]"] = photoresisterReadings[6];
      //json["photoresisterReadings[7]"] = photoresisterReadings[7];
      //json["photoresisterReadings[8]"] = photoresisterReadings[8];
      //json["photoresisterReadings[9]"] = photoresisterReadings[9];
      //json["photoresisterReadings[10]"] = photoresisterReadings[10];
      //json["photoresisterReadings[11]"] = photoresisterReadings[11];
      //json["photoresisterReadings[12]"] = photoresisterReadings[12];
      //json["photoresisterReadings[13]"] = photoresisterReadings[13];
    #endif
    json["prevTime"] = prevTime;
    json["prevTime2"] = prevTime2;
    json["previousTimeDay"] = previousTimeDay;
    json["previousTimeHour"] = previousTimeHour;
    json["previousTimeMin"] = previousTimeMin;
    json["previousTimeMonth"] = previousTimeMonth;
    json["previousTimeWeek"] = previousTimeWeek;
    json["randomDayPassed"] = randomDayPassed;
    json["randomHourPassed"] = randomHourPassed;
    json["randomMinPassed"] = randomMinPassed;
    json["randomMonthPassed"] = randomMonthPassed;
    #if HAS_SOUNDDETECTOR
      json["randomSpectrumMode"] = randomSpectrumMode;
    #endif
    json["randomWeekPassed"] = randomWeekPassed;
    json["readIndex"] = readIndex;
    json["realtimeMode"] = realtimeMode;
    json["scoreboardLeft"] = scoreboardLeft;
    json["scoreboardRight"] = scoreboardRight;
    json["scrollColorSettings"] = scrollColorSettings;
    json["scrollFrequency"] = scrollFrequency;
    json["scrollOptions1"] = scrollOptions1;
    json["scrollOptions2"] = scrollOptions2;
    json["scrollOptions3"] = scrollOptions3;
    json["scrollOptions4"] = scrollOptions4;
    json["scrollOptions5"] = scrollOptions5;
    json["scrollOptions6"] = scrollOptions6;
    json["scrollOptions7"] = scrollOptions7;
    json["scrollOptions8"] = scrollOptions8;
    json["scrollText"] = scrollText.c_str();
    json["sleepTimerCurrent"] = sleepTimerCurrent;
    json["snakeLastDirection"] = snakeLastDirection;
    json["snakePosition"] = snakePosition;
    json["snakeWaiting"] = snakeWaiting;
    #if HAS_BUZZER
      json["specialAudibleAlarm"] = SONGS[specialAudibleAlarm];
    #endif
    #if HAS_SOUNDDETECTOR
      json["spectrumBackgroundSettings"] = spectrumBackgroundSettings;
      json["spectrumColorSettings"] = spectrumColorSettings;
      json["spectrumMode"] = spectrumMode;
    #endif
    json["spotlightsColorSettings"] = spotlightsColorSettings;
    json["suspendFrequency"] = suspendFrequency;
    json["suspendType"] = suspendType;
    #if HAS_ONLINEWEATHER || HAS_DHT
      json["tempColorSettings"] = tempColorSettings;
      json["tempDisplayType"] = tempDisplayType;
      json["temperatureCorrection"] = temperatureCorrection;
      json["temperature_outdoor_enable"] = temperature_outdoor_enable;
      json["temperatureSymbol"] = temperatureSymbol;
    #endif
    json["updateSettingsRequired"] = updateSettingsRequired;
    #if HAS_BUZZER
      json["useAudibleAlarm"] = useAudibleAlarm;
    #endif
    json["useSpotlights"] = useSpotlights;

    serializeJson(json, output);
    server.send(200, "application/json", output);
  });

  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
      HTTPUpload& upload = server.upload();
    //  Serial.println("Update..");
      Serial.println(upload.status);
      if (upload.status == UPLOAD_FILE_START) {
        Serial.printf("Update: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        /* flashing firmware to ESP*/
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(Serial);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(Serial);
        }
      }
  });



#if HAS_BUZZER
  /*handling uploading song file */
server.on("/uploadSong", HTTP_POST, []() {
}, []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");  // Set CORS header
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
        const char* filename = upload.filename.c_str();
        char fullFilename[256];
        if (filename[0] != '/') {
            snprintf(fullFilename, sizeof(fullFilename), "/songs/%s", filename);
        } else {
            strncpy(fullFilename, filename, sizeof(fullFilename));
        }
        Serial.print("Upload Name: ");
        Serial.println(fullFilename);
        fsUploadFile = FileFS.open(fullFilename, "w");
        if (!fsUploadFile) {
            Serial.println("Failed to open file for writing");
        } else {
            Serial.println("Upload started");
        }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        Serial.print(".");
        if (fsUploadFile) {
            fsUploadFile.write(upload.buf, upload.currentSize);
        }
    } else if (upload.status == UPLOAD_FILE_END) {
        Serial.println("");
        if (fsUploadFile) {
            fsUploadFile.close();
            Serial.print("Upload Size: ");
            Serial.println(upload.totalSize);
            server.send(200, "text/json", "{\"error\":0,\"message\":\"Upload Complete\"}");
        //    listDir(FileFS, "/songs/", 0);
            getListOfSongs();
        } else {
            server.send(500, "text/json", "{\"error\":1,\"message\":\"Upload failed\"}");
        }
  server.sendHeader("Connection", "close");
    }
});


  server.on("/deleteSong", HTTP_POST, []() {
    DynamicJsonDocument json(1500);
      Serial.print("Upload Size: "); Serial.println(server.args());
    if(server.args() > 0) {
      String body = server.arg(0);
      Serial.println("----");
      Serial.println(body);
      Serial.println("----");
      deserializeJson(json, server.arg(0));
      if (!json["song"].isNull()) {
      Serial.print("Upload Size: "); Serial.println(json["song"].as<String>());
      deleteFile(FileFS, SONGS[json["song"].as<String>()]);
      getListOfSongs();
      }
      server.send(200, "text/json", "{\"result\":\"ok\"}");
    } 
    server.send(401);
  });
#endif

}

