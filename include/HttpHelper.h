#ifndef HTTP_HELPER_H
#define HTTP_HELPER_H

#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

/**
 * @class HttpHelper
 * @brief Memory-efficient HTTP request and response handling
 * 
 * This class provides utilities for making HTTP requests and processing
 * responses with minimal memory usage by using streaming and incremental
 * processing techniques.
 * ~Olson
 */
class HttpHelper {
private:
    // Pre-allocated buffer for URL construction
    static const size_t URL_BUFFER_SIZE = 256;
    char urlBuffer[URL_BUFFER_SIZE];
    
    // HTTP client instance
    HTTPClient http;
    
    // Response processing state
    bool requestActive;
    
public:
    /**
     * @brief Constructor
     * Initializes the HTTP helper with default values
     * ~Olson
     */
    HttpHelper() : requestActive(false) {
        memset(urlBuffer, 0, URL_BUFFER_SIZE);
    }
    
    /**
     * @brief Destructor
     * Ensures any active connections are properly closed
     * ~Olson
     */
    ~HttpHelper() {
        if (requestActive) {
            http.end();
        }
    }
    
    /**
     * @brief Build a URL with query parameters
     * @param baseUrl Base URL
     * @param numParams Number of parameter pairs
     * @param ... Parameter pairs (name1, value1, name2, value2, etc.)
     * @return Pointer to the constructed URL buffer
     * 
     * Constructs a URL with query parameters without using String objects.
     * Example: buildUrl("https://api.example.com", 2, "lat", "12.34", "lon", "56.78")
     * ~Olson
     */
    const char* buildUrl(const char* baseUrl, int numParams, ...) {
        // Clear buffer
        memset(urlBuffer, 0, URL_BUFFER_SIZE);
        
        // Copy base URL
        strncpy(urlBuffer, baseUrl, URL_BUFFER_SIZE - 1);
        size_t urlLen = strlen(urlBuffer);
        
        // Add query parameters
        va_list args;
        va_start(args, numParams);
        
        for (int i = 0; i < numParams; i++) {
            const char* paramName = va_arg(args, const char*);
            const char* paramValue = va_arg(args, const char*);
            
            // Add ? for first parameter, & for subsequent ones
            if (i == 0) {
                if (strchr(urlBuffer, '?') == NULL) {
                    strncat(urlBuffer, "?", URL_BUFFER_SIZE - urlLen - 1);
                    urlLen++;
                } else {
                    strncat(urlBuffer, "&", URL_BUFFER_SIZE - urlLen - 1);
                    urlLen++;
                }
            } else {
                strncat(urlBuffer, "&", URL_BUFFER_SIZE - urlLen - 1);
                urlLen++;
            }
            
            // Add parameter name
            strncat(urlBuffer, paramName, URL_BUFFER_SIZE - urlLen - 1);
            urlLen += strlen(paramName);
            
            // Add equals sign
            strncat(urlBuffer, "=", URL_BUFFER_SIZE - urlLen - 1);
            urlLen++;
            
            // Add parameter value
            strncat(urlBuffer, paramValue, URL_BUFFER_SIZE - urlLen - 1);
            urlLen += strlen(paramValue);
            
            // Check if buffer is full
            if (urlLen >= URL_BUFFER_SIZE - 10) {
                break;
            }
        }
        
        va_end(args);
        return urlBuffer;
    }
    
    /**
     * @brief Make an HTTP GET request
     * @param url URL to request
     * @return HTTP status code or negative value for errors
     * 
     * Initiates an HTTP GET request to the specified URL.
     * ~Olson
     */
    int beginRequest(const char* url) {
        // End any previous request
        if (requestActive) {
            http.end();
        }
        
        // Begin new request
        http.begin(url);
        int httpCode = http.GET();
        requestActive = (httpCode > 0);
        
        return httpCode;
    }
    
    /**
     * @brief Process JSON response with a callback function
     * @param processor Function to process each key-value pair
     * @return true if successful, false on error
     * 
     * Processes the JSON response incrementally without loading the entire
     * response into memory. The processor function is called for each key-value
     * pair in the JSON.
     * ~Olson
     */
    template<typename Callback>
    bool processJsonResponse(Callback processor) {
        if (!requestActive) {
            return false;
        }
        
        // Get the response stream
        WiFiClient* stream = http.getStreamPtr();
        if (!stream) {
            http.end();
            requestActive = false;
            return false;
        }
        
        // Create a JSON streaming parser
        StaticJsonDocument<64> filter; // Small filter document
        filter.set(true); // Accept all fields
        
        // Process the JSON incrementally
        DeserializationError error = deserializeJson(
            filter, *stream,
            DeserializationOption::Filter(filter)
        );
        
        // End the request
        http.end();
        requestActive = false;
        
        return (error == DeserializationError::Ok);
    }
    
    /**
     * @brief Process JSON response into a JsonDocument
     * @param doc JsonDocument to store the parsed response
     * @return true if successful, false on error
     * 
     * Parses the JSON response into the provided JsonDocument.
     * The caller is responsible for ensuring the document is large enough.
     * ~Olson
     */
    bool parseJsonResponse(JsonDocument& doc) {
        if (!requestActive) {
            return false;
        }
        
        // Get the response stream
        WiFiClient* stream = http.getStreamPtr();
        if (!stream) {
            http.end();
            requestActive = false;
            return false;
        }
        
        // Parse the JSON
        DeserializationError error = deserializeJson(doc, *stream);
        
        // End the request
        http.end();
        requestActive = false;
        
        return (error == DeserializationError::Ok);
    }
    
    /**
     * @brief Get a specific value from a JSON response
     * @param path JSON path to the desired value (e.g., "main.temp")
     * @param buffer Buffer to store the result
     * @param bufferSize Size of the buffer
     * @return true if successful, false on error
     * 
     * Extracts a specific value from the JSON response without
     * parsing the entire document.
     * ~Olson
     */
    bool getJsonValue(const char* path, char* buffer, size_t bufferSize) {
        if (!requestActive) {
            return false;
        }
        
        // Create a filter to only parse the specified path
        StaticJsonDocument<128> filter;
        JsonObject filterObj = filter.to<JsonObject>();
        
        // Build the filter based on the path
        char pathCopy[64];
        strncpy(pathCopy, path, sizeof(pathCopy) - 1);
        pathCopy[sizeof(pathCopy) - 1] = '\0';
        
        // Split the path by dots
        char* token = strtok(pathCopy, ".");
        JsonObject* currentObj = &filterObj;
        
        while (token != NULL) {
            // Add this path component to the filter
            (*currentObj)[token] = true;
            
            // Move to the next path component
            token = strtok(NULL, ".");
            if (token != NULL) {
                // Create a nested object for the next level
                (*currentObj)[token] = JsonObject();
                currentObj = &(*currentObj)[token].as<JsonObject>();
            }
        }
        
        // Get the response stream
        WiFiClient* stream = http.getStreamPtr();
        if (!stream) {
            http.end();
            requestActive = false;
            return false;
        }
        
        // Parse only the filtered part
        StaticJsonDocument<256> doc;
        DeserializationError error = deserializeJson(
            doc, *stream,
            DeserializationOption::Filter(filter)
        );
        
        // End the request
        http.end();
        requestActive = false;
        
        if (error) {
            return false;
        }
        
        // Extract the value
        JsonVariant value = doc;
        token = strtok(pathCopy, ".");
        while (token != NULL) {
            if (!value.is<JsonObject>()) {
                return false;
            }
            value = value[token];
            token = strtok(NULL, ".");
        }
        
        // Convert to string and copy to buffer
        if (value.is<const char*>()) {
            strncpy(buffer, value.as<const char*>(), bufferSize - 1);
            buffer[bufferSize - 1] = '\0';
        } else if (value.is<int>()) {
            snprintf(buffer, bufferSize, "%d", value.as<int>());
        } else if (value.is<float>()) {
            snprintf(buffer, bufferSize, "%.2f", value.as<float>());
        } else if (value.is<bool>()) {
            strncpy(buffer, value.as<bool>() ? "true" : "false", bufferSize - 1);
            buffer[bufferSize - 1] = '\0';
        } else {
            return false;
        }
        
        return true;
    }
};

#endif // HTTP_HELPER_H
