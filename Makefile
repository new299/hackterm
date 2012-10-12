example_linux: main.cpp nfont.cpp nfont.h
	convert font.png -depth 2 gray:font_data
	ld -r -b binary -o font_data.o font_data
	g++ -g main.cpp nfont.cpp -o example -lSDL


clean:
	rm example
