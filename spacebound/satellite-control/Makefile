
CFLAGS = -lpigpio -lrt -lpthread -lncurses -lmenu -g

femta: femta.o i2c-interface.o temperature-monitoring.o graphics.o logger.o timing.o linked-list.o
	gcc $(CFLAGS) -o femta femta.o i2c-interface.o temperature-monitoring.o graphics.o logger.o timing.o linked-list.o 

femta.o: femta.c femta.h i2c-interface.h temperature-monitoring.h graphics.h logger.h colors.h
	gcc -c femta.c femta.h i2c-interface.h temperature-monitoring.h graphics.h logger.h colors.h

i2c-interface.o: i2c-interface.c i2c-interface.h femta.h graphics.h timing.h logger.h colors.h
	gcc -c i2c-interface.c i2c-interface.h femta.h graphics.h timing.h logger.h colors.h

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
	rm *~ \#*       # blow away backups
	rm femta        # blow away binary

