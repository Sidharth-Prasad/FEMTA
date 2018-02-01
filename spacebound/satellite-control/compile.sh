mv *~ backups/
gcc -o unified-controller unified-controller.c i2c-interface.c temperature-monitoring.c -lpigpio -lrt -lpthread
