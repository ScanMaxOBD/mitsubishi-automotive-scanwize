#include "mitsubishi.h"
#include <ArduinoJson.h>

ECUStruct MitsubishiEcus[] = {
  { 0x7E0, 0x7E8, "MPI/GDI/DIESEL"},
  { 0x6E0, 0x51C, "SRS-Air Bag "},
  { 0x620, 0x504, "ETACS"},
  { 0x718, 0x719, "ACC/FCM"},
  { 0x7E2, 0x7EA, "PHEV"},
  { 0x763, 0x764, "CORNER SENSOR/SONAR"},
  { 0x796, 0x797, "4WS/Electric Power Steering"},
  { 0x784, 0x785, "ABS/ASC/ASTC/WSS"},
  { 0x688, 0x511, "Air Conditioner"},
  { 0x71A, 0x71B, "Lane Keep"},
  { 0x600, 0x500, "KOS/Immo/Keyless"},
  { 0x724, 0x725, "Compressor"},
  { 0x6A0, 0x514, "Meter"},
  { 0x790, 0x532, "Multi Around Monitor"},
  { 0x765, 0x766, "OBC"},
  { 0x761, 0x762, "BMU"},
  { 0x755, 0x756, "F-MCU"},
  { 0x753, 0x754, "R-MCU"},
  { 0x73C, 0x73D, "GCU"},
  { 0x773, 0x774, "OSS"},
};

MitsubishiClass::MitsubishiClass(){
  UDS = new UDSClass();
}

MitsubishiClass::~MitsubishiClass(){
  delete UDS;
}

int MitsubishiClass::diagInit(){
  return UDS->begin();
}

String MitsubishiClass::readAllEcuDtcs(String deviceId){
    int numECUs = sizeof(MitsubishiEcus) / sizeof(MitsubishiEcus[0]);
    // Serial.print("Total ECUs: ");
    // Serial.println(numECUs);

    DynamicJsonDocument doc(1024);
    doc["deviceId"] = deviceId;
    
    for (int i = 0; i < numECUs; i++) {
        if(!this->ecuOn()) return "Off"; 
        String combinedDtcs = "";
        bool moduleEquipped;
        bool hasErrors;

        Serial.print("Scanning ECU : ");
        Serial.print(MitsubishiEcus[i].ecuName);
        Serial.println();

        uint8_t buffer[64];
        memset(buffer, 0x55, sizeof(uint8_t)*64);

        int sizeOfBuffer = sizeof(buffer) / sizeof(buffer[0]);

        UDS->readDTCKWP(MitsubishiEcus[i].ecuReqAddress, MitsubishiEcus[i].ecuResAddress, 0xFF, buffer);

        if(buffer[0] == 0x55){
          //Serial.println("Module not equiped");
          moduleEquipped = false;
          //break;
        }else if(buffer[1] == 0x58 && buffer[2] == 0x00){
          moduleEquipped = true;
          hasErrors = false;
        }else if(buffer[1] == 0x58 && buffer[2] > 0x00){
          moduleEquipped = true;
          hasErrors = true;

          for(int dtcCount = 0; dtcCount < buffer[2]*3; dtcCount=dtcCount+3){
            String dtc = this->decodeDTC(buffer[dtcCount+3], buffer[dtcCount+4]); 
            dtc.toUpperCase();
            if(buffer[dtcCount+5] == 0x28 || buffer[dtcCount+5] == 0x2A || buffer[dtcCount+5] == 0x68){
              dtc = dtc + "|Stored,";
            }else {
              dtc = dtc + "|Active,";
            }            
            combinedDtcs = combinedDtcs + dtc;
          }   
        }

        // Serial.println("Buffer");
        // for(int bufferCount = 0; bufferCount < sizeOfBuffer; bufferCount++){
        //   Serial.println(buffer[bufferCount], HEX);
        // }

        if(moduleEquipped){
          if(hasErrors){
            doc[MitsubishiEcus[i].ecuName] = combinedDtcs;
          }else {
            doc[MitsubishiEcus[i].ecuName] = "OK";         
          } 
      }
    }

  // Serialize the JSON document to a string
  String jsonString;
  serializeJson(doc, jsonString);
  delay(100);

  return jsonString;

}

String MitsubishiClass::getVinRead(){
  uint8_t buffer[20];
  UDS->getVIn(0x7E0, 0x7E8, 0xFF, buffer);
  String vin = "";

  int sizeOfBuffer = sizeof(buffer) / sizeof(buffer[0]);

  for(int i = 0; i < sizeOfBuffer; i++){
      Serial.println(buffer[i], HEX);
  }

  for (int i = 2; i < 20; i++){
    char ASCII =  UDS->decimalToASCII((int)buffer[i]);
    vin = vin + ASCII;
  } 

  Serial.print(vin);

  return vin;
}

String MitsubishiClass::decodeDTC(byte highByte, byte lowByte) {
  unsigned int dtcCode = (highByte << 8) | lowByte;

  char category;
  switch (dtcCode & 0xC000) {
    case 0x0000:
      category = 'P'; // Powertrain
      break;
    case 0x4000:
      category = 'C'; // Chassis
      break;
    case 0x8000:
      category = 'B'; // Body
      break;
    case 0xC000:
      category = 'U'; // Network
      break;
    default:
      category = ' '; // Unknown category
      break;
  }

  unsigned int code = dtcCode & 0x3FFF;

  String dtcString = "";
  dtcString += category;
  
  if (code < 0x10) {
    dtcString += "0";
  }
  if (code < 0x100) {
    dtcString += "0";
  }
  if (code < 0x1000) {
    dtcString += "0";
  }

  dtcString += String(code, HEX);

  dtcString.toUpperCase();

  return dtcString;
}

String MitsubishiClass::liveData(String deviceId, String firmwareVersion, float latitude, float longitude){
  int pids[] = {
    ENGINE_COOLANT_TEMPERATURE,
    ENGINE_RPM,
    VEHICLE_SPEED,
    AIR_INTAKE_TEMPERATURE,
    MAF_AIR_FLOW_RATE,
    THROTTLE_POSITION,
    RUN_TIME_SINCE_ENGINE_START,
    DISTANCE_TRAVELED_WITH_MIL_ON,
    DISTANCE_TRAVELED_SINCE_CODES_CLEARED,
    CONTROL_MODULE_VOLTAGE,
    TIME_RUN_WITH_MIL_ON,
    TIME_SINCE_TROUBLE_CODES_CLEARED
  };

  DynamicJsonDocument doc(512);
  doc["deviceId"] = deviceId;
  doc["firmwareVersion"] = firmwareVersion;
  doc["latitude"] = latitude;
  doc["longitude"] = longitude;

  int pidCount = sizeof(pids) / sizeof(pids[0]);

  for(int i = 0; i < pidCount; i++){
    if(!this->ecuOn()) return "Off"; 
    uint8_t buffer[8];
    memset(buffer, 0x55, sizeof(uint8_t)*8);

    UDS->readLiveData(0x7DF, 0x7E8, 0xFF, buffer, pids[i]);

    // for (int j = 0; j < 8; j++){
    
    //   Serial.print(buffer[j], HEX);
    //   Serial.print("\t");
    // }
    // Serial.println();

    switch (pids[i]){
      case ENGINE_COOLANT_TEMPERATURE:
        if(buffer[0] == 0x55) break;
        doc["Engine_coolant_temperature"] = (buffer[3] - 40);
        break;
      case ENGINE_RPM:
        if(buffer[0] == 0x55) break;
        doc["Engine_RPM"] = ((buffer[3] * 256.0 + buffer[4]) / 4.0);
        break;
      case VEHICLE_SPEED:
        if(buffer[0] == 0x55) break;
        doc["Vehicle_Speed"] = buffer[3];
        break;
      case AIR_INTAKE_TEMPERATURE:
        if(buffer[0] == 0x55) break;
         doc["Air_intake_temperature"] = (buffer[3] - 40);
        break;
      case MAF_AIR_FLOW_RATE:
        if(buffer[0] == 0x55) break;
        doc["MAF_air_flow_rate"] = ((buffer[3] * 256.0 + buffer[4]) / 100.0);
        break;
      case THROTTLE_POSITION:
        if(buffer[0] == 0x55) break;
        doc["Throttle_position"] = (buffer[3]/2.55);
        break;
      case RUN_TIME_SINCE_ENGINE_START:
        if(buffer[0] == 0x55) break;
        doc["Run_time_since_engine_start"] = (buffer[3] * 256.0 + buffer[4]);
        break;
      case DISTANCE_TRAVELED_WITH_MIL_ON:
        if(buffer[0] == 0x55) break;
        doc["Distance_traveled_with_malfunction_indicator_lamp_MIL_on"] = (buffer[3] * 256.0 + buffer[4]);
        break;
      case DISTANCE_TRAVELED_SINCE_CODES_CLEARED:
        if(buffer[0] == 0x55) break;
        doc["Distance_traveled_since_codes_cleared"] = (buffer[3] * 256.0 + buffer[4]);
        break;
      case CONTROL_MODULE_VOLTAGE:
        if(buffer[0] == 0x55) break;
        doc["Control_module_voltage"] = ((buffer[3] * 256.0 + buffer[4]) / 1000.0);
        break;
      case TIME_RUN_WITH_MIL_ON:
        if(buffer[0] == 0x55) break;
        doc["Time_run_with_MIL_on"] = (buffer[3] * 256.0 + buffer[4]);
        break;
      case TIME_SINCE_TROUBLE_CODES_CLEARED:
        if(buffer[0] == 0x55) break;
        doc["Time_since_trouble_codes_cleared"] = (buffer[3] * 256.0 + buffer[4]);
        break;
    }


  }

    // Serialize the JSON document to a string
  String jsonString;
  serializeJson(doc, jsonString);
  delay(100);

  return jsonString;

}

bool MitsubishiClass::ecuOn(){
  for(int retries = 10; retries > 0; retries--){
    uint8_t buffer[8];
    memset(buffer, 0x55, sizeof(uint8_t)*8);

    UDS->readLiveData(0x7DF, 0x7E8, 0xFF, buffer, ENGINE_RPM);
    // for(int i =0; i<8; i++){
    //   Serial.println(buffer[i], HEX);
    // }

    if(buffer[0] != 0x55){
      break;
    }

    if(retries <= 1){
      return false;
    }

  }

  return true;
}

float MitsubishiClass::batteryVoltage(){
  uint8_t buffer[8];
  memset(buffer, 0x55, sizeof(uint8_t)*8);
  UDS->readLiveData(0x7DF, 0x7E8, 0xFF, buffer, CONTROL_MODULE_VOLTAGE);
  // delay(100);

  return  ((buffer[3] * 256.0 + buffer[4]) / 1000.0);
}

String MitsubishiClass::clearAllDTC(String deviceId){
  int numECUs = sizeof(MitsubishiEcus) / sizeof(MitsubishiEcus[0]);
  bool moduleEquipped;
  bool errorCleared = false;

  DynamicJsonDocument doc(1024);
  doc["deviceId"] = deviceId;

  for (int i = 0; i < numECUs; i++) {
    // Serial.print("Scanning ECU : ");
    // Serial.print(MitsubishiEcus[i].ecuName);
    // Serial.println();

    uint8_t buffer[8];
    memset(buffer, 0x55, sizeof(uint8_t)*8);

    UDS->clearAllDTC(MitsubishiEcus[i].ecuReqAddress, MitsubishiEcus[i].ecuResAddress, 0xFF, buffer);

    // for(int bufferCount = 0; bufferCount < 8; bufferCount++){
    //   Serial.println(buffer[bufferCount], HEX);
    // }

    if(buffer[0] == 0x55){
        //Serial.println("Module not equiped");
        moduleEquipped = false;
    }

    if(buffer[1] == 0x54){
        moduleEquipped = true;
        errorCleared = true;
    }

    if(moduleEquipped){
      if (errorCleared){
        doc[MitsubishiEcus[i].ecuName] = "success";
      }else {
        doc[MitsubishiEcus[i].ecuName] = "failed";
      }
    }
  }

    // Serialize the JSON document to a string
  String jsonString;
  serializeJson(doc, jsonString);
  delay(100);

  return jsonString;
}
