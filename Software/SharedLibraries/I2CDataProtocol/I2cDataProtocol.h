#ifndef _I2CDATAPROTOCOL_H_
#define _I2CDATAPROTOCOL_H_

#include <Arduino.h>
#include "I2cExchangeData.h"

I2cExchangeData exSonarDist(1, toMaster);
I2cExchangeData exEngineLeftSpeed(2, toSlave);
I2cExchangeData exEngineRightSpeed(2, toSlave);

#endif /*_I2CDATAPROTOCOL_H_*/
