#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <Arduino.h>

/**
 * @class StringUtils
 * @brief Utilities for memory-efficient string handling
 * 
 * This class provides static methods for common string operations
 * that minimize heap allocations and fragmentation.
 * ~Olson
 */
class StringUtils {
public:
    /**
     * @brief Safely format a string into a pre-allocated buffer
     * 
     * @param buffer The destination buffer
     * @param bufferSize Size of the destination buffer
     * @param format Format string (printf style)
     * @param ... Variable arguments for format string
     * @return Number of characters written (excluding null terminator)
     * 
     * Uses snprintf internally to prevent buffer overflows.
     * ~Olson
     */
    static int formatString(char* buffer, size_t bufferSize, const char* format, ...) {
        va_list args;
        va_start(args, format);
        int result = vsnprintf(buffer, bufferSize, format, args);
        va_end(args);
        return result;
    }
    
    /**
     * @brief Append formatted string to an existing buffer
     * 
     * @param buffer The destination buffer
     * @param bufferSize Total size of the destination buffer
     * @param currentPos Current position in buffer (will be updated)
     * @param format Format string (printf style)
     * @param ... Variable arguments for format string
     * @return Number of characters appended
     * 
     * Safely appends formatted text to a buffer without overflowing.
     * Updates the currentPos parameter to point to the new end of string.
     * ~Olson
     */
    static int appendFormatted(char* buffer, size_t bufferSize, int& currentPos, const char* format, ...) {
        va_list args;
        va_start(args, format);
        int result = vsnprintf(buffer + currentPos, bufferSize - currentPos, format, args);
        va_end(args);
        
        if (result > 0) {
            currentPos += result;
        }
        
        return result;
    }
    
    /**
     * @brief Convert a C++ String to a C-string with bounds checking
     * 
     * @param str The source String
     * @param buffer The destination buffer
     * @param bufferSize Size of the destination buffer
     * @return true if successful, false if buffer too small
     * 
     * Safely copies a String to a char buffer with bounds checking.
     * ~Olson
     */
    static bool stringToBuffer(const String& str, char* buffer, size_t bufferSize) {
        if (str.length() >= bufferSize) {
            // String too large for buffer, copy what we can and null-terminate
            strncpy(buffer, str.c_str(), bufferSize - 1);
            buffer[bufferSize - 1] = '\0';
            return false;
        }
        
        // Copy the entire string
        strcpy(buffer, str.c_str());
        return true;
    }
    
    /**
     * @brief Create a JSON string in a pre-allocated buffer
     * 
     * @param buffer The destination buffer
     * @param bufferSize Size of the destination buffer
     * @param fields Array of key-value pairs
     * @param fieldCount Number of fields in the array
     * @return Number of characters written
     * 
     * Creates a JSON object without using dynamic memory.
     * ~Olson
     */
    struct JsonField {
        const char* key;
        const char* value;
    };
    
    static int createJson(char* buffer, size_t bufferSize, const JsonField* fields, size_t fieldCount) {
        int pos = 0;
        
        // Start JSON object
        pos += appendFormatted(buffer, bufferSize, pos, "{");
        
        // Add fields
        for (size_t i = 0; i < fieldCount; i++) {
            pos += appendFormatted(buffer, bufferSize, pos, "\"%s\":%s", 
                                  fields[i].key, fields[i].value);
            
            // Add comma if not the last field
            if (i < fieldCount - 1) {
                pos += appendFormatted(buffer, bufferSize, pos, ",");
            }
        }
        
        // Close JSON object
        pos += appendFormatted(buffer, bufferSize, pos, "}");
        
        return pos;
    }
    
    /**
     * @brief Efficiently concatenate multiple strings into a buffer
     * 
     * @param buffer The destination buffer
     * @param bufferSize Size of the destination buffer
     * @param strings Array of strings to concatenate
     * @param count Number of strings in the array
     * @return Number of characters written
     * 
     * Concatenates multiple strings without creating intermediate String objects.
     * ~Olson
     */
    static int concatStrings(char* buffer, size_t bufferSize, const char** strings, size_t count) {
        int pos = 0;
        
        for (size_t i = 0; i < count && pos < bufferSize - 1; i++) {
            int len = strlen(strings[i]);
            int copyLen = (pos + len < bufferSize - 1) ? len : bufferSize - pos - 1;
            
            strncpy(buffer + pos, strings[i], copyLen);
            pos += copyLen;
        }
        
        // Ensure null termination
        buffer[pos] = '\0';
        
        return pos;
    }
};

#endif // STRING_UTILS_H
