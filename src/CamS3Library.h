/**
 * @file CamS3Library.h
 * @brief Library for M5Stack Unit CamS3-5MP (ESP32-S3 with OV5640 sensor)
 * @version 1.2.0
 * @date 2024
 *
 * @copyright MIT License
 */

#ifndef _CAMS3_LIBRARY_H_
#define _CAMS3_LIBRARY_H_

#include <Arduino.h>
#include <esp_camera.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <driver/i2s_pdm.h>

// ============================================
// M5Stack Unit CamS3-5MP GPIO Pin Definitions
// ============================================

// Camera pins
#define CAMS3_PWDN_GPIO_NUM   -1
#define CAMS3_RESET_GPIO_NUM  21
#define CAMS3_XCLK_GPIO_NUM   11
#define CAMS3_SIOD_GPIO_NUM   17
#define CAMS3_SIOC_GPIO_NUM   41

#define CAMS3_Y9_GPIO_NUM     13
#define CAMS3_Y8_GPIO_NUM     4
#define CAMS3_Y7_GPIO_NUM     10
#define CAMS3_Y6_GPIO_NUM     5
#define CAMS3_Y5_GPIO_NUM     7
#define CAMS3_Y4_GPIO_NUM     16
#define CAMS3_Y3_GPIO_NUM     15
#define CAMS3_Y2_GPIO_NUM     6

#define CAMS3_VSYNC_GPIO_NUM  42
#define CAMS3_HREF_GPIO_NUM   18
#define CAMS3_PCLK_GPIO_NUM   12

#define CAMS3_LED_GPIO        14

// SD Card pins (SPI)
#define CAMS3_SD_CS_PIN       9
#define CAMS3_SD_MOSI_PIN     38
#define CAMS3_SD_SCK_PIN      39
#define CAMS3_SD_MISO_PIN     40

// Default SD SPI frequency (40 MHz)
#define CAMS3_SD_SPI_FREQ     40000000

// PDM Microphone pins
#define CAMS3_MIC_CLK_PIN     47
#define CAMS3_MIC_DATA_PIN    48

// Default microphone settings
#define CAMS3_MIC_SAMPLE_RATE     16000
#define CAMS3_MIC_SAMPLE_BITS     16
#define CAMS3_MIC_CHANNEL_NUM     1

// ============================================
// Supported camera sensors
// ============================================
typedef enum {
    CAMS3_SENSOR_UNKNOWN = 0,
    CAMS3_SENSOR_OV5640,
    CAMS3_SENSOR_OV3660,
    CAMS3_SENSOR_OV2640
} cams3_sensor_type_t;

// ============================================
// Camera Class
// ============================================
class CamS3_Camera {
   private:
    cams3_sensor_type_t _sensorType = CAMS3_SENSOR_UNKNOWN;
    bool _initialized               = false;

    void _applySensorDefaults();

   public:
    camera_fb_t* fb       = nullptr;
    sensor_t* sensor      = nullptr;
    camera_config_t* config = nullptr;

    /**
     * @brief Initialize the camera
     * @param frameSize Frame size (default: FRAMESIZE_VGA)
     * @param pixelFormat Pixel format (default: PIXFORMAT_JPEG)
     * @param jpegQuality JPEG quality 0-63, lower is better (default: 12)
     * @param fbCount Frame buffer count (default: 2)
     * @return true if successful
     */
    bool begin(framesize_t frameSize   = FRAMESIZE_VGA,
               pixformat_t pixelFormat = PIXFORMAT_JPEG,
               uint8_t jpegQuality     = 12,
               uint8_t fbCount         = 2);

    /**
     * @brief Get a frame from the camera
     * @return true if successful
     */
    bool get();

    /**
     * @brief Free the current frame buffer
     * @return true if successful
     */
    bool free();

    /**
     * @brief Deinitialize the camera
     * @return true if successful
     */
    bool deinit();

    /**
     * @brief Get the detected sensor type
     * @return Sensor type enum
     */
    cams3_sensor_type_t getSensorType();

    /**
     * @brief Get the sensor name as string
     * @return Sensor name
     */
    const char* getSensorName();

    /**
     * @brief Check if camera is initialized
     * @return true if initialized
     */
    bool isInitialized() {
        return _initialized;
    }

    // LED control
    void ledOn();
    void ledOff();
    void ledSet(bool state);

    // Sensor settings
    bool setFrameSize(framesize_t size);
    bool setQuality(uint8_t quality);
    bool setVFlip(bool flip);
    bool setHMirror(bool mirror);
    bool setBrightness(int level);
    bool setSaturation(int level);
    bool setContrast(int level);
    bool setSpecialEffect(int effect);
    bool setWhiteBalance(bool enable);
    bool setExposureCtrl(bool enable);
    bool setGainCtrl(bool enable);
};

// ============================================
// PDM Microphone Class
// ============================================
class CamS3_Mic {
   private:
    bool _initialized    = false;
    uint32_t _sampleRate = CAMS3_MIC_SAMPLE_RATE;
    uint8_t _sampleBits  = CAMS3_MIC_SAMPLE_BITS;
    int16_t* _buffer     = nullptr;
    size_t _bufferSize   = 0;
    i2s_chan_handle_t _i2sHandle = nullptr;

   public:
    /**
     * @brief Initialize the PDM microphone
     * @param sampleRate Sample rate in Hz (default: 16000)
     * @param sampleBits Bits per sample: 16 or 32 (default: 16)
     * @return true if successful
     */
    bool begin(uint32_t sampleRate = CAMS3_MIC_SAMPLE_RATE, uint8_t sampleBits = CAMS3_MIC_SAMPLE_BITS);

    /**
     * @brief Stop and deinitialize the microphone
     */
    void end();

    /**
     * @brief Check if microphone is initialized
     * @return true if initialized
     */
    bool isInitialized() {
        return _initialized;
    }

    /**
     * @brief Read audio samples from the microphone
     * @param buffer Output buffer for samples (int16_t array)
     * @param samples Number of samples to read
     * @param timeoutMs Timeout in milliseconds (default: 1000)
     * @return Number of samples actually read, or -1 on error
     */
    int32_t read(int16_t* buffer, size_t samples, uint32_t timeoutMs = 1000);

    /**
     * @brief Read raw audio bytes from the microphone
     * @param buffer Output buffer
     * @param len Buffer length in bytes
     * @param timeoutMs Timeout in milliseconds
     * @return Number of bytes read, or -1 on error
     */
    int32_t readBytes(uint8_t* buffer, size_t len, uint32_t timeoutMs = 1000);

    /**
     * @brief Get the current sample rate
     * @return Sample rate in Hz
     */
    uint32_t getSampleRate() {
        return _sampleRate;
    }

    /**
     * @brief Get the bits per sample
     * @return Bits per sample (16 or 32)
     */
    uint8_t getSampleBits() {
        return _sampleBits;
    }

    /**
     * @brief Record audio to a buffer for a specified duration
     * @param durationMs Duration in milliseconds
     * @param outSamples Pointer to store the number of samples recorded
     * @return Pointer to allocated buffer (caller must free with freeSamples()), or nullptr on error
     */
    int16_t* record(uint32_t durationMs, size_t* outSamples);

    /**
     * @brief Free a buffer allocated by record()
     * @param buffer Buffer to free
     */
    void freeSamples(int16_t* buffer);

    /**
     * @brief Get the peak amplitude from recent samples
     * @param samples Number of samples to analyze
     * @return Peak amplitude (0-32767 for 16-bit)
     */
    uint16_t getPeakAmplitude(size_t samples = 256);

    /**
     * @brief Get the RMS (root mean square) level from recent samples
     * @param samples Number of samples to analyze
     * @return RMS level (0-32767 for 16-bit)
     */
    uint16_t getRMSLevel(size_t samples = 256);

    /**
     * @brief Check if sound is detected above a threshold
     * @param threshold Amplitude threshold (default: 500)
     * @param samples Number of samples to check
     * @return true if sound detected
     */
    bool isSoundDetected(uint16_t threshold = 500, size_t samples = 256);
};

// ============================================
// SD Card Class
// ============================================
class CamS3_SD {
   private:
    bool _initialized  = false;
    uint32_t _spiFreq  = CAMS3_SD_SPI_FREQ;
    SPIClass* _spi     = nullptr;
    uint32_t _fileCounter = 0;

   public:
    /**
     * @brief Initialize the SD card
     * @param spiFreq SPI frequency in Hz (default: 40MHz)
     * @return true if successful
     */
    bool begin(uint32_t spiFreq = CAMS3_SD_SPI_FREQ);

    /**
     * @brief End SD card operations
     */
    void end();

    /**
     * @brief Check if SD card is initialized
     * @return true if initialized
     */
    bool isInitialized() {
        return _initialized;
    }

    /**
     * @brief Get SD card type
     * @return Card type (CARD_NONE, CARD_MMC, CARD_SD, CARD_SDHC)
     */
    sdcard_type_t getCardType();

    /**
     * @brief Get card type as string
     * @return Card type name
     */
    const char* getCardTypeName();

    /**
     * @brief Get total card size in bytes
     * @return Total size in bytes
     */
    uint64_t getTotalBytes();

    /**
     * @brief Get used space in bytes
     * @return Used space in bytes
     */
    uint64_t getUsedBytes();

    /**
     * @brief Get free space in bytes
     * @return Free space in bytes
     */
    uint64_t getFreeBytes();

    /**
     * @brief Save a buffer to a file
     * @param path File path (e.g., "/image.jpg")
     * @param data Data buffer
     * @param len Data length
     * @return true if successful
     */
    bool writeFile(const char* path, const uint8_t* data, size_t len);

    /**
     * @brief Append data to a file
     * @param path File path
     * @param data Data buffer
     * @param len Data length
     * @return true if successful
     */
    bool appendFile(const char* path, const uint8_t* data, size_t len);

    /**
     * @brief Read a file into a buffer
     * @param path File path
     * @param buffer Output buffer (must be pre-allocated)
     * @param maxLen Maximum bytes to read
     * @return Number of bytes read, or -1 on error
     */
    int32_t readFile(const char* path, uint8_t* buffer, size_t maxLen);

    /**
     * @brief Check if a file exists
     * @param path File path
     * @return true if exists
     */
    bool exists(const char* path);

    /**
     * @brief Delete a file
     * @param path File path
     * @return true if successful
     */
    bool remove(const char* path);

    /**
     * @brief Rename a file
     * @param pathFrom Original path
     * @param pathTo New path
     * @return true if successful
     */
    bool rename(const char* pathFrom, const char* pathTo);

    /**
     * @brief Create a directory
     * @param path Directory path
     * @return true if successful
     */
    bool mkdir(const char* path);

    /**
     * @brief Remove a directory
     * @param path Directory path
     * @return true if successful
     */
    bool rmdir(const char* path);

    /**
     * @brief Get file size
     * @param path File path
     * @return File size in bytes, or -1 if not found
     */
    int64_t getFileSize(const char* path);

    /**
     * @brief List directory contents to Serial
     * @param dirname Directory path
     * @param levels How many levels deep to list
     */
    void listDir(const char* dirname, uint8_t levels = 0);

    /**
     * @brief Save camera frame buffer to SD card
     * @param fb Camera frame buffer
     * @param path File path (if nullptr, auto-generates name)
     * @return true if successful
     */
    bool saveFrame(camera_fb_t* fb, const char* path = nullptr);

    /**
     * @brief Generate a unique filename for saving images
     * @param prefix Filename prefix (default: "IMG")
     * @param extension File extension (default: "jpg")
     * @return Generated filename
     */
    String generateFilename(const char* prefix = "IMG", const char* extension = "jpg");

    /**
     * @brief Get the underlying SDFS object for advanced operations
     * @return Reference to SD filesystem
     */
    fs::SDFS& getFS() {
        return SD;
    }
};

// ============================================
// Main Library Class
// ============================================
class CamS3Library {
   public:
    CamS3_Camera Camera;
    CamS3_SD Sd;
    CamS3_Mic Mic;

    /**
     * @brief Initialize camera only
     * @return true if successful
     */
    bool begin();

    /**
     * @brief Initialize camera and optionally SD card
     * @param initSD Whether to initialize SD card
     * @return true if successful
     */
    bool begin(bool initSD);

    /**
     * @brief Initialize camera, SD card, and/or microphone
     * @param initSD Whether to initialize SD card
     * @param initMic Whether to initialize microphone
     * @return true if successful
     */
    bool begin(bool initSD, bool initMic);

    /**
     * @brief Capture and save image to SD card
     * @param path File path (if nullptr, auto-generates name)
     * @return true if successful
     */
    bool captureToSD(const char* path = nullptr);

    /**
     * @brief Record audio and save to SD card as WAV file
     * @param path File path (if nullptr, auto-generates name)
     * @param durationMs Duration in milliseconds
     * @return true if successful
     */
    bool recordToSD(const char* path, uint32_t durationMs);
};

extern CamS3Library CamS3;

#endif  // _CAMS3_LIBRARY_H_
