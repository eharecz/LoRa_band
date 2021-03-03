#include "config.h"
TTGOClass *ttgo;
S7XG_Class *s7xg;

void setup(){
    Serial.begin(115200);

    Serial.println("Log: begin");

    ttgo = TTGOClass::getWatch();
    ttgo->begin();

    Serial.println("Log: getWatch and begin");

    //! Open s7xg chip power
    ttgo->enableLDO4();
    //! Open s7xg gps reset power
    ttgo->enableLDO3();

    Serial.println("Log: enable LDO");

    ttgo->s7xg_begin();
    s7xg = ttgo->s7xg;

    Serial.println("Log: s7xg_begin");

    int len = 0;
    int retry = 0;

    do {
        len = s7xg->getHardWareModel().length();
        if (len == 0 && retry++ == 5) {
            s7xg->reset();
            retry = 0;
            Serial.println("Reset s7xg chip");
        }
        if (len == 0)
            delay(1000);
    } while (len == 0);

    Serial.println("Found s7xg module,Start gps module");
    
    s7xg->loraSetFrequency(480000000);
    s7xg->loraSetSyncWord(52);
    s7xg->loraSetSpreadingFactor(7);
    s7xg->loraSetPower(20);
    s7xg->loraSetCodingRate(5);
    s7xg->loraSetBandWidth(125);
    s7xg->loraSetPreambleLength(8);
    s7xg->loraSetPingPongSender();


    Serial.println("Log: setting");
}

void loop(){
    char str[64] = "3c31303038363e48656c6c6f20576f726c6421";

    s7xg->loraTransmit(str);

    Serial.println("Log: send data");

    delay(60000);
}
