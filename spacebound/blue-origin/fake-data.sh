
echo "t:Fake Data Stream"
echo "a:Axis 0"
echo "a:Axis 1"
echo "a:Axis 2"

pi=3.14159265

for i in $(seq 0 10240); do

    
    x=$(echo "s($i/180 + 0*$pi/3.0)" | bc -l)
    y=$(echo "s($i/180 + 1*$pi/3.0)" | bc -l)
    z=$(echo "s($i/180 + 2*$pi/3.0)" | bc -l)
    
    echo "d:$x $y $z"
    #sleep 0.001
done

echo "q"
