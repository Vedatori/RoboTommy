#include <Arduino.h>
#include <NewPing.h>
#include "Engine.h"
#include "StateVector.h"
#include "I2cDataProtocol.h"

#define ENGINE_LEFT_A 6
#define ENGINE_LEFT_B 9
#define ENGINE_RIGHT_A 10
#define ENGINE_RIGHT_B 11

#define SONAR_TRIGGER_PIN 7
#define SONAR_ECHO_PIN 8
#define SONAR_MAX_DISTANCE 100  //[cm]

#define I2C_ADDRESS 4
#define ACTIVE_MASTER_DELAY 300 //[ms]

Engine engineLeft(ENGINE_LEFT_A, ENGINE_LEFT_B);
Engine engineRight(ENGINE_RIGHT_A, ENGINE_RIGHT_B);

NewPing sonar(SONAR_TRIGGER_PIN, SONAR_ECHO_PIN, SONAR_MAX_DISTANCE);
StateVector stateVector;

void setup() {
    Serial.begin(115200);
    exSonarDist.setVarAddress(&stateVector.sonarDistance);
    exEngineLeftSpeed.setVarAddress(&stateVector.engineLeftSpeed);
    exEngineRightSpeed.setVarAddress(&stateVector.engineRightSpeed);
    I2cExchangeData::beginSlave(I2C_ADDRESS, ACTIVE_MASTER_DELAY);

}
void loop() {
    //Get inputs
    stateVector.sonarDistance = sonar.ping_cm();
    if(I2cExchangeData::isMasterActive() == false){
        stateVector.engineLeftSpeed = 0;
        stateVector.engineRightSpeed = 0;
    }

    //Set outputs
    engineLeft.setSpeed(stateVector.engineLeftSpeed);
    engineRight.setSpeed(stateVector.engineRightSpeed);
    Serial.println(stateVector.engineLeftSpeed);
    delay(20);
}