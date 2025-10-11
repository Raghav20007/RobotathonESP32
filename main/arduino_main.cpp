
#include "sdkconfig.h"
#include <Arduino.h>

#define IN1  16  // Control pin 1
#define IN2  17  // Control pin 2

void setup() {
  Serial.begin(115200);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
}

void loop() {
  // Spin motor
  analogWrite(IN1, 255);  // PWM signal
  digitalWrite(IN2, LOW); // Direction control

  delay(1000);  // Run for 1 second

  // Stop motor
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  delay(1000); // Stop for 1 second

  vTaskDelay(1); // Yield CPU to not starve other ESP32 processes and cause WDT reset
}