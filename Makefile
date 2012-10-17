hterm: main.c nunifont.c nunifont.h
	gcc -g main.c nunifont.c -o hterm -I./libvterm/include -L./libvterm/.libs -lvterm -lSDL -std=c99 -lutil -L./utf8proc -I./utf8proc -lutf8proc

clean:
	rm hterm
