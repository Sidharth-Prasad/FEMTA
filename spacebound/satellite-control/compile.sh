mv *~ backups/ 2>/dev/null
gcc -o unified-controller unified-controller.c i2c-interface.c temperature-monitoring.c UART-interface.c -lpigpio -lrt -lpthread
