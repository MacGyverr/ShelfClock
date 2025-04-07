# Memory Optimization Guide for ShelfClock

This guide outlines strategies for optimizing memory usage in your ShelfClock project to reduce heap fragmentation and improve overall stability on the ESP32 platform.

## Table of Contents
1. [Understanding Memory Issues](#understanding-memory-issues)
2. [String Handling Optimizations](#string-handling-optimizations)
3. [JSON Processing Optimizations](#json-processing-optimizations)
4. [HTTP Request Optimizations](#http-request-optimizations)
5. [Text Scrolling Optimizations](#text-scrolling-optimizations)
6. [Heap Fragmentation Monitoring](#heap-fragmentation-monitoring)
7. [Implementation Examples](#implementation-examples)
8. [Monitoring Memory Usage](#monitoring-memory-usage)

## Understanding Memory Issues

The ESP32 has limited memory resources, and improper memory management can lead to:

- **Heap Fragmentation**: Frequent allocation and deallocation of memory can fragment the heap, making it difficult to allocate larger blocks even when total free memory is sufficient.
- **Memory Leaks**: Failing to free allocated memory can gradually reduce available memory.
- **Stack Overflow**: Deep function call nesting or large local variables can overflow the stack.

The most common issue in Arduino-based projects is heap fragmentation caused by dynamic memory allocations, especially with the `String` class and other dynamic data structures.

## String Handling Optimizations

### Use Pre-allocated Buffers

Instead of using the `String` class which dynamically allocates memory, use pre-allocated character arrays:

```cpp
// Bad: Uses dynamic memory allocation
String message = "Hello, " + name + "!";

// Good: Uses pre-allocated buffer
char message[64];
snprintf(message, sizeof(message), "Hello, %s!", name);
```

### Use StringUtils Helper Class

The `StringUtils` class provides memory-efficient string handling functions:

```cpp
// Format a string safely
char buffer[64];
StringUtils::formatString(buffer, sizeof(buffer), "Temp: %d°C", temperature);

// Append formatted text to an existing buffer
StringUtils::appendFormatted(buffer, sizeof(buffer), ", Humidity: %d%%", humidity);
```

## JSON Processing Optimizations

### Use Static JSON Buffers

When working with ArduinoJson, use static buffers instead of dynamic ones:

```cpp
// Bad: Dynamic allocation
DynamicJsonDocument doc(1024);

// Good: Static allocation
StaticJsonDocument<1024> doc;
```

### Use JsonHelper Class

The `JsonHelper` class provides memory-efficient JSON creation and processing:

```cpp
// Create a JSON object with pre-allocated buffer
const char* json = jsonHelper.createObject(3, 
    "temperature", temperature,
    "humidity", humidity,
    "status", "ok"
);
```

## HTTP Request Optimizations

### Process Responses Incrementally

Instead of storing entire HTTP responses in memory, process them incrementally:

```cpp
// Bad: Stores entire response in memory
String payload = http.getString();
DynamicJsonDocument doc(1024);
deserializeJson(doc, payload);

// Good: Process response incrementally
httpHelper.processJsonResponse([](JsonObject& obj) {
    // Process each key-value pair as it's parsed
    if (obj.containsKey("temperature")) {
        float temp = obj["temperature"];
        // Do something with temp
    }
});
```

### Use HttpHelper Class

The `HttpHelper` class provides memory-efficient HTTP request handling:

```cpp
// Make an HTTP request and process the response
httpHelper.beginRequest("https://api.example.com/data");
httpHelper.processJsonResponse([](JsonObject& obj) {
    // Process JSON data
});
```

## Text Scrolling Optimizations

### Use ScrollText Class

The `ScrollText` class provides memory-efficient text scrolling with pre-allocated buffers:

```cpp
// Create a ScrollText instance
ScrollText scrollText;

// Prepare text for scrolling
scrollText.prepareText("Hello, World!");

// Display the scrolling text
scrollText.displayScroll(leds, numLeds, displayNumber, handleClient, allBlank);
```

### Replace Original Scroll Function

The original `scroll` function can be replaced with a memory-optimized version:

```cpp
// Include the memory optimizations header
#include "MemoryOptimizations.h"

// The scroll function is now automatically optimized
// No changes needed to existing code that calls scroll()
scroll("Hello, World!"); // Uses the optimized implementation
```

### Key Improvements in ScrollText

The `ScrollText` class offers several memory optimizations:

1. **Pre-allocated Buffers**: Uses fixed-size buffers instead of dynamic String objects
2. **Efficient Character Translation**: Uses a lookup table approach for character mapping
3. **Reusable Instance**: The ScrollText object can be reused for multiple scrolling operations
4. **Direct C-string Support**: Accepts C-strings directly without creating String objects
5. **Bounds Checking**: Prevents buffer overflows and memory corruption

## Heap Fragmentation Monitoring

The ShelfClock project includes built-in heap fragmentation monitoring through the `MemoryMonitor` class.

### Understanding Fragmentation

Heap fragmentation occurs when memory is allocated and freed in patterns that create small, unusable gaps in the heap. Even if you have plenty of total free memory, these gaps can prevent large allocations from succeeding.

The fragmentation percentage is calculated as:

```cpp
fragmentation = 100 - (maxAllocHeap * 100) / freeHeap;
```

Where:
- `maxAllocHeap` is the size of the largest contiguous block of memory that can be allocated
- `freeHeap` is the total free memory available

A higher percentage indicates more fragmentation and potential allocation problems.

### Using the Fragmentation Monitor

The `MemoryMonitor` class provides several methods for tracking fragmentation:

```cpp
// Get current fragmentation percentage (0-100%)
uint8_t fragmentation = memoryMonitor.getFragmentation();

// Check if fragmentation is dangerously high (above threshold)
if (memoryMonitor.isFragmentationHigh()) {
    // Take corrective action
}

// Get JSON with all memory metrics including fragmentation
const char* memoryStatus = memoryMonitor.getStatusJson();
```

### Fragmentation Alerts

The memory monitor automatically sets alerts when fragmentation exceeds the configured threshold (default set to 80%):

```cpp
// Configure custom fragmentation threshold
MemoryMonitor memoryMonitor(60000, 10000, 70); // 70% fragmentation threshold

// Check for high fragmentation alert
if (memoryMonitor.isFragmentationHigh()) {
    Serial.println("WARNING: High memory fragmentation detected!");
    // Consider taking corrective action
}
```

### Visualizing Fragmentation

Fragmentation data is included in the memory status JSON and can be visualized through the web interface at `/memory.html`. This allows you to track fragmentation trends over time and identify operations that may be causing fragmentation issues.

## Implementation Examples

### Including Memory Optimizations

To use all memory optimizations, include the main header file in your ShelfClock.cpp:

```cpp
#include "MemoryOptimizations.h"
```

This provides optimized implementations of:
- `scroll()` function (both String and C-string versions)
- JSON creation and processing functions
- HTTP request handling functions
- String formatting and manipulation functions

### Using Optimized Scroll Function

The optimized scroll function is a drop-in replacement for the original:

```cpp
// Original code (uses dynamic memory)
scroll("Hello, World!");

// No changes needed - the function is now optimized
// Behind the scenes, it uses pre-allocated buffers
```

### Creating JSON Status

Use the optimized JSON creation functions:

```cpp
// Create a JSON status string without dynamic allocations
const char* status = createJsonStatus(3,
    "heap", ESP.getFreeHeap(),
    "uptime", millis() / 1000,
    "status", "ok"
);
```

## Monitoring Memory Usage

Use the `MemoryMonitor` class to track memory usage and detect fragmentation:

```cpp
// Get memory status as JSON
const char* memoryStatus = memoryMonitor.getStatusJson();

// Log memory usage periodically
if (millis() - lastMemoryCheck > 30000) {
    Serial.println(memoryMonitor.getStatusJson());
    lastMemoryCheck = millis();
}
```

Monitor for patterns that indicate fragmentation, such as decreasing free heap despite releasing memory, or failed allocations despite sufficient total free memory.

---

By implementing these memory optimization strategies throughout the ShelfClock project, you can significantly reduce heap fragmentation and improve overall stability and performance on the ESP32 platform.
