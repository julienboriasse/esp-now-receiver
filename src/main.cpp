#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>

// Structure to receive data
typedef struct {
  uint8_t id;
  float temperature;
  float humidity;
  int rssi;
} EspNowMessage;

// Global variable to store received data
EspNowMessage receivedData;
volatile bool dataReceived = false;

// Callback function that gets called when data is received
void onDataReceive(const uint8_t *senderMac, const uint8_t *incomingData, int len) {
  // Copy the received data to our structure
  memcpy(&receivedData, incomingData, sizeof(receivedData));
  dataReceived = true;
  
  // Print received data immediately
  Serial.println("================================");
  Serial.printf("Data received from MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                senderMac[0], senderMac[1], senderMac[2],
                senderMac[3], senderMac[4], senderMac[5]);
  Serial.printf("Message ID: %d\n", receivedData.id);
  Serial.printf("Temperature: %.2f°C\n", receivedData.temperature);
  Serial.printf("Humidity: %.2f%%\n", receivedData.humidity);
  Serial.printf("RSSI: %d dBm\n", receivedData.rssi);
  Serial.println("================================");
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
  // The ESP-NOW callback will handle data reception
  // This loop just keeps the device alive
  delay(1000);
}