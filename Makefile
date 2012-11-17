hterm: main.c nunifont.c nunifont.h *.c *.h
	gcc -std=gnu99 -g main.c nunifont.c nsdl.c -o hterm -I./libvterm/include -L./libvterm/.libs -lvterm -lSDL -lutil -L./utf8proc -I./utf8proc -lutf8proc 

clean:
	rm hterm
