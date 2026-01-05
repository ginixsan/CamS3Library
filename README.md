# CamS3Library

Arduino library for **M5Stack Unit CamS3-5MP** (ESP32-S3 with OV5640 sensor).

## Features

- Easy camera initialization with sensible defaults
- Automatic sensor detection (OV5640, OV3660, OV2640)
- **Hardware version detection** (New/Old version identification)
- **SD card support** with file operations
- **PDM microphone** with audio recording and WAV export
- Built-in LED control
- **Comprehensive sensor control API:**
  - Image quality (brightness, contrast, saturation, sharpness, denoise)
  - Auto white balance (AWB, AWB gain, WB modes)
  - Auto exposure & gain control (AEC, AEC2, AGC with manual override)
  - Image processing (special effects, lens correction, pixel correction)
  - Advanced register access and low-level control
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
    Serial.printf("Hardware version: %s\n", CamS3.Camera.getHardwareVersionName());
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

#### Basic Settings

```cpp
CamS3.Camera.setFrameSize(FRAMESIZE_VGA);     // Change resolution
CamS3.Camera.setPixelFormat(PIXFORMAT_JPEG);  // RGB565, JPEG, YUV422, etc.
CamS3.Camera.setQuality(10);                  // 0-63, lower=better quality
CamS3.Camera.setVFlip(true);                  // Vertical flip
CamS3.Camera.setHMirror(true);                // Horizontal mirror
CamS3.Camera.resetSensor();                   // Reset to defaults
```

#### Image Quality & Enhancement

```cpp
CamS3.Camera.setBrightness(0);        // -2 to +2
CamS3.Camera.setSaturation(0);        // -2 to +2
CamS3.Camera.setContrast(0);          // -2 to +2
CamS3.Camera.setSharpness(0);         // -2 to +2
CamS3.Camera.setDenoise(0);           // 0-8, higher = more denoising
CamS3.Camera.setGainCeiling(GAINCEILING_2X);  // 2X, 4X, 8X, 16X, 32X, 64X, 128X
CamS3.Camera.setColorbar(false);      // Enable test pattern
```

#### Auto White Balance

```cpp
CamS3.Camera.setWhiteBalance(true);   // Enable/disable AWB
CamS3.Camera.setAWBGain(true);        // Enable/disable AWB gain
CamS3.Camera.setWBMode(0);            // 0=Auto, 1=Sunny, 2=Cloudy, 3=Office, 4=Home
```

#### Auto Exposure & Gain Control

```cpp
CamS3.Camera.setExposureCtrl(true);   // Enable/disable AEC
CamS3.Camera.setAEC2(false);          // Enable/disable AEC2 (DSP)
CamS3.Camera.setAELevel(0);           // -2 to +2 (exposure compensation)
CamS3.Camera.setAECValue(300);        // 0-1200 (manual exposure when AEC off)
CamS3.Camera.setGainCtrl(true);       // Enable/disable AGC
CamS3.Camera.setAGCGain(0);           // 0-30 (manual gain when AGC off)
```

#### Image Processing

```cpp
CamS3.Camera.setSpecialEffect(0);     // 0=None, 1=Negative, 2=Grayscale,
                                      // 3=Red/Green/Blue tint, 4=Sepia,
                                      // 5=BW, 6=Antique
CamS3.Camera.setDCW(true);            // Enable downsize/crop window
CamS3.Camera.setBPC(false);           // Black pixel correction
CamS3.Camera.setWPC(false);           // White pixel correction
CamS3.Camera.setRawGMA(true);         // Raw gamma
CamS3.Camera.setLensCorrection(true); // Lens distortion correction
```

#### Advanced Register Access

```cpp
// Read/write sensor registers directly
int value = CamS3.Camera.getRegister(0x3008, 0xFF);
CamS3.Camera.setRegister(0x3008, 0xFF, 0x42);

// Advanced resolution control with cropping/scaling
CamS3.Camera.setResolutionRaw(startX, startY, endX, endY,
                              offsetX, offsetY, totalX, totalY,
                              outputX, outputY, scale, binning);

// PLL and clock configuration (advanced users)
CamS3.Camera.setPLL(bypass, mul, sys, root, pre, seld5, pclken, pclk);
CamS3.Camera.setXCLK(timer, xclk);
```

**Complete Example - Fine-tuning Image Quality:**

```cpp
void setup() {
    Serial.begin(115200);

    if (!CamS3.begin()) {
        Serial.println("Camera init failed!");
        return;
    }

    // Set resolution and format
    CamS3.Camera.setFrameSize(FRAMESIZE_HD);
    CamS3.Camera.setQuality(10);

    // Image quality adjustments
    CamS3.Camera.setSharpness(1);           // Slightly sharper
    CamS3.Camera.setDenoise(3);             // Moderate noise reduction
    CamS3.Camera.setBrightness(0);          // Normal brightness
    CamS3.Camera.setContrast(0);            // Normal contrast
    CamS3.Camera.setSaturation(0);          // Normal saturation

    // Exposure and gain control
    CamS3.Camera.setExposureCtrl(true);     // Auto exposure ON
    CamS3.Camera.setAELevel(-1);            // Slightly underexpose
    CamS3.Camera.setGainCtrl(true);         // Auto gain ON
    CamS3.Camera.setGainCeiling(GAINCEILING_16X);  // Limit max gain

    // White balance
    CamS3.Camera.setWhiteBalance(true);     // Auto WB ON
    CamS3.Camera.setWBMode(0);              // Auto mode

    // Image processing
    CamS3.Camera.setLensCorrection(true);   // Fix lens distortion
    CamS3.Camera.setBPC(true);              // Remove black pixels
    CamS3.Camera.setWPC(true);              // Remove white pixels

    Serial.println("Camera configured!");
}

void loop() {
    if (CamS3.Camera.get()) {
        Serial.printf("Captured: %dx%d, %d bytes\n",
                     CamS3.Camera.fb->width,
                     CamS3.Camera.fb->height,
                     CamS3.Camera.fb->len);
        CamS3.Camera.free();
    }
    delay(1000);
}
```

#### Sensor Settings Quick Reference

| Function | Parameter Range | Default | Description |
|----------|----------------|---------|-------------|
| **Image Quality** ||||
| `setBrightness()` | -2 to +2 | 0 | Brightness level |
| `setContrast()` | -2 to +2 | 0 | Contrast level |
| `setSaturation()` | -2 to +2 | 0 | Color saturation |
| `setSharpness()` | -2 to +2 | 0 | Edge sharpness |
| `setDenoise()` | 0 to 8 | 0 | Noise reduction (higher = more) |
| `setQuality()` | 0 to 63 | 12 | JPEG quality (lower = better) |
| **Exposure & Gain** ||||
| `setExposureCtrl()` | true/false | true | Auto exposure control |
| `setAELevel()` | -2 to +2 | 0 | Exposure compensation |
| `setAECValue()` | 0 to 1200 | - | Manual exposure value |
| `setGainCtrl()` | true/false | true | Auto gain control |
| `setAGCGain()` | 0 to 30 | - | Manual gain value |
| `setGainCeiling()` | 2X to 128X | 2X | Maximum auto gain |
| **White Balance** ||||
| `setWhiteBalance()` | true/false | true | Auto white balance |
| `setAWBGain()` | true/false | true | AWB gain control |
| `setWBMode()` | 0 to 4 | 0 | WB mode (0=Auto, 1=Sunny, 2=Cloudy, 3=Office, 4=Home) |
| **Image Processing** ||||
| `setSpecialEffect()` | 0 to 6 | 0 | Effect (0=None, 1=Neg, 2=Gray, 3=Tint, 4=Sepia, 5=BW, 6=Antique) |
| `setLensCorrection()` | true/false | false | Fix lens distortion |
| `setBPC()` | true/false | false | Black pixel correction |
| `setWPC()` | true/false | false | White pixel correction |
| `setRawGMA()` | true/false | true | Raw gamma |
| `setDCW()` | true/false | true | Downsize enable |

**Note:** Not all settings are supported by all camera sensors. The library checks for function availability before calling.

### Hardware Version Detection

Check the hardware version of your CamS3 unit:

```cpp
// Get version as enum
cams3_hw_version_t version = CamS3.Camera.getHardwareVersion();
switch (version) {
    case CAMS3_HW_VERSION_NEW:
        Serial.println("New Version (0x01)");
        break;
    case CAMS3_HW_VERSION_OLD:
        Serial.println("Old Version (0xFF)");
        break;
    case CAMS3_HW_VERSION_UNKNOWN:
        Serial.println("Unknown Version");
        break;
}

// Get version as string
const char* versionName = CamS3.Camera.getHardwareVersionName();
Serial.println(versionName);  // "New Version", "Old Version", or "Unknown"
```

**Hardware Version Values:**
- `CAMS3_HW_VERSION_NEW` (0x01) - New hardware version
- `CAMS3_HW_VERSION_OLD` (0xFF) - Old hardware version
- `CAMS3_HW_VERSION_UNKNOWN` (0x00) - Unknown or error reading version

**Note:** This function reads the hardware version register via I2C. I2C is automatically initialized on first use.

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
