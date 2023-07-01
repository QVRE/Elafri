#pragma once
#include <SDL2/SDL.h>
#include <stdlib.h>
#include <math.h>
#include "evar.h"

#define BLACK (color){0,0,0,255}
#define WHITE (color){255,255,255,255}
#define RED (color){255,0,0,255}
#define GREEN (color){0,255,0,255}
#define BLUE (color){0,0,255,255}
#define YELLOW (color){255,255,0,255}
#define CYAN (color){0,255,255,255}

#define ROT 1024 //accuracy of precomputes for sine/cosine

extern F32 sinebuf[ROT*2+ROT/4]; //precomputes for fast Circle drawing and lookup
extern F32 *wsin;
extern F32 *wcos;

typedef struct {u8 r,g,b,a;} color;
typedef struct GrBuffer {color *dat; u32 w,h;} gr;

extern SDL_Window *window;
extern SDL_Renderer *renderer;
extern SDL_Texture *texture;
extern uvec2 res; //window resolution

static inline void GrInit() {
	for (int i=0; i<ROT*2+ROT/4; i++) //Initialize sinewave precomputes
		sinebuf[i] = sinf((2*M_PI/ROT)*i);
	wsin = sinebuf+ROT; //allows for negative indexes too (up to ROT)
	wcos = &wsin[ROT/4];
}

static inline gr GrBuffer(u32 width, u32 height) {
	return (gr){calloc(width*height, sizeof(color)), width, height};
}
static inline void GrFree(gr *buf) {
	free(buf->dat);
}

void GrPixel(gr *buf, int x, int y, color clr);
void GrFill(gr *buf, const color clr);

void GrLine(gr *b, ivec2 A, ivec2 B, color clr);

void GrTriangleWire(gr *buf, ivec2 A, ivec2 B, ivec2 C, color clr);
void GrTriangle(gr *buf, ivec2 A, ivec2 B, ivec2 C, color clr);

//for very big circles, set skip to 1. You can raise it higher for optimizing smaller circles
void GrCircle(gr *b, const ivec2 O, int radius, int skip, color clr);
void GrCircleFilled(gr *b, const ivec2 O, const int radius, color clr);

void Draw(gr *buf);

static inline void DrawFill(gr *buffer, color clr) { //draw and then fill with color
	Draw(buffer);
	GrFill(buffer, clr);
}
