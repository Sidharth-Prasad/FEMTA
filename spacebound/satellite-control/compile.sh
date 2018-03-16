mv *~ backups/ 2>/dev/null
gcc -o unified-controller unified-controller.c linked-list.c i2c-interface.c temperature-monitoring.c UART-interface.c graphics.c -lpigpio -lrt -lpthread -lncurses -lmenu
