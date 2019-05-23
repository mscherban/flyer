CFLAGS=-Wall -O1
LIBS=-lpthread
CC=g++
OBJS=main.o Pwm.o BNO055_Imu.o
OUT=main.out

all: main.out

main.out: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean: 
	rm -rf $(OUT) $(OBJS)

%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)
	
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)