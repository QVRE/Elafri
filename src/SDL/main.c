#include "Elafri.c"

#define FPS 60
#define MPS 1000000 // s/μs

int main() {
	ElafriInit("Test", (uvec2){640, 480});

	gr Gr = GrBuffer(res.x,res.y);

	/*Timekeeping-related Inits*/
	InitTimer(ftimer); //timer
	struct timeval mtv = {0,1};
	u32 fps = FPS;
	int exec_time, wt;
	F32 delta=0; //Δt per frame

    u32 circle_count = 0, mouse_click = 1;
    ivec2 circles[2048];
    F32 time = 0, time_offsets[2048];

	RenderLoop:
	StartTimer(ftimer);

	Input(&mtv); //https://wiki.libsdl.org/SDL_Keycode
	if (kbd[SDL_SCANCODE_SPACE]) circle_count = 0;
	if (kbd[SDL_SCANCODE_ESCAPE]) Exit();
	if (mC & SDL_BUTTON_LMASK & mouse_click || mC & SDL_BUTTON_RMASK) { //left click or right down
		circles[circle_count] = m;
		time_offsets[circle_count] = time;
		circle_count = (circle_count + 1) & 2047;
		mouse_click = 0;
	} else mouse_click = ~mC & SDL_BUTTON_LMASK;

    GrCircle(&Gr, m, 8, 16, GREEN); //put circle around mouse
    time = fmodf(time + delta, 2*pi);
    for (int i=0; i<circle_count; i++) {
		GrCircle(&Gr, circles[i], 8.-7*sinf(time-time_offsets[i]), 8, CYAN);
	}

	drawc(&Gr);

	StopTimer(ftimer);
	exec_time = mod32(dt_usec(ftimer),MPS); //Compute execution Δt
	wt = MPS/fps-exec_time; //Check if behind/ahead
	fps = max(min(MPS*fps/(MPS-wt*fps), FPS), 1); //Compute new fps
	delta = 1. / fps; //sligtly accurate Δt

	ftimer_tend.tv_usec = max(MPS/FPS-exec_time, 0);
	ftimer_tend.tv_sec=0;
	select(1, NULL, NULL, NULL, &ftimer_tend); //wait if over max fps
	goto RenderLoop;
}
