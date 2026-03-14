/**
 * InnerOracle - Receiver & Display Node
 * * Function: Receives stress levels via ESP-NOW and controls an X27 stepper motor 
 * to point at tarot card symbols on a physical dial. Updates an OLED display 
 * and NeoPixel LED based on the received data.
 */

#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <SwitecX25.h> 
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Adafruit_NeoPixel.h>

// --- [Calibration Settings] ---
// Adjust homeOffset if the needle is misaligned at the starting position (The Fool).
int homeOffset = 100; 
// Adjust totalSteps to ensure the needle reaches the final position (The World) accurately.
int totalSteps = 950;  
// ------------------------------

// Motor instance (Steps, Pins: D2, D3, D0, D1)
SwitecX25 motor(1200, D2, D3, D0, D1); 

#define LED_PIN 20    
Adafruit_NeoPixel strip(1, LED_PIN, NEO_GRB + NEO_KHZ800);
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// Global State Variables
volatile bool motorReadyToSpin = false;
int receivedStress = 0;
unsigned long lastOracleTime = 0; 

// Data structure for ESP-NOW
typedef struct struct_message { 
    int gsrValue; 
    int stressLevel; 
} struct_message;
struct_message incomingData;

int cardPositions[8]; 

const char* cardNames[8] = {"The Fool", "The Lovers", "The Hermit", "The Death", "The Devil", "The Tower", "The Star", "The World"};
const char* fortunes[8][3] = {
  {"Fresh Start", "Step Wisely", "Trust You"}, {"Deep Harmony", "Listen In", "You're Loved"},
  {"Peaceful Mind", "Quiet Down", "Just Breathe"}, {"New Horizon", "Let It Flow", "Better Days"},
  {"Strong Will", "Stay True", "Stay Gold"}, {"Clear View", "Hold Firm", "Rise Again"},
  {"Bright Hope", "Follow Light", "Still Shine"}, {"Peak State", "Keep Going", "Proud of You"}
};

uint32_t cardColors[8] = {
  strip.Color(255, 255, 255), strip.Color(255, 50, 100), strip.Color(255, 150, 0),
  strip.Color(100, 0, 255), strip.Color(255, 0, 0), strip.Color(255, 255, 0),
  strip.Color(0, 150, 255), strip.Color(0, 255, 150)
};

// --- Helper Functions ---

/**
 * Handles centered text alignment on the OLED with dynamic font scaling.
 */
void printLargeCentered(String text, int yPos) {
  int16_t x1, y1; uint16_t w, h;
  display.setFont(&FreeSans9pt7b);
  display.setTextSize(1);
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  
  // Scale down font size if the text exceeds screen width
  if (w > 120) { 
    display.setFont(NULL); 
    display.setTextSize(2); 
    display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h); 
    yPos -= 5; 
  }
  display.setCursor((128 - w) / 2, yPos);
  display.print(text);
}

/**
 * Displays onboarding instructions for the user.
 */
void printOnboarding() {
  display.clearDisplay();
  display.setFont(&FreeSans9pt7b);
  printLargeCentered("Place hand", 25);
  printLargeCentered("on the Orb", 50);
  display.display();
}

/**
 * Opening sequence: Title typing effect and motor/LED calibration ceremony.
 */
void startupAnimation() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  
  // 1. Staggered typing effect for the title
  String t1 = "Inner"; String t2 = "Oracle";
  for(int i=1; i<=t1.length(); i++) {
    display.clearDisplay();
    printLargeCentered(t1.substring(0, i), 25);
    display.display(); delay(100);
  }
  delay(300);
  for(int i=1; i<=t2.length(); i++) {
    display.clearDisplay();
    printLargeCentered(t1, 25);
    printLargeCentered(t2.substring(0, i), 50);
    display.display(); delay(100);
  }

  // 2. Needle sweep and progress bar (Ceremony)
  motor.setPosition(cardPositions[7]); 
  for (int i = 0; i <= 100; i++) {
    motor.update();
    display.drawRect(20, 58, 88, 4, SSD1306_WHITE);
    display.fillRect(20, 58, (i * 88 / 100), 4, SSD1306_WHITE);
    display.display();
    strip.setPixelColor(0, strip.Color(i, i, i)); strip.show();
    // Maintain torque during movement
    for(int j=0; j<15; j++) { motor.update(); delay(2); } 
  }

  // 3. Return to Home position (The Fool)
  motor.setPosition(cardPositions[0]);
  while (motor.currentStep != motor.targetStep) {
    motor.update();
    strip.setPixelColor(0, strip.Color(100, 100, 100)); strip.show();
    delay(2);
  }
}

/**
 * Callback function executed when data is received via ESP-NOW.
 */
void OnDataRecv(const uint8_t * mac, const uint8_t *ptr, int len) {
  memcpy(&incomingData, ptr, sizeof(incomingData));
  receivedStress = incomingData.stressLevel;
  motorReadyToSpin = true;
}

void setup() {
  strip.begin(); 
  strip.setBrightness(150);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  
  // Pre-calculate step positions for each card
  int gap = totalSteps / 7;
  for (int i = 0; i < 8; i++) {
    cardPositions[i] = homeOffset + (i * gap);
  }

  motor.zero(); // Physical homing sequence
  delay(500);
  startupAnimation();
  printOnboarding();

  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  motor.update(); // Continuous motor update

  if (motorReadyToSpin && (millis() - lastOracleTime > 5000)) {
    motorReadyToSpin = false;
    strip.setPixelColor(0, strip.Color(0, 50, 200)); strip.show(); // Analyzing status (Blue)
    
    display.clearDisplay();
    display.setFont(NULL);
    display.setCursor(25, 30); display.print("Reading Soul...");
    display.display();

    int targetIdx = random(0, 8); // Select a random tarot card

    // Special animation for "The Fool" (Index 0): Full sweep before stopping
    if (targetIdx == 0) {
      motor.setPosition(cardPositions[7]);
      while (motor.currentStep != motor.targetStep) { motor.update(); delay(2); }
      delay(300);
    }

    // Move to target card position
    motor.setPosition(cardPositions[targetIdx]);
    while (motor.currentStep != motor.targetStep) { motor.update(); delay(3); }

    // Final result display and LED update
    strip.setPixelColor(0, cardColors[targetIdx]); strip.show();
    display.clearDisplay();
    display.setFont(NULL);
    display.setCursor(0, 0); display.print(cardNames[targetIdx]);
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    printLargeCentered(fortunes[targetIdx][receivedStress], 45);
    display.display();

    delay(9000); // Wait for the user to read the fortune

    // Reset to idle state: Turn off LED and return home
    strip.setPixelColor(0, 0); strip.show();
    display.clearDisplay();
    printLargeCentered("Resetting...", 38);
    display.display();

    motor.setPosition(cardPositions[0]); 
    while (motor.currentStep != motor.targetStep) { motor.update(); delay(2); }

    printOnboarding();
    lastOracleTime = millis();
  }
}