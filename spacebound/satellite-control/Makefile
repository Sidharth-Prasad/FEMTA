
CFLAGS = -lpigpio -lrt -lpthread -lncurses -lmenu -g

femta: femta.o i2c-interface.o serial-interface.o temperature-monitoring.o graphics.o logger.o timing.o linked-list.o
	gcc $(CFLAGS) -o femta femta.o i2c-interface.o serial-interface.o temperature-monitoring.o graphics.o logger.o timing.o linked-list.o 

femta.o: femta.c femta.h i2c-interface.h serial-interface.h temperature-monitoring.h graphics.h logger.h colors.h
	gcc -c femta.c femta.h i2c-interface.h serial-interface.h temperature-monitoring.h graphics.h logger.h colors.h

i2c-interface.o: i2c-interface.c i2c-interface.h femta.h graphics.h timing.h logger.h colors.h
	gcc -c i2c-interface.c i2c-interface.h femta.h graphics.h timing.h logger.h colors.h

serial-interface.o: serial-interface.c serial-interface.h femta.h linked-list.h graphics.h timing.h logger.h colors.h
	gcc -c serial-interface.c serial-interface.h femta.h linked-list.h graphics.h timing.h logger.h colors.h

temperature-monitoring.o: temperature-monitoring.c temperature-monitoring.h femta.h i2c-interface.h linked-list.h graphics.h colors.h
	gcc -c temperature-monitoring.c temperature-monitoring.h femta.h i2c-interface.h linked-list.h graphics.h colors.h

graphics.o: graphics.c graphics.h femta.h
	gcc -c graphics.c graphics.h femta.h

logger.o: logger.c logger.h
	gcc -c logger.c logger.h

timing.o: timing.c timing.h
	gcc -c timing.c timing.h

linked-list.o: linked-list.c linked-list.h
	gcc -c linked-list.c linked-list.h 

clean:
	rm *.o *.gch    # blow away compiled objects
	rm femta        # blow away 
#	mv *~ backups/
#	mv \#* backups/


#gcc -o unified-controller unified-controller.c linked-list.c timing.c logger.c i2c-interface.c temperature-monitoring.c serial-interface.c graphics.c -lpigpio -lrt -lpthread -lncurses -lmenu
