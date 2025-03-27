
// generate grid square based on GPS: lon,lat


void togrid(float lat, float lon) {
    // Adjust longitude to 0-360 and latitude to 0-180
    lon += 180.0;
	  lat += 90.0;
    //char grid[10];
    // First pair: letters (A-R)
    grid[0]  = 'A' + (int)(lon / 20);
    grid[1] = 'A' + (int)(lat / 10);

    // Second pair: digits (0-9)
    grid[2] = '0' + (int)((lon - (int)(lon / 20) * 20) / 2);
    grid[3] = '0' + (int)((lat - (int)(lat / 10) * 10));

    // Third pair: letters (A-X)
    grid[4] = 'A' + (int)((lon - (int)(lon / 2) * 2) * 12);
    grid[5] =  'A'+ (int)((lat - (int)(lat))*24);

    // Fourth pair: digits (0-9) for extended precision
    int lonRemainder = (lon/(1/12.0) - (int)(lon/(1.0/12.0)))*10;
    int latRemainder = (lat/(1/24.0) - (int)(lat/(1.0/24.0)))*10;
    grid[6] = '0' + lonRemainder;
    grid[7] = '0' + (int)(latRemainder);

    // Construct the Maidenhead grid square
    //return grid;
}



void GPS_sleep(){
  pinMode(6,OUTPUT);
  digitalWrite(6,LOW);
}

void GPS_on(){
  pinMode(6,OUTPUT);
  digitalWrite(6,HIGH);
  delay(10);
  pinMode(6,INPUT);

}