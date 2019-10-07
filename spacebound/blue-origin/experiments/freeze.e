
ad15_gnd 1Hz {};
ad15_vdd 1Hz {};
ad15_sda 1Hz {};
ad15_scl 1Hz {};

ds18 1Hz { [print]; };

ds32 20/3Hz {
  
  Time > 4s trigger {
       +27 [pulse :: 350ms]
  };
  
  [calibrate : Time, poly, s : 0.0009765625, 0.0];
  //  [print : gray : 5Hz];
};
