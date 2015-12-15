
CC = g++
LFLAGS = -Wall -pthread
CFLAGS = -c -Wall -pthread

PROGRAM = ossim
SRCS = driver.cpp ossim.cpp Timer.cpp
OBJS = driver.o ossim.o Timer.o
HDRS = ossim.h Timer.h

all: $(PROGRAM)

$(PROGRAM): $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o $(PROGRAM)

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

ossim.o: ossim.cpp ossim.h
Timer.o: Timer.cpp Timer.h

clean:
	rm -f *.o *~ $(PROGRAM)
