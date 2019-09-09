
ad15_gnd 100Hz {
  
  A0 > 750mV trigger {
    -17
  } forever;
  
  A1 < 10F trigger {
    -17
  } singular;
  
  [calibrate : A0, poly, V : 0.0, 1.0, 2.0];
  [calibrate : A1, hart, C : 1.0, 1.0, 4.0];
  [print];
};

ds18 1Hz {
  //[print];
};

ds32 1Hz {
     
  Time > 1.12min trigger {
    +27
  } singular, reverses;
    	      
  [calibrate : Time, poly, s : 0.0, 2, 0.0];
  //[print];
};
