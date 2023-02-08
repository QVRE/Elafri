#pragma once
#define ELAFRI_SDL
#include <sys/select.h>
#include <sys/time.h>
#include <SDL2/SDL.h>
#include "graphics.h"
#include "evar.h"

#define InitTimer(t) struct timeval t ## _tstart, t ## _tend
#define StartTimer(t) gettimeofday(&t ## _tstart, NULL)
#define StopTimer(t) gettimeofday(&t ## _tend, NULL)
#define dt_sec(t) (t ## _tend.tv_sec-t ## _tstart.tv_sec)
#define dt_usec(t) (t ## _tend.tv_usec-t ## _tstart.tv_usec)
#define Sleep(time) select(1, NULL, NULL, NULL, &time)

u8 kbd[322]; //array of key states
ivec2 m; //last observed mouse cords
u32 mC; //mouse bitmask

void ElafriInit(char* win_title, u32 w, u32 h); //will initialize Elafri and open a new window

void Exit(); //closes window and calls exit()

//limits your loop to a max framerate and returns delta time
F32 FramerateHandler(u32 max_fps);

enum Elafri_KeyState {
	KEY_UP = 0, KEY_DOWN = 1, KEY_PRESSED = 2, KEY_RELEASED = 3
};

//fetches keyboard and mouse input
void Input();
