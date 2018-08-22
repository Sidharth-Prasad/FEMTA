mv *~ backups/ 2>/dev/null
mv \#* backups/ 2>/dev/null
gcc -o unified-controller unified-controller.c linked-list.c timing.c logger.c i2c-interface.c temperature-monitoring.c serial-interface.c graphics.c -lpigpio -lrt -lpthread -lncurses -lmenu -lm




