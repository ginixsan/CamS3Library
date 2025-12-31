/**
 * @file SimpleCapture.ino
 * @brief Simple image capture example for M5Stack Unit CamS3-5MP
 *
 * This example demonstrates basic camera initialization and image capture.
 * It captures images periodically and prints the frame size.
 */

#include <CamS3Library.h>

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n[CamS3] Simple Capture Example");
    Serial.println("===============================");

    // Initialize camera with default settings
    // FRAMESIZE_VGA, PIXFORMAT_JPEG, quality=12, fb_count=2
    if (!CamS3.begin()) {
        Serial.println("[CamS3] Camera init failed!");
        while (1) {
            delay(1000);
        }
    }

    Serial.printf("[CamS3] Sensor: %s\n", CamS3.Camera.getSensorName());
    Serial.println("[CamS3] Camera ready!\n");

    // Optional: Change settings after initialization
    CamS3.Camera.setFrameSize(FRAMESIZE_VGA);
    CamS3.Camera.setQuality(10);  // 0-63, lower = better
    CamS3.Camera.setBrightness(0);
    CamS3.Camera.setSaturation(0);
}

void loop() {
    // Turn on LED while capturing
    CamS3.Camera.ledOn();

    // Capture a frame
    if (CamS3.Camera.get()) {
        Serial.printf("[Capture] Frame size: %d bytes (%dx%d)\n",
                      CamS3.Camera.fb->len,
                      CamS3.Camera.fb->width,
                      CamS3.Camera.fb->height);

        // Access the raw image data
        // uint8_t* imageData = CamS3.Camera.fb->buf;
        // size_t imageLen = CamS3.Camera.fb->len;

        // Don't forget to free the frame buffer!
        CamS3.Camera.free();
    } else {
        Serial.println("[Capture] Failed to get frame");
    }

    CamS3.Camera.ledOff();

    // Wait before next capture
    delay(1000);
}
