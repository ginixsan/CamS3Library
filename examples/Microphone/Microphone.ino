/**
 * @file Microphone.ino
 * @brief PDM Microphone example for M5Stack Unit CamS3-5MP
 *
 * This example demonstrates basic microphone usage:
 * - Reading audio samples
 * - Sound level detection
 * - Peak amplitude monitoring
 */

#include <CamS3Library.h>

// Sound detection threshold (adjust based on your environment)
#define SOUND_THRESHOLD 1000

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n[CamS3] Microphone Example");
    Serial.println("==========================");

    // Initialize only the microphone (camera not needed for this example)
    if (!CamS3.Mic.begin()) {
        Serial.println("[CamS3] Microphone init failed!");
        while (1) {
            delay(1000);
        }
    }

    Serial.printf("[CamS3] Microphone ready: %d Hz, %d-bit\n",
                  CamS3.Mic.getSampleRate(),
                  CamS3.Mic.getSampleBits());

    // Also init LED for visual feedback
    pinMode(CAMS3_LED_GPIO, OUTPUT);

    Serial.println("\n[CamS3] Monitoring audio levels...\n");
}

void loop() {
    // Get audio levels
    uint16_t peak = CamS3.Mic.getPeakAmplitude(512);
    uint16_t rms  = CamS3.Mic.getRMSLevel(512);

    // Visual level meter
    int bars = map(peak, 0, 10000, 0, 30);
    bars = constrain(bars, 0, 30);

    Serial.print("[");
    for (int i = 0; i < 30; i++) {
        Serial.print(i < bars ? "=" : " ");
    }
    Serial.printf("] Peak: %5d  RMS: %5d", peak, rms);

    // Sound detection
    if (peak > SOUND_THRESHOLD) {
        Serial.print("  ** SOUND **");
        digitalWrite(CAMS3_LED_GPIO, HIGH);
    } else {
        digitalWrite(CAMS3_LED_GPIO, LOW);
    }

    Serial.println();

    delay(100);  // Update ~10 times per second
}
