
echo "start"

while [[ ! -f ~/FEMPTA/spacebound/satellite-control/shell-processes/start.txt ]];
do
    if [[ -f ~/FEMTA/spacebound/satellite-control/shell-processes/stop.txt ]];
    then
	exit
    fi
done

echo "here"
printf "\n\n\e[32mStarting CPU log\n" >> ../logs/cpu_log.txt
SECONDS_SINCE_START=0

while [[ ! -f ~/FEMTA/spacebound/satellite-control/shell-processes/stop.txt ]];
do
    printf ${SECONDS_SINCE_START}"\t" >> ../logs/cpu_log.txt
    mpstat 1 1 | awk '$12 ~ /[0-9.]+/ { print 100 - $12 }' | head -n 1 >> ../logs/cpu_log.txt
    (( SECONDS_SINCE_START += 1 ))
done
