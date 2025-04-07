# Memory Optimization

## Memory Monitoring System

The ShelfClock project now includes a comprehensive memory monitoring system that helps track, visualize, and manage the ESP32's limited memory resources. This system was implemented on April 6, 2025, to address potential memory-related stability issues.

### Implementation Details

#### MemoryMonitor Class
- **Location**: `include/MemoryMonitor.h`
- **Purpose**: Tracks heap usage, fragmentation, and memory allocation patterns
- **Features**:
  - Real-time memory metrics collection
  - Historical data tracking using circular buffer
  - Configurable alert thresholds
  - JSON output for web interface integration

#### Memory Metrics Tracked
- Total heap size
- Current free heap
- Minimum free heap since boot
- Maximum allocatable heap block
- Heap fragmentation percentage

#### Web Interface
- **Location**: `data/memory.html`
- **Access**: http://[device-ip]/memory.html
- **Features**:
  - Real-time memory status display
  - Interactive chart showing memory history
  - Visual alerts for low memory or high fragmentation
  - Memory usage tips and recommendations

### Integration Points
The memory monitor is integrated into the main application at several key points:

1. **Initialization**: A global `MemoryMonitor` instance is created at startup
2. **Regular Updates**: Memory metrics are updated in the main loop
3. **Periodic Logging**: Memory status is logged to Serial every 5 minutes
4. **Web API**: A `/memory` endpoint provides JSON data for the web interface

### Best Practices
- Monitor memory usage regularly during development
- Watch for increasing fragmentation, which indicates potential memory leaks
- Consider a device restart if fragmentation exceeds 70%
- Aim to keep at least 30KB of free heap available at all times

### Future Improvements
- Implement automatic defragmentation routines
- Add PSRAM support for memory-intensive operations
- Create a memory allocation tracking system to identify leak sources
- Implement memory usage alerts via email or push notifications

## Learning Points
- ESP32's memory is limited (approximately 320KB) and must be managed carefully
- Heap fragmentation is often more problematic than total memory usage
- Regular monitoring helps identify memory issues before they cause crashes
- Circular buffers provide efficient historical data storage with minimal memory impact
- Web-based visualization makes complex system metrics more accessible ~Olson
