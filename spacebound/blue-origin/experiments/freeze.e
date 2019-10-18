
ad15_gnd 1Hz {};
ad15_vdd 1Hz {};
ad15_sda 1Hz {};
ad15_scl 1Hz {};

ds18 2Hz { [print]; };

ds32 1Hz {
  
  Time > 0.1s trigger {
       +27
  };
  
  [calibrate : Time, poly, s : 0.0009765625, 0.0];
  //  [print : gray : 5Hz];
};
