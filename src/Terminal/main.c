#include "Elafri.c"

#define FPS 50

int main()
{
	ElafriInit(); //calls all the init functions, input, term flags, sine, output buffer

	gr Gr = GrBuffer(res.x,res.y); //allocates drawing buffer

	RenderLoop:
	Input(); //get user input since last frame (returns written size to kbd)

	GrLine(&Gr,(ivec2){0,0},m,CYAN); //line from 0,0 to mouse on Gr buffer
	GrCircle(&Gr,m,5,ROT/16,GREEN); //draw unfinished circle around mouse (r=5)
	drawc(&Gr, BLACK); //outputs and clears buffer

	FramerateHandler(FPS);
	goto RenderLoop;
}
