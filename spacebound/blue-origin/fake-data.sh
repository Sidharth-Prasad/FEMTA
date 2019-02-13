
echo "t:Fake Data Stream"
echo "a:Axis 0"
echo "a:Axis 1"
echo "a:Axis 2"

for i in $(seq 0 10240); do
    echo "d:$i $i $i"
    sleep 0.1
done

echo "q"
