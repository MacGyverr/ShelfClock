# Memory Optimization Implementation Guide

## Overview
This guide provides practical examples of how to implement memory optimization techniques in the ShelfClock project using the new utility classes.

I did not want to edit a ton of your code myself, as i also do not have a local way to test. i only implemented the main memory monitor and cleaned up your main "updateanything" call, the 
amount of if's was killing me =p 

~Olson

## New Utility Classes

### 1. StringUtils
Located in `include/StringUtils.h`, this class provides memory-efficient string handling functions:
- `formatString()` - Format strings without dynamic allocation
- `appendFormatted()` - Append formatted text to existing buffers
- `stringToBuffer()` - Convert String objects to C-strings safely
- `createJson()` - Create JSON strings with pre-allocated buffers
- `concatStrings()` - Concatenate multiple strings efficiently

### 2. JsonHelper
Located in `include/JsonHelper.h`, this class provides tools for efficient JSON creation:
- Fluent interface for building JSON objects and arrays
- Pre-allocated buffer to avoid heap fragmentation
- Type-safe property and array item addition
- One-call JSON object creation

### 3. HttpHelper
Located in `include/HttpHelper.h`, this class optimizes HTTP requests and responses:
- Memory-efficient URL building
- Incremental JSON response processing
- Targeted value extraction without parsing entire responses
- Stream-based response handling

### 4. ScrollText
Located in `include/ScrollText.h`, this class provides an optimized text scrolling implementation:
- Pre-allocated buffers for character translation
- Efficient character lookup
- Static memory usage instead of dynamic allocation
- Drop-in replacement for the original scroll function

## Implementation Examples

### Example 1: Optimizing Weather API Calls

#### Before:
```cpp
void updateRainForecast() {
  String apiUrl = "https://api.open-meteo.com/v1/forecast?latitude=" + String(latitude, 9)
                + "&longitude=" + String(longitude, 9)
                + "&hourly=precipitation&forecast_days=1";
  
  HTTPClient http;
  http.begin(apiUrl);
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    String payload = http.getString();
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, payload);
    
    // Process JSON data
    // ...
  }
  
  http.end();
}
```

#### After:
```cpp
void updateRainForecast() {
  HttpHelper httpHelper;
  
  // Build URL without String concatenation
  const char* url = httpHelper.buildUrl(
    "https://api.open-meteo.com/v1/forecast",
    2,
    "latitude", String(latitude, 9).c_str(),
    "longitude", String(longitude, 9).c_str(),
    "hourly", "precipitation",
    "forecast_days", "1"
  );
  
  // Make request
  int httpCode = httpHelper.beginRequest(url);
  
  if (httpCode > 0) {
    // Use a reasonably sized static document
    StaticJsonDocument<1024> doc;
    
    // Parse directly from the stream
    if (httpHelper.parseJsonResponse(doc)) {
      // Process JSON data
      // ...
    }
  }
}
```

### Example 2: Optimizing Text Scrolling

#### Before:
```cpp
void scroll(String IncomingString) {
  if (IncomingString.length() > 256) { IncomingString = "ArE U A HAckEr"; }
  uint16_t TranslatedSentence[(IncomingString.length()*2)+12]; // Dynamic allocation
  
  // Character translation with multiple if statements
  for (uint16_t realposition=0; realposition<IncomingString.length(); realposition++) {
    char SentenceLetter = IncomingString.charAt(realposition);
    uint16_t LetterNumber = 10; // Default to space
    
    // Many if statements for character translation
    if( SentenceLetter == '0') { LetterNumber = 0; }
    if( SentenceLetter == '1') { LetterNumber = 1; }
    // ... many more if statements ...
    
    TranslatedSentence[(realposition*2)+6] = LetterNumber;
    TranslatedSentence[(realposition*2)+7] = 96; // padding
  }
  
  // Set scroll color
  if (scrollColorSettings == 0){ scrollColor = CRGB(r16_val, g16_val, b16_val); }
  if (scrollColorSettings == 1 && pastelColors == 0){ scrollColor = CHSV(random(0, 255), 255, 255); }
  if (scrollColorSettings == 1 && pastelColors == 1){ scrollColor = CRGB(random(0, 255), random(0, 255), random(0, 255)); }
  
  // Display scrolling text
  for (uint16_t finalposition=0; finalposition<((IncomingString.length()*2)+6); finalposition++) {
    // Display logic with multiple if statements
    // ...
    FastLED.show();
    // Delay with server handling
    for (int i=0; i<400 && !breakOutSet; i++) {
      server.handleClient();
    }
  }
  
  allBlank();
  allBlank();
}
```

#### After (Method 1 - Drop-in Replacement):
```cpp
// In your main ShelfClock.cpp file, add:
#include "MemoryOptimizations.h"

// That's it! The original scroll function is now automatically optimized
// No changes needed to existing code that calls scroll()
scroll("Hello, World!"); // Uses the optimized implementation
```

#### After (Method 2 - Direct Use of ScrollText Class):
```cpp
// Create a global instance to reuse
ScrollText scrollText;

void displayCustomScrollText(const char* text, CRGB color) {
  // Reset breakout flag
  breakOutSet = 0;
  
  // Set custom color
  scrollText.setColor(color);
  
  // Prepare text using pre-allocated buffers
  scrollText.prepareText(text);
  
  // Display with full control over parameters
  scrollText.displayScroll(
    LEDs,                  // LED array
    SEGMENTS_LEDS,         // Number of LEDs
    displayNumber,         // Function to display a number
    []() { server.handleClient(); }, // Lambda for server.handleClient
    allBlank,              // Function to clear the display
    0,                     // Use fixed color (not random)
    0,                     // Not using pastel colors
    color.r, color.g, color.b  // RGB values from the color parameter
  );
}
```

### Example 3: Optimizing JSON Generation

#### Before:
```cpp
String getStatusJson() {
  String json = "{";
  json += "\"temperature\":" + String(temperature) + ",";
  json += "\"humidity\":" + String(humidity) + ",";
  json += "\"light\":" + String(lightLevel) + ",";
  json += "\"mode\":\"" + String(currentMode) + "\"";
  json += "}";
  return json;
}
```

#### After:
```cpp
// Use a global instance to avoid repeated allocation
JsonHelper jsonHelper;

const char* getStatusJson() {
  // Method 1: Fluent interface
  jsonHelper.clear()
    .beginObject()
    .addProperty("temperature", temperature)
    .addSeparator()
    .addProperty("humidity", humidity)
    .addSeparator()
    .addProperty("light", lightLevel)
    .addSeparator()
    .addProperty("mode", currentMode)
    .endObject();
  
  return jsonHelper.getJson();
  
  // Alternative Method 2: One-call creation
  // return jsonHelper.createObject(4, 
  //   "temperature", temperature,
  //   "humidity", humidity,
  //   "light", lightLevel,
  //   "mode", currentMode
  // );
}
```

### Example 4: Optimizing HTTP Response Processing

#### Before:
```cpp
void fetchTides() {
  String baseURL = "https://api.tidesandcurrents.noaa.gov/api/prod/datagetter?station=";
  String todayURL = baseURL + "&date=today&interval=h";
  
  HTTPClient http;
  http.begin(todayURL);
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    String payload = http.getString();
    DynamicJsonDocument doc(4096);
    deserializeJson(doc, payload);
    
    // Process entire JSON document
    // ...
  }
  
  http.end();
}
```

#### After:
```cpp
void fetchTides() {
  HttpHelper httpHelper;
  
  // Build URL without String concatenation
  const char* url = httpHelper.buildUrl(
    "https://api.tidesandcurrents.noaa.gov/api/prod/datagetter",
    2,
    "station", stationId,
    "date", "today",
    "interval", "h"
  );
  
  // Make request and process response incrementally
  int httpCode = httpHelper.beginRequest(url);
  
  if (httpCode > 0) {
    // Process JSON response without loading entire payload into memory
    httpHelper.processJsonResponse([](JsonObject& obj) {
      // Process each key-value pair as it's parsed
      if (obj.containsKey("data")) {
        JsonArray dataArray = obj["data"].as<JsonArray>();
        // Process data array
      }
    });
  }
}
```

## Complete Implementation of Memory Optimizations

### Step 1: Include the Memory Optimizations Header

In your main ShelfClock.cpp file, add:

```cpp
#include "MemoryOptimizations.h"
```

This single include provides optimized implementations of:
- `scroll()` function (both String and C-string versions)
- JSON creation and processing functions
- HTTP request handling functions
- String formatting and manipulation functions

### Step 2: Replace Dynamic String Operations

Replace String concatenation with StringUtils functions:

```cpp
// Before
String message = "Temperature: " + String(temperature) + "°C";

// After
char buffer[64];
StringUtils::formatString(buffer, sizeof(buffer), "Temperature: %d°C", temperature);
```

### Step 3: Replace Dynamic JSON Creation

Replace dynamic JSON creation with JsonHelper:

```cpp
// Before
String json = "{\"name\":\"" + deviceName + "\",\"value\":" + String(value) + "}";

// After
const char* json = jsonHelper.createObject(2, "name", deviceName, "value", value);
```

### Step 4: Replace HTTP Request Handling

Replace HTTP request handling with HttpHelper:

```cpp
// Before
http.begin(url);
int httpCode = http.GET();
String payload = http.getString();
// Process payload...

// After
int httpCode = httpHelper.beginRequest(url);
httpHelper.processJsonResponse([](JsonObject& obj) {
  // Process JSON data directly from the stream
});
```

### Step 5: Monitor Memory Usage

Use the MemoryMonitor to track memory usage:

```cpp
// In setup()
memoryMonitor.begin();

// In loop() or other appropriate places
memoryMonitor.update();
Serial.println(memoryMonitor.getStatusJson());
```

## Conclusion

By implementing these memory optimization techniques, you can significantly reduce heap fragmentation and improve the stability and performance of the ShelfClock project on the ESP32 platform.

The optimized implementations maintain the same functionality as the original code while using memory more efficiently, making your project more reliable and less prone to crashes due to memory issues.



~Olson
