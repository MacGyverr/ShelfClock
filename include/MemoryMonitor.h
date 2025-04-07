#ifndef MEMORY_MONITOR_H
#define MEMORY_MONITOR_H

#include <Arduino.h>
#include <esp_system.h>

/*How It Works
The memory monitor runs in the background, collecting data about the ESP32's memory usage. It tracks:
- Total heap size
- Free heap memory
- Minimum free heap (lowest point since boot)
- Maximum allocatable heap block
- Heap fragmentation percentage
This data is collected at regular intervals and stored in a circular buffer, allowing you to see trends over time. The system will alert you when memory is running low or when fragmentation becomes problematic.
Data can be viewed at http://<device-ip>/memory.html
`Olson*/

/**
 * @class MemoryMonitor
 * @brief Monitors ESP32 memory usage and provides alerts for potential issues
 * 
 * This class tracks heap memory usage, fragmentation, and allocation patterns over time.
 * It maintains historical data to help identify memory leaks and fragmentation issues
 * before they cause system instability or crashes.
 * ~Olson
 */
class MemoryMonitor {
private:
    // Memory metrics
    size_t totalHeapSize;     // Total heap size in bytes
    size_t freeHeap;          // Current free heap in bytes
    size_t minFreeHeap;       // Minimum free heap since boot in bytes
    size_t maxAllocHeap;      // Maximum allocatable block size in bytes
    uint8_t heapFragmentation; // Heap fragmentation percentage (0-100%)
    
    // Timestamps for update management
    unsigned long lastUpdateTime;  // Last time memory metrics were updated
    unsigned long updateInterval;  // How often to update memory metrics (milliseconds)
    
    /**
     * Historical data storage using circular buffer pattern
     * Allows tracking memory trends over time without using excessive memory
     * ~Olson
     */
    #define HISTORY_SIZE 24 // Store 24 data points (covers 24 minutes with default interval)
    size_t freeHeapHistory[HISTORY_SIZE];  // Historical free heap values
    uint8_t fragHistory[HISTORY_SIZE];     // Historical fragmentation percentages
    uint8_t historyIndex;                  // Current position in circular buffer
    
    // Alert thresholds for proactive monitoring
    size_t lowMemoryThreshold;       // Threshold for low memory warning (bytes)
    uint8_t highFragmentationThreshold; // Threshold for high fragmentation warning (%)
    
    // Status flags for current memory state
    bool lowMemoryAlert;          // True if free memory is below threshold
    bool highFragmentationAlert;  // True if fragmentation is above threshold

    // Pre-allocated buffer for JSON string to reduce heap fragmentation
    // ~Olson
    static const size_t JSON_BUFFER_SIZE = 1024;
    char jsonBuffer[JSON_BUFFER_SIZE];

public:
    /**
     * @brief Constructor for MemoryMonitor
     * 
     * @param interval How often to update memory metrics (milliseconds)
     * @param lowMemThreshold Threshold for low memory warnings (bytes)
     * @param highFragThreshold Threshold for high fragmentation warnings (%)
     * 
     * Initializes the memory monitor with specified thresholds and update interval.
     * Default values are conservative for most ESP32 applications.
     * ~Olson
     */
    MemoryMonitor(unsigned long interval = 60000, // Default update interval: 1 minute
                 size_t lowMemThreshold = 10000,  // Default low memory threshold: 10KB
                 uint8_t highFragThreshold = 80): // Default high fragmentation threshold: 80%
        updateInterval(interval),
        lowMemoryThreshold(lowMemThreshold),
        highFragmentationThreshold(highFragThreshold),
        lastUpdateTime(0),
        historyIndex(0),
        lowMemoryAlert(false),
        highFragmentationAlert(false)
    {
        // Initialize history arrays with zeros
        for (int i = 0; i < HISTORY_SIZE; i++) {
            freeHeapHistory[i] = 0;
            fragHistory[i] = 0;
        }
        
        // Initialize JSON buffer
        memset(jsonBuffer, 0, JSON_BUFFER_SIZE);
        
        // Initial update to populate metrics
        update();
    }
    
    /**
     * @brief Updates memory metrics if update interval has elapsed
     * 
     * Collects current memory statistics from ESP32, updates historical data,
     * and checks for alert conditions. Only updates if the specified interval
     * has passed since the last update to minimize performance impact.
     * ~Olson
     */
    void update() {
        unsigned long currentTime = millis();
        
        // Only update at specified intervals to reduce overhead
        if (currentTime - lastUpdateTime >= updateInterval) {
            // Update memory metrics using ESP32 system functions
            totalHeapSize = ESP.getHeapSize();
            freeHeap = ESP.getFreeHeap();
            minFreeHeap = ESP.getMinFreeHeap();
            maxAllocHeap = ESP.getMaxAllocHeap();
            
            // Calculate heap fragmentation percentage
            // Higher values indicate more fragmented memory
            heapFragmentation = 100 - (maxAllocHeap * 100) / freeHeap;
            
            // Update history using circular buffer pattern
            freeHeapHistory[historyIndex] = freeHeap;
            fragHistory[historyIndex] = heapFragmentation;
            historyIndex = (historyIndex + 1) % HISTORY_SIZE; // Wrap around when buffer is full
            
            // Check for alert conditions
            lowMemoryAlert = (freeHeap < lowMemoryThreshold);
            highFragmentationAlert = (heapFragmentation > highFragmentationThreshold);
            
            lastUpdateTime = currentTime;
        }
    }
    
    /**
     * @brief Get current free heap memory
     * @return Current free heap in bytes
     * 
     * Updates memory metrics if needed before returning value.
     * ~Olson
     */
    size_t getFreeHeap() {
        update();
        return freeHeap;
    }
    
    /**
     * @brief Get minimum free heap since boot
     * @return Minimum free heap in bytes
     * 
     * Useful for detecting memory usage spikes.
     * ~Olson
     */
    size_t getMinFreeHeap() {
        update();
        return minFreeHeap;
    }
    
    /**
     * @brief Get maximum allocatable heap block
     * @return Size of largest allocatable block in bytes
     * 
     * This is important for determining if large allocations will succeed.
     * Even with plenty of free memory, fragmentation can prevent large allocations.
     * ~Olson
     */
    size_t getMaxAllocHeap() {
        update();
        return maxAllocHeap;
    }
    
    /**
     * @brief Get heap fragmentation percentage
     * @return Fragmentation percentage (0-100%)
     * 
     * Higher values indicate more fragmented memory, which can lead to allocation failures.
     * ~Olson
     */
    uint8_t getFragmentation() {
        update();
        return heapFragmentation;
    }
    
    /**
     * @brief Get total heap size
     * @return Total heap size in bytes
     * 
     * This is the total memory available to the application.
     * ~Olson
     */
    size_t getTotalHeapSize() {
        return totalHeapSize;
    }
    
    /**
     * @brief Check if memory is critically low
     * @return true if free memory is below threshold
     * 
     * Use this to trigger memory-saving measures or alert the user.
     * ~Olson
     */
    bool isMemoryLow() {
        update();
        return lowMemoryAlert;
    }
    
    /**
     * @brief Check if fragmentation is dangerously high
     * @return true if fragmentation is above threshold
     * 
     * High fragmentation can cause allocation failures even with sufficient free memory.
     * ~Olson
     */
    bool isFragmentationHigh() {
        update();
        return highFragmentationAlert;
    }
    
    /**
     * @brief Get JSON representation of memory status
     * @return JSON string with memory metrics and history
     * 
     * Used by the web interface to display memory status and history charts.
     * Format: {"totalHeap":N,"freeHeap":N,"minFreeHeap":N,"maxAllocHeap":N,
     *          "fragmentation":N,"lowMemory":bool,"highFragmentation":bool,
     *          "freeHeapHistory":[...],"fragHistory":[...]}
     * 
     * Optimized to use a pre-allocated buffer instead of String concatenation
     * to reduce heap fragmentation and memory allocations.
     * ~Olson
     */
    const char* getStatusJson() {
        update();
        
        // Clear buffer
        memset(jsonBuffer, 0, JSON_BUFFER_SIZE);
        
        // Use snprintf to safely write to the buffer without overflow
        int offset = 0;
        
        // Start JSON object
        offset += snprintf(jsonBuffer + offset, JSON_BUFFER_SIZE - offset, 
                          "{\"totalHeap\":%u,\"freeHeap\":%u,\"minFreeHeap\":%u,\"maxAllocHeap\":%u,"
                          "\"fragmentation\":%u,\"lowMemory\":%s,\"highFragmentation\":%s,",
                          totalHeapSize, freeHeap, minFreeHeap, maxAllocHeap,
                          heapFragmentation, 
                          lowMemoryAlert ? "true" : "false",
                          highFragmentationAlert ? "true" : "false");
        
        // Add free heap history array
        offset += snprintf(jsonBuffer + offset, JSON_BUFFER_SIZE - offset, "\"freeHeapHistory\":[");
        for (int i = 0; i < HISTORY_SIZE; i++) {
            int idx = (historyIndex + i) % HISTORY_SIZE;
            offset += snprintf(jsonBuffer + offset, JSON_BUFFER_SIZE - offset, "%u", freeHeapHistory[idx]);
            if (i < HISTORY_SIZE - 1) {
                offset += snprintf(jsonBuffer + offset, JSON_BUFFER_SIZE - offset, ",");
            }
        }
        offset += snprintf(jsonBuffer + offset, JSON_BUFFER_SIZE - offset, "],");
        
        // Add fragmentation history array
        offset += snprintf(jsonBuffer + offset, JSON_BUFFER_SIZE - offset, "\"fragHistory\":[");
        for (int i = 0; i < HISTORY_SIZE; i++) {
            int idx = (historyIndex + i) % HISTORY_SIZE;
            offset += snprintf(jsonBuffer + offset, JSON_BUFFER_SIZE - offset, "%u", fragHistory[idx]);
            if (i < HISTORY_SIZE - 1) {
                offset += snprintf(jsonBuffer + offset, JSON_BUFFER_SIZE - offset, ",");
            }
        }
        offset += snprintf(jsonBuffer + offset, JSON_BUFFER_SIZE - offset, "]}");
        
        return jsonBuffer;
    }
    
    /**
     * @brief Print memory status to Serial console
     * 
     * Outputs formatted memory metrics for debugging and monitoring.
     * Includes warnings for low memory or high fragmentation conditions.
     * ~Olson
     */
    void printStatus() {
        update();
        Serial.println("=== Memory Status ===");
        Serial.print("Total Heap: ");
        Serial.print(totalHeapSize);
        Serial.println(" bytes");
        
        Serial.print("Free Heap: ");
        Serial.print(freeHeap);
        Serial.println(" bytes");
        
        Serial.print("Min Free Heap: ");
        Serial.print(minFreeHeap);
        Serial.println(" bytes");
        
        Serial.print("Max Alloc Heap: ");
        Serial.print(maxAllocHeap);
        Serial.println(" bytes");
        
        Serial.print("Heap Fragmentation: ");
        Serial.print(heapFragmentation);
        Serial.println("%");
        
        if (lowMemoryAlert) {
            Serial.println("WARNING: Low memory!");
        }
        
        if (highFragmentationAlert) {
            Serial.println("WARNING: High fragmentation!");
        }
        
        Serial.println("====================");
    }
};

#endif // MEMORY_MONITOR_H
