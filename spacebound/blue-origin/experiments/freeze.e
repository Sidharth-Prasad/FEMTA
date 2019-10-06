
ad15_gnd 10Hz { //[print];
};
ad15_vdd 10Hz {};

ds18 1/2Hz {
  [print];
};

ds32 1Hz {
  
  Time < 0.12min trigger { -27 } reverses;
    
  [calibrate : Time, poly, s : 0.0009765625, 0.0];
};
