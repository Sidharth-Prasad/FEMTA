
ad15_gnd 10Hz {};
ad15_vdd 10Hz {};
ad15_sda 10Hz {};
ad15_scl 10Hz {};

ds18 1Hz {
  //[print];
};

ds32 1Hz {
  
  Time > 1.12min trigger {
    +27
  } singular, reverses;
  
  //[print];
};
