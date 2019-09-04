
ds18 1Hz {
    
    Temperature less_than 36C {
        +12, -13, +5
    } singular;
    
    Temperature more_than 216K {
        -8, +32
    } forever, reverses;
    
    [calibrate 48.0, 24.5, 0.1]
    [smooth 0.9]
    [print 5Hz]
};

