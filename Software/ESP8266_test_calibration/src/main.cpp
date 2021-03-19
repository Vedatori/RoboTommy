#include <Arduino.h>
#include <Wire.h>
#include "I2cDataProtocol.h"
#include "Gyroscope.h"
#include "DifferentialChassis.h"
#include "DisplayClass.h"
#include "Strings.h"
#include "StateVector.h"
#include "Node.h"
#include "EEPROM_data.h"
#include "GyroCalibration.h"

#define I2C_ADDRESS_NANO 4
#define CONTROL_PERIOD 10
#define DISPLAY_HOLD_TIME 3000

uint32_t prevControlTime = 0;
uint16_t serOutCounter = 0;

StateVector stateVector;
DifferentialChassis chassis;
DisplayClass displayOLED;

int16_t gyroOffsets[6] = {0, }; //xAccel, yAccel, zAccel, xGyro, yGyro, zGyro

EEPROM_data gyroOffsetsEEPROM(gyroOffsets, 6*sizeof(gyroOffsets[0]));

void setup() {
    Serial.begin(115200);
    delay(1500);    //needed for correct i2c startup
    exEngineLeftSpeed.setVarAddress(&(stateVector.engineLeftSpeed));
    exEngineRightSpeed.setVarAddress(&(stateVector.engineRightSpeed));
    chassis.setDirSpeed(0, 0);
    I2cExchangeData::beginMaster(I2C_ADDRESS_NANO);
    displayOLED.init();
    displayOLED.writeWelcome();
    delay(1500);
    displayOLED.writeString("Place Robot on a narrow plane");
    delay(2000);
    if(initTestGyro())
        displayOLED.writeString("MPU connected");
    else{
        displayOLED.writeString("MPU connection failed");
        while(1)
            delay(10);
    }
    delay(1500);
    displayOLED.writeString("Calibration running\nShould take max 1 min");
    gyroCalibration(gyroOffsets);
    gyroOffsetsEEPROM.write();
    displayOLED.writeString("Calibration done");
    gyroOffsetsEEPROM.read();
    Serial.print("Gyro offsets \t");
    Serial.print("\t");
    Serial.print(gyroOffsets[0]);
    Serial.print("\t");
    Serial.print(gyroOffsets[1]);
    Serial.print("\t");
    Serial.print(gyroOffsets[2]);
    Serial.println();
}

void loop() {
}