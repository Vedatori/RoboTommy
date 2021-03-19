#ifndef _STATE_VECTOR_
#define _STATE_VECTOR_

#include <Arduino.h>

struct StateVector{
    //INPUTS
    uint8_t sonarDistance = 0;

    //OUTPUTS
    int16_t engineLeftSpeed = 0;
    int16_t engineRightSpeed = 0;

};
#endif /*_STATE_VETOR_*/