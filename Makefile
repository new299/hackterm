hterm: main.c nunifont.c nunifont.h
	gcc -g main.c nunifont.c -o hterm -I./libvterm/include -L./libvterm/.libs -lvterm -lSDL -std=c99 -lutil 

clean:
	rm hterm
