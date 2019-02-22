
seconds=2

if [[ $# -eq 1 ]]; then
    seconds=$1
fi
   

sleep $seconds
printf "q"
