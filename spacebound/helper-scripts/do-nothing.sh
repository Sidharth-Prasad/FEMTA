while read -n1 line
do
    :
    #echo "$line"
    printf "q\n"
done < "${1:-/dev/stdin}"
