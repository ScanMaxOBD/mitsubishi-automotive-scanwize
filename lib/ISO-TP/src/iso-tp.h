#ifndef _ISOTP_H
#define _ISOTP_H

#include "mcp_can.h"

//#define ISO_TP_DEBUG
typedef enum {
  ISOTP_IDLE = 0,
  ISOTP_SEND,
  ISOTP_SEND_FF,
  ISOTP_SEND_CF,
  ISOTP_WAIT_FIRST_FC,
  ISOTP_WAIT_FC,
  ISOTP_WAIT_DATA,
  ISOTP_FINISHED,
  ISOTP_ERROR
} isotp_states_t;

#define CAN_DLEN 8  //Not extended CAN

/* N_PCI type values in bits 7-4 of N_PCI bytes */
#define N_PCI_SF  0x00  /* single frame */
#define N_PCI_FF  0x10  /* first frame */
#define N_PCI_CF  0x20  /* consecutive frame */
#define N_PCI_FC  0x30  /* flow control */
#define N_PCI_ERROR  0x40  /* Custom Defined Error */

#define FC_CONTENT_SZ 3 /* flow control content size in byte (FS/BS/STmin) */

/* Timeout values */
#define TIMEOUT_SESSION  2000 /* Timeout between successfull send and receive */
#define TIMEOUT_FC       250 /* Timeout between FF and FC or Block CF and FC */
#define TIMEOUT_CF       250 /* Timeout between CFs                          */
#define MAX_FCWAIT_FRAME  10

#define BLOCK_SIZE  0x08
#define SEPARATION_TIME  0x0A

class ISOTPClass {
	public: 
		ISOTPClass();
		virtual ~ISOTPClass();

		int begin();
  		void end();

		int setEcuFilter(uint32_t ecuAddres);
		int send (int ecuReqAddress, int subEcuAddress, int payloadLength, int *payload);
		int receive(int ecuReqAddress, int ecuResAddress, int subEcuAddress, uint8_t *buffer);
		int flowControl(int ecuReqAddress, int subEcuAddress, int blockSize, int seperationTime);
		int receiveOnly(int ecuResAddress, uint8_t *buffer);

	private:
		unsigned long _responseTimeout;
		unsigned long _lastResponseMillis;

        MCPCANClass *CAN;

		int frameType (int pciHeader);
		int totalNumberofResponseBytes (int pciHeaderOne, int pciHeaderTwo);
};

#endif