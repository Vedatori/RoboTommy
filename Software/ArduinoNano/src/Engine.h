#ifndef _ENGINE_H_
#define _ENGINE_H_

#include <Arduino.h>

#define START_TURN_PWM 65
#define START_TIME 100  //[ms]
#define KEEP_TURN_PWM 50    //Engines minimal uint8_t value for movement
#define ACCEL_K 255  //[desiredSpeed/second]
#define ACCEL_MAX_INCREMENT 30

enum EngineState{
    runningPositive = 0,
    startingPositive = 1,
    idle = 2,
    startingNegative = 3,
    runningNegative = 4
};

class Engine{
    uint8_t pinA, pinB;
    int16_t desiredSpeed = 0;
    int16_t outputSpeed = 0;
    int16_t prevOutputSpeed = 0;
    uint32_t startBeginTime = 0;
    uint32_t accelPrevTime = 0;
    EngineState engineState = idle;
public:
    Engine(uint8_t aPinA, uint8_t aPinB){
        pinA = aPinA;
        pinB = aPinB;
        pinMode(pinA, OUTPUT);
        pinMode(pinB, OUTPUT);
        digitalWrite(pinA, LOW);
        digitalWrite(pinB, LOW);
    }
    void setSpeed(int16_t aSpeed){
        if(aSpeed > 255)
            aSpeed = 255;
        else if(aSpeed < -255)
            aSpeed = -255;
        desiredSpeed = aSpeed;

        int16_t accelIncrement;
        switch(engineState){
            case runningPositive:
                if(desiredSpeed <= 0){
                    engineState = idle;
                    break;
                }
                outputSpeed = map(desiredSpeed, 1, 255, KEEP_TURN_PWM, 255);

                accelIncrement = ACCEL_K*(millis() - accelPrevTime)/1000;
                if(accelIncrement > ACCEL_MAX_INCREMENT)
                    accelIncrement = ACCEL_MAX_INCREMENT;
                if(outputSpeed > prevOutputSpeed + accelIncrement)
                    outputSpeed = prevOutputSpeed + accelIncrement;

                accelPrevTime = millis();
                break;
            case startingPositive:
                if(desiredSpeed <= 0)
                    engineState = idle;
                else if(millis() > startBeginTime + START_TIME)
                    engineState = runningPositive;

                outputSpeed = START_TURN_PWM;
                break;
            case idle:
                if(desiredSpeed > 0){
                    engineState = startingPositive;
                    startBeginTime = millis();
                }
                else if(desiredSpeed < 0){
                    engineState = startingNegative;
                    startBeginTime = millis();
                }
                outputSpeed = 0;
                break;
            case startingNegative:
                if(desiredSpeed >= 0)
                    engineState = idle;
                else if(millis() > startBeginTime + START_TIME)
                    engineState = runningNegative;

                outputSpeed = -START_TURN_PWM;
                break;
            case runningNegative:
                if(desiredSpeed >= 0){
                    engineState = idle;
                    break;
                }
                outputSpeed = map(desiredSpeed, -255, 1, -255, -KEEP_TURN_PWM);

                accelIncrement = ACCEL_K*(millis() - accelPrevTime)/1000;
                if(accelIncrement > ACCEL_MAX_INCREMENT)
                    accelIncrement = ACCEL_MAX_INCREMENT;
                if(outputSpeed < prevOutputSpeed - accelIncrement)
                    outputSpeed = prevOutputSpeed - accelIncrement;

                accelPrevTime = millis();
                break;
        }

        if(outputSpeed > 0){
            analogWrite(pinA, outputSpeed);
            analogWrite(pinB, 0);
        }
        else{
            analogWrite(pinA, 0);
            analogWrite(pinB, -outputSpeed);
        }
        prevOutputSpeed = outputSpeed;
    }
};


#endif /*_ENGINE_H_*/