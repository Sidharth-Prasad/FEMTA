
// Notes
//   GPIO numbers are specified by their Broadcom IDs

ad15_vdd {

    hertz = 50                          // log at 100Hz
    
    A01 less_than 8192  trigger 22      // triggers GPIO 22 when less than 1V is read on diff 01
    A23 more_than 26214 trigger 23      // triggers GPIO 23 when more than 4V is read on diff 23
    
    print                               // print at 5Hz to console
}

adxl {
    hertz = 100
    
    Z less_than -2 trigger 4            // triggers GPIO 4 when less than -2g is read on Z-axis
}

ds32 {
    hertz = 1
}

