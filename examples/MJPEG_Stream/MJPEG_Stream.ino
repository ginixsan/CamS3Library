/**
 * @file MJPEG_Stream.ino
 * @brief MJPEG Stream Server example for M5Stack Unit CamS3-5MP
 *
 * This example creates a simple MJPEG stream server that can be viewed
 * in any web browser by navigating to the ESP32's IP address.
 */

#include <WiFi.h>
#include <CamS3Library.h>

// WiFi credentials
const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

WiFiServer server(80);

// MJPEG Stream constants
#define PART_BOUNDARY "123456789000000000000987654321"
static const char* STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* STREAM_BOUNDARY     = "--" PART_BOUNDARY "\r\n";
static const char* STREAM_PART         = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n[CamS3] MJPEG Stream Example");
    Serial.println("============================");

    // Initialize camera
    if (!CamS3.begin()) {
        Serial.println("[CamS3] Camera init failed!");
        while (1) {
            delay(1000);
        }
    }

    Serial.printf("[CamS3] Camera ready: %s\n", CamS3.Camera.getSensorName());

    // Optional: Adjust camera settings
    // CamS3.Camera.setFrameSize(FRAMESIZE_SVGA);
    // CamS3.Camera.setQuality(10);
    // CamS3.Camera.setBrightness(0);

    // Connect to WiFi
    WiFi.begin(ssid, password);
    WiFi.setSleep(false);

    Serial.print("[WiFi] Connecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        CamS3.Camera.ledSet((millis() / 500) % 2);  // Blink LED
    }

    CamS3.Camera.ledOff();
    Serial.println("\n[WiFi] Connected!");
    Serial.printf("[WiFi] Stream URL: http://%s/\n", WiFi.localIP().toString().c_str());

    server.begin();
}

void loop() {
    WiFiClient client = server.available();
    if (client) {
        Serial.println("[Stream] Client connected");
        CamS3.Camera.ledOn();

        while (client.connected()) {
            if (client.available()) {
                handleStream(&client);
            }
        }

        client.stop();
        CamS3.Camera.ledOff();
        Serial.println("[Stream] Client disconnected");
    }
}

void handleStream(WiFiClient* client) {
    // Send HTTP headers
    client->println("HTTP/1.1 200 OK");
    client->printf("Content-Type: %s\r\n", STREAM_CONTENT_TYPE);
    client->println("Access-Control-Allow-Origin: *");
    client->println();

    int64_t lastFrame = esp_timer_get_time();

    while (client->connected()) {
        if (!CamS3.Camera.get()) {
            delay(10);
            continue;
        }

        // Send frame boundary and headers
        client->print(STREAM_BOUNDARY);
        client->printf(STREAM_PART, CamS3.Camera.fb->len);

        // Send frame data in chunks
        uint8_t* buf       = CamS3.Camera.fb->buf;
        size_t remaining   = CamS3.Camera.fb->len;
        const size_t chunk = 8 * 1024;

        while (remaining > 0) {
            size_t toSend = (remaining > chunk) ? chunk : remaining;
            if (client->write(buf, toSend) == 0) {
                CamS3.Camera.free();
                return;
            }
            buf += toSend;
            remaining -= toSend;
        }

        // Calculate FPS
        int64_t now       = esp_timer_get_time();
        int64_t frameTime = (now - lastFrame) / 1000;
        lastFrame         = now;

        Serial.printf("[Stream] %luKB %lums (%.1f fps)\n",
                      (unsigned long)(CamS3.Camera.fb->len / 1024),
                      (unsigned long)frameTime,
                      1000.0 / frameTime);

        CamS3.Camera.free();
    }
}
