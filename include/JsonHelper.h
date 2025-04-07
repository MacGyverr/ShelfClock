#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "StringUtils.h"

/**
 * @class JsonHelper
 * @brief Memory-efficient JSON processing utilities
 * 
 * This class provides methods for creating and processing JSON data
 * with minimal memory allocations and heap fragmentation.
 * ~Olson
 */
class JsonHelper {
private:
    // Pre-allocated buffer for JSON output
    static const size_t JSON_BUFFER_SIZE = 1024;
    char jsonBuffer[JSON_BUFFER_SIZE];
    
    // Current position in buffer
    int bufferPos;
    
public:
    /**
     * @brief Constructor
     * Initializes the JSON helper with an empty buffer
     * ~Olson
     */
    JsonHelper() : bufferPos(0) {
        clear();
    }
    
    /**
     * @brief Clear the JSON buffer
     * Resets the buffer to an empty state
     * ~Olson
     */
    void clear() {
        memset(jsonBuffer, 0, JSON_BUFFER_SIZE);
        bufferPos = 0;
    }
    
    /**
     * @brief Begin a new JSON object
     * @return Reference to this JsonHelper for method chaining
     * 
     * Starts a new JSON object with an opening brace.
     * ~Olson
     */
    JsonHelper& beginObject() {
        StringUtils::appendFormatted(jsonBuffer, JSON_BUFFER_SIZE, bufferPos, "{");
        return *this;
    }
    
    /**
     * @brief End the current JSON object
     * @return Reference to this JsonHelper for method chaining
     * 
     * Closes the current JSON object with a closing brace.
     * ~Olson
     */
    JsonHelper& endObject() {
        StringUtils::appendFormatted(jsonBuffer, JSON_BUFFER_SIZE, bufferPos, "}");
        return *this;
    }
    
    /**
     * @brief Begin a new JSON array
     * @return Reference to this JsonHelper for method chaining
     * 
     * Starts a new JSON array with an opening bracket.
     * ~Olson
     */
    JsonHelper& beginArray() {
        StringUtils::appendFormatted(jsonBuffer, JSON_BUFFER_SIZE, bufferPos, "[");
        return *this;
    }
    
    /**
     * @brief End the current JSON array
     * @return Reference to this JsonHelper for method chaining
     * 
     * Closes the current JSON array with a closing bracket.
     * ~Olson
     */
    JsonHelper& endArray() {
        StringUtils::appendFormatted(jsonBuffer, JSON_BUFFER_SIZE, bufferPos, "]");
        return *this;
    }
    
    /**
     * @brief Add a property separator (comma)
     * @return Reference to this JsonHelper for method chaining
     * 
     * Adds a comma separator between JSON properties or array items.
     * ~Olson
     */
    JsonHelper& addSeparator() {
        StringUtils::appendFormatted(jsonBuffer, JSON_BUFFER_SIZE, bufferPos, ",");
        return *this;
    }
    
    /**
     * @brief Add a string property
     * @param key Property name
     * @param value Property value
     * @return Reference to this JsonHelper for method chaining
     * 
     * Adds a string property to the JSON object.
     * ~Olson
     */
    JsonHelper& addProperty(const char* key, const char* value) {
        StringUtils::appendFormatted(jsonBuffer, JSON_BUFFER_SIZE, bufferPos, 
                                    "\"%s\":\"%s\"", key, value);
        return *this;
    }
    
    /**
     * @brief Add a numeric property
     * @param key Property name
     * @param value Property value
     * @return Reference to this JsonHelper for method chaining
     * 
     * Adds a numeric property to the JSON object.
     * ~Olson
     */
    JsonHelper& addProperty(const char* key, int value) {
        StringUtils::appendFormatted(jsonBuffer, JSON_BUFFER_SIZE, bufferPos, 
                                    "\"%s\":%d", key, value);
        return *this;
    }
    
    /**
     * @brief Add a floating-point property
     * @param key Property name
     * @param value Property value
     * @param precision Number of decimal places
     * @return Reference to this JsonHelper for method chaining
     * 
     * Adds a floating-point property to the JSON object.
     * ~Olson
     */
    JsonHelper& addProperty(const char* key, float value, int precision = 2) {
        char format[16];
        snprintf(format, sizeof(format), "\"%%s\":%%.%df", precision);
        StringUtils::appendFormatted(jsonBuffer, JSON_BUFFER_SIZE, bufferPos, 
                                    format, key, value);
        return *this;
    }
    
    /**
     * @brief Add a boolean property
     * @param key Property name
     * @param value Property value
     * @return Reference to this JsonHelper for method chaining
     * 
     * Adds a boolean property to the JSON object.
     * ~Olson
     */
    JsonHelper& addProperty(const char* key, bool value) {
        StringUtils::appendFormatted(jsonBuffer, JSON_BUFFER_SIZE, bufferPos, 
                                    "\"%s\":%s", key, value ? "true" : "false");
        return *this;
    }
    
    /**
     * @brief Add a raw JSON property
     * @param key Property name
     * @param rawJson Raw JSON value (without quotes)
     * @return Reference to this JsonHelper for method chaining
     * 
     * Adds a property with a raw JSON value to the JSON object.
     * ~Olson
     */
    JsonHelper& addRawProperty(const char* key, const char* rawJson) {
        StringUtils::appendFormatted(jsonBuffer, JSON_BUFFER_SIZE, bufferPos, 
                                    "\"%s\":%s", key, rawJson);
        return *this;
    }
    
    /**
     * @brief Add a string array item
     * @param value Array item value
     * @return Reference to this JsonHelper for method chaining
     * 
     * Adds a string item to a JSON array.
     * ~Olson
     */
    JsonHelper& addArrayItem(const char* value) {
        StringUtils::appendFormatted(jsonBuffer, JSON_BUFFER_SIZE, bufferPos, 
                                    "\"%s\"", value);
        return *this;
    }
    
    /**
     * @brief Add a numeric array item
     * @param value Array item value
     * @return Reference to this JsonHelper for method chaining
     * 
     * Adds a numeric item to a JSON array.
     * ~Olson
     */
    JsonHelper& addArrayItem(int value) {
        StringUtils::appendFormatted(jsonBuffer, JSON_BUFFER_SIZE, bufferPos, 
                                    "%d", value);
        return *this;
    }
    
    /**
     * @brief Add a floating-point array item
     * @param value Array item value
     * @param precision Number of decimal places
     * @return Reference to this JsonHelper for method chaining
     * 
     * Adds a floating-point item to a JSON array.
     * ~Olson
     */
    JsonHelper& addArrayItem(float value, int precision = 2) {
        char format[16];
        snprintf(format, sizeof(format), "%%.%df", precision);
        StringUtils::appendFormatted(jsonBuffer, JSON_BUFFER_SIZE, bufferPos, 
                                    format, value);
        return *this;
    }
    
    /**
     * @brief Add a boolean array item
     * @param value Array item value
     * @return Reference to this JsonHelper for method chaining
     * 
     * Adds a boolean item to a JSON array.
     * ~Olson
     */
    JsonHelper& addArrayItem(bool value) {
        StringUtils::appendFormatted(jsonBuffer, JSON_BUFFER_SIZE, bufferPos, 
                                    "%s", value ? "true" : "false");
        return *this;
    }
    
    /**
     * @brief Add a raw JSON array item
     * @param rawJson Raw JSON value
     * @return Reference to this JsonHelper for method chaining
     * 
     * Adds a raw JSON item to a JSON array.
     * ~Olson
     */
    JsonHelper& addRawArrayItem(const char* rawJson) {
        StringUtils::appendFormatted(jsonBuffer, JSON_BUFFER_SIZE, bufferPos, 
                                    "%s", rawJson);
        return *this;
    }
    
    /**
     * @brief Get the JSON string
     * @return Pointer to the JSON buffer
     * 
     * Returns the generated JSON string.
     * ~Olson
     */
    const char* getJson() const {
        return jsonBuffer;
    }
    
    /**
     * @brief Get the current buffer position
     * @return Current position in the buffer
     * 
     * Returns the current position in the buffer, which is also the length
     * of the generated JSON string.
     * ~Olson
     */
    int getLength() const {
        return bufferPos;
    }
    
    /**
     * @brief Create a complete JSON object in one call
     * @param numProps Number of properties
     * @param ... Property pairs (key1, value1, key2, value2, etc.)
     * @return Pointer to the JSON buffer
     * 
     * Creates a complete JSON object with the specified properties.
     * Values can be strings, integers, floats, or booleans.
     * ~Olson
     */
    template<typename... Args>
    const char* createObject(int numProps, Args... args) {
        clear();
        beginObject();
        
        // Process property pairs
        addProperties(numProps, args...);
        
        endObject();
        return jsonBuffer;
    }
    
private:
    /**
     * @brief Add multiple properties recursively
     * @param numProps Number of properties remaining
     * @param key Current property key
     * @param value Current property value
     * @param args Remaining arguments
     * 
     * Helper method for createObject to process variable arguments.
     * ~Olson
     */
    template<typename T, typename... Args>
    void addProperties(int numProps, const char* key, T value, Args... args) {
        // Add the current property
        addProperty(key, value);
        
        // Add separator if more properties follow
        if (numProps > 1) {
            addSeparator();
        }
        
        // Process remaining properties
        addProperties(numProps - 1, args...);
    }
    
    /**
     * @brief Terminal case for addProperties recursion
     * @param numProps Number of properties (should be 0)
     * 
     * End of recursion for addProperties.
     * ~Olson
     */
    void addProperties(int numProps) {
        // End of recursion
    }
    
    /**
     * @brief Add a property with type detection
     * @param key Property key
     * @param value Property value
     * 
     * Adds a property with automatic type detection.
     * ~Olson
     */
    void addProperty(const char* key, const char* value) {
        StringUtils::appendFormatted(jsonBuffer, JSON_BUFFER_SIZE, bufferPos, 
                                    "\"%s\":\"%s\"", key, value);
    }
    
    void addProperty(const char* key, int value) {
        StringUtils::appendFormatted(jsonBuffer, JSON_BUFFER_SIZE, bufferPos, 
                                    "\"%s\":%d", key, value);
    }
    
    void addProperty(const char* key, float value) {
        StringUtils::appendFormatted(jsonBuffer, JSON_BUFFER_SIZE, bufferPos, 
                                    "\"%s\":%.2f", key, value);
    }
    
    void addProperty(const char* key, bool value) {
        StringUtils::appendFormatted(jsonBuffer, JSON_BUFFER_SIZE, bufferPos, 
                                    "\"%s\":%s", key, value ? "true" : "false");
    }
};

#endif // JSON_HELPER_H
