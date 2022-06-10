#include "Elafri.c"

#define FPS 60

int main() {
	ElafriInit("Test", (uvec2){640, 480});

	gr Gr = GrBuffer(res.x,res.y);

	u32 circle_count = 0, mouse_click = 1;
	ivec2 circles[2048];
	F32 time = 0, time_offsets[2048];

	RenderLoop:
	Input(); //https://wiki.libsdl.org/SDL_Keycode
	if (kbd[SDL_SCANCODE_SPACE]) circle_count = 0;
	if (kbd[SDL_SCANCODE_ESCAPE]) Exit();
	if (mC & SDL_BUTTON_LMASK & mouse_click || mC & SDL_BUTTON_RMASK) { //left click or right down
		circles[circle_count] = m;
		time_offsets[circle_count] = time;
		circle_count = (circle_count + 1) & 2047;
		mouse_click = 0;
	} else mouse_click = ~mC & SDL_BUTTON_LMASK;

	GrCircle(&Gr, m, 8, 16, GREEN); //put circle around mouse
	for (int i=0; i<circle_count; i++) {
		GrCircle(&Gr, circles[i], 8.-7*sinf(time-time_offsets[i]), 8, CYAN);
	}
	DrawFill(&Gr, BLACK);

	time = fmodf(time + FramerateHandler(FPS), 2*M_PI);
	goto RenderLoop;
}
