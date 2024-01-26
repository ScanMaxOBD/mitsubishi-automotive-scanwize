#include "SPI.h"
#include "mcp2515.h"

#include "mcp_can.h"


MCPCANClass::MCPCANClass(Stream *debugStream){
    _debugStream = debugStream;

    writeDebugStream(F("Initialization\n"));

    mcp2515 = new MCP2515(&spi);
    mcp2515->reset();
    mcp2515->setConfigMode();
}

MCPCANClass::~MCPCANClass(){
    writeDebugStream(F("Destroying mcp pointer\n"));
    delete mcp2515;
}

void MCPCANClass::setClock(CAN_CLOCK clock){
    canClock = clock;
}

MCPCANClass::ERROR MCPCANClass::connectCan(){
    MCP2515::ERROR error = mcp2515->setBitrate(bitrate, canClock);

    if (error != MCP2515::ERROR_OK){
        writeDebugStream(F("setBitrate error:\n"));
        writeDebugStream((int)error);
        writeDebugStream("\n");
        return ERROR_MCP2515_INIT_BITRATE;
    } 

    if (_loopback) {
        error = mcp2515->setLoopbackMode();
    } else if (_listenOnly) {
        error = mcp2515->setListenOnlyMode();
    } else {
        error = mcp2515->setNormalMode();
    }

    if (error != MCP2515::ERROR_OK) {
        return ERROR_MCP2515_INIT_SET_MODE;
    }

    _isConnected = true;
    return ERROR_OK;
}

MCPCANClass::ERROR MCPCANClass::disconnectCan() {
    _isConnected = false;
    mcp2515->setConfigMode();
    return ERROR_OK;
}

bool MCPCANClass::isConnected() {
    return _isConnected;
}

MCPCANClass::ERROR MCPCANClass::writeCan(const struct can_frame *frame) {
    if (mcp2515->sendMessage(frame) != MCP2515::ERROR_OK) {
        return ERROR_MCP2515_SEND;
    }

    return ERROR_OK;
}

MCPCANClass::ERROR MCPCANClass::pollReceiveCan(struct can_frame *frame) {
    if (!isConnected()) {
        return ERROR_NOT_CONNECTED;
    }

    while (mcp2515->checkReceive()) {
        
        if (mcp2515->readMessage(frame) != MCP2515::ERROR_OK) {
            return ERROR_MCP2515_READ;
        }
    }

    return ERROR_OK;
}

MCPCANClass::ERROR MCPCANClass::receiveCan(struct can_frame *frame) {
    if (!isConnected()) {
        return ERROR_NOT_CONNECTED;
    }

    MCP2515::ERROR result = mcp2515->readMessage(frame);
    if (result == MCP2515::ERROR_NOMSG) {
        return ERROR_OK;
    }

    if (result != MCP2515::ERROR_OK) {
        return ERROR_MCP2515_READ;
    }

    return ERROR_OK;
}

MCP2515 *MCPCANClass::getMcp2515() {
    return mcp2515;
}

uint16_t MCPCANClass::getTimestamp() {
    return millis() % TIMESTAMP_LIMIT;
}

MCPCANClass::ERROR MCPCANClass::processInterrupt(struct can_frame *frame) {
    if (!isConnected()) {
        writeDebugStream(F("Process interrupt while not connected\n"));
        return ERROR_NOT_CONNECTED;
    }

    uint8_t irq = mcp2515->getInterrupts();

    if (irq & MCP2515::CANINTF_ERRIF) {
        // reset RXnOVR errors
        mcp2515->clearRXnOVR();
    }

    if (irq & MCP2515::CANINTF_RX0IF) {
        ERROR error = receiveCan(frame);
        if (error != ERROR_OK) {
            return error;
        }
    }

    if (irq & MCP2515::CANINTF_RX1IF) {
        ERROR error = receiveCan(frame);
        if (error != ERROR_OK) {
            return error;
        }
    }

    /*if (irq & (MCP2515::CANINTF_TX0IF | MCP2515::CANINTF_TX1IF | MCP2515::CANINTF_TX2IF)) {
        _debugStream->print("MCP_TXxIF\r\n");
        //stopAndBlink(1);
    }*/



    if (irq & MCP2515::CANINTF_WAKIF) {
        _debugStream->print(F("MCP_WAKIF\r\n"));
        mcp2515->clearInterrupts();
    }

    if (irq & MCP2515::CANINTF_ERRIF) {
        _debugStream->print(F("ERRIF\r\n"));

        //return ERROR_MCP2515_MERRF;
        mcp2515->clearMERR();
    }

    if (irq & MCP2515::CANINTF_MERRF) {
        _debugStream->print(F("MERRF\r\n"));

        //return ERROR_MCP2515_MERRF;
        mcp2515->clearInterrupts();
    }

    return ERROR_OK;
}

MCPCANClass::ERROR MCPCANClass::setFilter(const uint32_t filter) {
    if (isConnected()) {
        writeDebugStream(F("Filter cannot be set while connected\n"));
        return ERROR_CONNECTED;
    }

    MCP2515::RXF filters[] = {MCP2515::RXF0, MCP2515::RXF1, MCP2515::RXF2, MCP2515::RXF3, MCP2515::RXF4, MCP2515::RXF5};
    for (int i=0; i<6; i++) {
        MCP2515::ERROR result = mcp2515->setFilter(filters[i], false, filter);
        if (result != MCP2515::ERROR_OK) {
            return ERROR_MCP2515_FILTER;
        }
    }

    return ERROR_OK;
}

MCPCANClass::ERROR MCPCANClass::setFilterMask(const uint32_t mask) {
    if (isConnected()) {
        writeDebugStream(F("Filter mask cannot be set while connected\n"));
        return ERROR_CONNECTED;
    }

    MCP2515::MASK masks[] = {MCP2515::MASK0, MCP2515::MASK1};
    for (int i=0; i<2; i++) {
        MCP2515::ERROR result = mcp2515->setFilterMask(masks[i], false, mask);
        if (result != MCP2515::ERROR_OK) {
            return ERROR_MCP2515_FILTER;
        }
    }

    return ERROR_OK;
}

MCPCANClass::ERROR MCPCANClass::writeDebugStream(const char character) {
    if (_debugStream != NULL) {
        _debugStream->write(character);
    }
    return ERROR_OK;
}

MCPCANClass::ERROR MCPCANClass::writeDebugStream(const char *buffer) {
    if (_debugStream != NULL) {
        _debugStream->print(buffer);
    }
    return ERROR_OK;
}

MCPCANClass::ERROR MCPCANClass::writeDebugStream(const __FlashStringHelper *ifsh) {
    if (_debugStream != NULL) {
        _debugStream->print(ifsh);
    }
    return ERROR_OK;
}

MCPCANClass::ERROR MCPCANClass::writeDebugStream(const uint8_t *buffer, size_t size) {
    if (_debugStream != NULL) {
        _debugStream->write(buffer, size);
    }
    return ERROR_OK;
}

MCPCANClass::ERROR MCPCANClass::writeDebugStream(const int buffer) {
    if (_debugStream != NULL) {
        _debugStream->print(buffer);
    }
    return ERROR_OK;
}

MCPCANClass::ERROR MCPCANClass::sendFrame(const struct can_frame *frame) {
    return this->writeCan(frame);
}

MCPCANClass::ERROR MCPCANClass::enableLoopback() {
    if (isConnected()) {
        writeDebugStream(F("Loopback cannot be changed while connected\n"));
        return ERROR_CONNECTED;
    }

    _loopback = true;

    return ERROR_OK;
}

MCPCANClass::ERROR MCPCANClass::disableLoopback() {
    if (isConnected()) {
        writeDebugStream(F("Loopback cannot be changed while connected\n"));
        return ERROR_CONNECTED;
    }

    _loopback = false;

    return ERROR_OK;
}

MCPCANClass::ERROR MCPCANClass::setEcuFilter(uint32_t ecuAddress){
    this->disconnectCan();
    //delay(100);
     mcp2515->reset();
     mcp2515->setConfigMode();
     mcp2515->setBitrate(bitrate, canClock);
    
    if (isConnected()) {
        writeDebugStream(F("Filter mask cannot be set while connected\n"));
        return ERROR_CONNECTED;
    }

    MCP2515::MASK masks[] = {MCP2515::MASK0, MCP2515::MASK1};
    for (int i=0; i<2; i++) {
        MCP2515::ERROR result = mcp2515->setFilterMask(masks[i], false, (ecuAddress & 0x7FF));
        if (result != MCP2515::ERROR_OK) {
            return ERROR_MCP2515_FILTER;
        }
    }

    MCP2515::RXF filters[] = {MCP2515::RXF0, MCP2515::RXF1, MCP2515::RXF2, MCP2515::RXF3, MCP2515::RXF4, MCP2515::RXF5};
    for (int i=0; i<6; i++) {
        MCP2515::ERROR result = mcp2515->setFilter(filters[i], false, (ecuAddress & 0x7FF));
        if (result != MCP2515::ERROR_OK) {
            return ERROR_MCP2515_FILTER;
        }
    }

    mcp2515->setNormalMode();
    _isConnected = true;
    
    
    return ERROR_OK;

}

