#include "Elafri.h"

void ElafriInit(char* win_title, u32 w, u32 h) { //Init procedures
	GrInit();

	SDL_Init(SDL_INIT_VIDEO); //SDL init
	res = (uvec2){w, h};
	const int undef = SDL_WINDOWPOS_UNDEFINED;
	//make window
	window = SDL_CreateWindow(win_title, undef, undef, w, h, SDL_WINDOW_OPENGL);
	//make renderer that's accelerated and synced with refresh rate
	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
	//SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); //likely not necessary for graphics.c
	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, w, h);
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

	exec_time = dt_usec(ftimer); //calculate time it took for execution
	exec_time += exec_time < 0 ? 1000000 : 0; //loop back if negative
	wait_time = 1000000 / fps - exec_time; //check if behind/ahead
	fps = 1000000*fps / (1000000 - wait_time*fps); //compute new fps
	fps = fps > max_fps ? max_fps : fps; //clamp it to max framerate

	ftimer_tend.tv_usec = 1000000 / max_fps - exec_time;
	if (ftimer_tend.tv_usec < 0) ftimer_tend.tv_usec = 0;
	ftimer_tend.tv_sec = 0;
	select(1, NULL, NULL, NULL, &ftimer_tend); //sleep if over max fps

	StartTimer(ftimer);
	return 1. / fps; //return delta time
}

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
