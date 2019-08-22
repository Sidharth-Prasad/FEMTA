#!/usr/bin/env bash

aSum=0
bSum=0
cSum=0

reads=0
counter=0

while read -r a b c; do
    
    aSum=bc <<< "$aSum + $a"
    bSum=bc <<< "$bSum + $b"
    cSum=bc <<< "$cSum + $c"
    
    #bSum=$(echo "$bSum + $b" | bc -l)
        
    (( reads = reads + 1 ))
    (( counter = counter + 1 ))
    
    if [[ reads -eq 65536 ]]; then
        reads=1
        printf "\r$counter"
    fi
    
done < "$1"

echo "Sum: $aSum $bSum $cSum"
echo "Avg:" $(echo "$aSum / $(cat $1 | wc -l)" | bc -l)
echo "Avg:" $(echo "$bSum / $(cat $1 | wc -l)" | bc -l)
echo "Avg:" $(echo "$cSum / $(cat $1 | wc -l)" | bc -l)
