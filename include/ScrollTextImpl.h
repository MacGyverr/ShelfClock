#ifndef SCROLL_TEXT_IMPL_H
#define SCROLL_TEXT_IMPL_H

#include <Arduino.h>
#include <FastLED.h>
#include "ScrollText.h"

// Global instance of the ScrollText class
ScrollText gScrollText;

// External variables needed from ShelfClock.cpp
extern uint8_t breakOutSet;
extern CRGB scrollColor;
extern uint8_t r16_val, g16_val, b16_val;
extern CRGB* LEDs;
extern uint16_t SEGMENTS_LEDS;
extern uint8_t scrollColorSettings;
extern uint8_t pastelColors;

// Function declarations for functions used in the original scroll implementation
extern void displayNumber(uint8_t number, uint8_t position, CRGB color);
extern void allBlank();

// Forward declaration of server.handleClient()
extern void handleClient();

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
        handleClient,          // Function to handle client requests
        allBlank,              // Function to clear the display
        scrollColorSettings,   // Scroll color mode
        pastelColors,          // Pastel colors flag
        r16_val,               // Red value
        g16_val,               // Green value
        b16_val                // Blue value
    );
}

/**
 * @brief Wrapper for the original scroll function that accepts a String
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
 * @brief Overloaded version that accepts a C-string directly
 * 
 * This version is more memory-efficient as it avoids creating a String object.
 * Use this version for new code.
 * ~Olson
 */
void scroll(const char* text) {
    scrollOptimized(text);
}

// Function to handle server client requests
// This is a wrapper for server.handleClient() that will be passed to the ScrollText class
void handleClient() {
    // Call the actual server.handleClient() function from your main code
    // You'll need to replace this with the correct function call
    server.handleClient();
}

#endif // SCROLL_TEXT_IMPL_H
