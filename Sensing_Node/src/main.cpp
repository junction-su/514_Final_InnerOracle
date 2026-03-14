/**
 * InnerOracle - Sensing Node
 * Function: Detects GSR (Galvanic Skin Response) and sends stress data via ESP-NOW.
 */

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

const int SENSOR_PIN = 2;    // GSR Sensor Pin (GPIO 2)
const int LED_PIN = D3;      // NeoPixel Pin (D3)
const int NUM_LEDS = 2;    
const int SENSITIVITY = 25;  // Delta value for trigger detection

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Data structure for ESP-NOW communication
typedef struct struct_message {
  int gsrValue;
  int stressLevel;
} struct_message;
struct_message myData;

int baseline = 0;

// Visual feedback: Rainbow cycle effect
void startRainbowEffect(int duration_ms) {
  unsigned long startTime = millis();
  uint32_t firstPixelHue = 0;
  while (millis() - startTime < duration_ms) {
    strip.setPixelColor(0, strip.gamma32(strip.ColorHSV(firstPixelHue)));
    strip.show();
    firstPixelHue += 1024; 
    delay(20); 
  }
}

void setup() {
  Serial.begin(115200);
  analogSetAttenuation(ADC_11db);
  
  strip.begin();
  strip.setBrightness(255); // Max brightness for showcase visibility
  strip.show();

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) return;

  // Broadcast address setup for peer communication
  uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  // Initial calibration for sensor baseline
  long sum = 0;
  for(int i=0; i<30; i++) { sum += analogRead(SENSOR_PIN); delay(20); }
  baseline = sum / 30;
}

void loop() {
  int currentVal = analogRead(SENSOR_PIN);
  
  // Trigger condition: current value exceeds adaptive baseline + sensitivity
  if (currentVal > (baseline + SENSITIVITY)) {
    // 1. Alert: Red flash then Rainbow effect
    strip.setPixelColor(0, strip.Color(255, 0, 0)); 
    strip.show();
    delay(100);
    startRainbowEffect(3000); 

    // 2. Data Transmission
    myData.gsrValue = currentVal;
    myData.stressLevel = random(0, 3); // 0:Low, 1:Mid, 2:High
    uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}; 
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));

    // 3. Reset and Cool-down
    strip.setPixelColor(0, strip.Color(150, 150, 150)); strip.show();
    delay(5000); 
    baseline = analogRead(SENSOR_PIN); // Recalibrate baseline after trigger
  } else {
    // Idle state: Subtle white light
    strip.setPixelColor(0, strip.Color(150, 150, 150)); strip.show();
    // Smooth baseline tracking
    baseline = (baseline * 0.99) + (currentVal * 0.01); 
    delay(50);
  }
}