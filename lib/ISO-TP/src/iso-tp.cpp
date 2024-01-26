#include "iso-tp.h"

ISOTPClass::ISOTPClass() :
    _responseTimeout(TIMEOUT_SESSION),
    _lastResponseMillis(0)
{
    Stream *debugStream = &Serial;
    CAN = new MCPCANClass(debugStream);
}

ISOTPClass::~ISOTPClass(){
    delete CAN;
}

int ISOTPClass::begin(){
    return CAN->connectCan();
}

void ISOTPClass::end(){
   CAN->disconnectCan();
}

int ISOTPClass::receiveOnly(int ecuResAddress, uint8_t *buffer){
    struct can_frame canResMsg;
    for (unsigned long start = millis(); (millis() - start) < 5000;) {
         if(CAN->pollReceiveCan(&canResMsg) == 0){ 
             if(canResMsg.can_id == ecuResAddress){
                for(int i = 0; i < 3; i++){
                    buffer[i] = canResMsg.data[i];
                }
                return 1;
             }
         }
    }
    return 0;
}

int ISOTPClass::send(int ecuReqAddress, int subEcuAddress, int payloadLength, int *payload){
    // make sure at least 60 ms have passed since the last response
    unsigned long lastResponseDelta = millis() - _lastResponseMillis;
    if (lastResponseDelta < 60) {
        delay(60 - lastResponseDelta);
    }
   
    struct can_frame canMsg;
   
    canMsg.can_id = ecuReqAddress;
    canMsg.can_dlc = 8;

    if(subEcuAddress == 0xFF){
        canMsg.data[0] = payloadLength;
        for(int i = 0; i < payloadLength; i++){
            canMsg.data[i+1] = payload[i];
        }

        for (int i = payloadLength+1; i < 8; i++){
            canMsg.data[i] = 0xFF;
        }
    }else {
        canMsg.data[0] = subEcuAddress;
        canMsg.data[1] = payloadLength;
        for(int i = 0; i < payloadLength+1; i++){
            canMsg.data[i+2] = payload[i];
        }

        for (int i = payloadLength+2; i < 8; i++){
            canMsg.data[i] = 0xFF;
        }
    }

    return  CAN->sendFrame(&canMsg);

}

int ISOTPClass::flowControl(int ecuReqAddress, int subEcuAddress, int blockSize, int seperationTime){
    delay(2);

    struct can_frame canFCMsg;
   
    canFCMsg.can_id = ecuReqAddress;
    canFCMsg.can_dlc = 8;

    if(subEcuAddress == 0xFF){
        canFCMsg.data[0] = 0x30;
        canFCMsg.data[1] = blockSize;
        canFCMsg.data[2] = seperationTime;

        for (int i = 3; i < 8; i++){
            canFCMsg.data[i] = 0xFF;
        }
    } else {
        canFCMsg.data[0] = subEcuAddress;
        canFCMsg.data[1] = 0x30;
        canFCMsg.data[2] = blockSize;
        canFCMsg.data[3] = seperationTime;

        for (int i = 4; i < 8; i++){
            canFCMsg.data[i] = 0xFF;
        }
    }
   
    
    return CAN->sendFrame(&canFCMsg);
}

int ISOTPClass::frameType (int pciHeader){
   
    int upperNibble = ((pciHeader >> 4) & 0x0F) << 4;  // Retrieve the upper nibble

    int typeOfFrame;

    switch (upperNibble)
    {
    case N_PCI_SF:
        typeOfFrame = N_PCI_SF;
        break;
    case N_PCI_FF:
        typeOfFrame = N_PCI_FF;
        break;
    case N_PCI_CF:
        typeOfFrame = N_PCI_CF;
        break;
    case N_PCI_FC:
        typeOfFrame = N_PCI_FC;
        break;
    
    default:
        typeOfFrame = N_PCI_ERROR; //Custom defined value to show error
        break;
    }

    return typeOfFrame;
}

int ISOTPClass::totalNumberofResponseBytes (int pciHeaderOne, int pciHeaderTwo) {
    //int totalBytes = (int)(((pciHeaderOne >> 4) & 0x0F) << 8) | pciHeaderTwo;
    return pciHeaderTwo;
}

int ISOTPClass::receive(int ecuReqAddress, int ecuResAddress, int subEcuAddress, uint8_t *buffer){
    delay(10);
    bool hasSubEcu = subEcuAddress != 0xFF ? true : false;
    //hasSubEcu ? Serial.println("Has Sub Ecu") : Serial.println("No SUb ECU");

    struct can_frame canResMsg;
    for (unsigned long start = millis(); (millis() - start) < _responseTimeout;) {
   
        if(CAN->pollReceiveCan(&canResMsg) == 0){
             _lastResponseMillis = millis();

            int pciHeader = hasSubEcu ? canResMsg.data[1] : canResMsg.data[0];
            //Check if response is a Single Frame
            int responseFrame = this->frameType(pciHeader);
            if(responseFrame == N_PCI_SF){
                if(canResMsg.can_id == ecuResAddress){
                    // Serial.print(canResMsg.can_id, HEX); // print ID
                    // Serial.print(" "); 
                    // Serial.print(canResMsg.can_dlc, HEX); // print DLC
                    // Serial.print(" ");
                    
                    // for (int i = 0; i<canResMsg.can_dlc; i++)  {  // print the data
                    // Serial.print(canResMsg.data[i],HEX);
                    // Serial.print(" ");
                    // }
                    // Serial.println();  

                    int errorResponse = hasSubEcu ? canResMsg.data[2] : canResMsg.data[1];  
                    int waitResponse = hasSubEcu ? canResMsg.data[4] : canResMsg.data[3];
                    if(errorResponse == 0x7F && waitResponse == 0x78) return 3;
                    for(int i = 0; i <= pciHeader; i++){
                        buffer[i] = hasSubEcu ?  canResMsg.data[i+1] : canResMsg.data[i];
                    }
                }

                return 1;
            }else if(responseFrame == N_PCI_FF){
                if(canResMsg.can_id == ecuResAddress){
                    int errorResponse = hasSubEcu ? canResMsg.data[3] : canResMsg.data[2];
                    int waitResponse = hasSubEcu ? canResMsg.data[5] : canResMsg.data[4];
                    if(errorResponse == 0x7F && waitResponse == 0x78) return 3;
                    //get actual message length
                    int pciHeaderTwo = hasSubEcu ? canResMsg.data[2] : canResMsg.data[1];
                    int totalBytes = totalNumberofResponseBytes(pciHeader, pciHeaderTwo);

                    //Store the bytes on the first frame.
                    int firstFrameByteLength = hasSubEcu ? 6 : 7;
                    
                    //store the first frame bytes
                    for(int i = 0; i < firstFrameByteLength; i++){
                        buffer[i] = canResMsg.data[i+1];
                    }
                    //Send FC

                    float fcCountFloat =  hasSubEcu ? ((float)totalBytes-5.00)/6.00 : ((float)totalBytes-6.00)/7.00;

                    int fcCount =ceil(fcCountFloat);

                    for(int i = 1; i <= fcCount; i++){
                        this->flowControl(ecuReqAddress, subEcuAddress, BLOCK_SIZE, SEPARATION_TIME);

                        struct can_frame canResMsg;
                        for (unsigned long start = millis(); (millis() - start) < _responseTimeout;) { 
                            if(CAN->pollReceiveCan(&canResMsg) == 0 && canResMsg.can_id == ecuResAddress && canResMsg.data[0] == (0x20+i)){
                                int index = hasSubEcu ? i*6 : i*7;
                                for(int bufferCount = hasSubEcu ? 1 : 0; bufferCount < 7; bufferCount++){
                                    buffer[index++] = canResMsg.data[bufferCount+1];
                                }
                                break;
                            }
                        }
                    }

                    return 2;
                }                                 
            }
        }
    } 

    return 0;

}


int ISOTPClass::setEcuFilter(uint32_t ecuAddress){
    return CAN->setEcuFilter(ecuAddress);
}
