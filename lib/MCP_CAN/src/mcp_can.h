#ifndef _MCP_CAN_H
#define _MCP_CAN_H

#include "Arduino.h"

#include "mcp2515.h"

class MCPCANClass {
	public: 
         enum ERROR {
            ERROR_OK,
            ERROR_CONNECTED,
            ERROR_NOT_CONNECTED,
            ERROR_UNKNOWN_COMMAND,
            ERROR_INVALID_COMMAND,
            ERROR_ERROR_FRAME_NOT_SUPPORTED,
            ERROR_BUFFER_OVERFLOW,
            ERROR_SERIAL_TX_OVERRUN,
            ERROR_LISTEN_ONLY,
            ERROR_MCP2515_INIT,
            ERROR_MCP2515_INIT_CONFIG,
            ERROR_MCP2515_INIT_BITRATE,
            ERROR_MCP2515_INIT_SET_MODE,
            ERROR_MCP2515_SEND,
            ERROR_MCP2515_READ,
            ERROR_MCP2515_FILTER,
            ERROR_MCP2515_ERRIF,
            ERROR_MCP2515_MERRF
        };

		MCPCANClass(Stream *debugStream);
		virtual ~MCPCANClass();

		ERROR connectCan();
        ERROR disconnectCan();

        void setClock(const CAN_CLOCK clock);

        ERROR sendFrame(const struct can_frame *);
        ERROR enableLoopback();
        ERROR disableLoopback();
        ERROR pollReceiveCan(struct can_frame *);
        ERROR receiveCan(struct can_frame *);
        MCP2515 *getMcp2515();
        ERROR processInterrupt(struct can_frame *);
        ERROR setEcuFilter(uint32_t ecuAddress);
        bool isConnected();
    
        

	private:

        bool _listenOnly = false;
        bool _loopback = false;

        static const uint16_t TIMESTAMP_LIMIT = 0xEA60;
        static const char CR  = '\r';
        
        CAN_CLOCK canClock = MCP_8MHZ;
        bool _timestampEnabled = false;
        spi_device_handle_t spi;
        MCP2515 *mcp2515;
        CAN_SPEED bitrate = CAN_500KBPS;
        bool _isConnected = false;
        Stream *_stream;
        Stream *_debugStream;


        uint16_t getTimestamp();

        ERROR writeCan(const struct can_frame *);
        ERROR setFilter(const uint32_t filter);
        ERROR setFilterMask(const uint32_t mask);
        
       
        ERROR writeDebugStream(const char character);
        ERROR writeDebugStream(const char *buffer);
        ERROR writeDebugStream(const int buffer);
        ERROR writeDebugStream(const uint8_t *buffer, size_t size);
        ERROR writeDebugStream(const __FlashStringHelper *ifsh);
};

#endif