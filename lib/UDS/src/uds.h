#ifndef UDS_H
#define UDS_H

#include "iso-tp.h"

typedef struct {
   String ecuName;
   String troubleCode;
   String dtcStatus; 
} DTCSTruct;


class UDSClass {
    private:
        ISOTPClass *ISOTP;
    public:
        UDSClass();
		virtual ~UDSClass();
        int begin();
        void end();
        int getVIn(int ecuReqAddress, int ecuResAddress, int subEcuAddress, uint8_t *buffer);
        int startDiagSession(int ecuReqAddress, int ecuResAddress, int subEcuAddress, uint8_t *buffer, int sessionId);
        int readDTC(int ecuReqAddress, int ecuResAddress, int subEcuAddress, uint8_t *buffer, int statusMask);
        int readDTCKWP(int ecuReqAddress, int ecuResAddress, int subEcuAddress, uint8_t *buffer);
        char decimalToASCII(int decimal);
        int readLiveData(int ecuReqAddress, int ecuResAddress, int subEcuAddress, uint8_t *buffer, int pid);
        int clearAllDTC(int ecuReqAddress, int ecuResAddress, int subEcuAddress, uint8_t *buffer);
        int readMileage(int ecuResAddress, uint8_t *buffer);
};

#endif