#include "uds.h"


UDSClass::UDSClass(){
    ISOTP = new ISOTPClass();
}

UDSClass::~UDSClass(){
    delete ISOTP;
}


int UDSClass::begin(){
    return ISOTP->begin();
}


void UDSClass::end(){
    ISOTP->end();
}


char UDSClass::decimalToASCII(int decimal){
    // Make sure the decimal value is within the valid ASCII range
    decimal = constrain(decimal, 0, 127);
    // Convert decimal to ASCII character
    return (char)decimal;
}

int UDSClass::startDiagSession(int ecuReqAddress, int ecuResAddress, int subEcuAddress, uint8_t *buffer, int sessionId){
    ISOTP->setEcuFilter(ecuResAddress);
    delay(60);
    int payloadLength = 2;
    int payload[] = {0x10, sessionId};
    if(ISOTP->send(ecuReqAddress, subEcuAddress, payloadLength, payload) == 0){
       if(ISOTP->receive(ecuReqAddress, ecuResAddress, subEcuAddress, buffer) == 3){
            delay(5);
            ISOTP->receive(ecuReqAddress, ecuResAddress, subEcuAddress, buffer);
        }
       return 1; 
    }

    return 0;
}


int UDSClass::getVIn(int ecuReqAddress, int ecuResAddress, int subEcuAddress, uint8_t *buffer){
    ISOTP->setEcuFilter(ecuResAddress);
    delay(60);
    int payloadLength = 3;
    int payload[] = {0x22, 0xF1, 0x90};
    if(ISOTP->send(ecuReqAddress, subEcuAddress, payloadLength, payload) == 0){
        if(ISOTP->receive(ecuReqAddress, ecuResAddress, subEcuAddress, buffer) == 3){
            delay(5);
            ISOTP->receive(ecuReqAddress, ecuResAddress, subEcuAddress, buffer);
        }
        delay(100);   
        return 1; 
    }

    return 0;
}


int UDSClass::readDTC(int ecuReqAddress, int ecuResAddress, int subEcuAddress, uint8_t *buffer, int statusMask){
    ISOTP->setEcuFilter(ecuResAddress);
    delay(60);
    int payloadLength = 3;
    int payload[] = {0x19, 0x02, statusMask};

    if(ISOTP->send(ecuReqAddress, subEcuAddress, payloadLength, payload) == 0){
        if(ISOTP->receive(ecuReqAddress, ecuResAddress, subEcuAddress, buffer) == 3){
            delay(5);
            ISOTP->receive(ecuReqAddress, ecuResAddress, subEcuAddress, buffer);
        }
        
        delay(100);
        return 1;
    }  

    return 0;
}


int UDSClass::readDTCKWP(int ecuReqAddress, int ecuResAddress, int subEcuAddress, uint8_t *buffer){
    ISOTP->setEcuFilter(ecuResAddress);
    delay(60);
    int payloadLength = 4;
    int payload[] = { 0x18, 0x00, 0xFF, 0x00 };

    if(ISOTP->send(ecuReqAddress, subEcuAddress, payloadLength, payload) == 0){
        if(ISOTP->receive(ecuReqAddress, ecuResAddress, subEcuAddress, buffer) == 3){
            delay(5);
            ISOTP->receive(ecuReqAddress, ecuResAddress, subEcuAddress, buffer);
        }
        
        delay(100);
        return 1;
    }  

    return 0;
}


int UDSClass::readLiveData(int ecuReqAddress, int ecuResAddress, int subEcuAddress, uint8_t *buffer, int pid){
    ISOTP->setEcuFilter(ecuResAddress);
    delay(60);
    int payloadLength = 2;
    int payload[] = { 0x01, pid };
    if(ISOTP->send(ecuReqAddress, subEcuAddress, payloadLength, payload) == 0){
        if(ISOTP->receive(ecuReqAddress, ecuResAddress, subEcuAddress, buffer) == 3){
            delay(5);
            ISOTP->receive(ecuReqAddress, ecuResAddress, subEcuAddress, buffer);
        }
        delay(100);
        return 1;
    }

    return 0;
}

int UDSClass::clearAllDTC(int ecuReqAddress, int ecuResAddress, int subEcuAddress, uint8_t *buffer){
    ISOTP->setEcuFilter(ecuResAddress);
    delay(60);
    int payloadLength = 3;
    int payload[] = { 0x14, 0xFF, 0x00 };
    
    if(ISOTP->send(ecuReqAddress, subEcuAddress, payloadLength, payload) == 0){
        if(ISOTP->receive(ecuReqAddress, ecuResAddress, subEcuAddress, buffer) == 3){
            delay(5);
            ISOTP->receive(ecuReqAddress, ecuResAddress, subEcuAddress, buffer);
        }
        delay(100);
        return 1;
    }

    return 0;
}


int UDSClass::readMileage(int ecuResAddress, uint8_t *buffer){
    ISOTP->setEcuFilter(ecuResAddress);
    delay(60);
    
    int mileageResponse = ISOTP->receiveOnly(ecuResAddress, buffer);
    delay(100);
    return mileageResponse;
}



