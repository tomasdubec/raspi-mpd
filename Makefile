LFLAGS=-l pthread -l wiringPiDev -l wiringPi -l mpdclient
INCLUDE=include

all: objects/main.o objects/input.o objects/lcd.o objects/mpd.o
	gcc ${LFLAGS} objects/*.o -o raspi-mpd

objects/main.o: src/main.c include/logging.h
	gcc -c -I ${INCLUDE} src/main.c -o objects/main.o

objects/input.o: src/input.c include/logging.h
	gcc -c -I ${INCLUDE} src/input.c -o objects/input.o

objects/lcd.o: src/lcd.c include/logging.h
	gcc -c -I ${INCLUDE} src/lcd.c -o objects/lcd.o

objects/mpd.o: src/mpd.c include/logging.h
	gcc -c -I ${INCLUDE} src/mpd.c -o objects/mpd.o

clean:
	rm -f objects/* 2> /dev/null
	[ -e raspi-mpd ] && rm raspi-mpd
