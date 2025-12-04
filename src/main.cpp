#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <ArduinoJson.h>

  // Structure to receive ISS coordinates data
typedef struct {
  float latitude;
  float longitude;
  uint32_t timestamp;
} ISSCoordinates;

// Buffer for JSON string (max 256 bytes)
char jsonBuffer[256];
int jsonBufferLen = 0;

// Buffer to store sender MAC address
uint8_t senderMacBuffer[6];

// Global variable to store received data
ISSCoordinates issData;
volatile bool dataReceived = false;

// Callback function that gets called when data is received
// Keep this minimal - just copy data and set flag
void onDataReceive(const uint8_t *senderMac, const uint8_t *incomingData, int len) {
  // Ensure we don't exceed buffer size
  if (len >= sizeof(jsonBuffer)) {
    return;
  }
  
  // Copy the received data to our buffer
  memcpy(jsonBuffer, incomingData, len);
  jsonBufferLen = len;
  jsonBuffer[len] = '\0'; // Null-terminate the string
  
  // Copy sender MAC address
  memcpy(senderMacBuffer, senderMac, 6);
  
  // Set flag for main loop to process
  dataReceived = true;
}

// Process received data (called from main loop, not interrupt)
void processReceivedData() {
  // Parse JSON
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, jsonBuffer);
  
  if (error) {
    Serial.print("JSON parse error: ");
    Serial.println(error.f_str());
    return;
  }
  
  // Extract values from JSON
  float latitude = doc["latitude"];
  float longitude = doc["longitude"];
  uint32_t timestamp = doc["timestamp"];
  
  // Build complete output string in a buffer before sending to Serial
  // This prevents incomplete prints that can occur with multiple Serial calls
  char outputBuffer[512];
  int pos = 0;
  
  pos += sprintf(outputBuffer + pos, "================================\n");
  pos += sprintf(outputBuffer + pos, "ISS Data received from MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                 senderMacBuffer[0], senderMacBuffer[1], senderMacBuffer[2],
                 senderMacBuffer[3], senderMacBuffer[4], senderMacBuffer[5]);
  pos += sprintf(outputBuffer + pos, "Raw JSON: %s\n", jsonBuffer);
  pos += sprintf(outputBuffer + pos, "Latitude:  %.6f°\n", latitude);
  pos += sprintf(outputBuffer + pos, "Longitude: %.6f°\n", longitude);
  pos += sprintf(outputBuffer + pos, "Timestamp: %lu\n", timestamp);
  pos += sprintf(outputBuffer + pos, "================================\n");
  
  // Send complete output in one atomic operation
  Serial.print(outputBuffer);
}

void initWiFi() {
  Serial.println("Setting WiFi to station mode for ESP-NOW...");
  
  // Set WiFi mode to station (required for ESP-NOW)
  WiFi.mode(WIFI_STA);
  // Disconnect from any previous WiFi connection
  WiFi.disconnect();
  
  Serial.println("WiFi ready for ESP-NOW");
}

void initESPNow() {
  Serial.println("Initializing ESP-NOW...");
  
  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register the receive callback
  esp_now_register_recv_cb(onDataReceive);
  
  Serial.println("ESP-NOW initialized successfully!");
}

void setup() {
  // Initialize serial communication at 115200 baud
  Serial.begin(115200);
  
  // Give some time for the serial to initialize
  delay(1000);
  
  // Wait 5 seconds before printing
  delay(5000);
  
  Serial.println("\n\n================================");
  Serial.println("ESP-NOW Receiver Starting");
  Serial.println("================================");
  
  // Initialize WiFi
  initWiFi();
  
  // Initialize ESP-NOW
  initESPNow();
  
  // Print receiver MAC address for students
  Serial.println("\n=== RECEIVER MAC ADDRESS ===");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println("=============================\n");
  
  Serial.println("Setup complete! Waiting for ESP-NOW messages...");
}

void loop() {
  // Check if data was received
  if (dataReceived) {
    dataReceived = false; // Clear flag
    processReceivedData(); // Process in main loop, not in interrupt
  }
  
  delay(10);
}