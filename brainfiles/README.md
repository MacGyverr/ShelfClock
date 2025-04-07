# ShelfClock

## Overview
ShelfClock is an advanced ESP32-based digital clock featuring music visualization, environmental monitoring, and smart control capabilities. Built with precision and efficiency in mind, it combines modern IoT features with an elegant LED display system.

## Setup
### Prerequisites
- ESP32 development environment
- Arduino IDE or PlatformIO
- Required libraries (see `platformio.ini`)
- WS2812B LED strips (273 units)
- DHT11 temperature/humidity sensor
- INMP441 microphone module
- LDR (Light Dependent Resistor)

### Installation
1. Clone the repository
2. Install dependencies using PlatformIO or Arduino Library Manager
3. Configure hardware connections according to `diagrams/wiring.pdf`
4. Upload firmware using PlatformIO or Arduino IDE
5. Access web interface at `http://<device-ip>`

## Features
- Digital clock with multiple display modes
- Real-time music visualization with FFT processing
- Temperature and humidity monitoring
- Ambient light adaptation
- Web-based control interface
- OTA update support
- Memory monitoring and diagnostics
- Customizable visual effects

## Project Structure
```
ShelfClock/
├── src/                # Source code
│   ├── ShelfClock.cpp  # Main application file
│   ├── MemoryMonitor.cpp # Memory monitoring implementation
│   ├── ScrollTextImpl.cpp # Text scrolling implementation
│   └── Effects.cpp     # Visual effects implementation
|
├── include/            # Header files
│   ├── ShelfClock.h    # Core functionality declarations
│   ├── MemoryMonitor.h # Memory monitoring system
│   ├── ScrollText.h    # Text scrolling interface
│   ├── MemoryOptimizations.h # Memory optimization utilities
│   └── Effects.h       # Visual effects declarations
|
├── data/               # Web interface assets
│   ├── index.html      # Main control interface
│   ├── settings.html   # Configuration page
│   ├── scheduler.html  # Schedule management
│   ├── debug.html      # Debug interface
│   ├── memory.html     # Memory monitoring dashboard
│   ├── css/            # Stylesheet files
│   ├── js/             # JavaScript files
│   ├── scheduler/      # Schedule configuration files
│   ├── settings/       # System settings files
│   └── songs/          # RTTTL song files
|
├── brainfiles/         # Project documentation
│   ├── README.md       # Project overview
│   ├── ARCHITECTURE.md # Technical decisions
│   ├── TASKS.md        # Progress tracking
│   ├── CHANGELOG.md    # Version history
│   ├── DECISIONS.md    # Key technical decisions
│   ├── API.md          # API documentation
│   ├── Memory Optimization.md # Memory management documentation
│   ├── ProjectOverview.canvas # Visual project mapping
│   ├── ProjectOverviewCode.canvas # Code analysis
│   ├── brainstorm.md   # Project conversations
│   ├── progress/       # Progress tracking
│   │   ├── daily/      # Daily reports
│   │   └── weekly/     # Weekly summaries
│   └── reports/        # Generated reports
│       └── test-coverage/ # Test coverage reports
|
├── 3D Printed Parts/   # 3D model files for enclosure
├── diagrams/           # Circuit and design diagrams
├── platformio.ini      # PlatformIO configuration
└── Readme.md           # Root README file
```

## Technical Stack
- ESP32 microcontroller
- C++ firmware (Arduino framework)
- FastLED library for LED control
- ESPAsyncWebServer for web interface
- ArduinoJSON for configuration management
- LittleFS file system
- Web interface (HTML/CSS/JavaScript)
- Sensors: DHT11, INMP441, LDR
- Memory monitoring system

## Usage
Detailed usage instructions available in the web interface help section.

### Memory Monitoring
Access the memory monitoring dashboard at `http://<device-ip>/memory.html` to:
- View real-time memory usage statistics
- Monitor heap fragmentation
- Track historical memory trends
- Receive alerts for low memory conditions

## Learnings
- Efficient memory management in embedded systems is critical for stability
- Real-time audio processing techniques require careful optimization
- LED matrix optimization strategies significantly impact performance
- IoT device security best practices should be implemented from the start
- Proactive system monitoring prevents runtime failures ~Olson
- Structured documentation improves project maintainability ~Olson
