# ShelfClock API Documentation

## Overview
The ShelfClock provides a RESTful API for remote control and monitoring. This document details all available endpoints, parameters, and response formats.

## Base URL
All API endpoints are relative to: `http://<device-ip>/api/`

## Authentication
Currently, the API does not require authentication. Future versions could implement uses of token-based authentication.

## Endpoints

### GET /api/status
Returns the current status of the device.

**Response:**
```json
{
  "time": "12:34:56",
  "date": "April 6, 2025",
  "temperature": 22.5,
  "humidity": 45.2,
  "brightness": 75,
  "mode": "clock",
  "memory": {
    "freeHeap": 123456,
    "fragmentation": 12,
    "minFreeHeap": 98765
  }
}
```

### POST /api/mode
Sets the display mode.

**Parameters:**
- `mode` (string): One of "clock", "date", "temperature", "humidity", "visualizer", "animation"

**Response:**
```json
{
  "success": true,
  "mode": "clock"
}
```

### POST /api/brightness
Sets the display brightness.

**Parameters:**
- `level` (integer): Brightness level from 0-255
- `auto` (boolean, optional): Enable automatic brightness adjustment

**Response:**
```json
{
  "success": true,
  "brightness": 128,
  "auto": true
}
```

### GET /api/memory
Returns detailed memory statistics.

**Response:**
```json
{
  "freeHeap": 123456,
  "totalHeap": 320000,
  "minFreeHeap": 98765,
  "maxAllocHeap": 65432,
  "fragmentation": 12,
  "history": [
    {"time": "12:30:00", "free": 123000},
    {"time": "12:31:00", "free": 122500},
    {"time": "12:32:00", "free": 123400}
  ]
}
```

### POST /api/restart
Restarts the device.

**Response:**
```json
{
  "success": true,
  "message": "Device restarting..."
}
```

### POST /api/update
Triggers OTA update if available.

**Parameters:**
- `url` (string, optional): URL to firmware binary

**Response:**
```json
{
  "success": true,
  "message": "Update started",
  "version": "2.0.1"
}
```

### GET /api/effects
Returns available visual effects.

**Response:**
```json
{
  "effects": [
    "rainbow",
    "twinkle",
    "matrix",
    "fire",
    "rain",
    "snake",
    "cylon"
  ]
}
```

### POST /api/effects
Activates a visual effect.

**Parameters:**
- `effect` (string): Name of the effect
- `speed` (integer, optional): Effect speed (1-10)
- `color` (string, optional): Hex color code

**Response:**
```json
{
  "success": true,
  "effect": "rainbow",
  "speed": 5
}
```

## Error Handling
All API endpoints return standard HTTP status codes. In case of an error, the response will include an error message:

```json
{
  "success": false,
  "error": "Invalid mode specified"
}
```

## WebSocket API
Real-time updates are available via WebSocket connection at `ws://<device-ip>/ws`

### Messages
The WebSocket sends JSON messages with the following format:

```json
{
  "type": "update",
  "data": {
    "time": "12:34:56",
    "temperature": 22.5,
    "memory": {
      "freeHeap": 123456
    }
  }
}
```

## Implementation Notes
The API is implemented using the ESPAsyncWebServer library with minimal overhead to preserve memory resources
