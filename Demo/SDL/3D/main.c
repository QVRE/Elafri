#include "Elafri.c"
#include "render.c"

#define FPS 60

//Some small code I wrote to read a basic untextured / uncolored 3D object file
obj ReadObjFile(char *filename) {
	FILE *fp = fopen(filename, "r");
	F32 a,b,c; //these are used temporarily
	u32 v[3],i,w; //3 temp face vertices and counters
	u32 v_off=0, f_off=0, v_alloc=128, f_alloc=64; //offsets and alloc sizes
	vec3 *vert = malloc(v_alloc*sizeof(vec3)), *norm = malloc(f_alloc*sizeof(vec3));
	face *faces = malloc(f_alloc*sizeof(face));
	char ch, buf[256];
	while ((ch=getc(fp)) != EOF) {
		switch (ch) {
			case 'v':
				if (getc(fp) == ' ') { //make sure not vn or vt
					fscanf(fp, "%f%f%f\n", &a, &b, &c);
					vert[v_off] = (vec3){a,b,c};
					v_off++;
					if (v_off >= v_alloc) {
						v_alloc *= 2;
						vert = realloc(vert, v_alloc*sizeof(vec3));
					}
				}
				break;
			case 'f':
				fgets(buf, 256, fp); //get line, we have to process it manually
				w = 1, i = 0;
				while (buf[w]) {
					if (buf[w] >= '0' && buf[w] <= '9') {
						sscanf(&buf[w], "%u", &v[i]);
						v[i]--, i++;
						if (i > 2) {
							faces[f_off] = (face){v[0], v[1], v[2], WHITE};
							norm[f_off] = GetTriangleNormal(vert[v[2]],vert[v[1]],vert[v[0]]);
							v[1] = v[2], f_off++;
							if (f_off >= f_alloc) {
								f_alloc *= 2;
								norm = realloc(norm, f_alloc*sizeof(vec3));
								faces = realloc(faces, f_alloc*sizeof(face));
							}
							i=2;
						}
						do w++; while (buf[w] >= '0' && buf[w] <= '9');
					}
					if (buf[w] == '/') {
						w++;
						while (buf[w] != ' ' && buf[w]) w++;
						if (!buf[w]) break;
					}
					w++;
				}
				break;
			default:
				fscanf(fp, "%*[^\n]\n");
				break;
		}
	}
	v_off++, f_off++;
	vert = realloc(vert, v_off*sizeof(vec3));
	norm = realloc(norm, f_off*sizeof(vec3));
	faces = realloc(faces, f_off*sizeof(face));
	return (obj){{0},{0},{1,1,1},MallocMat4x4(),f_off,v_off,vert,norm,faces};
}

int main() {
	ElafriInit("Elafri 3D SDL example", (uvec2){1280, 960});

	gr Gr = GrBuffer(res.x,res.y);

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
	obj house = ReadObjFile("house.obj");
	house.pos.z -= 10;

	vec3 pos={0},rot={0}; //camera pos & rot
	vec2 vFront = {0,1};
	F32 speed = 4;
	AllocDepthBuffer(res);
	AllocProjectedPointBuffer(50); //set this as high as the biggest object vert count
	vec3 param = GrMake3DParams(90, (F32)res.y/res.x, 100);
	u32 t = 0;
	F32 delta=0; //Δt per frame

	RenderLoop:
	Input(); //https://wiki.libsdl.org/SDL_Keycode
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
	const vec3 light = {0,1,0}; //light source from above
	GrObject(&Gr, cube, pos, rot, light, param);
	GrObject(&Gr, house, pos, rot, light, param);
	if (t) //depth mode, colors screen by depth
		for (int y=0; y<res.y; y++)
			for (int x=0; x<res.x; x++) {
				const F32 u = 2000*depth[y*res.x+x] > 255 ? 255 : (int)roundf(2000*depth[y*res.x+x]);
				Gr.pal[y*res.x+x] = (color){u,u,u,255};
			}
	cube.rot.x = modF32(cube.rot.x+delta*0.31*pi,2*pi),
	cube.rot.y = modF32(cube.rot.y+delta*0.44*pi,2*pi),
	cube.rot.z = modF32(cube.rot.z+delta*0.75*pi,2*pi);

	drawc(&Gr);

	delta = FramerateHandler(FPS);
	goto RenderLoop;
}
