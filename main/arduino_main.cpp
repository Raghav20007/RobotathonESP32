
#include "sdkconfig.h"
#include <Arduino.h>
#include <Bluepad32.h>
#include <uni.h>
#include "controller_callbacks.h"

#define IN1  16  // Control pin 1
#define IN2  17  // Control pin 2
#define IN3  18  // Control pin 3
#define IN4  19  // Control pin 4

extern ControllerPtr myControllers[BP32_MAX_GAMEPADS]; // BP32 library allows for up to 4 concurrent controller connections, but we only need 1

// Controller code
void foo(ControllerPtr myController) {
  while(1) {
    BP32.update();
    if(myController->axisY()) {
      // Spin motor
      digitalWrite(IN1, 1);  // PWM signal
      digitalWrite(IN2, LOW); // Direction control
      digitalWrite(IN3, LOW);  // PWM signal
      digitalWrite(IN4, 1); // Direction control
      return;
    }
    else {
      // Stop motor
      digitalWrite(IN1, LOW);
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);
      digitalWrite(IN4, LOW);
    }
  }
}

void setup() {
  // Motor Setup
  Serial.begin(115200);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Controller Setup
  BP32.setup(&onConnectedController, &onDisconnectedController);
  BP32.forgetBluetoothKeys(); 
  esp_log_level_set("gpio", ESP_LOG_ERROR); // Suppress info log spam from gpio_isr_service
  uni_bt_allowlist_set_enabled(true);
}

void loop() {

    // Controller Loop Code
    vTaskDelay(1); // Ensures WDT does not get triggered when no controller is connected
    BP32.update(); 

    for (auto myController : myControllers) { // Only execute code when controller is connected
        if (myController && myController->isConnected() && myController->hasData()) {        
            
            foo(myController);

        }
    }
}