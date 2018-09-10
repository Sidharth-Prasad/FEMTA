mv *~ backups/ 2>/dev/null
mv \#* backups/ 2>/dev/null
gcc -o femta femta.c linked-list.c timing.c logger.c i2c-interface.c temperature-monitoring.c graphics.c -lpigpio -lrt -lpthread -lncurses -lmenu
