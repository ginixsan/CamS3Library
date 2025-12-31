/**
 * @file CaptureToSD.ino
 * @brief Capture images to SD card example for M5Stack Unit CamS3-5MP
 *
 * This example captures images periodically and saves them to the SD card.
 * It demonstrates the integrated Camera + SD card functionality.
 */

#include <CamS3Library.h>

// Capture interval in milliseconds
#define CAPTURE_INTERVAL 5000

unsigned long lastCaptureTime = 0;
uint32_t imageCount           = 0;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n[CamS3] Capture to SD Card Example");
    Serial.println("===================================");

    // Initialize camera and SD card together
    if (!CamS3.begin(true)) {  // true = also init SD card
        Serial.println("[CamS3] Initialization failed!");
        while (1) {
            delay(1000);
        }
    }

    Serial.printf("[CamS3] Camera: %s\n", CamS3.Camera.getSensorName());
    Serial.printf("[CamS3] SD Card: %s\n", CamS3.Sd.getCardTypeName());
    Serial.printf("[CamS3] SD Size: %llu MB\n", CamS3.Sd.getTotalBytes() / (1024 * 1024));
    Serial.printf("[CamS3] SD Free: %llu MB\n", CamS3.Sd.getFreeBytes() / (1024 * 1024));

    // Optional: Configure camera for high quality captures
    CamS3.Camera.setFrameSize(FRAMESIZE_UXGA);  // 1600x1200
    CamS3.Camera.setQuality(10);                 // High quality

    // Create images directory
    if (!CamS3.Sd.exists("/images")) {
        CamS3.Sd.mkdir("/images");
        Serial.println("[CamS3] Created /images directory");
    }

    // List existing files
    Serial.println("\n[CamS3] Existing files on SD card:");
    CamS3.Sd.listDir("/", 1);

    Serial.println("\n[CamS3] Ready! Capturing every 5 seconds...\n");
}

void loop() {
    unsigned long now = millis();

    if (now - lastCaptureTime >= CAPTURE_INTERVAL) {
        lastCaptureTime = now;

        CamS3.Camera.ledOn();

        // Method 1: Simple one-liner capture
        // CamS3.captureToSD();  // Auto-generates filename

        // Method 2: Custom filename
        char filename[32];
        snprintf(filename, sizeof(filename), "/images/IMG_%04d.jpg", imageCount++);

        if (CamS3.captureToSD(filename)) {
            int64_t fileSize = CamS3.Sd.getFileSize(filename);
            Serial.printf("[Capture] Saved: %s (%lld bytes)\n", filename, fileSize);
        } else {
            Serial.println("[Capture] Failed!");
        }

        CamS3.Camera.ledOff();

        // Print SD card status every 10 captures
        if (imageCount % 10 == 0) {
            Serial.printf("[Status] Images: %d, SD Free: %llu MB\n",
                          imageCount,
                          CamS3.Sd.getFreeBytes() / (1024 * 1024));
        }
    }
}
