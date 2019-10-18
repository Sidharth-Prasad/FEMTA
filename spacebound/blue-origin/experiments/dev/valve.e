

ds32 1Hz {
     Time > 0s trigger {
     	  +26 [pulse :: 350ms],
	  -19
     } singular;
     
     [calibrate : Time, poly, s : 0.0009765625, 0.0];
     //[print : red : 5hz];
};