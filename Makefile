UNAME := $(shell sh -c 'uname 2>/dev/null || echo not')

ifeq ($(UNAME), Linux)
OPTS = -DLINUX_BUILD -DLOCAL_ENABLE -pthread -I./linux
endif
ifeq ($(UNAME), Darwin)
SDLOPTS =  -lm -liconv -Wl,-framework,OpenGL  -Wl,-framework,ForceFeedback -lobjc -Wl,-framework,Cocoa -Wl,-framework,Carbon -Wl,-framework,IOKit -Wl,-framework,CoreAudio -Wl,-framework,AudioToolbox -Wl,-framework,AudioUnit -I./osx
OPTS = -DOSX_BUILD -DLOCAL_ENABLE -framework CoreAudio -framework Cocoa -framework OpenGL -lSDLmain -Wl,-framework,Cocoa -Wl,-framework,OpenGL $(SDLOPTS)
EXTRAC = ./osx/osx_pasteboard.m
endif

LIBVTERMC = ./libvterm/src/parser.c  ./libvterm/src/screen.c  ./libvterm/src/input.c   ./libvterm/src/vterm.c   ./libvterm/src/unicode.c ./libvterm/src/state.c   ./libvterm/src/encoding.c ./libvterm/src/pen.c

LIBSSH2C = ./libssh2/src/agent.c    ./libssh2/src/transport.c ./libssh2/src/version.c  ./libssh2/src/scp.c      ./libssh2/src/knownhost.c ./libssh2/src/publickey.c ./libssh2/src/mac.c      ./libssh2/src/keepalive.c ./libssh2/src/misc.c     ./libssh2/src/kex.c      ./libssh2/src/sftp.c     ./libssh2/src/session.c  ./libssh2/src/packet.c   ./libssh2/src/openssl.c  ./libssh2/src/comp.c     ./libssh2/src/pem.c      ./libssh2/src/global.c   ./libssh2/src/hostkey.c  ./libssh2/src/channel.c  ./libssh2/src/userauth.c ./libssh2/src/libgcrypt.c ./libssh2/src/crypt.c

LIBSDLC = ./libsdl

OURC = main.c base64.c inlinedata.c regis.c nunifont.c nsdl.c ngui.c ssh.c local.c ngui_info_prompt.c ngui_textlabel.c ngui_textbox.c ngui_button.c ngui_stringselect.c

hterm: main.c nunifont.c nunifont.h *.c *.h
	#You'll need to uncomment this for a new build.
	#cd libsdl ;./configure
	#make -C ./libsdl
	#cd libpng ;./configure
	#make -C ./libpng
	find . -name *.dylib -exec rm {} \;
	find . -name *.so* -exec rm {} \;
	#don't need this: find ./libsdl -name SDL_config.h -exec rm {} \;
	gcc -O3 -std=gnu99 $(LIBVTERMC) $(OURC) $(LIBSSH2C) $(OPTS) $(EXTRAC) ./utf8proc/utf8proc.c -o hterm -I./libpng -I./utf8proc -I./libvterm/include -I./libsdl/include -L./libsdl/build -L./libpng/.libs -L./libsdl/build/.libs -lpng15 -lSDL2 -lutil -lcrypto -I./libssh2/include -lz -lm
	

unifont_conv: unifont_conv.c nunifont.c
	gcc -g -std=gnu99 unifont_conv.c nunifont.c -o unifont_conv


clean:
	rm -rf hterm

install:
	@mkdir -p $(DESTDIR)$(PREFIX)/bin/
	cp ./hterm $(DESTDIR)$(PREFIX)/bin/

deb: 
	tar czvf ../hterm_0.0.1.orig.tar.gz ../hackterm
	debuild -us -uc

#xxd -i ./fontmap_static  fontmap_static.h

