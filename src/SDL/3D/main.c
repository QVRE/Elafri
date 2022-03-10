#include "Elafri.c"
#include "render.c"

#define FPS 60
#define MPS 1000000 // s/μs

int main() {
	ElafriInit("Elafri 3D SDL example", (uvec2){1280, 960});

	gr Gr = GrBuffer(res.x,res.y);

	/*Timekeeping-related Inits*/
	InitTimer(ftimer); //timer
	struct timeval mtv = {0,1};
	u32 fps = FPS;
	int exec_time, wt;
	F32 delta=0; //Δt per frame

	obj cube = {{0,0,5},{0},{1,1,1},MallocMat4x4(),12,8,malloc(8*sizeof(vec3)),malloc(12*sizeof(vec3)),malloc(12*sizeof(face))};
	const vec3 ct_p[8] = {{1,1,1}, {-1,1,1}, {1,1,-1}, {-1,1,-1},
		{1,-1,1}, {-1,-1,1}, {1,-1,-1}, {-1,-1,-1}};
	const u32 ct_f[] = {0,1,2,3,2,1,0,4,1,5,1,4,0,2,4,6,4,2,2,3,6,7,6,3,1,5,3,5,7,3,5,4,6,5,6,7};
	const color ct_c[6] = {GREEN,RED,BLUE,YELLOW,CYAN,WHITE};
	for (int i=0; i<8; i++) cube.vert[i] = ct_p[i];
	for (int i=0; i<12; i++) {
		cube.f[i] = (face){ct_f[i*3],ct_f[i*3+1],ct_f[i*3+2],ct_c[i/2]};
		cube.norm[i] = GetTriangleNormal(cube.vert[cube.f[i].a],
										cube.vert[cube.f[i].b],
										cube.vert[cube.f[i].c]);
	}

	vec3 pos={0},rot={0}; //camera pos & rot
	vec2 vFront = {0,1};
	F32 speed = 4;
	AllocDepthBuffer(res);
	AllocProjectedPointBuffer(50); //set this as high as the biggest object vert count
	vec3 param = GrMake3DParams(90, (F32)res.y/res.x, 100);
	u32 t = 0;

	RenderLoop:
	StartTimer(ftimer);

	Input(&mtv); //https://wiki.libsdl.org/SDL_Keycode
	if (kbd[SDL_SCANCODE_W]) pos.x += vFront.x*speed*delta, pos.z += vFront.y*speed*delta;
	if (kbd[SDL_SCANCODE_S]) pos.x -= vFront.x*speed*delta, pos.z -= vFront.y*speed*delta;
	if (kbd[SDL_SCANCODE_D]) pos.x += vFront.y*speed*delta, pos.z -= vFront.x*speed*delta;
	if (kbd[SDL_SCANCODE_A]) pos.x -= vFront.y*speed*delta, pos.z += vFront.x*speed*delta;
	if (kbd[SDL_SCANCODE_E]) pos.y += speed*delta;
	if (kbd[SDL_SCANCODE_Q]) pos.y -= speed*delta;
	if (kbd[SDL_SCANCODE_T] == KEY_PRESSED) t = !t;
	if (kbd[SDL_SCANCODE_LEFTBRACKET] == KEY_PRESSED) speed *= 0.5;
	if (kbd[SDL_SCANCODE_RIGHTBRACKET] == KEY_PRESSED) speed *= 2;
	if (kbd[SDL_SCANCODE_LEFT]) rot.y = modF32(rot.y-delta*2, 2*pi), vFront = (vec2){sinf(rot.y), cosf(rot.y)};
	if (kbd[SDL_SCANCODE_RIGHT]) rot.y = fmodf(rot.y+delta*2, 2*pi), vFront = (vec2){sinf(rot.y), cosf(rot.y)};
	if (kbd[SDL_SCANCODE_UP]) rot.x = fmodf(rot.x-delta*2, 2*pi);
	if (kbd[SDL_SCANCODE_DOWN]) rot.x = modF32(rot.x+delta*2, 2*pi);

	DepthReset(res);
	GrObject(&Gr, cube, pos, rot, param);
	if (t) //depth mode, colors screen by depth
		for (int y=0; y<res.y; y++)
			for (int x=0; x<res.x; x++) {
				const F32 u = (0.5-depth[y*res.x+x]/2)*255 > 255 ? 255 : (int)roundf(255*depth[y*res.x+x]);
				Gr.pal[y*res.x+x] = (color){u,u,u,255};
			}
	cube.rot.x = modF32(cube.rot.x+delta*0.31*pi,2*pi),
	cube.rot.y = modF32(cube.rot.y+delta*0.44*pi,2*pi),
	cube.rot.z = modF32(cube.rot.z+delta*0.75*pi,2*pi);

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
