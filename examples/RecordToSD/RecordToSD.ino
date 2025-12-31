/**
 * @file RecordToSD.ino
 * @brief Record audio to SD card example for M5Stack Unit CamS3-5MP
 *
 * This example records audio clips to the SD card as WAV files.
 * Press the button (or detect loud sound) to trigger recording.
 */

#include <CamS3Library.h>

// Recording settings
#define RECORD_DURATION_MS   5000   // 5 seconds
#define SOUND_TRIGGER_LEVEL  2000   // Auto-record when loud sound detected
#define AUTO_RECORD_ENABLED  true   // Set to false to disable auto-record

uint32_t recordingCount = 0;
bool isRecording        = false;

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n[CamS3] Record to SD Example");
    Serial.println("============================");

    // Initialize SD card and microphone (camera optional)
    CamS3.Camera.begin();  // Init camera for LED control

    if (!CamS3.Sd.begin()) {
        Serial.println("[CamS3] SD card init failed!");
        while (1) {
            delay(1000);
        }
    }

    if (!CamS3.Mic.begin(16000, 16)) {  // 16kHz, 16-bit
        Serial.println("[CamS3] Microphone init failed!");
        while (1) {
            delay(1000);
        }
    }

    Serial.printf("[CamS3] SD Card: %s, Free: %llu MB\n",
                  CamS3.Sd.getCardTypeName(),
                  CamS3.Sd.getFreeBytes() / (1024 * 1024));
    Serial.printf("[CamS3] Mic: %d Hz, %d-bit\n",
                  CamS3.Mic.getSampleRate(),
                  CamS3.Mic.getSampleBits());

    // Create recordings directory
    if (!CamS3.Sd.exists("/recordings")) {
        CamS3.Sd.mkdir("/recordings");
    }

    Serial.println("\n[CamS3] Ready!");
    Serial.println("- Send 'r' via Serial to record");
    if (AUTO_RECORD_ENABLED) {
        Serial.printf("- Auto-record triggers at sound level > %d\n", SOUND_TRIGGER_LEVEL);
    }
    Serial.println();
}

void loop() {
    // Check for serial command
    if (Serial.available()) {
        char c = Serial.read();
        if (c == 'r' || c == 'R') {
            doRecording();
        }
    }

    // Auto-record on loud sound
    if (AUTO_RECORD_ENABLED && !isRecording) {
        uint16_t level = CamS3.Mic.getPeakAmplitude(256);
        if (level > SOUND_TRIGGER_LEVEL) {
            Serial.printf("[Trigger] Sound detected: %d\n", level);
            doRecording();
        }
    }

    // Show current audio level periodically
    static uint32_t lastPrint = 0;
    if (millis() - lastPrint > 500) {
        lastPrint = millis();
        uint16_t level = CamS3.Mic.getRMSLevel(256);
        Serial.printf("[Monitor] RMS Level: %d\n", level);
    }

    delay(10);
}

void doRecording() {
    if (isRecording) return;
    isRecording = true;

    CamS3.Camera.ledOn();
    Serial.printf("\n[Recording] Starting %d ms recording...\n", RECORD_DURATION_MS);

    // Generate filename
    char filename[64];
    snprintf(filename, sizeof(filename), "/recordings/REC_%04d.wav", recordingCount++);

    // Record and save
    uint32_t startTime = millis();

    if (CamS3.recordToSD(filename, RECORD_DURATION_MS)) {
        uint32_t elapsed = millis() - startTime;
        int64_t fileSize = CamS3.Sd.getFileSize(filename);
        Serial.printf("[Recording] Saved: %s\n", filename);
        Serial.printf("[Recording] Size: %lld bytes, Time: %lu ms\n", fileSize, elapsed);
    } else {
        Serial.println("[Recording] Failed!");
    }

    CamS3.Camera.ledOff();
    isRecording = false;

    // Brief pause to prevent immediate re-trigger
    delay(1000);
}
