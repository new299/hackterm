UNAME := $(shell sh -c 'uname 2>/dev/null || echo not')

ifeq ($(UNAME), Linux)
endif
ifeq ($(UNAME), Darwin)
OPTS = -L/opt/local/lib -I/opt/local/include -framework Cocoa -lSDLmain -Wl,-framework,Cocoa
endif

LIBVTERMC = ./libvterm/src/parser.c  ./libvterm/src/screen.c  ./libvterm/src/input.c   ./libvterm/src/vterm.c   ./libvterm/src/unicode.c ./libvterm/src/state.c   ./libvterm/src/encoding.c ./libvterm/src/pen.c

LIBSSH2C = ./libssh2/src/agent.c    ./libssh2/src/transport.c ./libssh2/src/version.c  ./libssh2/src/scp.c      ./libssh2/src/knownhost.c ./libssh2/src/publickey.c ./libssh2/src/mac.c      ./libssh2/src/keepalive.c ./libssh2/src/misc.c     ./libssh2/src/kex.c      ./libssh2/src/sftp.c     ./libssh2/src/session.c  ./libssh2/src/packet.c   ./libssh2/src/openssl.c  ./libssh2/src/comp.c     ./libssh2/src/pem.c      ./libssh2/src/global.c   ./libssh2/src/hostkey.c  ./libssh2/src/channel.c  ./libssh2/src/userauth.c ./libssh2/src/libgcrypt.c ./libssh2/src/crypt.c

OURC = main.c base64.c inlinedata.c regis.c nunifont.c nsdl.c ngui.c ssh.c local.c 

hterm: main.c nunifont.c nunifont.h *.c *.h
	gcc -g -std=gnu99 $(LIBVTERMC) $(OURC) $(OPTS) $(LIBSSH2C) -o hterm -I./libvterm/include -lpng -lSDL -lutil -lcrypto -I./libssh2/include  

clean:
	rm -rf hterm

install:
	cp ./hterm $(DESTDIR)/bin

deb: 
	tar czvf ../hterm_0.0.1.orig.tar.gz ../hackterm
	debuild -us -uc
