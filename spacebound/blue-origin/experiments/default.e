
ad15_sda 100Hz {
	 
    A01 less_than 0.915v trigger {     
      +23, -24
    } forever, reverses;
    
    A01 less_than 0.614v trigger {
      +25
    } forever, reverses;
    
} print;

ad15_vdd 100Hz {};
ad15_scl 100Hz {};
ds32 1Hz {};
