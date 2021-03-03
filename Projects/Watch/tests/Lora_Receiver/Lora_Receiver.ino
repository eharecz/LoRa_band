#include "config.h"

TTGOClass *ttgo;
S7XG_Class *s7xg;


void setup() {
  Serial.begin(115200);

    ttgo = TTGOClass::getWatch();
    ttgo->begin();

    Serial.println("Log: getWatch and begin");

    ttgo->enableLDO4();
    ttgo->enableLDO3();

    Serial.println("Log: open the power");

    ttgo->s7xg_begin();

    s7xg = ttgo->s7xg;

    Serial.println("Log: get serial object");

    Serial.println("Log: s7xg begin");
    
    s7xg->loraSetFrequency(480000000);
    s7xg->loraSetSyncWord(52);
    /*
    s7xg->loraSetSpreadingFactor(7);
    s7xg->loraSetPower(20);
    s7xg->loraSetCodingRate(5);
    s7xg->loraSetBandWidth(125);
    s7xg->loraSetPreambleLength(8);
    */
    Serial.println("Log: set syncword");
    s7xg->loraSetPingPongReceiver();
    s7xg->loraReceiveContinuous(true);
}

void loop() {
  // try to parse packet

  
    // received a packet

    // read packet
    String str = s7xg->loraGetPingPongMessage();
    if(str != "")
    {
        Serial.print("Get Data: ");
        Serial.println(str);
    }
}