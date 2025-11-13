#include <Arduino.h>
#include <Wire.h>
#include <Arduino_APDS9960.h>
#include <Bluepad32.h>
#include "controller_callbacks.h"
#include <ESP32SharpIR.h>


#define APDS9960_INT 2
#define I2C_SDA 21
#define I2C_SCL 22
#define I2C_FREQ 100000

APDS9960 sensor = APDS9960(Wire, APDS9960_INT);


#define IN1 16
#define IN2 17
#define IN3 18
#define IN4 19

#include "sdkconfig.h"
#include <Arduino.h>

#include <QTRSensors.h>

// make a line calibration (?) funciton , if setup isnt already one
//^ tomorrow work on it and finish it. Then, test at EER to later do wall sensor. yay. 
//test wall sensor on friday or thrusday if you are genuinely try hard :')


extern ControllerPtr myControllers[BP32_MAX_GAMEPADS];
int colorCollect[4];   // saved color
bool isCollected = false;
bool colorFound = false;


int r, g, b, a;

void setupColor() {
    Wire.begin(I2C_SDA, I2C_SCL, I2C_FREQ);
    sensor.setInterruptPin(APDS9960_INT);
    if (!sensor.begin()) {
        Serial.println("APDS9960 error");
        while (1);
    }
    Serial.begin(115200);
}

void ActivationC() {
    while (!sensor.colorAvailable()) {
        vTaskDelay(1);
    }
    sensor.readColor(r, g, b, a);
    colorCollect[0] = r;
    colorCollect[1] = g;
    colorCollect[2] = b;
    colorCollect[3] = a;
    isCollected = true;
    Serial.printf("Saved color: R=%d G=%d B=%d A=%d\n", r, g, b, a);
}

void moveForward() { //motor goes forward
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW); // flip 3 and 4 depending on arrangement of motors
}

void stopMotors() { //motor stops
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
}

void moveBackwards(){
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW); //same comment as moveForward
    digitalWrite(IN4, HIGH);
}

void turnLeft(){
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH); //if it in fact does not turn left, switch. be consist to changes made to mF
    digitalWrite(IN4, LOW);
}
void turnRight(){
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); //if it in fact does not turn right, switch. be consist to changes made to mF
    digitalWrite(IN4, LOW);
}


void foo(ControllerPtr myController) {
    BP32.update();

  
    if (myController->b() && !isCollected) {
        ActivationC();
        return;
    }

    if (myController->y() && isCollected) {
        Serial.println("Moving");
        colorFound = false;
        moveForward();

        while (!colorFound) {
            if (!sensor.colorAvailable()) {
                vTaskDelay(1);
                continue;
            }

            sensor.readColor(r, g, b, a);
            Serial.printf("R:%d G:%d B:%d\n", r, g, b);

            int tolerance = 15;
            bool match =
                abs(r - colorCollect[0]) < tolerance &&
                abs(g - colorCollect[1]) < tolerance &&
                abs(b - colorCollect[2]) < tolerance;

            if (match) {
                colorFound = true;
                stopMotors();
                Serial.println("Target color detected");
                isCollected = false;
            }
            vTaskDelay(10);
        }
    }
    if(myController->axisY() && myController->axisY() > 100){
        moveForward();
        return;
    }
    if(myController->axisY() && myController->axisY() < -100){ // adjust parameters if data cant be negative
        moveBackwards();
        return;
    }
    else{
        stopMotors();
    }
}


void setup() {
    Serial.begin(115200);

  //add new setup functions here
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

  
    setupColor();
    //Sensorsetup();

   
    BP32.setup(&onConnectedController, &onDisconnectedController);
    BP32.forgetBluetoothKeys();
    esp_log_level_set("gpio", ESP_LOG_ERROR); // suppress extra logs
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
