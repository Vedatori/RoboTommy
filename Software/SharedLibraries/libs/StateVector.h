#ifndef _STATE_VECTOR_
#define _STATE_VECTOR_

#include <Arduino.h>

struct StateVector{
    //INPUTS
    int16_t joystickX = 0;
    int16_t joystickY = 0;
	uint8_t sonarDistance = 0;
    float ypr[3] = {0.0, };
    float speed = 0;
    float direction = 0;
    
    //OUTPUTS
    int16_t engineLeftSpeed = 0;
    int16_t engineRightSpeed = 0;

};
#endif /*_STATE_VETOR_*/