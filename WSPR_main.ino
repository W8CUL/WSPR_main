
#include <SoftwareSerial.h>

SoftwareSerial gps(3, 2);  // RX, TX

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Testing");

  gps.begin(9600);
}

#define sim 1

void loop() {
  // put your main code here, to run repeatedly:
  while (gps.available() > 0) {
    String gps_raw = gps.readStringUntil('\n');
    Serial.println(gps_raw);
    //sim packet  $GPGLL,3938.76279,N,07958.40013,W,175359.00,A,A*7E
    if (gps_raw.substring(0, 6) == "$GPGLL") {
      if (digitalRead(sim_packet) == 0) {
        gps_raw = "$GPGLL,3938.76279,N,07958.40013,W,175359.00,A,A*7E";
        //v4.2 test data
      } else {
        //print raw GPS data
        Serial.println(gps_raw);
      }
    }
  }
