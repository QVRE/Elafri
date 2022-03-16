# Builds all of Elafri's ports into example executables

TERM_BUILD_FLAGS = -O3 -flto -fno-math-errno -lm
SDL_BUILD_FLAGS = -O3 -flto -fno-math-errno -lSDL2 -lm

all: terminal sdl

terminal: src/Terminal/main.c
	$(CC) $(TERM_BUILD_FLAGS) src/Terminal/main.c -o terminal

sdl: src/SDL/main.c
	$(CC) $(SDL_BUILD_FLAGS) src/SDL/main.c -o sdl

clean:
	rm terminal sdl
