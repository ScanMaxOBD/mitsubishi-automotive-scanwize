#ifndef MITSUBISHI_H
#define MITSUBISHI_H

#include "uds.h"

typedef struct {
    int ecuReqAddress;
    int ecuResAddress;
    String ecuName;
} ECUStruct;


enum {
  ENGINE_COOLANT_TEMPERATURE                        = 0x05,
  ENGINE_RPM                                        = 0x0c,
  VEHICLE_SPEED                                     = 0x0d,
  AIR_INTAKE_TEMPERATURE                            = 0x0f,
  MAF_AIR_FLOW_RATE                                 = 0x10,
  THROTTLE_POSITION                                 = 0x11,
  RUN_TIME_SINCE_ENGINE_START                       = 0x1f,
  DISTANCE_TRAVELED_WITH_MIL_ON                     = 0x21,
  DISTANCE_TRAVELED_SINCE_CODES_CLEARED             = 0x31,
  CONTROL_MODULE_VOLTAGE                            = 0x42,
  TIME_RUN_WITH_MIL_ON                              = 0x4d,
  TIME_SINCE_TROUBLE_CODES_CLEARED                  = 0x4e,
};


class MitsubishiClass {
    public:
        MitsubishiClass();
        virtual ~MitsubishiClass();

        int diagInit();
        String readAllEcuDtcs(String deviceId);
        String getVinRead();
        String clearAllDTC(String deviceId);
        String decodeDTC(byte highByte, byte lowByte);
        String liveData(String deviceId, String firmwareVersion, float latitude = 0.0, float longitude = 0.0 );
        bool ecuOn();
        float batteryVoltage();

    private:
        UDSClass *UDS;

};

#endif