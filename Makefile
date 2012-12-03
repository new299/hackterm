hterm: main.c nunifont.c nunifont.h *.c *.h
	gcc -std=gnu99 -g main.c regis.c nunifont.c nsdl.c ssh.c -o hterm -I./libvterm/include -L./libvterm/.libs -L./libssh2/src/.libs -lssh2 -lvterm -lSDL -lutil -I./libssh2/include  
#gcc -std=gnu99 -g main.c regis.c nunifont.c nsdl.c ssh.c -o hterm -I./libvterm/include -L./libvterm/.libs -lvterm -lSDL -lutil -L./utf8proc -I./utf8proc -I./libssh2/include -lutf8proc

clean:
	rm hterm
