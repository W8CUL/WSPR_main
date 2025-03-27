
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
int msg_id = 0;
int loc_valid = 0;
int ts = 0;
float lon, lat, utc, alt_num,adc_0;

char call[] = "W8CUL";
char loc_public[] = "xxxx";


//#define FT8 1
#define WSPR 1
#define sim 0
#define disable_encode 1
#define GPS_PD1616 1  //PD1616 GPS - PCB v1.4X


#define adc_threshold 3.95  //5V max ADC: voltage devided Vcc/2
#define alt_test 1

//#define debug_low 1 -  //unconnent only when needed

char grid[10];  //global variable


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println(F("KE8TJE WSPR testing - 20 m"));

#ifdef WSPR
  Serial.println("mode\t: WSPR");
#endif

#ifdef disable_encode
  Serial.println(F("HF Tx\t: disable - Testing "));
#else
  Serial.println(F("HF Tx\t:ENABLE"));

#endif

  delay(5000);
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

#ifdef GPS_PD1616
    if (gps_raw.substring(0, 6) == "$GNGGA") {
#else
    if (gps_raw.substring(0, 6) == "$GPGLL") {
#endif

#ifdef debug_low
      if (sim == 1) {

#ifdef GPS_PD1616
        gps_raw = "$GNGGA,020722.000,3938.3269,N,07957.1170,W,1,05,2.76,340.9,M,-33.0,M,,*44";
#else
        gps_raw = "$GPGLL,3964.61834,N,07997.41000,W,175359.00,A,A*7E";
#endif
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

#ifndef GPS_PD1616
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
    togrid(lat, lon);
    loc_valid = 1;
  } else {
    msg_valid = 0;
  }

#else
  //GPS PD1616 used

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

  #ifdef alt_test
    alt_num = msg_id*1000 + atof(p);
  #endif

#endif

  int sec = long(utc) % 100;
  int min = long(utc) % 10000 - sec;

  Serial.print("UTC:");
  Serial.print(min);
  Serial.print(",");
  Serial.println(sec);
  Serial.println(grid);
  Serial.print("alt:");
  Serial.println(alt_num);
  uint8_t pwr_i = encode_pwr(alt_num,adc_0);

  Serial.print("pwr_data:");
  Serial.println(pwr_i);

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

    if ((min % 1000) == 0){
        ts = 0;
        //every 10 mins
        // send first 4 digits -ts = 0
      }

    //location update on TX data
    for (int i = 0; i < 4; i++) {
          loc_public[i] = grid[i + ts * 4];
        }
    ts = 1; //return to grid[5:8]
    
    Serial.print("WSPR-20m :");
    Serial.print(loc_public);
    Serial.println();

    uint8_t pwr = encode_pwr(alt_num,adc_0);

#ifdef disable_encode
    delay(5000);
    Serial.println("simulated encode");
    Serial.print("pwr_data:");
    Serial.println(pwr);
#else

    set_tx_buffer(pwr);   //update the location
    encode();             //Send WSPR
             
#endif
    adc_0 = 0;            //reset battery average ever 2 mins
    delay(50);            //delay to avoid extra triggers
    msg_id++;             //increase msg id for simulated sequence
    gps.listen(); 
    Serial.println("End WSPR");
  }else{
    // non critical functions 
    // limit execution 0.75 s 

    for(int i=0;i<20;i++){
        adc_0 = (adc_0 + analogRead(A0)*5.0/1023)/2;
        delay(5);
    }
    Serial.print("ADC:");
    Serial.println(adc_0);

  }

#endif
}