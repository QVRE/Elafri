#ifndef _ELAFRI_SDL
#define _ELAFRI_SDL
#include <SDL2/SDL.h>
#include "evar.c"
#include "graphics.c" //this is a bit too important to be considered an optional component

u8 kbd[322]; //array of key states
ivec2 m; //last observed mouse cords
u32 mC; //mouse bitmask

void ElafriInit(char* win_title, uvec2 resolution) { //Init procedures should be added here
	SineInit();

	SDL_Init(SDL_INIT_VIDEO); //SDL init
	res = resolution;
	const int undef = SDL_WINDOWPOS_UNDEFINED;
	//make window
	window = SDL_CreateWindow(win_title, undef, undef, res.x, res.y, SDL_WINDOW_OPENGL);
	//make renderer that's accelerated and synced with refresh rate
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); //likely not necessary for graphics.c
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, res.x, res.y);
}

void Exit() { //intended for any important exit procedures
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
	exit(0);
}

F32 FramerateHandler(u32 max_fps) { //handles a framerate limit and returns delta time
	static InitTimer(ftimer); //timer
	int exec_time, wait_time;
	static u32 fps; //last frame's fps
	fps ^= (fps==0); //safeguards division by 0

	StopTimer(ftimer);

	exec_time = mod32(dt_usec(ftimer), 1000000); //Compute time it took for execution
	wait_time = 1000000 / fps - exec_time; //Check if behind/ahead
	fps = max(min(1000000*fps / (1000000-wait_time*fps), max_fps), 1); //Compute new fps

	ftimer_tend.tv_usec = max(1000000 / max_fps - exec_time, 0);
	ftimer_tend.tv_sec=0;
	select(1, NULL, NULL, NULL, &ftimer_tend); //sleep if over max fps

	StartTimer(ftimer);
	return 1. / fps;
}

enum Elafri_KeyState {
	KEY_UP = 0, KEY_DOWN = 1, KEY_PRESSED = 2, KEY_RELEASED = 3
};

void Input() { //read input into buffers
	SDL_Event event;
	for (u32 i=0; i<322; i++)
		if (kbd[i] == KEY_PRESSED) kbd[i] = KEY_DOWN;
		else if (kbd[i] == KEY_RELEASED) kbd[i] = KEY_UP;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT: //if window closed
				Exit(); //write your important exit procedures here
				break;
			case SDL_KEYDOWN:
				kbd[event.key.keysym.scancode] = KEY_PRESSED;
				break;
			case SDL_KEYUP:
				kbd[event.key.keysym.scancode] = KEY_RELEASED;
				break;
		}
	}
	mC = SDL_GetMouseState(&m.x, &m.y); //read mouse info
}
#endif
