#pragma mark - Depend SparkFun_MAX3010x_Sensor_Library
/*
cd ~/Arduino/libraries
git clone https://github.com/sparkfun/SparkFun_MAX3010x_Sensor_Library
*/

#include "config.h"
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;

void setup() {
    Serial.begin(115200);
    Serial.println("Initializing...");
    // Initialize sensor
    Wire.setPins(25, 26);
    particleSensor.begin(Wire, I2C_SPEED_FAST); //Use default I2C port, 400kHz speed
    particleSensor.setup(); //Configure sensor with default settings
    //particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
}

void loop() {
    long irValue = particleSensor.getIR(); //读取IR值
    if (irValue < 7000){ //如果没检测到手指，则显示下面的内容
        beatAvg=0;
        Serial.println("Place your index finger on the sensor with steady pressure.");
    }
    long delta = millis() - lastBeat; //Measure duration between two beats, Millis() is the time when the program runs here
    lastBeat = millis();
    beatsPerMinute = 60 / (delta / 1000.0); //Calculating the BPM
    if (beatsPerMinute < 255 && beatsPerMinute > 20) //To calculate the average we strore some values (4) then do some math to calculate the average
    {
        rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
        rateSpot %= RATE_SIZE; //Wrap variable
        //Take average of readings
        beatAvg = 0;
        for (byte x = 0 ; x < RATE_SIZE ; x++){
            beatAvg += rates[x];
            beatAvg /= RATE_SIZE;
        }
        Serial.println(beatAvg);
    }
}