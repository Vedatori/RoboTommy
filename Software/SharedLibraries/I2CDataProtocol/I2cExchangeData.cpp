#include <Arduino.h>
#include <Wire.h>
#include "I2cExchangeData.h"

uint8_t I2cExchangeData::varCounter = 0;
uint8_t I2cExchangeData::destAddress = 0;
bool I2cExchangeData::isMaster = true;
I2cExchangeData * I2cExchangeData::firstDataPtr = NULL;
uint8_t I2cExchangeData::toSlaveLength = 0;
uint8_t I2cExchangeData::toMasterLength = 0;
uint32_t I2cExchangeData::lastReceivedTime = 0;
uint16_t I2cExchangeData::masterActiveDelay = 0;

I2cExchangeData::I2cExchangeData(uint8_t aVarLength, DataDestination aDataDestination){
    if(aVarLength == 0)
        return; //error
    varLength = aVarLength;
    dataDestination = aDataDestination;

    I2cExchangeData * iterateDataPtr = firstDataPtr;
    if(firstDataPtr == NULL)
        firstDataPtr = this;
    else{
        while(iterateDataPtr->nextDataPtr != NULL)
            iterateDataPtr = iterateDataPtr->nextDataPtr;
        iterateDataPtr->nextDataPtr = this;
    }
    varID = varCounter++;
}
void I2cExchangeData::beginMaster(uint8_t aAddress){
    if(aAddress > 127)
        return; //error
    isMaster = true;
    destAddress = aAddress;
    I2cExchangeData::countExDataLengths();
    Wire.begin();
    Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties
}
void I2cExchangeData::beginSlave(uint8_t aAddress, uint16_t aMasterActiveDelay){
    if(aAddress > 127)
        return; //error
    masterActiveDelay = aMasterActiveDelay;
    isMaster = false;
    I2cExchangeData::countExDataLengths();
    Wire.begin(aAddress);
    Wire.onReceive(receiveData);
    Wire.onRequest(sendData);
}
void I2cExchangeData::exchange(){
    if(isMaster == false)
        return; //only master initiates exchange
    if(firstDataPtr == NULL)
        return; //nothing to exchange

    I2cExchangeData * iterateDataPtr = firstDataPtr;

    //MASTER -> SLAVE
    Wire.beginTransmission(destAddress);
    while(iterateDataPtr != NULL){
        if(iterateDataPtr->dataDestination == toSlave){
            Wire.write(iterateDataPtr->varID);
            Wire.write((byte *)iterateDataPtr->varPtr, iterateDataPtr->varLength);
        }
        iterateDataPtr = iterateDataPtr->nextDataPtr;
    }
    Wire.endTransmission(); //transmit data

    //SLAVE -> MASTER
    Wire.requestFrom(destAddress, toMasterLength);
    uint32_t masterSlaveBeginTime = millis();
    while(Wire.available() < toMasterLength)
    {
        delay(1);
        if(millis() > masterSlaveBeginTime + MASTER_WAIT_FOR_SLAVE) //incoming message not received completely
            return;
    }
        
    iterateDataPtr = firstDataPtr;
    while(iterateDataPtr != NULL){
        if(iterateDataPtr->dataDestination == toMaster){
            Wire.read();    //varID
            Wire.readBytes((byte *)iterateDataPtr->varPtr, iterateDataPtr->varLength);
        }
        iterateDataPtr = iterateDataPtr->nextDataPtr;
    }
}
void I2cExchangeData::countExDataLengths(){
    if(firstDataPtr == NULL)
        return; //nothing to exchange
    I2cExchangeData * iterateDataPtr = firstDataPtr;
    //Master -> Slave
    while(iterateDataPtr != NULL){
        if(iterateDataPtr->dataDestination == toSlave){
            toSlaveLength += (iterateDataPtr->varLength + 1);
        }
        iterateDataPtr = iterateDataPtr->nextDataPtr;
    }
    iterateDataPtr = firstDataPtr;
    //Slave -> Master
    while(iterateDataPtr != NULL){
        if(iterateDataPtr->dataDestination == toMaster){
            toMasterLength += (iterateDataPtr->varLength + 1);
        }
        iterateDataPtr = iterateDataPtr->nextDataPtr;
    }
}
void I2cExchangeData::receiveData(size_t dataSize){
    if(dataSize < toSlaveLength)
        return; //error, not enough data came
    if(firstDataPtr == NULL)
        return; //nothing to exchange
    I2cExchangeData * iterateDataPtr = firstDataPtr;
    while(iterateDataPtr != NULL){
        if(iterateDataPtr->dataDestination == toSlave){
            Wire.read();    //varID
            Wire.readBytes((byte *)iterateDataPtr->varPtr, iterateDataPtr->varLength);
        }
        iterateDataPtr = iterateDataPtr->nextDataPtr;
    }
    lastReceivedTime = millis();
}
void I2cExchangeData::sendData(){
    if(firstDataPtr == NULL)
        return; //nothing to exchange
    I2cExchangeData * iterateDataPtr = firstDataPtr;
    while(iterateDataPtr != NULL){
        if(iterateDataPtr->dataDestination == toMaster){
            Wire.write(iterateDataPtr->varID);
            Wire.write((byte *)iterateDataPtr->varPtr, iterateDataPtr->varLength);
        }
        iterateDataPtr = iterateDataPtr->nextDataPtr;
    }
}