
while true;
do
    mpstat 1 1 | awk '$12 ~ /[0-9.]+/ { print 100 - $12 }' | head -n 1 >> cpu_log.txt
done
