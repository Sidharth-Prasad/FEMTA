
# C linking flags
CFLAGS = -lpigpio -lrt -lpthread -lncurses -lmenu -lm

# Granular flags
GFLAGS =

femta-debug: GFLAGS += -g
femta-debug: femta

femta: femta.o controller.o i2c-interface.o serial.o temperature.o graphics.o selector.o scripter.o logger.o timing.o hashmap.o linked-list.o quaternion.o error.o
	gcc $(CFLAGS) $(GFLAGS) -o femta.x femta.o controller.o i2c-interface.o serial.o temperature.o graphics.o selector.o scripter.o logger.o timing.o hashmap.o linked-list.o quaternion.o error.o

femta.o: femta.c femta.h controller.h i2c-interface.h serial.h temperature.h graphics.h selector.h scripter.h linked-list.h logger.h error.h colors.h
	gcc $(GFLAGS) -c femta.c femta.h controller.h i2c-interface.h serial.h temperature.h graphics.h selector.h linked-list.h scripter.h logger.h error.h colors.h

controller.o: controller.c controller.h femta.h quaternion.h graphics.h selector.h colors.h
	gcc $(GFLAGS) -c controller.c controller.h femta.h quaternion.h graphics.h selector.h colors.h

i2c-interface.o: i2c-interface.c i2c-interface.h femta.h graphics.h timing.h logger.h colors.h types.h
	gcc $(GFLAGS) -c i2c-interface.c i2c-interface.h femta.h graphics.h timing.h logger.h colors.h types.h

serial.o: serial.c serial.h femta.h graphics.h timing.h error.h colors.h types.h
	gcc $(GFLAGS) -c serial.c serial.h femta.h graphics.h timing.h error.h colors.h types.h

temperature.o: temperature.c temperature.h femta.h i2c-interface.h linked-list.h graphics.h logger.h colors.h
	gcc $(GFLAGS) -c temperature.c temperature.h femta.h i2c-interface.h linked-list.h graphics.h logger.h colors.h

graphics.o: graphics.c graphics.h femta.h selector.h
	gcc $(GFLAGS) -c graphics.c graphics.h femta.h selector.h

selector.o: selector.c selector.h linked-list.h femta.h
	gcc $(GFLAGS) -c selector.c selector.h linked-list.h femta.h

scripter.o: scripter.c scripter.h hashmap.h error.h colors.h
	gcc $(GFLAGS) -c scripter.c scripter.h hashmap.h error.h colors.h

linked-list.o: linked-list.c linked-list.h
	gcc $(GFLAGS) -c linked-list.c linked-list.h 

logger.o: logger.c logger.h
	gcc $(GFLAGS) -c logger.c logger.h

timing.o: timing.c timing.h
	gcc $(GFLAGS) -c timing.c timing.h

hashmap.o: hashmap.c hashmap.h linked-list.h
	gcc $(GFLAGS) -c hashmap.c hashmap.h linked-list.h

quaternion.o: quaternion.c quaternion.h
	gcc $(GFLAGS) -c quaternion.c quaternion.h

error.o: error.c error.h logger.h
	gcc $(GFLAGS) -c error.c error.h logger.h

clean:
	rm -f *.o *.gch                          # blow away compiled objects
	mv *~ \#* backups/ 2>/dev/null || true   # blow away old backups
	rm -f *.x                                # blow away binaries

