#ifndef _PSD_
#define _PSD_

#include <Arduino.h>

struct VariableParamsPSD{
    float kp;
    float ks;
    float kd;
    float sum_saturation;
    float neutral_dead_input;
};

class PSD{
    float kP;   //proportional gain
    float kS;   //summative gain
    float kD;   //differential gain

    float e = 0.0;  //regulation error
    float prevE = 0.0;  //previous regulation error
    float x = 0.0;  //regulation action

    float sum = 0.0;
    float sumSaturation = 0.0;
    float diff = 0.0;
    float maxW = 0.0;
    float minW = 0.0;
    float deadBandW = 0.0;
    float maxX = 0.0;
    float minX = 0.0;
    

    uint32_t prevTime = 0;
    float timeDiff; //[sec]
public:
    PSD(float aKP, float aKS, float aKD, float aSumSaturation, float aMaxW, float aMinW, float aDeadBandW, float aMaxX, float aMinX){
        kP = aKP;
        kS = aKS;
        kD = aKD;
        sumSaturation = aSumSaturation;
        maxW = aMaxW;
        minW = aMinW;
        deadBandW = aDeadBandW;
        maxX = aMaxX;
        minX = aMinX;
    }
    PSD(VariableParamsPSD * aVarParams, float aMaxW, float aMinW, float aMaxX, float aMinX){
        kP = aVarParams->kp;
        kS = aVarParams->ks;
        kD = aVarParams->kd;
        sumSaturation = aVarParams->sum_saturation;
        deadBandW = aVarParams->neutral_dead_input;
        maxW = aMaxW;
        minW = aMinW;
        maxX = aMaxX;
        minX = aMinX;
    }
    float getRegAction(float aW, float aY){
        if(aW > maxW)
            aW = maxW;
        else if(aW < minW)
            aW = minW;
        e = aW - aY;

        timeDiff = (millis() - prevTime)/1000.0;
        prevTime = millis();

        sum += e*timeDiff;
        if(sum > sumSaturation)
            sum = sumSaturation;
        else if(sum < -sumSaturation)
            sum = -sumSaturation;

        diff = (e - prevE)/timeDiff;
        prevE = e;

        if(e < deadBandW && e > -deadBandW){
            x = 0;
            sum = 0;
        }
        else
            x = kP*e + kS*sum + kD*diff;

        if(x > maxX)
            x = maxX;
        else if(x < minX)
            x = minX;
        return x;
    }
    void reset(){
        sum = 0.0;
        prevE = 0;
    }
    void printComponents(){
        Serial.print(x);
        Serial.print("\t");
        Serial.print(kP*e);
        Serial.print("\t");
        Serial.print(kS*sum);
        Serial.print("\t");
        Serial.print(kD*diff);
        Serial.println();
    }
    void setParameters(VariableParamsPSD * aVarParams){
        kP = aVarParams->kp;
        kS = aVarParams->ks;
        kD = aVarParams->kd;
        sumSaturation = aVarParams->sum_saturation;
        deadBandW = aVarParams->neutral_dead_input;
    }
};

#endif /*_PSD_*/