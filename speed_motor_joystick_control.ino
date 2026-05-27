/*
Control speed with joystick 1

Wire Connection
1. OLED Display (SSD1306 128x64 I2C)
OLED Pin	Connect to Arduino
VCC	5V
GND	GND
SCL	A5 (for UNO)
SDA	A4 (for UNO)
2. Battery (7.4V):
  + → VCC (L298N)
  - → GND (L298N) + GND (Arduino)
3. Motor Driver (L298N / H-Bridge-like circuit)
Assuming a single DC motor:
Motor Driver Pin	Connect to Arduino
IN1	Pin 8
IN2	Pin 9
ENA	Pin 10 (PWM)
GND	GND
VCC	5V (or external 9-12V)
Motor Output A/B	Connected to motor
⚠️ Note: Ensure motor driver has sufficient power. 
For large motors, use an external power supply and 
common ground between Arduino and driver.
4. Joystick Module (with Analog X/Y and GND/VCC)
Joystick Pin	Arduino Pin	Function
VCC	5V	Power
GND	GND	Ground
VRx	A0	X-axis (Left/Right)
VRy	A1	Y-axis (Fwd/Bwd)
SW (optional)	Not used	Button press
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Motor pins
const int in1 = 8;
const int in2 = 9;
const int enA = 10;

// Joystick pins
const int joyX = A0;
const int joyY = A1;

// Joystick movement thresholds
const int thresholdLow = 450;
const int thresholdHigh = 550;

// OLED object position
int objectX = SCREEN_WIDTH / 2;
int currentSpeed = 0;

void setup() {
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(enA, OUTPUT);

  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true);  // Freeze if OLED fails
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Joystick Motor Control");
  display.display();
  delay(1000);
}

void loop() {
  int xVal = analogRead(joyX);  // Left/Right
  int yVal = analogRead(joyY);  // Forward/Backward

  // --- OLED object movement ---
  int step = map(abs(xVal - 512), 0, 512, 0, 3); // Speed of X move
  if (xVal < thresholdLow) {
    objectX -= step;
  } else if (xVal > thresholdHigh) {
    objectX += step;
  }
  objectX = constrain(objectX, 0, SCREEN_WIDTH - 8);

  // --- Motor control based on Y-axis ---
  if (yVal > thresholdHigh) {
    // Forward
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    currentSpeed = map(yVal, thresholdHigh, 1023, 0, 255);
  } else if (yVal < thresholdLow) {
    // Backward
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    currentSpeed = map(yVal, thresholdLow, 0, 0, 255);
  } else {
    // Stop
    currentSpeed = 0;
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }

  analogWrite(enA, currentSpeed);

  // --- Display updates ---
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Speed: ");
  display.print(map(currentSpeed, 0, 255, 0, 100));
  display.println("%");

  // Draw speed bar
  display.drawRect(10, 12, 100, 10, SSD1306_WHITE);
  display.fillRect(10, 12, map(currentSpeed, 0, 255, 0, 100), 10, SSD1306_WHITE);

  // Draw moving object
  display.fillRect(objectX, 40, 8, 8, SSD1306_WHITE);

  display.display();
  delay(50);
}
