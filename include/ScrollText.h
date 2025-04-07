#ifndef SCROLL_TEXT_H
#define SCROLL_TEXT_H

#include <Arduino.h>
#include <FastLED.h>

/**
 * @class ScrollText
 * @brief Memory-optimized text scrolling implementation
 * 
 * This class provides text scrolling functionality with minimal dynamic
 * memory allocations by using pre-allocated buffers and efficient character
 * translation.
 * ~Olson
 */
class ScrollText {
private:
    // Maximum string length that can be scrolled
    static const uint16_t MAX_SCROLL_LENGTH = 256;
    
    // Pre-allocated buffers for character translation
    // Using static buffers instead of dynamic allocation
    char sentenceBuffer[MAX_SCROLL_LENGTH + 1];
    uint16_t translatedBuffer[(MAX_SCROLL_LENGTH * 2) + 12]; // +12 for padding (6 at front, 6 at back)
    
    // Scroll settings
    CRGB scrollColor;
    uint8_t breakOutSet;
    
    /**
     * @brief Translate a character to its display code
     * @param c Character to translate
     * @return Display code for the character
     * 
     * Uses a lookup table approach instead of multiple if statements
     * for better performance and code readability.
     * ~Olson
     */
    uint16_t translateChar(char c) {
        // Default for unknown characters
        uint16_t letterNumber = 10;
        
        // Digits
        if (c >= '0' && c <= '9') {
            letterNumber = c - '0';
        }
        // Space
        else if (c == ' ') {
            letterNumber = 10;
        }
        // Special characters
        else if (c == '`' || c == '\'') {
            letterNumber = 17;
        }
        else if (c == '%') {
            letterNumber = 15;
        }
        else if (c == ',') {
            letterNumber = 22;
        }
        else if (c == '-') {
            letterNumber = 23;
        }
        else if (c == '.') {
            letterNumber = 24;
        }
        else if (c == '^') {
            letterNumber = 26;
        }
        else if (c == ':') {
            letterNumber = 27;
        }
        // Uppercase letters
        else if (c >= 'A' && c <= 'Z') {
            letterNumber = c - 'A' + 34;
        }
        // Lowercase letters
        else if (c >= 'a' && c <= 'z') {
            letterNumber = c - 'a' + 66;
        }
        
        return letterNumber;
    }
    
public:
    /**
     * @brief Constructor
     * Initializes the scroll text processor with default values
     * ~Olson
     */
    ScrollText() : breakOutSet(0) {
        // Clear buffers
        memset(sentenceBuffer, 0, sizeof(sentenceBuffer));
        memset(translatedBuffer, 0, sizeof(translatedBuffer));
    }
    
    /**
     * @brief Set the scroll color
     * @param color RGB color for the scrolling text
     * ~Olson
     */
    void setColor(CRGB color) {
        scrollColor = color;
    }
    
    /**
     * @brief Set the scroll color using RGB components
     * @param r Red component (0-255)
     * @param g Green component (0-255)
     * @param b Blue component (0-255)
     * ~Olson
     */
    void setColor(uint8_t r, uint8_t g, uint8_t b) {
        scrollColor = CRGB(r, g, b);
    }
    
    /**
     * @brief Set the breakout flag
     * Used to interrupt scrolling
     * ~Olson
     */
    void setBreakOut(bool breakOut) {
        breakOutSet = breakOut ? 1 : 0;
    }
    
    /**
     * @brief Get the breakout flag
     * @return Current breakout state
     * ~Olson
     */
    bool getBreakOut() const {
        return breakOutSet != 0;
    }
    
    /**
     * @brief Prepare text for scrolling
     * @param text Text to scroll (will be truncated if longer than MAX_SCROLL_LENGTH)
     * @return true if successful, false if text was truncated
     * 
     * Translates the input text to display codes and prepares the buffers
     * for scrolling. Uses pre-allocated buffers to avoid dynamic memory
     * allocations.
     * ~Olson
     */
    bool prepareText(const char* text) {
        bool truncated = false;
        size_t textLen = strlen(text);
        
        // Check if text needs truncation
        if (textLen > MAX_SCROLL_LENGTH) {
            textLen = MAX_SCROLL_LENGTH;
            truncated = true;
        }
        
        // Copy text to buffer
        strncpy(sentenceBuffer, text, textLen);
        sentenceBuffer[textLen] = '\0';
        
        // Add padding markers (value 96) at front and back
        for (int i = 0; i < 6; i++) {
            translatedBuffer[i] = 96;
            translatedBuffer[(textLen * 2) + 6 + i] = 96;
        }
        
        // Translate each character
        for (uint16_t i = 0; i < textLen; i++) {
            uint16_t letterNumber = translateChar(sentenceBuffer[i]);
            
            // Store in translated buffer (2 positions per character)
            translatedBuffer[6 + (i * 2)] = letterNumber;
            translatedBuffer[6 + (i * 2) + 1] = 96; // Separator
        }
        
        return !truncated;
    }
    
    /**
     * @brief Alternative version that accepts a String object
     * @param text String object containing text to scroll
     * @return true if successful, false if text was truncated
     * 
     * This is provided for compatibility with existing code,
     * but the char* version is preferred for memory efficiency.
     * ~Olson
     */
    bool prepareText(const String& text) {
        return prepareText(text.c_str());
    }
    
    /**
     * @brief Display the scrolling text
     * @param leds LED array to display on
     * @param numLeds Number of LEDs in the array
     * @param displayNumberFunc Function pointer to the displayNumber function
     * @param handleClientFunc Function pointer to the server.handleClient function
     * @param scrollColorSettings Scroll color mode (0=fixed, 1=random)
     * @param pastelColors Use pastel colors if true
     * @param r16_val Red value for fixed color
     * @param g16_val Green value for fixed color
     * @param b16_val Blue value for fixed color
     * 
     * Displays the prepared text on the LED array with scrolling effect.
     * This method replicates the exact behavior of the original scroll function.
     * ~Olson
     */
    void displayScroll(CRGB* leds, uint16_t numLeds, 
                      void (*displayNumberFunc)(uint8_t, uint8_t, CRGB),
                      void (*handleClientFunc)(),
                      void (*allBlankFunc)(),
                      uint8_t scrollColorSettings = 0,
                      uint8_t pastelColors = 0,
                      uint8_t r16_val = 255,
                      uint8_t g16_val = 255,
                      uint8_t b16_val = 255) {
        
        // Reset breakout flag
        breakOutSet = 0;
        
        // Determine scroll color based on settings
        if (scrollColorSettings == 0) {
            scrollColor = CRGB(r16_val, g16_val, b16_val);
        }
        else if (scrollColorSettings == 1) {
            if (pastelColors == 0) {
                scrollColor = CHSV(random(0, 255), 255, 255);
            } else {
                scrollColor = CRGB(random(0, 255), random(0, 255), random(0, 255));
            }
        }
        
        // Calculate total length of translated text
        uint16_t totalLength = 0;
        while (translatedBuffer[totalLength] != 0 && totalLength < sizeof(translatedBuffer)/sizeof(translatedBuffer[0])) {
            totalLength++;
        }
        
        // Determine actual scroll length based on content
        uint16_t scrollLength = 0;
        for (uint16_t i = 0; i < MAX_SCROLL_LENGTH * 2 + 12; i++) {
            if (translatedBuffer[i] != 0) {
                scrollLength = i;
            }
        }
        
        // Perform scrolling
        for (uint16_t finalposition = 0; finalposition < scrollLength && !breakOutSet; finalposition++) {
            // Clear LEDs
            for (int i = 0; i < numLeds; i++) {
                leds[i] = CRGB::Black;
            }
            
            // Display characters in the current window
            if (translatedBuffer[finalposition] != 96) {
                displayNumberFunc(translatedBuffer[finalposition], 6, scrollColor);
            }
            if (finalposition + 1 < scrollLength && translatedBuffer[finalposition + 1] != 96) {
                displayNumberFunc(translatedBuffer[finalposition + 1], 5, scrollColor);
            }
            if (finalposition + 2 < scrollLength && translatedBuffer[finalposition + 2] != 96) {
                displayNumberFunc(translatedBuffer[finalposition + 2], 4, scrollColor);
            }
            if (finalposition + 3 < scrollLength && translatedBuffer[finalposition + 3] != 96) {
                displayNumberFunc(translatedBuffer[finalposition + 3], 3, scrollColor);
            }
            if (finalposition + 4 < scrollLength && translatedBuffer[finalposition + 4] != 96) {
                displayNumberFunc(translatedBuffer[finalposition + 4], 2, scrollColor);
            }
            if (finalposition + 5 < scrollLength && translatedBuffer[finalposition + 5] != 96) {
                displayNumberFunc(translatedBuffer[finalposition + 5], 1, scrollColor);
            }
            if (finalposition + 6 < scrollLength && translatedBuffer[finalposition + 6] != 96) {
                displayNumberFunc(translatedBuffer[finalposition + 6], 0, scrollColor);
            }
            
            // Update display
            FastLED.show();
            
            // Handle client requests and slow down scrolling
            for (int i = 0; i < 400 && !breakOutSet; i++) {
                handleClientFunc();
            }
        }
        
        // Clear display when done
        allBlankFunc();
        allBlankFunc();
    }
};

#endif // SCROLL_TEXT_H
