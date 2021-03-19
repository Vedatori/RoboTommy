#ifndef _I2CEXCHANGEDATA_H_
#define _I2CEXCHANGEDATA_H_

#include <Arduino.h>
#include <Wire.h>

#define MASTER_WAIT_FOR_SLAVE 10

enum DataDestination{
    toMaster = 0,
    toSlave = 1
};

class I2cExchangeData{
    void * varPtr = NULL;
    uint8_t varLength = 0;
    uint8_t varID = 0;
    static uint8_t varCounter;
    static uint8_t destAddress;
    static bool isMaster;
    static uint8_t toSlaveLength;
    static uint8_t toMasterLength;
    static I2cExchangeData * firstDataPtr;
    I2cExchangeData * nextDataPtr = NULL;
    DataDestination dataDestination = toSlave;
    static uint32_t lastReceivedTime;
    static uint16_t masterActiveDelay;

    static void receiveData(size_t dataSize);  //Slave receives data from master
    static void sendData();     //Slave sends desired data to master
    static void countExDataLengths();
public:
    I2cExchangeData(uint8_t aVarLength, DataDestination aDataDestination);
    static void beginMaster(uint8_t aAddress);
    static void beginSlave(uint8_t aAddress, uint16_t aMasterActiveDelay);
    void setVarAddress(void * aVarPtr){
        if(aVarPtr == NULL)
            return; //error
        varPtr = aVarPtr;
    }
    static void exchange();     //Master initiates data exchange
    static bool isMasterActive(){
        if(millis() < lastReceivedTime + masterActiveDelay)
            return true;
        else
            return false;
    }
};

#endif /*_I2CEXCHANGEDATA_H_*/