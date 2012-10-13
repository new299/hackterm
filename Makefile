hterm: main.cpp nunifont.cpp nunifont.h
	g++ -g main.cpp nunifont.cpp -o hterm -lSDL

clean:
	rm hterm
