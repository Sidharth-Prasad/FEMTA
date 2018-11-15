turn_ccw 90
sleep 1m

hold 0
sleep 1m

set_proportional_gain .4
sleep 1m

message "looking for proportional gain"
print_gain proportional
print_gain integral
print_gain derivative

hold 180
sleep 16s

increase_proportional_gain .1
print_gain proportional
hold 0
sleep 16s

hold 180
sleep 16s

increase_proportional_gain .1
print_gain proportional
hold 0
sleep 16s

message "done testing gains"



