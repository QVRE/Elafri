#ifndef _EGFX_SDL
#define _EGFX_SDL
#include <SDL2/SDL.h>
#include "evar.c"

#define BLACK (color){0,0,0,255}
#define WHITE (color){255,255,255,255}
#define RED (color){255,0,0,255}
#define GREEN (color){0,255,0,255}
#define BLUE (color){0,0,255,255}
#define YELLOW (color){255,255,0,255}
#define CYAN (color){0,255,255,255}

typedef struct Color {u8 r,g,b,a;} color;
typedef struct GrBuffer {color *pal; u32 w,h;} gr;

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
uvec2 res; //window resolution

gr GrBuffer(u32 width, u32 height) {
	return (gr){calloc(width*height, sizeof(color)), width, height};
}
static inline void GrPixel(gr *buf, int x, int y, color clr) {
	if (x >= 0 && y >= 0 && x < buf->w && y < buf->h)
		buf->pal[y*buf->w+x] = clr;
}
void GrFill(gr *buf, const color clr) {
	color *ptr = buf->pal; //point to start of graphics buffer
	u32 size = buf->w*buf->h;
	while (size--)
		*ptr++ = clr;
}
void GrFree(gr *buf) {
	free(buf->pal);
}

void GrLine(gr *b, ivec2 A, ivec2 B, color clr) {
	int dx = B.x-A.x; //Δx & Δy
	int dy = B.y-A.y;
	GrPixel(b, B.x, B.y, clr);
	if (abs(dx) > abs(dy))
		for (int c=0,i=A.x; i != B.x; i+=s(dx), c++)
			GrPixel(b, i, A.y+(c*dy)/abs(dx), clr);
	else
		for (int c=0,i=A.y; i != B.y; i+=s(dy), c++)
			GrPixel(b, A.x+(c*dx)/abs(dy), i, clr);
}

void GrCircle(gr *b, const ivec2 O, int radius, int skip, color clr) {
	for (int i=0; i<ROT; i+=skip) //loop through precomputes
		GrPixel(b, O.x+(int)roundf(wcos[i]*radius), O.y+(int)roundf(wsin[i]*radius), clr);
}
void GrCircleFilled(gr *b, const ivec2 O, const int radius, color clr) {
	for (int y=-radius; y<=radius; y++)
		for (int x=-radius; x<=radius; x++)
			if (x*x+y*y <= radius*radius) //check if inside
				GrPixel(b, O.x+x, O.y+y, clr);
}

void draw(gr *buf) {
	SDL_UpdateTexture(texture, NULL, buf->pal, buf->w * sizeof(color));
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}
static inline void drawc(gr *buffer) { //draw and then clear
	draw(buffer);
	GrFill(buffer, BLACK);
}
#endif
