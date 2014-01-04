LFLAGS=-l pthread -l wiringPiDev -l wiringPi -l mpdclient -lasound -lm
INCLUDE=include
CFLAGS=-g

all: objects/main.o objects/input.o objects/lcd.o objects/mpd.o objects/ctrl.o objects/alsa.o objects/menu.o
	gcc ${LFLAGS} objects/*.o -o raspi-mpd

objects/main.o: src/main.c include/logging.h
	gcc -c ${CFLAGS} -I ${INCLUDE} src/main.c -o objects/main.o

objects/input.o: src/input.c include/logging.h
	gcc -c ${CFLAGS} -I ${INCLUDE} src/input.c -o objects/input.o

objects/lcd.o: src/lcd.c include/logging.h
	gcc -c ${CFLAGS} -I ${INCLUDE} src/lcd.c -o objects/lcd.o

objects/mpd.o: src/mpd.c include/logging.h
	gcc -c ${CFLAGS} -I ${INCLUDE} src/mpd.c -o objects/mpd.o

objects/ctrl.o: src/ctrl.c include/logging.h
	gcc -c ${CFLAGS} -I ${INCLUDE} src/ctrl.c -o objects/ctrl.o

objects/alsa.o: src/alsa.c include/logging.h
	gcc -c ${CFLAGS} -I ${INCLUDE} src/alsa.c -o objects/alsa.o

objects/menu.o: src/menu.c include/logging.h
	gcc -c ${CFLAGS} -I ${INCLUDE} src/menu.c -o objects/menu.o

clean:
	rm -f objects/* 2> /dev/null
	[ -e raspi-mpd ] && rm raspi-mpd
