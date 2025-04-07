# ShelfClock Architecture

## Technical Decisions

### Hardware Architecture
- **Decision**: ESP32 as main controller
- **Context**: Required WiFi, BLE, multiple GPIO, and real-time processing capabilities
- **Alternatives**: ESP8266 (insufficient processing power), Raspberry Pi (overkill, higher power consumption)
- **Implementation**: ESP32 38-pin ESP-D Devkit
- **Learning**: ESP32's dual-core architecture enables efficient parallel processing for display and sensor tasks
- **Links**: [ESP32 Technical Reference](https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf)

### LED Control System
- **Decision**: WS2812B RGB LEDs with FastLED library
- **Context**: Need for individually addressable RGB LEDs with precise timing
- **Alternatives**: APA102 (higher cost), traditional 7-segment (limited functionality)
- **Implementation**: 273 LEDs arranged in 7-segment configuration with power injection points
- **Learning**: DMA-based LED control reduces CPU overhead
- **Links**: [FastLED Documentation](https://github.com/FastLED/FastLED/wiki/Overview)

### Data Storage
- **Decision**: LittleFS with JSON configuration
- **Context**: Reliable persistent storage for settings and calibration data
- **Alternatives**: SPIFFS (deprecated), SD Card (unnecessary complexity)
- **Implementation**: Structured JSON files with versioning
- **Learning**: Regular checkpointing prevents data corruption
- **Links**: [LittleFS Documentation](https://github.com/littlefs-project/littlefs)

### Web Interface Architecture
- **Decision**: Asynchronous web server with WebSocket support
- **Context**: Real-time control and monitoring requirements
- **Alternatives**: Traditional HTTP polling (higher latency)
- **Implementation**: ESPAsyncWebServer with minimal client-side JavaScript
- **Learning**: WebSocket reduces network overhead for real-time updates
- **Links**: [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)

### Memory Management System
- **Decision**: Implemented a comprehensive memory monitoring and tracking system
- **Context**: ESP32 has limited RAM (320KB) and is prone to memory fragmentation issues
- **Alternatives**: Manual memory debugging, simple free heap reporting
- **Implementation**: MemoryMonitor class with historical tracking, web visualization, and alert thresholds
- **Learning**: Proactive memory monitoring prevents crashes and helps identify memory leaks early
- **Links**: [[Memory Optimization.md]]

### Text Scrolling Implementation
- **Decision**: Custom memory-efficient text scrolling system
- **Context**: Standard string operations cause heap fragmentation on ESP32
- **Alternatives**: Standard String class, char arrays with manual scrolling
- **Implementation**: ScrollText class with static buffer allocation and circular indexing
- **Learning**: Avoiding dynamic allocations significantly improves stability
- **Links**: [[ScrollText.h]], [[MemoryOptimizations.h]]

## System Components

### Core Components
- ESP32 Microcontroller
- RTC Module (DS3231)
- LED Display System (273 WS2812B LEDs)
- Sensor Array (DHT11, INMP441, LDR)
- Power Management System (5V 10A supply with injection points)

### Software Modules
- Clock Control (time management, RTC synchronization)
- LED Display Manager (pattern generation, brightness control)
- Web Interface (user controls, configuration, monitoring)
- Sensor Management (reading, filtering, calibration)
- Audio Processing (FFT analysis, beat detection)
- File System Handler (configuration storage, file operations)
- Memory Monitor (heap tracking, fragmentation analysis)
- OTA Update System (firmware updates, version management)

### Data Flow
1. Sensor Input → Processing → Display Output
2. Web Interface → Settings → Configuration Storage
3. Audio Input → FFT Processing → Visualization
4. Memory Metrics → Historical Storage → Web Dashboard
5. Time Events → Scheduler → Display Mode Changes

### Monitoring Systems
1. **Memory Monitoring**
   - Tracks heap usage, fragmentation, and allocation patterns
   - Provides historical data visualization via web interface
   - Alerts on low memory or high fragmentation conditions
   - Accessible at http://[device-ip]/memory.html

2. **System Health Monitoring**
   - Tracks CPU temperature and utilization
   - Monitors WiFi signal strength and connection stability
   - Logs system restarts and error conditions
   - Provides diagnostic information via debug interface

## Performance Considerations

### Memory Optimization
- Static allocation preferred over dynamic where possible
- String operations minimized in core loops
- Memory fragmentation actively monitored and managed
- Buffer reuse implemented for repetitive operations

### Power Efficiency
- LED brightness dynamically adjusted based on ambient light
- WiFi power management optimized for battery operation
- Sensor polling rates adjusted based on activity
- Sleep modes utilized during periods of inactivity

### Processing Optimization
- FFT calculations optimized for ESP32 architecture
- Display updates batched to minimize refresh overhead
- Task prioritization based on real-time requirements
- Dual-core utilization for parallel processing
