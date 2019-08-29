#!/usr/bin/env bash

aSum=0
counter=0
reads=0

while read -r entry; do
    
    aSum=$(./add $aSum $entry)
    
    (( reads = reads + 1 ))
    (( counter = counter + 1 ))
    
    if [[ reads -eq 65536 ]]; then
        reads=1
        printf "\r$counter"
    fi
    
done < "$1"

echo "Sum: $aSum"
echo "Avg:" $(echo "$aSum / $(cat $1 | wc -l)" | bc -l)
