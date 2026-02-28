#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

// --- Pin Definitions (Based on Schematic) ---
const int GSR_PIN = A0;         // Grove GSR Sensor SIG connected to A0
const int NEOPIXEL_PIN = D6;    // NeoPixel DIN connected to D6
const int NUMPIXELS = 1;

Adafruit_NeoPixel pixels(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// --- ESP-NOW Setup ---
uint8_t displayDeviceAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 

// Structure to send data
typedef struct struct_message {
    char stressLevel[10]; // "Low" or "High"
    int rawGSR;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
  Serial.begin(115200);
  
  // Initialize NeoPixel
  pixels.begin();
  pixels.clear();
  pixels.show();

  // Initialize Wi-Fi in Station mode for ESP-NOW
  WiFi.mode(WIFI_STA);

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register peer
  esp_now_register_send_cb(OnDataSent);
  memcpy(peerInfo.peer_addr, displayDeviceAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  // 1. Read GSR Sensor Data
  int gsrValue = analogRead(GSR_PIN);
  myData.rawGSR = gsrValue;
  
  Serial.print("GSR value: ");
  Serial.println(gsrValue);

  // 2. Determine Stress Level (Threshold needs calibration)
  // Note: GSR values vary by person and skin moisture. 
  int threshold = 2000; 
  
  if (gsrValue < threshold) {
    strcpy(myData.stressLevel, "High");
    pixels.setPixelColor(0, pixels.Color(150, 0, 0)); // Red for High Stress
  } else {
    strcpy(myData.stressLevel, "Low");
    pixels.setPixelColor(0, pixels.Color(0, 0, 150)); // Blue for Low Stress
  }
  pixels.show();

  // 3. Send Data via ESP-NOW
  esp_err_t result = esp_now_send(displayDeviceAddress, (uint8_t *) &myData, sizeof(myData));
  
  delay(500); // Read and send data every 0.5 seconds
}
