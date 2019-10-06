
ad15_gnd 1Hz {};
ad15_vdd 1Hz {};
ad15_sda 1Hz {};
ad15_scl 1Hz {};

//ds18 1Hz {};

ds32 1Hz {
  
  Time > 0.0min trigger { +27 } reverses;
  
  [calibrate : Time, poly, s : 0.0009765625, 0.0];
};
