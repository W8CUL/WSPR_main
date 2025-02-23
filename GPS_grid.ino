
// generate grid square based on GPS: lon,lat


void togrid(float lat, float lon) {
    // Adjust longitude to 0-360 and latitude to 0-180
    lon += 180.0;
    lat += 90.0;

    char grid[10];
    // First pair: letters (A-R)
    grid[0]  = 'A' + int(lon / 20);
    grid[1] = 'A' + int(lat / 10);

    // Second pair: digits (0-9)
    grid[2] = '0' + int((lon - int(lon / 20) * 20) / 2);
    grid[3] = '0' + int((lat - int(lat / 10) * 10));

    // Third pair: letters (A-X)
    grid[4] = 'A' + int((lon - int(lon / 2) * 2) * 12);
    grid[5] = 'A' + int((lat - int(lat) * 60 / 2.5));


    // Fourth pair: digits (0-9) for extended precision
    float lonRemainder = (lon - int(lon)) * 120;
    float latRemainder = (lat - int(lat)) * 240;
    grid[6] = '0' + int(lonRemainder);
    grid[7] = '0' + int(latRemainder);

    // Construct the Maidenhead grid square
    return grid;
}