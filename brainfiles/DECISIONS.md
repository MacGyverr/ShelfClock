# Key Technical Decisions

This document records significant technical decisions made during the ShelfClock project development.

## Memory Management System [2025-04-06]

### Decision
Implement a comprehensive memory monitoring and optimization system for the ESP32.

### Context
The ESP32 has limited RAM (320KB) and is prone to memory fragmentation issues, especially with dynamic text handling and visualization effects.

### Alternatives Considered
1. Manual memory debugging with Serial.print statements
2. Simple free heap reporting without historical tracking
3. Third-party memory management libraries

### Decision Rationale
A custom memory monitoring system provides:
- Real-time visibility into memory usage patterns
- Historical data for identifying memory leaks
- Web-based visualization for easier debugging and follows your website setup
- Alert system for proactive issue detection

### Implementation Details
- MemoryMonitor class with circular buffer for historical data (about 24mins to stay small)
- Memory.html dashboard with graphical representation
- Heap fragmentation calculation and tracking
- Memory-efficient text scrolling implementation

### Consequences
- Slight increase in code complexity
- Small memory overhead for tracking (justified by benefits)
- Significantly improved stability and reliability
- Better development experience with visual debugging

### Learnings
Proactive memory monitoring prevents crashes and helps identify memory leaks early in the development cycle
