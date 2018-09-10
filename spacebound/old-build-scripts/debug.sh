./compile-debug.sh
if [[ $? -eq 0 ]];
then
    sudo gdb ./unified-controller
fi
