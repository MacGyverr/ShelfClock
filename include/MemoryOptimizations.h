#ifndef MEMORY_OPTIMIZATIONS_H
#define MEMORY_OPTIMIZATIONS_H

/**
 * Memory Optimizations for ShelfClock
 * 
 * This file provides memory-optimized implementations of key functions
 * in the ShelfClock project to reduce heap fragmentation and improve stability.
 * 
 * To use these optimizations, simply include this header in your main ShelfClock.cpp file:
 * #include "MemoryOptimizations.h"
 * 
 * ~Olson
 */

#include <Arduino.h>
#include "ScrollText.h"
#include "StringUtils.h"
#include "JsonHelper.h"
#include "HttpHelper.h"

// Global instances of utility classes
ScrollText gScrollText;
JsonHelper jsonHelper;
HttpHelper httpHelper;

// External variables from ShelfClock.cpp needed for the optimized implementations
extern uint8_t breakOutSet;
extern CRGB scrollColor;
extern uint8_t r16_val, g16_val, b16_val;
extern CRGB* LEDs;
extern uint16_t SEGMENTS_LEDS;
extern uint8_t scrollColorSettings;
extern uint8_t pastelColors;
extern class ESPAsyncWebServer::AsyncWebServer server;

// Function declarations for functions used in the original implementations
extern void displayNumber(uint8_t number, uint8_t position, CRGB color);
extern void allBlank();

/**
 * @brief Memory-optimized implementation of the scroll function
 * 
 * This is a drop-in replacement for the original scroll function that uses
 * pre-allocated buffers to avoid dynamic memory allocations.
 * ~Olson
 */
void scrollOptimized(const char* text) {
    // Reset breakout flag (original function does this)
    breakOutSet = 0;
    
    // Configure the ScrollText instance
    gScrollText.setColor(r16_val, g16_val, b16_val);
    
    // Prepare the text for scrolling
    gScrollText.prepareText(text);
    
    // Perform the actual scrolling using the same behavior as the original function
    gScrollText.displayScroll(
        LEDs,                  // LED array
        SEGMENTS_LEDS,         // Number of LEDs
        displayNumber,         // Function to display a number
        []() { server.handleClient(); }, // Lambda for server.handleClient
        allBlank,              // Function to clear the display
        scrollColorSettings,   // Scroll color mode
        pastelColors,          // Pastel colors flag
        r16_val,               // Red value
        g16_val,               // Green value
        b16_val                // Blue value
    );
}

/**
 * @brief Memory-optimized scroll function (String version)
 * 
 * This maintains compatibility with existing code that calls scroll with a String.
 * It converts the String to a C-string and calls the optimized implementation.
 * ~Olson
 */
void scroll(String incomingString) {
    // Check for oversized input (same as original function)
    if (incomingString.length() > 256) {
        incomingString = "ArE U A HAckEr";
    }
    
    // Call the optimized implementation with the C-string
    scrollOptimized(incomingString.c_str());
}

/**
 * @brief Memory-optimized scroll function (C-string version)
 * 
 * This version is more memory-efficient as it avoids creating a String object.
 * Use this version for new code.
 * ~Olson
 */
void scroll(const char* text) {
    scrollOptimized(text);
}

/**
 * @brief Memory-optimized JSON status generator
 * 
 * Creates a JSON status string without using dynamic memory allocations.
 * ~Olson
 */
template<typename... Args>
const char* createJsonStatus(int numProps, Args... args) {
    return jsonHelper.createObject(numProps, args...);
}

/**
 * @brief Memory-optimized HTTP request function
 * 
 * Makes an HTTP request without storing the entire response in memory.
 * @param url The URL to request
 * @param callback Function to process the response
 * @return HTTP status code
 * ~Olson
 */
template<typename Callback>
int httpGetWithCallback(const char* url, Callback callback) {
    int httpCode = httpHelper.beginRequest(url);
    
    if (httpCode > 0) {
        // Process the response with the callback
        httpHelper.processJsonResponse(callback);
    }
    
    return httpCode;
}

/**
 * @brief Memory-optimized URL builder
 * 
 * Builds a URL with query parameters without using String concatenation.
 * @param baseUrl Base URL
 * @param numParams Number of parameter pairs
 * @param ... Parameter pairs (name1, value1, name2, value2, etc.)
 * @return Pointer to the constructed URL buffer
 * ~Olson
 */
template<typename... Args>
const char* buildUrl(const char* baseUrl, int numParams, Args... args) {
    return httpHelper.buildUrl(baseUrl, numParams, args...);
}

/**
 * @brief Memory-optimized string formatter
 * 
 * Formats a string without using dynamic memory allocations.
 * @param buffer Buffer to store the formatted string
 * @param bufferSize Size of the buffer
 * @param format Format string (printf style)
 * @param ... Format arguments
 * @return Number of characters written
 * ~Olson
 */
template<typename... Args>
int formatString(char* buffer, size_t bufferSize, const char* format, Args... args) {
    return StringUtils::formatString(buffer, bufferSize, format, args...);
}

#endif // MEMORY_OPTIMIZATIONS_H
