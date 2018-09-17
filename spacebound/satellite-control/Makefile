
CFLAGS = -lpigpio -lrt -lpthread -lncurses -lmenu   # C linking flags

GFLAGS=                                             # Granular flags

femta-debug: GFLAGS += -g
femta-debug: femta

femta: femta.o i2c-interface.o temperature-monitoring.o graphics.o selector.o scripter.o logger.o timing.o hashmap.o linked-list.o error.o
	gcc $(CFLAGS) $(GFLAGS) -o femta femta.o i2c-interface.o temperature-monitoring.o graphics.o selector.o scripter.o logger.o timing.o hashmap.o linked-list.o error.o

femta.o: femta.c femta.h i2c-interface.h temperature-monitoring.h graphics.h selector.h scripter.h logger.h colors.h
	gcc $(GFLAGS) -c femta.c femta.h i2c-interface.h temperature-monitoring.h graphics.h selector.h scripter.h logger.h colors.h

i2c-interface.o: i2c-interface.c i2c-interface.h femta.h graphics.h timing.h logger.h colors.h
	gcc $(GFLAGS) -c i2c-interface.c i2c-interface.h femta.h graphics.h timing.h logger.h colors.h

temperature-monitoring.o: temperature-monitoring.c temperature-monitoring.h femta.h i2c-interface.h linked-list.h graphics.h colors.h
	gcc $(GFLAGS) -c temperature-monitoring.c temperature-monitoring.h femta.h i2c-interface.h linked-list.h graphics.h colors.h

graphics.o: graphics.c graphics.h femta.h
	gcc $(GFLAGS) -c graphics.c graphics.h femta.h

selector.o: selector.c selector.h linked-list.h femta.h
	gcc $(GFLAGS) -c selector.c selector.h linked-list.h femta.h

scripter.o: scripter.c scripter.h hashmap.h error.h colors.h
	gcc $(GFLAGS) -c scripter.c scripter.h hashmap.h error.h colors.h

logger.o: logger.c logger.h
	gcc $(GFLAGS) -c logger.c logger.h

timing.o: timing.c timing.h
	gcc $(GFLAGS) -c timing.c timing.h

hashmap.o: hashmap.c hashmap.h linked-list.h
	gcc $(GFLAGS) -c hashmap.c hashmap.h linked-list.h

linked-list.o: linked-list.c linked-list.h
	gcc $(GFLAGS) -c linked-list.c linked-list.h 

error.o: error.c error.h
	gcc $(GFLAGS) -c error.c error.h

clean:
	rm -f *.o *.gch                          # blow away compiled objects
	mv *~ \#* backups/ 2>/dev/null || true   # blow away old backups
	rm -f femta                              # blow away binary

