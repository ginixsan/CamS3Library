/**
 * @file SDCard_Advanced.ino
 * @brief Advanced SD card operations example for M5Stack Unit CamS3-5MP
 *
 * This example demonstrates various SD card operations:
 * - File read/write
 * - Directory operations
 * - Manual frame saving
 */

#include <CamS3Library.h>

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n[CamS3] Advanced SD Card Example");
    Serial.println("=================================");

    // Initialize only the SD card (camera optional for this demo)
    if (!CamS3.Sd.begin()) {
        Serial.println("[CamS3] SD card init failed!");
        while (1) {
            delay(1000);
        }
    }

    // Also init camera for capture demos
    CamS3.Camera.begin(FRAMESIZE_VGA);

    printSDInfo();
    testFileOperations();
    testDirectoryOperations();
    testCameraCapture();

    Serial.println("\n[CamS3] All tests completed!");
}

void loop() {
    // Nothing to do in loop
    delay(10000);
}

void printSDInfo() {
    Serial.println("\n--- SD Card Info ---");
    Serial.printf("Card Type: %s\n", CamS3.Sd.getCardTypeName());
    Serial.printf("Total:     %llu MB\n", CamS3.Sd.getTotalBytes() / (1024 * 1024));
    Serial.printf("Used:      %llu MB\n", CamS3.Sd.getUsedBytes() / (1024 * 1024));
    Serial.printf("Free:      %llu MB\n", CamS3.Sd.getFreeBytes() / (1024 * 1024));
}

void testFileOperations() {
    Serial.println("\n--- File Operations Test ---");

    // Write a text file
    const char* testFile = "/test.txt";
    const char* testData = "Hello from CamS3Library!\nThis is a test file.\n";

    Serial.printf("Writing to %s...\n", testFile);
    if (CamS3.Sd.writeFile(testFile, (const uint8_t*)testData, strlen(testData))) {
        Serial.println("Write successful!");
    }

    // Check if file exists
    if (CamS3.Sd.exists(testFile)) {
        Serial.printf("File exists, size: %lld bytes\n", CamS3.Sd.getFileSize(testFile));
    }

    // Read the file back
    uint8_t buffer[256];
    int32_t bytesRead = CamS3.Sd.readFile(testFile, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        Serial.printf("Read %d bytes:\n%s\n", bytesRead, (char*)buffer);
    }

    // Append to file
    const char* appendData = "Appended line!\n";
    Serial.println("Appending to file...");
    CamS3.Sd.appendFile(testFile, (const uint8_t*)appendData, strlen(appendData));

    // Read again
    bytesRead = CamS3.Sd.readFile(testFile, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        Serial.printf("After append:\n%s\n", (char*)buffer);
    }

    // Rename file
    const char* newName = "/test_renamed.txt";
    Serial.printf("Renaming to %s...\n", newName);
    if (CamS3.Sd.rename(testFile, newName)) {
        Serial.println("Rename successful!");
    }

    // Delete file
    Serial.printf("Deleting %s...\n", newName);
    if (CamS3.Sd.remove(newName)) {
        Serial.println("Delete successful!");
    }
}

void testDirectoryOperations() {
    Serial.println("\n--- Directory Operations Test ---");

    // Create nested directories
    Serial.println("Creating directories...");
    CamS3.Sd.mkdir("/testdir");
    CamS3.Sd.mkdir("/testdir/subdir1");
    CamS3.Sd.mkdir("/testdir/subdir2");

    // Create some files
    const char* data = "test";
    CamS3.Sd.writeFile("/testdir/file1.txt", (const uint8_t*)data, strlen(data));
    CamS3.Sd.writeFile("/testdir/subdir1/file2.txt", (const uint8_t*)data, strlen(data));

    // List directory contents
    Serial.println("\nDirectory listing (2 levels deep):");
    CamS3.Sd.listDir("/testdir", 2);

    // Cleanup
    Serial.println("\nCleaning up test directories...");
    CamS3.Sd.remove("/testdir/file1.txt");
    CamS3.Sd.remove("/testdir/subdir1/file2.txt");
    CamS3.Sd.rmdir("/testdir/subdir1");
    CamS3.Sd.rmdir("/testdir/subdir2");
    CamS3.Sd.rmdir("/testdir");
    Serial.println("Cleanup done!");
}

void testCameraCapture() {
    Serial.println("\n--- Camera Capture Test ---");

    if (!CamS3.Camera.isInitialized()) {
        Serial.println("Camera not initialized, skipping capture test");
        return;
    }

    // Create captures directory
    if (!CamS3.Sd.exists("/captures")) {
        CamS3.Sd.mkdir("/captures");
    }

    // Capture multiple images with different settings
    framesize_t sizes[] = {FRAMESIZE_QVGA, FRAMESIZE_VGA, FRAMESIZE_SVGA};
    const char* sizeNames[] = {"QVGA", "VGA", "SVGA"};

    for (int i = 0; i < 3; i++) {
        CamS3.Camera.setFrameSize(sizes[i]);
        delay(100);  // Let sensor adjust

        CamS3.Camera.ledOn();

        if (CamS3.Camera.get()) {
            char filename[64];
            snprintf(filename, sizeof(filename), "/captures/test_%s.jpg", sizeNames[i]);

            // Manual save using Sd.saveFrame
            if (CamS3.Sd.saveFrame(CamS3.Camera.fb, filename)) {
                Serial.printf("Captured %s: %dx%d, %d bytes\n",
                              sizeNames[i],
                              CamS3.Camera.fb->width,
                              CamS3.Camera.fb->height,
                              CamS3.Camera.fb->len);
            }
            CamS3.Camera.free();
        }

        CamS3.Camera.ledOff();
        delay(500);
    }

    // List captured files
    Serial.println("\nCaptured files:");
    CamS3.Sd.listDir("/captures", 0);
}
