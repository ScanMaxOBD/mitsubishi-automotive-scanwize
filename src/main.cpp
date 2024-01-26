
#include "mitsubishi.h"

MitsubishiClass *Mitsubishi = NULL;
String deviceId = "I-am-string-id";
String firmwareVersion = "1.0.3";


void setup(){
  Serial.begin(115200);
  Mitsubishi = new MitsubishiClass();
  Mitsubishi->diagInit();
  delay(1000);

}


void loop(){ 
  if(Mitsubishi->ecuOn()){
    delay(100);
    float batteryVoltage = Mitsubishi->batteryVoltage();
    Serial.print("Battery Voltage: ");
    Serial.println(batteryVoltage);
    delay(100);
    
    Serial.println();
    Serial.println();
    Serial.println("Getting Live Data....");
    String liveData = Mitsubishi->liveData(deviceId, firmwareVersion);
    Serial.println(liveData);   
    delay(100);

    Serial.println();
    Serial.println();
    Serial.println("Reading DTC......");
    String dtcString = Mitsubishi->readAllEcuDtcs(deviceId);
    Serial.println(dtcString);
    delay(100);

    // Serial.println();
    // Serial.println();
    // Serial.println("Clearing DTC......");
    // String clearDTC = Mitsubishi->clearAllDTC(deviceId);
    // Serial.println(clearDTC);
    // delay(100);

    
  } else {
    Serial.println("ECUs OFF - Turn on ignition to atleast position 2 to read ECU");
  }

  delay(3000);
}

