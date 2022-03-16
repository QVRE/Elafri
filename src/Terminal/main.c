#include "Elafri.c"

#define rx 100 //resolution
#define ry 50
#define FPS 50
#define MPS 1000000 // s/μs

int main()
{
	ElafriInit(); //calls all the init functions, input, term flags, sine, output buffer

	gr Gr = GrBuffer(res.x,res.y); //allocates drawing buffer with blanking spaces

	/*Timekeeping-related Inits*/
	InitTimer(ftimer); //timer
	struct timeval mtv = {0,1}; //needed for querying input
	u32 fps = FPS;
	int exec_time, wt;
	F32 delta=0; //Δt per frame

	RenderLoop:
		StartTimer(ftimer);

		Input(&mtv); //get user input since last frame
		GrLine(&Gr,(ivec2){0,0},m,CYAN); //line from 0,0 to mouse on Gr buffer
		GrCircle(&Gr,m,5,ROT/16,GREEN); //draw unfinished circle around mouse (r=5)
		drawc(&Gr); //outputs and clears buffer

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
