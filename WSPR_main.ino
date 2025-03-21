
#include <SoftwareSerial.h>
#include <si5351.h>
#include <JTEncode.h>
#include <rs_common.h>
#include <int.h>
#include <string.h>

#include "Wire.h"

SoftwareSerial gps(3, 2);  // RX, TX
char test_data[100];


//char Lat[] = "xxxx.xxxxxx";
//char Lon[] = "xxxxx.xxxxxxxx";
char time_str[12];
char tmp_str[15];
int msg_valid = 0;
int loc_valid = 0;
int ts = 0;
float lon, lat, utc;

char call[] = "W8CUL";
char loc_public[] = "xxxx";


//#define FT8 1
#define WSPR 1
#define sim 0
//#define disable_encode 1

//#define debug_low 1 - //unconnent only when needed

char grid[10];  //global variable


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
      
#ifdef debug_low
      if (sim == 1) {
        gps_raw = "$GPGLL,3964.61834,N,07997.41000,W,175359.00,A,A*7E";
        //v4.2 test data
        //Serial.println(gps_raw);
      }
#endif
      Serial.println(gps_raw);
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
  lat = int(lat) + (lat-int(lat))*100/60;
  

  p = strtok(NULL, ",");  // lat_dir - <2>
  // handel south

  p = strtok(NULL, ",");  //lng - <3>
  lon = atof(p) / 100.00;
  lon = int(lon) + (lon-int(lon))*100/60; //fix for minutes

  p = strtok(NULL, ",");  //dir - <4>
  if (p[0] == 'W') {
    // change lon to negative;
    lon = -lon;
  }

  p = strtok(NULL, ",");  //time - <5>
  utc = atof(p);

  p = strtok(NULL, ",");  //status- <6>

  // need GPS testing - TJE
  if (p[0] == 'A') {
    msg_valid = 1;
    togrid(lat, lon);
    loc_valid = 1;
  } else {
    msg_valid = 0;
  }


  int sec = long(utc) % 100;
  int min = long(utc) % 10000 - sec;

  Serial.print(min);
  Serial.print(",");
  Serial.println(sec);
  Serial.print(lat);
  Serial.print(",");
  Serial.println(lon);

  Serial.println(grid);

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
  if (sec == 0 & (min % 200) == 0 & loc_valid) {

    gps.stopListening();

    //Function to handel ts and locations
    for (int i = 0; i < 4;i++) {
      loc_public[i] = grid[i + ts * 4];
    }
    if (ts == 0) {
      ts = 1;
    } else {
      ts = 0;
    }
    Serial.print("WSPR-20m :");
    Serial.print(loc_public);
    Serial.println();

    // Send WSPR
    #ifndef disable_encode 
      set_tx_buffer(); //update the location
      encode();
    #endif

    #ifdef disable_encode
      delay(5000);
      Serial.println("simulated encode");
    #endif

    delay(50);  //delay to avoid extra triggers
    gps.listen();
    Serial.println("End WSPR");
  }

#endif
}