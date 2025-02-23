
#include <SoftwareSerial.h>
#include <si5351.h>
#include <JTEncode.h>
#include <rs_common.h>
#include <int.h>
#include <string.h>

#include "Wire.h"

SoftwareSerial gps(3, 2);  // RX, TX
char test_data[100];


char Lat[] = "xxxx.xxxxxx";
char Lon[] = "xxxxx.xxxxxxxx";
char time_str[12];
char tmp_str[15];
int msg_valid = 0;
float lon, lat, utc;


//#define FT8 1
#define WSPR 1
#define sim 0
//#define debug_low 1 - //unconnent only when needed



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println(F("KE8TJE WSPR testing - 20 m"));

  gps.begin(9600);
  setup_WSPR();
  utc = 3489.00;  //random value till GPS lock, 0 would send a packet
}



void loop() {
  // put your main code here, to run repeatedly:
  while (gps.available() > 0) {
    String gps_raw = gps.readStringUntil('\n');
#ifdef debug_low
    Serial.println(gps_raw);
#endif
    //sim packet  $GPGLL,3938.76279,N,07958.40013,W,175359.00,A,A*7E
    if (gps_raw.substring(0, 6) == "$GPGLL") {
      //print GPS data with or without lock
      Serial.println(gps_raw);
#ifdef debug_low
      if (sim == 1) {
        gps_raw = "$GPGLL,3938.76279,N,07958.40013,W,175359.00,A,A*7E";
        //v4.2 test data
        //Serial.println(gps_raw);
      } else {
        //print raw GPS data
        //Serial.println(gps_raw);
      }
#endif
      gps_raw.toCharArray(test_data, 100);
      char *p = strtok(test_data, ",");
      update_GPS(p);
    }
  }
}


void update_GPS(char *p) {

  //data is sent with 2 decimal places
  // reformat the data to be used by the APRS library
  //"$GPGGA,191757.00,3938.28486,N,07957.13511,W,1,03,2.71,274.5,M,-33.9,M,,*6F";
  // v3
  // $GNGGA,165006.000,2241.9107,N,12017.2383,E,1,14,0.79,22.6,M,18.5,M,,*42

  // Documentation: https://openrtk.readthedocs.io/en/latest/communication_port/nmea.html
  //$GNGGA<0>,000520.095<1>,<2>,<3>,<4>,<5>,0<6>,0<7>,<8>,<9>,M<10>,<11>,M<12>,<13>,*5D<14>

  //"$GPGLL,3938.76279,N,07958.40013,W,175359.00,A,A*7E";
  //"$GPGLL<0>,3938.76279<1>,N<2>,07958.40013<3>,W<4>,175359.00<5>,A,A*7E";

  //char *p = strtok(test_data, ",");  //code - <0>


  p = strtok(NULL, ",");  //lat - <1>
  //sprintf(Lat, "%s", p);
  lat = atof(p) / 100.00;


  // bug fix in v2
  if (Lat[4] == '.') {
    Lat[7] = '\0';
  } else {
    Lat[8] = '\0';
  }



  p = strtok(NULL, ",");  // lat_char - <2>
  //sprintf(Lat, "%s%s\0", Lat, p);

  //p = strtok(NULL, ",");             //lng
  //p = strtok(NULL, ",");             //dir

  p = strtok(NULL, ",");  //lng - <3>
  lon = atof(p);
  //sprintf(Lon, "%s", p);

  if (Lon[4] == '.') {
    Lon[7] = '\0';
  } else {
    Lon[8] = '\0';
  }

  p = strtok(NULL, ",");  //dir - <4>
  sprintf(Lon, "%s%s\0", Lon, p);

  p = strtok(NULL, ",");  //state - <5>
  utc = atof(p);

  //Serial.print("-");
  //Serial.println(p);


  msg_valid = 1;


  //Serial.println(utc);
  //Serial.print(long(utc));

  /*
  if(long(utc)%200==0){

    Serial.print(lat);
    Serial.print(",");
    Serial.println(lon);
    Serial.println(utc);

    Serial.println("WSPR - 20m");
    encode();
    delay(50); //delay to avoid extra triggers
    Serial.println("End of WSPR");
  }else{
    Serial.println(long(utc));
  }
  */


  int sec = long(utc) % 100;
  int min = long(utc) % 10000 - sec;

  Serial.print(min);
  Serial.print(",");
  Serial.println(sec);

#ifdef FT8
  if (sec == 30) {
    Serial.print(lat);
    Serial.print(",");
    Serial.println(lon);

    Serial.println("FT8 - 20m");
    gps.stopListening();
    encode();
    delay(50);  //delay to avoid extra triggers
    gps.listen();
    Serial.println("End FT8");
  } else {
    Serial.println(long(utc));
  }

#endif

  //test - fast encode without time sync
  //encode();

#ifdef WSPR
  // WSPR
  if (sec == 0 & (min % 200)==0) {
    Serial.println("WSPR-20 m");
    gps.stopListening();

    // Function needs to be added to send 8 digit location
  
    // Send WSPR
    encode();


    delay(50);  //delay to avoid extra triggers
    gps.listen();
    Serial.println("End WSPR");
  }

#endif
}