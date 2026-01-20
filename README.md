# The Inner Oracle: Biometric Fortune Teller

## 1. Project Overview
**The Inner Oracle** is an interactive fortune-telling machine that combines biometric data with Tarot symbolism. Instead of relying on pure chance, it measures the user's physiological state (skin conductance) to generate a personalized reading. By placing a hand on the sensing orb, the user triggers a mechanical dial that points to a Tarot card, while an OLED screen displays a cryptic message tailored to their inner condition.

![Concept Sketch](https://github.com/SuHyun/Final_Project/blob/main/images/concept_sketch.jpg)
*(Note: Upload your 'Inner Oracle' sketch from page 2 of your PDF here)*



## 2. Sensing Device
The input mechanism simulates a "crystal ball" reading using Galvanic Skin Response (GSR).

* **Component:** Grove - GSR Sensor (Seeed Studio, Part #101020052)
* **Functionality:** This sensor measures the electrical conductance of the skin, which varies with moisture level (sweat) indicating psychological arousal or stress.
* **Operation:** When the user places their hand on the electrodes embedded in the orb, the ESP32C3 reads the analog signal. If the value crosses a threshold (indicating touch), it triggers the fortune-telling sequence.

![Sensing Device Sketch](https://github.com/SuHyun/Final_Project/blob/main/images/sensing_device.jpg)
*(Note: Upload your 'Sensing Device' sketch from page 2 here)*



## 3. Display Device
The output consists of a dual-display system: a physical mechanical dial and a digital screen.

* **Components:** * **Stepper Motor:** 28BYJ-48 with ULN2003 Driver (Part #105990072)
    * **OLED Display:** 0.96" I2C OLED Display Module (White) (SSD1306) (Generic SSD1306)
* **Functionality:** 1.  **Mechanical:** The stepper motor rotates a pointer to one of the Major Arcana Tarot cards arranged on a circular face.
    2.  **Digital:** The OLED screen displays text interpretations (e.g., "Hope is near") corresponding to the selected card.
* **Operation:** Based on the random seed generated from the sensor input, the motor calculates the specific angle to rotate, and the OLED updates its text buffer.

![Display Device Sketch](https://github.com/SuHyun/Final_Project/blob/main/images/display_device.jpg)
*(Note: Upload your 'Display Device' sketch from page 2 here)*



## 4. System Architecture & Data Flow
The device operates on a closed-loop system where biometric input directly drives physical and digital outputs.

### Data Flow Diagram
`[User Hand] --> [GSR Sensor] --(Analog Signal)--> [XIAO ESP32C3] --(I2C/GPIO)--> [OLED & Motor]`

1.  **Input:** ESP32C3 reads analog voltage from the GSR sensor.
2.  **Processing:** * The code detects if a user is touching the device (Threshold check).
    * The analog value is used as a "seed" for the random number generator to select a Tarot card index (0-5).
3.  **Output:** * **Motor:** Moves to specific steps (e.g., Card 1 = 0 steps, Card 2 = 500 steps).
    * **OLED:** Displays the string associated with the index.

![System Diagram](https://github.com/SuHyun/Final_Project/blob/main/images/system_diagram.jpg)
*(Note: Draw a simple box diagram showing lines connecting Sensor -> ESP32 -> Motor/OLED and upload it)*
