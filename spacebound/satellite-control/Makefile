
CFLAGS = -lpigpio -lrt -lpthread -lncurses -lmenu   # C linking flags

GFLAGS=                                             # Granular flags

femta-debug: GFLAGS += -g
femta-debug: femta

femta: femta.o i2c-interface.o temperature-monitoring.o graphics.o logger.o timing.o linked-list.o
	gcc $(CFLAGS) $(GFLAGS) -o femta femta.o i2c-interface.o temperature-monitoring.o graphics.o logger.o timing.o linked-list.o 

femta.o: femta.c femta.h i2c-interface.h temperature-monitoring.h graphics.h logger.h colors.h
	gcc $(GFLAGS) -c femta.c femta.h i2c-interface.h temperature-monitoring.h graphics.h logger.h colors.h

i2c-interface.o: i2c-interface.c i2c-interface.h femta.h graphics.h timing.h logger.h colors.h
	gcc $(GFLAGS) -c i2c-interface.c i2c-interface.h femta.h graphics.h timing.h logger.h colors.h

temperature-monitoring.o: temperature-monitoring.c temperature-monitoring.h femta.h i2c-interface.h linked-list.h graphics.h colors.h
	gcc $(GFLAGS) -c temperature-monitoring.c temperature-monitoring.h femta.h i2c-interface.h linked-list.h graphics.h colors.h

graphics.o: graphics.c graphics.h femta.h
	gcc $(GFLAGS) -c graphics.c graphics.h femta.h

logger.o: logger.c logger.h
	gcc $(GFLAGS) -c logger.c logger.h

timing.o: timing.c timing.h
	gcc $(GFLAGS) -c timing.c timing.h

linked-list.o: linked-list.c linked-list.h
	gcc $(GFLAGS) -c linked-list.c linked-list.h 

clean:
	rm -f *.o *.gch                          # blow away compiled objects
	mv *~ \#* backups/ 2>/dev/null || true   # blow away old backups
	rm -f femta                              # blow away binary

