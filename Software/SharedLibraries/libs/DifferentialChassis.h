#ifndef _DIFFERENTIAL_CHASSSIS_H
#define _DIFFERENTIAL_CHASSSIS_H

#include <Arduino.h>
#include "PSD.h"

#define MAX_DIR_ERR 180.0
#define MIN_DIR_ERR -180.0
#define MAX_DIR_OUT 120.0
#define MIN_DIR_OUT -120.0

#define PSD_TRANSITION_1 -40
#define PSD_TRANSITION_2 -5
#define PSD_TRANSITION_3 5
#define PSD_TRANSITION_4 40
#define PSD_TRANSITION_5 150

VariableParamsPSD parameters[5] = {
    {1.0, 0.0, 0.3, 0.0, 0.0},  //Speed -100
    {0.5, 1.0, 0.2, 30.0, 3.0},  //Speed 0
    {0.6, 0.0, 0.15, 0.0, 0.0},  //Speed 10
    {1.2, 0.0, 0.15, 0.0, 0.0},  //Speed 100
    {3.0, 0.0, 0.3, 0.0, 0.0}   //Speed 200
};

class DifferentialChassis{
    int16_t desiredDirection = 0;
    int16_t desiredSpeed = 0;
    int16_t engineLeftSpeed = 0;
    int16_t engineRightSpeed = 0;
public:
    PSD * directionPSD;
    DifferentialChassis(){
        directionPSD = new PSD(&parameters[1], MAX_DIR_ERR, MIN_DIR_ERR, MAX_DIR_OUT, MIN_DIR_OUT);
    }
    void setDirSpeed(int16_t aDirection, int16_t aSpeed){
        if(aDirection > 180.0)
            aDirection = 180.0;
        else if(aDirection < -180.0)
            aDirection = -180.0;
        if(aSpeed > 255)
            aSpeed = 255;
        else if(aSpeed < -255)
            aSpeed = -255;
        desiredDirection = aDirection;
        desiredSpeed = aSpeed;

        if(desiredSpeed < PSD_TRANSITION_1)
            directionPSD->setParameters(&parameters[0]);
        else if(desiredSpeed < PSD_TRANSITION_2)
            directionPSD->setParameters(&parameters[2]);
        else if(desiredSpeed < PSD_TRANSITION_3)
            directionPSD->setParameters(&parameters[1]);
        else if(desiredSpeed < PSD_TRANSITION_4)
            directionPSD->setParameters(&parameters[2]);
        else if(desiredSpeed < PSD_TRANSITION_5)
            directionPSD->setParameters(&parameters[3]);
        else
            directionPSD->setParameters(&parameters[4]);
    }
    void updateOutput(float yaw){
        float artificialE = desiredDirection - yaw;
        if(artificialE > 180.0)
            artificialE = artificialE - 360.0;
        else if(artificialE < -180.0)
            artificialE = artificialE + 360.0;
        int16_t outputSteering = directionPSD->getRegAction(0, artificialE);
        int16_t slowDownCorrection = 0;
        if(desiredSpeed + abs(outputSteering) > 255)
            slowDownCorrection = desiredSpeed + abs(outputSteering) - 255;
        else if(desiredSpeed - abs(outputSteering) < -255)
            slowDownCorrection = desiredSpeed - abs(outputSteering) + 255;
        else
            slowDownCorrection = 0;
        engineLeftSpeed = desiredSpeed - outputSteering - slowDownCorrection;
        engineRightSpeed = desiredSpeed + outputSteering - slowDownCorrection;
    }
    int16_t getLeftSpeed(){
        return engineLeftSpeed;
    }
    int16_t getRightSpeed(){
        return engineRightSpeed;
    }
};

#endif /*_DIFFERENTIAL_CHASSSIS_H*/