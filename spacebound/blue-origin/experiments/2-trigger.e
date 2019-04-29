

ad15_sda {
    hertz = 100

    A01 less_than 7120 trigger 23       // 1.335v 20kpa trigger the white wire
    A01 less_than 4880 trigger 24       // 0.915v 10kpa trigger the green wire
      
    //print                               // print at 5Hz to console
}


ad15_vdd {
    hertz = 100                          // log at 100Hz

    print
}

ds32 {
    hertz = 1
}

