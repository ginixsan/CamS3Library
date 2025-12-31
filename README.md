# CamS3Library

Arduino library for **M5Stack Unit CamS3-5MP** (ESP32-S3 with OV5640 sensor).

## Features

- Easy camera initialization with sensible defaults
- Automatic sensor detection (OV5640, OV3660, OV2640)
- **SD card support** with file operations
- **PDM microphone** with audio recording and WAV export
- Built-in LED control
- Sensor settings API (brightness, saturation, contrast, etc.)
- One-liner capture to SD card
- One-liner audio recording to WAV
- Compatible with Arduino IDE and PlatformIO

## Hardware

| Specification | Value |
|--------------|-------|
| MCU | ESP32-S3 |
| Camera Sensor | OV5640 (5MP) |
| Max Resolution | 2592x1944 |
| PSRAM | 8MB |
| SD Card | MicroSD (SPI) |
| Microphone | PDM Digital |
| LED GPIO | 14 |

### Pin Mapping

| Function | GPIO |
|----------|------|
| **Camera** | |
| RESET | 21 |
| XCLK | 11 |
| SIOD (SDA) | 17 |
| SIOC (SCL) | 41 |
| D0-D7 | 6,15,16,7,5,10,4,13 |
| VSYNC | 42 |
| HREF | 18 |
| PCLK | 12 |
| LED | 14 |
| **SD Card** | |
| CS | 9 |
| MOSI | 38 |
| SCK | 39 |
| MISO | 40 |
| **Microphone** | |
| CLK | 47 |
| DATA | 48 |

## Installation

### Arduino IDE

1. Download this repository as ZIP
2. In Arduino IDE: `Sketch` → `Include Library` → `Add .ZIP Library...`
3. Select the downloaded ZIP file

### PlatformIO

Add to your `platformio.ini`:

```ini
lib_deps =
    https://github.com/yourusername/CamS3Library.git
```

## Quick Start

### Camera Only

```cpp
#include <CamS3Library.h>

void setup() {
    Serial.begin(115200);
    
    if (!CamS3.begin()) {
        Serial.println("Camera init failed!");
        return;
    }
    
    Serial.printf("Camera ready: %s\n", CamS3.Camera.getSensorName());
}

void loop() {
    if (CamS3.Camera.get()) {
        Serial.printf("Frame: %d bytes\n", CamS3.Camera.fb->len);
        CamS3.Camera.free();
    }
    delay(1000);
}
```

### Camera + SD Card

```cpp
#include <CamS3Library.h>

void setup() {
    Serial.begin(115200);
    
    // Initialize camera and SD card
    if (!CamS3.begin(true)) {
        Serial.println("Init failed!");
        return;
    }
}

void loop() {
    // One-liner capture to SD card
    CamS3.captureToSD("/photo.jpg");
    delay(5000);
}
```

### Camera + SD + Microphone

```cpp
#include <CamS3Library.h>

void setup() {
    Serial.begin(115200);
    
    // Initialize everything
    if (!CamS3.begin(true, true)) {  // (initSD, initMic)
        Serial.println("Init failed!");
        return;
    }
}

void loop() {
    // Capture image
    CamS3.captureToSD("/photo.jpg");
    
    // Record 5 seconds of audio
    CamS3.recordToSD("/audio.wav", 5000);
    
    delay(10000);
}
```

### Microphone Only

```cpp
#include <CamS3Library.h>

void setup() {
    Serial.begin(115200);
    
    if (!CamS3.Mic.begin(16000, 16)) {  // 16kHz, 16-bit
        Serial.println("Mic init failed!");
        return;
    }
}

void loop() {
    // Check sound level
    uint16_t level = CamS3.Mic.getPeakAmplitude(256);
    Serial.printf("Level: %d\n", level);
    
    // Detect sound
    if (CamS3.Mic.isSoundDetected(1000)) {
        Serial.println("Sound detected!");
    }
    
    delay(100);
}
```

## API Reference

### Initialization

```cpp
// Camera only
CamS3.begin();

// Camera + SD card
CamS3.begin(true);

// Camera + SD + Microphone
CamS3.begin(true, true);

// Individual components
CamS3.Camera.begin(FRAMESIZE_VGA, PIXFORMAT_JPEG, 12, 2);
CamS3.Sd.begin();
CamS3.Mic.begin(16000, 16);  // sampleRate, bits
```

### Frame Capture

```cpp
if (CamS3.Camera.get()) {
    uint8_t* data = CamS3.Camera.fb->buf;
    size_t len    = CamS3.Camera.fb->len;
    size_t width  = CamS3.Camera.fb->width;
    size_t height = CamS3.Camera.fb->height;
    
    CamS3.Camera.free();  // Always free!
}
```

### Capture to SD Card

```cpp
CamS3.captureToSD();              // Auto filename
CamS3.captureToSD("/image.jpg");  // Custom filename
```

### LED Control

```cpp
CamS3.Camera.ledOn();
CamS3.Camera.ledOff();
CamS3.Camera.ledSet(true);
```

### Camera Settings

```cpp
CamS3.Camera.setFrameSize(FRAMESIZE_VGA);
CamS3.Camera.setQuality(10);          // 0-63, lower=better
CamS3.Camera.setVFlip(true);
CamS3.Camera.setHMirror(true);
CamS3.Camera.setBrightness(0);        // -2 to 2
CamS3.Camera.setSaturation(0);        // -2 to 2
CamS3.Camera.setContrast(0);          // -2 to 2
```

### Microphone Operations

```cpp
// Initialize
CamS3.Mic.begin(16000, 16);       // 16kHz, 16-bit

// Read samples
int16_t buffer[1024];
int32_t samplesRead = CamS3.Mic.read(buffer, 1024);

// Record for duration
size_t numSamples;
int16_t* audio = CamS3.Mic.record(5000, &numSamples);  // 5 seconds
// ... use audio ...
CamS3.Mic.freeSamples(audio);

// Record to SD card (WAV format)
CamS3.recordToSD("/recording.wav", 5000);

// Audio levels
uint16_t peak = CamS3.Mic.getPeakAmplitude(256);
uint16_t rms  = CamS3.Mic.getRMSLevel(256);

// Sound detection
if (CamS3.Mic.isSoundDetected(1000)) {  // threshold
    Serial.println("Sound!");
}

// Info
CamS3.Mic.getSampleRate();  // 16000
CamS3.Mic.getSampleBits();  // 16
CamS3.Mic.isInitialized();
CamS3.Mic.end();
```

### SD Card Operations

```cpp
// Card info
CamS3.Sd.getCardTypeName();       // "SD", "SDHC"
CamS3.Sd.getTotalBytes();
CamS3.Sd.getFreeBytes();

// File operations
CamS3.Sd.writeFile("/file.txt", data, len);
CamS3.Sd.readFile("/file.txt", buffer, maxLen);
CamS3.Sd.exists("/file.txt");
CamS3.Sd.remove("/file.txt");
CamS3.Sd.getFileSize("/file.txt");

// Directory operations
CamS3.Sd.mkdir("/mydir");
CamS3.Sd.rmdir("/mydir");
CamS3.Sd.listDir("/", 2);
```

## Frame Sizes

| Constant | Resolution |
|----------|------------|
| `FRAMESIZE_QQVGA` | 160x120 |
| `FRAMESIZE_QVGA` | 320x240 |
| `FRAMESIZE_VGA` | 640x480 |
| `FRAMESIZE_SVGA` | 800x600 |
| `FRAMESIZE_XGA` | 1024x768 |
| `FRAMESIZE_HD` | 1280x720 |
| `FRAMESIZE_SXGA` | 1280x1024 |
| `FRAMESIZE_UXGA` | 1600x1200 |

## Examples

| Example | Description |
|---------|-------------|
| **SimpleCapture** | Basic frame capture |
| **MJPEG_Stream** | WiFi MJPEG streaming server |
| **CaptureToSD** | Periodic capture to SD card |
| **SDCard_Advanced** | File/directory operations |
| **Microphone** | Audio level monitoring |
| **RecordToSD** | Record audio to WAV files |

## License

MIT License
