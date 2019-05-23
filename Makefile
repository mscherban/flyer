CFLAGS=-Wall -O1
LIBS=-lpthread
CC=g++
OBJS=main.o Pwm.o BNO055_Imu.o
OUT=main.out

all: main.out kp

main.out: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

kp: killpwms.o Pwm.o
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean: 
	rm -rf $(OUT) $(OBJS) kp killpwms.o

%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS)
	
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS)