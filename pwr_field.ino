

// PWR filed supports 0 - 60 int (6bits max)

// altitude data: 0 - 30 
// Power state: 
// 2S lipos: max 8.4 V Nominal 3.7*2 = 7.4 V
// 7.9 V as a threshold: input > 7.4 or not - send as 1 bit


uint8_t encode_pwr(float alt,float adc){
  // in m
  // 0 - 30 km
  uint8_t alt_i = uint8_t(alt/1000);
  if(alt_i>29) alt_i = 29;


  // get ADC reading
  if(adc>adc_threshold){
    return alt_i + 30; 
  }else{
    return alt_i;
  }

}