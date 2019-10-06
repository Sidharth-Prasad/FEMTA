
ad15_gnd 1Hz { /* [print] */ };
ad15_vdd 1Hz { /* [print] */ };
ad15_sda 1Hz { /* [print] */ };
ad15_scl 1Hz { /* [print] */ };

ds32 1/2Hz {
  [calibrate : Time, poly, s : 0.0009765625, 0.0];
  [print];
};

/*
ds18 1/4Hz {
  [print];
};
*/