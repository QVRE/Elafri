#pragma once
#define ELAFRI_TERM
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>
#include <termios.h> //for term flags
#include "graphics.h"
#include "evar.h"

#define InitTimer(t) struct timeval t ## _tstart, t ## _tend
#define StartTimer(t) gettimeofday(&t ## _tstart, NULL)
#define StopTimer(t) gettimeofday(&t ## _tend, NULL)
#define dt_sec(t) (t ## _tend.tv_sec-t ## _tstart.tv_sec)
#define dt_usec(t) (t ## _tend.tv_usec-t ## _tstart.tv_usec)
#define Sleep(time) select(1, NULL, NULL, NULL, &time)

//extended keyboard codes. use these if you see 255 in kbd[] for the next element
#define INSERT 50 //in kbd[], this would be 255 50
#define DELETE 51
#define PgUP 53
#define PgDown 54
#define UP 65
#define DOWN 66
#define RIGHT 67
#define LEFT 68
#define END 70
#define HOME 72

u8 kbd[64]; //parsed keyboard input
u32 mC; //mouse code
ivec2 m; //mouse cords
uvec2 res; //term resolution

struct termios oldTermFlags; //used to restore previous state of terminal

void GetResolution(u32 *x, u32 *y); //writes resolution of terminal to x & y

void ElafriInit(); //call this to initialize

void Exit(); //resets terminal and calls exit()

//limits your loop to a max framerate and returns delta time
F32 FramerateHandler(u32 max_fps);

//fetches keyboard and mouse input. Returns number of bytes written to kbd
u32 Input();
