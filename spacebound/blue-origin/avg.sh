

sum=0

while read line; do

    sum=$(echo "$sum + $line" | bc -l)
    
done < $1

echo "Sum: $sum"
echo "Avg:" $(echo "$sum / $(cat $1 | wc -l)" | bc -l)
