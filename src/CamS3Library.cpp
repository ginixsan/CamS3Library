/**
 * @file CamS3Library.cpp
 * @brief Library for M5Stack Unit CamS3-5MP (ESP32-S3 with OV5640 sensor)
 * @version 1.2.0
 * @date 2024
 *
 * @copyright MIT License
 */

#include "CamS3Library.h"
#include <math.h>

// Global instance
CamS3Library CamS3;

// ============================================
// Camera Configuration
// ============================================
static camera_config_t camera_config = {
    .pin_pwdn     = CAMS3_PWDN_GPIO_NUM,
    .pin_reset    = CAMS3_RESET_GPIO_NUM,
    .pin_xclk     = CAMS3_XCLK_GPIO_NUM,
    .pin_sscb_sda = CAMS3_SIOD_GPIO_NUM,
    .pin_sscb_scl = CAMS3_SIOC_GPIO_NUM,

    .pin_d7 = CAMS3_Y9_GPIO_NUM,
    .pin_d6 = CAMS3_Y8_GPIO_NUM,
    .pin_d5 = CAMS3_Y7_GPIO_NUM,
    .pin_d4 = CAMS3_Y6_GPIO_NUM,
    .pin_d3 = CAMS3_Y5_GPIO_NUM,
    .pin_d2 = CAMS3_Y4_GPIO_NUM,
    .pin_d1 = CAMS3_Y3_GPIO_NUM,
    .pin_d0 = CAMS3_Y2_GPIO_NUM,

    .pin_vsync = CAMS3_VSYNC_GPIO_NUM,
    .pin_href  = CAMS3_HREF_GPIO_NUM,
    .pin_pclk  = CAMS3_PCLK_GPIO_NUM,

    .xclk_freq_hz = 20000000,
    .ledc_timer   = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format  = PIXFORMAT_JPEG,
    .frame_size    = FRAMESIZE_VGA,
    .jpeg_quality  = 12,
    .fb_count      = 2,
    .fb_location   = CAMERA_FB_IN_PSRAM,
    .grab_mode     = CAMERA_GRAB_LATEST,
    .sccb_i2c_port = 0,
};

// ============================================
// CamS3_Camera Implementation
// ============================================

bool CamS3_Camera::begin(framesize_t frameSize, pixformat_t pixelFormat, uint8_t jpegQuality, uint8_t fbCount) {
    if (_initialized) {
        return true;
    }

    // Configure LED pin
    pinMode(CAMS3_LED_GPIO, OUTPUT);
    ledOff();

    // Update config with parameters
    camera_config.frame_size   = frameSize;
    camera_config.pixel_format = pixelFormat;
    camera_config.jpeg_quality = jpegQuality;
    camera_config.fb_count     = fbCount;

    // Adjust fb_count for non-JPEG formats
    if (pixelFormat != PIXFORMAT_JPEG) {
        camera_config.fb_count    = 1;
        camera_config.fb_location = CAMERA_FB_IN_PSRAM;
    }

    config = &camera_config;

    // Initialize camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        Serial.printf("[CamS3] Camera init failed with error 0x%x\n", err);
        return false;
    }

    // Get sensor handle
    sensor = esp_camera_sensor_get();
    if (!sensor) {
        Serial.println("[CamS3] Failed to get camera sensor");
        return false;
    }

    // Detect sensor type
    switch (sensor->id.PID) {
        case OV5640_PID:
            _sensorType = CAMS3_SENSOR_OV5640;
            break;
        case OV3660_PID:
            _sensorType = CAMS3_SENSOR_OV3660;
            break;
        case OV2640_PID:
            _sensorType = CAMS3_SENSOR_OV2640;
            break;
        default:
            _sensorType = CAMS3_SENSOR_UNKNOWN;
            break;
    }

    Serial.printf("[CamS3] Sensor detected: %s (PID: 0x%X)\n", getSensorName(), sensor->id.PID);

    // Apply sensor-specific defaults
    _applySensorDefaults();

    _initialized = true;
    return true;
}

void CamS3_Camera::_applySensorDefaults() {
    if (!sensor) return;

    switch (_sensorType) {
        case CAMS3_SENSOR_OV5640:
            // OV5640 typically needs vflip
            sensor->set_vflip(sensor, 1);
            break;

        case CAMS3_SENSOR_OV3660:
            // OV3660 needs vflip and color adjustments
            sensor->set_vflip(sensor, 1);
            sensor->set_brightness(sensor, 1);
            sensor->set_saturation(sensor, -2);
            break;

        case CAMS3_SENSOR_OV2640:
            // OV2640 usually works with defaults
            break;

        default:
            break;
    }
}

bool CamS3_Camera::deinit() {
    if (!_initialized) {
        return true;
    }

    esp_err_t err = esp_camera_deinit();
    if (err != ESP_OK) {
        return false;
    }

    _initialized = false;
    sensor       = nullptr;
    fb           = nullptr;
    return true;
}

bool CamS3_Camera::get() {
    if (!_initialized) return false;

    fb = esp_camera_fb_get();
    if (!fb) {
        return false;
    }
    return true;
}

bool CamS3_Camera::free() {
    if (fb) {
        esp_camera_fb_return(fb);
        fb = nullptr;
        return true;
    }
    return false;
}

cams3_sensor_type_t CamS3_Camera::getSensorType() {
    return _sensorType;
}

const char* CamS3_Camera::getSensorName() {
    switch (_sensorType) {
        case CAMS3_SENSOR_OV5640:
            return "OV5640";
        case CAMS3_SENSOR_OV3660:
            return "OV3660";
        case CAMS3_SENSOR_OV2640:
            return "OV2640";
        default:
            return "Unknown";
    }
}

// ============================================
// LED Control
// ============================================

void CamS3_Camera::ledOn() {
    digitalWrite(CAMS3_LED_GPIO, HIGH);
}

void CamS3_Camera::ledOff() {
    digitalWrite(CAMS3_LED_GPIO, LOW);
}

void CamS3_Camera::ledSet(bool state) {
    digitalWrite(CAMS3_LED_GPIO, state ? HIGH : LOW);
}

// ============================================
// Sensor Settings
// ============================================

bool CamS3_Camera::setFrameSize(framesize_t size) {
    if (!sensor) return false;
    return sensor->set_framesize(sensor, size) == 0;
}

bool CamS3_Camera::setQuality(uint8_t quality) {
    if (!sensor) return false;
    return sensor->set_quality(sensor, quality) == 0;
}

bool CamS3_Camera::setVFlip(bool flip) {
    if (!sensor) return false;
    return sensor->set_vflip(sensor, flip ? 1 : 0) == 0;
}

bool CamS3_Camera::setHMirror(bool mirror) {
    if (!sensor) return false;
    return sensor->set_hmirror(sensor, mirror ? 1 : 0) == 0;
}

bool CamS3_Camera::setBrightness(int level) {
    if (!sensor) return false;
    return sensor->set_brightness(sensor, level) == 0;
}

bool CamS3_Camera::setSaturation(int level) {
    if (!sensor) return false;
    return sensor->set_saturation(sensor, level) == 0;
}

bool CamS3_Camera::setContrast(int level) {
    if (!sensor) return false;
    return sensor->set_contrast(sensor, level) == 0;
}

bool CamS3_Camera::setSpecialEffect(int effect) {
    if (!sensor) return false;
    return sensor->set_special_effect(sensor, effect) == 0;
}

bool CamS3_Camera::setWhiteBalance(bool enable) {
    if (!sensor) return false;
    return sensor->set_whitebal(sensor, enable ? 1 : 0) == 0;
}

bool CamS3_Camera::setExposureCtrl(bool enable) {
    if (!sensor) return false;
    return sensor->set_exposure_ctrl(sensor, enable ? 1 : 0) == 0;
}

bool CamS3_Camera::setGainCtrl(bool enable) {
    if (!sensor) return false;
    return sensor->set_gain_ctrl(sensor, enable ? 1 : 0) == 0;
}

// ============================================
// CamS3Library Implementation
// ============================================

bool CamS3Library::begin() {
    return Camera.begin();
}

bool CamS3Library::begin(bool initSD) {
    bool camOk = Camera.begin();
    if (!camOk) return false;

    if (initSD) {
        return Sd.begin();
    }
    return true;
}

bool CamS3Library::begin(bool initSD, bool initMic) {
    bool camOk = Camera.begin();
    if (!camOk) return false;

    bool sdOk = true;
    bool micOk = true;

    if (initSD) {
        sdOk = Sd.begin();
    }

    if (initMic) {
        micOk = Mic.begin();
    }

    return sdOk && micOk;
}

bool CamS3Library::captureToSD(const char* path) {
    if (!Camera.isInitialized()) {
        Serial.println("[CamS3] Camera not initialized");
        return false;
    }
    if (!Sd.isInitialized()) {
        Serial.println("[CamS3] SD card not initialized");
        return false;
    }

    if (!Camera.get()) {
        Serial.println("[CamS3] Failed to capture frame");
        return false;
    }

    bool result = Sd.saveFrame(Camera.fb, path);
    Camera.free();

    return result;
}

bool CamS3Library::recordToSD(const char* path, uint32_t durationMs) {
    if (!Mic.isInitialized()) {
        Serial.println("[CamS3] Microphone not initialized");
        return false;
    }
    if (!Sd.isInitialized()) {
        Serial.println("[CamS3] SD card not initialized");
        return false;
    }

    // Record audio
    size_t numSamples = 0;
    int16_t* samples = Mic.record(durationMs, &numSamples);
    if (!samples || numSamples == 0) {
        Serial.println("[CamS3] Recording failed");
        return false;
    }

    // Generate filename if not provided
    String filename;
    if (path == nullptr) {
        filename = "/REC_" + String(millis()) + ".wav";
    } else {
        filename = String(path);
    }

    // Create WAV file
    File file = SD.open(filename.c_str(), FILE_WRITE);
    if (!file) {
        Serial.println("[CamS3] Failed to create WAV file");
        Mic.freeSamples(samples);
        return false;
    }

    // WAV header
    uint32_t sampleRate = Mic.getSampleRate();
    uint16_t bitsPerSample = Mic.getSampleBits();
    uint16_t numChannels = 1;
    uint32_t dataSize = numSamples * (bitsPerSample / 8);
    uint32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8);
    uint16_t blockAlign = numChannels * (bitsPerSample / 8);

    // RIFF header
    file.write((const uint8_t*)"RIFF", 4);
    uint32_t chunkSize = 36 + dataSize;
    file.write((const uint8_t*)&chunkSize, 4);
    file.write((const uint8_t*)"WAVE", 4);

    // fmt subchunk
    file.write((const uint8_t*)"fmt ", 4);
    uint32_t subchunk1Size = 16;
    file.write((const uint8_t*)&subchunk1Size, 4);
    uint16_t audioFormat = 1;  // PCM
    file.write((const uint8_t*)&audioFormat, 2);
    file.write((const uint8_t*)&numChannels, 2);
    file.write((const uint8_t*)&sampleRate, 4);
    file.write((const uint8_t*)&byteRate, 4);
    file.write((const uint8_t*)&blockAlign, 2);
    file.write((const uint8_t*)&bitsPerSample, 2);

    // data subchunk
    file.write((const uint8_t*)"data", 4);
    file.write((const uint8_t*)&dataSize, 4);
    file.write((const uint8_t*)samples, dataSize);

    file.close();
    Mic.freeSamples(samples);

    Serial.printf("[CamS3] Saved WAV: %s (%d samples, %d bytes)\n", filename.c_str(), numSamples, dataSize + 44);
    return true;
}

// ============================================
// CamS3_Mic Implementation
// ============================================

bool CamS3_Mic::begin(uint32_t sampleRate, uint8_t sampleBits) {
    if (_initialized) {
        return true;
    }

    _sampleRate = sampleRate;
    _sampleBits = sampleBits;

    // Configure I2S channel
    i2s_chan_config_t chanCfg = {
        .id = I2S_NUM_0,
        .role = I2S_ROLE_MASTER,
        .dma_desc_num = 6,
        .dma_frame_num = 240,
        .auto_clear_after_cb = false,
        .auto_clear_before_cb = false,
        .intr_priority = 0,
    };

    esp_err_t err = i2s_new_channel(&chanCfg, nullptr, &_i2sHandle);
    if (err != ESP_OK) {
        Serial.printf("[CamS3 Mic] Failed to create I2S channel: 0x%x\n", err);
        return false;
    }

    // Configure PDM RX
    i2s_pdm_rx_config_t pdmRxCfg = {
        .clk_cfg = {
            .sample_rate_hz = _sampleRate,
            .clk_src = I2S_CLK_SRC_DEFAULT,
            .mclk_multiple = I2S_MCLK_MULTIPLE_256,
            .dn_sample_mode = I2S_PDM_DSR_8S,
        },
        .slot_cfg = {
            .data_bit_width = (_sampleBits == 32) ? I2S_DATA_BIT_WIDTH_32BIT : I2S_DATA_BIT_WIDTH_16BIT,
            .slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,
            .slot_mode = I2S_SLOT_MODE_MONO,
            .slot_mask = I2S_PDM_SLOT_LEFT,
        },
        .gpio_cfg = {
            .clk = (gpio_num_t)CAMS3_MIC_CLK_PIN,
            .din = (gpio_num_t)CAMS3_MIC_DATA_PIN,
            .invert_flags = {
                .clk_inv = false,
            },
        },
    };

    err = i2s_channel_init_pdm_rx_mode(_i2sHandle, &pdmRxCfg);
    if (err != ESP_OK) {
        Serial.printf("[CamS3 Mic] Failed to init PDM RX: 0x%x\n", err);
        i2s_del_channel(_i2sHandle);
        _i2sHandle = nullptr;
        return false;
    }

    err = i2s_channel_enable(_i2sHandle);
    if (err != ESP_OK) {
        Serial.printf("[CamS3 Mic] Failed to enable channel: 0x%x\n", err);
        i2s_del_channel(_i2sHandle);
        _i2sHandle = nullptr;
        return false;
    }

    _initialized = true;
    Serial.printf("[CamS3 Mic] Initialized: %d Hz, %d-bit\n", _sampleRate, _sampleBits);

    return true;
}

void CamS3_Mic::end() {
    if (_initialized && _i2sHandle) {
        i2s_channel_disable(_i2sHandle);
        i2s_del_channel(_i2sHandle);
        _i2sHandle = nullptr;
        _initialized = false;
    }
}

int32_t CamS3_Mic::read(int16_t* buffer, size_t samples, uint32_t timeoutMs) {
    if (!_initialized || !buffer) return -1;

    size_t bytesToRead = samples * sizeof(int16_t);
    size_t bytesRead = 0;

    esp_err_t err = i2s_channel_read(_i2sHandle, buffer, bytesToRead, &bytesRead, timeoutMs);
    if (err != ESP_OK) {
        return -1;
    }

    return bytesRead / sizeof(int16_t);
}

int32_t CamS3_Mic::readBytes(uint8_t* buffer, size_t len, uint32_t timeoutMs) {
    if (!_initialized || !buffer) return -1;

    size_t bytesRead = 0;
    esp_err_t err = i2s_channel_read(_i2sHandle, buffer, len, &bytesRead, timeoutMs);
    if (err != ESP_OK) {
        return -1;
    }

    return bytesRead;
}

int16_t* CamS3_Mic::record(uint32_t durationMs, size_t* outSamples) {
    if (!_initialized) return nullptr;

    size_t totalSamples = (_sampleRate * durationMs) / 1000;
    int16_t* buffer = (int16_t*)malloc(totalSamples * sizeof(int16_t));
    if (!buffer) {
        Serial.println("[CamS3 Mic] Failed to allocate recording buffer");
        return nullptr;
    }

    size_t samplesRecorded = 0;
    size_t bytesRead = 0;
    uint32_t startTime = millis();

    while (samplesRecorded < totalSamples && (millis() - startTime) < (durationMs + 1000)) {
        size_t remaining = totalSamples - samplesRecorded;
        size_t toRead = (remaining > 1024) ? 1024 : remaining;

        esp_err_t err = i2s_channel_read(_i2sHandle, &buffer[samplesRecorded], 
                                         toRead * sizeof(int16_t), &bytesRead, 100);
        if (err == ESP_OK) {
            samplesRecorded += bytesRead / sizeof(int16_t);
        }
    }

    if (outSamples) {
        *outSamples = samplesRecorded;
    }

    Serial.printf("[CamS3 Mic] Recorded %d samples in %lu ms\n", samplesRecorded, millis() - startTime);
    return buffer;
}

void CamS3_Mic::freeSamples(int16_t* buffer) {
    if (buffer) {
        free(buffer);
    }
}

uint16_t CamS3_Mic::getPeakAmplitude(size_t samples) {
    if (!_initialized) return 0;

    int16_t* buffer = (int16_t*)malloc(samples * sizeof(int16_t));
    if (!buffer) return 0;

    int32_t readSamples = read(buffer, samples, 500);
    if (readSamples <= 0) {
        free(buffer);
        return 0;
    }

    int16_t peak = 0;
    for (size_t i = 0; i < readSamples; i++) {
        int16_t absVal = abs(buffer[i]);
        if (absVal > peak) {
            peak = absVal;
        }
    }

    free(buffer);
    return (uint16_t)peak;
}

uint16_t CamS3_Mic::getRMSLevel(size_t samples) {
    if (!_initialized) return 0;

    int16_t* buffer = (int16_t*)malloc(samples * sizeof(int16_t));
    if (!buffer) return 0;

    int32_t readSamples = read(buffer, samples, 500);
    if (readSamples <= 0) {
        free(buffer);
        return 0;
    }

    int64_t sumSquares = 0;
    for (size_t i = 0; i < readSamples; i++) {
        sumSquares += (int64_t)buffer[i] * buffer[i];
    }

    free(buffer);
    return (uint16_t)sqrt((double)sumSquares / readSamples);
}

bool CamS3_Mic::isSoundDetected(uint16_t threshold, size_t samples) {
    return getPeakAmplitude(samples) > threshold;
}

// ============================================
// CamS3_SD Implementation
// ============================================

bool CamS3_SD::begin(uint32_t spiFreq) {
    if (_initialized) {
        return true;
    }

    _spiFreq = spiFreq;

    // Initialize SPI with CamS3 SD card pins
    _spi = new SPIClass(HSPI);
    _spi->begin(CAMS3_SD_SCK_PIN, CAMS3_SD_MISO_PIN, CAMS3_SD_MOSI_PIN, CAMS3_SD_CS_PIN);

    if (!SD.begin(CAMS3_SD_CS_PIN, *_spi, _spiFreq)) {
        Serial.println("[CamS3 SD] Mount failed");
        delete _spi;
        _spi = nullptr;
        return false;
    }

    sdcard_type_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("[CamS3 SD] No card attached");
        SD.end();
        delete _spi;
        _spi = nullptr;
        return false;
    }

    _initialized = true;
    Serial.printf("[CamS3 SD] Card mounted: %s, Size: %lluMB\n", getCardTypeName(), getTotalBytes() / (1024 * 1024));

    return true;
}

void CamS3_SD::end() {
    if (_initialized) {
        SD.end();
        if (_spi) {
            _spi->end();
            delete _spi;
            _spi = nullptr;
        }
        _initialized = false;
    }
}

sdcard_type_t CamS3_SD::getCardType() {
    if (!_initialized) return CARD_NONE;
    return SD.cardType();
}

const char* CamS3_SD::getCardTypeName() {
    switch (getCardType()) {
        case CARD_MMC:
            return "MMC";
        case CARD_SD:
            return "SD";
        case CARD_SDHC:
            return "SDHC";
        default:
            return "Unknown";
    }
}

uint64_t CamS3_SD::getTotalBytes() {
    if (!_initialized) return 0;
    return SD.totalBytes();
}

uint64_t CamS3_SD::getUsedBytes() {
    if (!_initialized) return 0;
    return SD.usedBytes();
}

uint64_t CamS3_SD::getFreeBytes() {
    if (!_initialized) return 0;
    return SD.totalBytes() - SD.usedBytes();
}

bool CamS3_SD::writeFile(const char* path, const uint8_t* data, size_t len) {
    if (!_initialized) return false;

    File file = SD.open(path, FILE_WRITE);
    if (!file) {
        Serial.printf("[CamS3 SD] Failed to open file for writing: %s\n", path);
        return false;
    }

    size_t written = file.write(data, len);
    file.close();

    if (written != len) {
        Serial.printf("[CamS3 SD] Write incomplete: %d/%d bytes\n", written, len);
        return false;
    }

    return true;
}

bool CamS3_SD::appendFile(const char* path, const uint8_t* data, size_t len) {
    if (!_initialized) return false;

    File file = SD.open(path, FILE_APPEND);
    if (!file) {
        Serial.printf("[CamS3 SD] Failed to open file for appending: %s\n", path);
        return false;
    }

    size_t written = file.write(data, len);
    file.close();

    return written == len;
}

int32_t CamS3_SD::readFile(const char* path, uint8_t* buffer, size_t maxLen) {
    if (!_initialized) return -1;

    File file = SD.open(path, FILE_READ);
    if (!file) {
        Serial.printf("[CamS3 SD] Failed to open file for reading: %s\n", path);
        return -1;
    }

    size_t bytesRead = file.read(buffer, maxLen);
    file.close();

    return bytesRead;
}

bool CamS3_SD::exists(const char* path) {
    if (!_initialized) return false;
    return SD.exists(path);
}

bool CamS3_SD::remove(const char* path) {
    if (!_initialized) return false;
    return SD.remove(path);
}

bool CamS3_SD::rename(const char* pathFrom, const char* pathTo) {
    if (!_initialized) return false;
    return SD.rename(pathFrom, pathTo);
}

bool CamS3_SD::mkdir(const char* path) {
    if (!_initialized) return false;
    return SD.mkdir(path);
}

bool CamS3_SD::rmdir(const char* path) {
    if (!_initialized) return false;
    return SD.rmdir(path);
}

int64_t CamS3_SD::getFileSize(const char* path) {
    if (!_initialized) return -1;

    File file = SD.open(path, FILE_READ);
    if (!file) {
        return -1;
    }

    int64_t size = file.size();
    file.close();
    return size;
}

void CamS3_SD::listDir(const char* dirname, uint8_t levels) {
    if (!_initialized) {
        Serial.println("[CamS3 SD] Not initialized");
        return;
    }

    Serial.printf("Listing directory: %s\n", dirname);

    File root = SD.open(dirname);
    if (!root) {
        Serial.println("Failed to open directory");
        return;
    }
    if (!root.isDirectory()) {
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            Serial.printf("  DIR : %s\n", file.name());
            if (levels > 0) {
                listDir(file.path(), levels - 1);
            }
        } else {
            Serial.printf("  FILE: %s  SIZE: %d\n", file.name(), file.size());
        }
        file = root.openNextFile();
    }
}

bool CamS3_SD::saveFrame(camera_fb_t* fb, const char* path) {
    if (!_initialized || !fb) return false;

    String filename;
    if (path == nullptr) {
        filename = generateFilename();
    } else {
        filename = String(path);
    }

    bool result = writeFile(filename.c_str(), fb->buf, fb->len);

    if (result) {
        Serial.printf("[CamS3 SD] Saved: %s (%d bytes)\n", filename.c_str(), fb->len);
    }

    return result;
}

String CamS3_SD::generateFilename(const char* prefix, const char* extension) {
    _fileCounter++;
    char filename[64];
    snprintf(filename, sizeof(filename), "/%s_%lu_%lu.%s", prefix, millis(), _fileCounter, extension);
    return String(filename);
}
