
# C linking flags
CFLAGS = -lpigpio -lpthread
PLOT_FLAGS = -lncurses

# Granular flags
GFLAGS =

.DEFAULT_GOAL := origin

debug: GFLAGS += -g
debug: origin

origin: adxl.o clock.o i2c.o list.o origin.o selector.o sensor.o
	gcc $(GFLAGS) -o origin.x adxl.o clock.o i2c.o list.o origin.o selector.o sensor.o $(CFLAGS)
origin:	plot

adxl.o: adxl.c adxl.h i2c.h
	gcc $(GFLAGS) -c adxl.c adxl.h i2c.h

clock.o: clock.c clock.h
	gcc $(GFLAGS) -c clock.c clock.h

i2c.o: i2c.c adxl.h clock.h color.h i2c.h list.h sensor.h
	gcc $(GFLAGS) -c i2c.c adxl.h clock.h color.h i2c.h list.h sensor.h

list.o: list.c list.h
	gcc $(GFLAGS) -c list.c list.h

origin.o: origin.c clock.h color.h i2c.h list.h selector.h types.h
	gcc $(GFLAGS) -c origin.c clock.h color.h i2c.h list.h selector.h types.h

selector.o: selector.c list.h selector.h
	gcc $(GFLAGS) -c selector.c list.h selector.h

sensor.o: sensor.c adxl.h list.h sensor.h
	gcc $(GFLAGS) -c sensor.c adxl.h list.h sensor.h

plot: plot.o list.o
	gcc $(PLOT_FLAGS) -o plot.x plot.o list.o

plot.o: plot.c list.h
	gcc $(GFLAGS) -c plot.c list.h

clean:
	rm -f *.o *.gch   # blow away compiled objects
	rm -f *.x         # blow away binaries
	rm -f gmon.out    # blow away profiling outputs
