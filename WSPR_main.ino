
#include <SoftwareSerial.h>
#include <si5351.h>
#include <JTEncode.h>
#include <rs_common.h>
#include <int.h>
#include <string.h>

#include "Wire.h"

//add WDT
#include<avr/wdt.h>


SoftwareSerial gps(3, 2);  // RX, TX
char test_data[100];

int msg_valid = 0;
int loc_valid = 0;
int ts = 0;
float lon, lat, utc,alt_num;

char call[] = "W8CUL";
char loc_public[] = "xxxx";


//#define FT8 1
#define WSPR 1
#define sim 0
//#define disable_encode 1

#define debug_low 1 - //unconnent only when needed

char grid[10];  //global variable


void setup() {
  wdt_disable();

  Serial.begin(9600);
  Serial.println(F("KE8TJE WSPR testing - 20 m"));

  gps.begin(9600);
  setup_WSPR();
  utc = 3489.00;  //random value till GPS lock, 0 would send a packet
}



void loop() {
  wdt_enable(WDTO_2S);
  // put your main code here, to run repeatedly:
  while (gps.available() > 0) {
    String gps_raw = gps.readStringUntil('\n');
#ifdef debug_low
    Serial.println(gps_raw);
#endif
    //sim packet  $GPGLL,3938.76279,N,07958.40013,W,175359.00,A,A*7E
    if (gps_raw.substring(0, 6) == "$GPGLL") {
      wdt_reset(); //WDT reset
      
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

      wdt_enable(WDTO_8S);
      update_GPS_PD1616(p);
    }
  }
}


void update_GPS_PD1616(char *p) {
  // V2 - Using PD1616

  p = strtok(NULL, ",");  //time
  utc = atof(p);

  p = strtok(NULL, ",");  //lat - <2>
  lat = atof(p) / 100.00;
  lat = int(lat) + (lat - int(lat)) * 100 / 60;

  p = strtok(NULL, ",");  // lat_char - <3>
  //South not handeld


  p = strtok(NULL, ",");  //lng - <4>
  lon = atof(p) / 100.00;
  lon = int(lon) + (lon - int(lon)) * 100 / 60;  //fix for minutes


  p = strtok(NULL, ",");  //dir - <5>
  if (p[0] == 'W') {
    // change lon to negative;
    lon = -lon;
  }

  p = strtok(NULL, ",");  //state - <6>

  if (p[0] >= '1' & p[0] <= '4') {
    togrid(lat, lon);
    loc_valid = 1;
  } else {
    // disable time update
    //return;
  }
  p = strtok(NULL, ",");
  p = strtok(NULL, ",");
  p = strtok(NULL, ",");  //<9> alt
  alt_num = atof(p);


  int sec = long(utc) % 100;
  int min = long(utc) % 10000 - sec;

  Serial.print(min);
  Serial.print(",");
  Serial.println(sec);
  Serial.print(lat);
  Serial.print(",");
  Serial.println(lon);

  Serial.println(grid);

  //test - fast encode without time sync
  //encode();

  // WSPR
  if (sec == 0 & (min % 200) == 0 & loc_valid) {

    gps.stopListening();

    //Function to handel ts and locations
    for (int i = 0; i < 4; i++) {
      loc_public[i] = grid[i + ts * 4];
    }
    ts = 1; //updated from old

    Serial.print("WSPR-20m :");
    Serial.print(loc_public);
    Serial.println();
    uint8_t pwr = 27;
// Send WSPR
#ifdef disable_encode
    delay(5000);
    Serial.println("simulated encode");
#else
    set_tx_buffer();  //update the location
    encode();
#endif

    delay(50);  //delay to avoid extra triggers
    gps.listen();
    Serial.println("End WSPR");
  }
}


void update_GPS(char *p) {
  // V1 - low altitude GPS data processing

  p = strtok(NULL, ",");  //lat - <1>
  //sprintf(Lat, "%s", p);
  lat = atof(p) / 100.00;
  lat = int(lat) + (lat - int(lat)) * 100 / 60;


  p = strtok(NULL, ",");  // lat_dir - <2>
  // handel south

  p = strtok(NULL, ",");  //lng - <3>
  lon = atof(p) / 100.00;
  lon = int(lon) + (lon - int(lon)) * 100 / 60;  //fix for minutes

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

  //test - fast encode without time sync
  //encode();

  // WSPR
  if (sec == 0 & (min % 200) == 0 & loc_valid) {

    gps.stopListening();

    //Function to handel ts and locations
    for (int i = 0; i < 4; i++) {
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
#ifdef disable_encode
    delay(5000);
    Serial.println("simulated encode");
#else
    set_tx_buffer();  //update the location
    encode();
#endif

    delay(50);  //delay to avoid extra triggers
    gps.listen();
    Serial.println("End WSPR");
  }
}