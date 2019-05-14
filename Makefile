CFLAGS=-Wall -O1
LIBS=
CC=g++
OBJS=main.o Pwm.o
OUT=main.out

all: main.out

main.out: $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean: 
	rm -rf $(OUT) $(OBJS)

%.o: %.cpp
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)
	
%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(LIBS)