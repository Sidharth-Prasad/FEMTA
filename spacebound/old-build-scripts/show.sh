gcc -o display display.c -lncurses -lmenu -lpanel
if [[ $? -eq 0 ]];
then
    ./display
fi
