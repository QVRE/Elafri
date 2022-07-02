#include "Elafri.c"
#include "3D.c"

// function to read a texture in the Quite OK Image format
// https://qoiformat.org
gr ReadQOI(char *filename) {
	FILE* fp = fopen(filename, "rb");
	fseek(fp, 4, SEEK_SET); //skip magic bytes (qoif)
	u32 w, h; //width & height
	fread(&w, 4, 1, fp);
	fread(&h, 4, 1, fp); //stored in Big Endian, will convert to Little Endian below
	w = (w >> 24) | (w << 24) | ((w >> 8) & 0xFF00) | ((w << 8) & 0xFF0000);
	h = (h >> 24) | (h << 24) | ((h >> 8) & 0xFF00) | ((h << 8) & 0xFF0000);
	fseek(fp, 0, SEEK_END);
	u64 data_size = ftell(fp) - 22; //get size of data minus header and 8 byte end marker size
	fseek(fp, 14, SEEK_SET); //offseted by header size
	gr img = GrBuffer(w,h);

	color pix = {0, 0, 0, 255};
	color table[64]; //a running array
	u8 *dat = malloc(data_size);
	fread(dat, 1, data_size, fp);
	fclose(fp);
	u32 offset = 0;
	for (u32 i = 0; i < w*h; i++) {
		u8 byte = dat[offset];
		switch (byte >> 6) {
			case 0: //index op, table lookup
				pix = table[byte & 63];
				break;
			case 1: //diff op, slight differences (-2..1) from previous pixel per channel
				pix.r += (int)((byte >> 4) & 3) - 2;
				pix.g += (int)((byte >> 2) & 3) - 2;
				pix.b += (int)(byte & 3) - 2;
				break;
			case 2: //luma op, bigger diffrences (-32..31 for green, -8..7 + dG for R & B)
				int dg = (int)(byte & 63) - 32; //get delta green
				pix.g += dg;
				offset++; //grab next byte
				byte = dat[offset];
				pix.r += (int)((byte >> 4) & 15) - 8 + dg; //do 4 bit delta + add delta green
				pix.b += (int)(byte & 15) - 8 + dg; //same as above
				break;
			default: //run, rgb or rgba op, we need subcases to confirm
				byte &= 63;
				switch (byte) {
					case 62:
						pix.r = dat[offset+1];
						pix.g = dat[offset+2];
						pix.b = dat[offset+3];
						offset += 3;
						break;
					case 63:
						pix = *(color*)&dat[offset+1];
						offset += 4;
						break;
					default:
						do {
							img.dat[i] = pix;
							i++;
						} while (byte--);
						i--;
						break;
				}
				break;
		}
		offset++;
		table[(pix.r * 3 + pix.g * 5 + pix.b * 7 + pix.a * 11) & 63] = pix;
		img.dat[i] = pix;
	}

	free(dat);
	return img;
}

int main() {
	ElafriInit("3D object example", 1280, 960);

	gr Gr = GrBuffer(res.x,res.y);
	F32 *depth = MallocDepthBuffer(res.x, res.y); //Depth buffer for 3D drawing

	gr texture = ReadQOI("testcard.qoi");

	//make cube parameters
	vec3 vert[8] = {
		{1, 1, 1}, {-1, 1, 1}, {1, 1, -1}, {-1, 1, -1},
		{1, -1, 1}, {-1, -1, 1}, {1, -1, -1}, {-1, -1, -1}
	};
	face tri[12] = { //3 vertices plus UV for each one
		{0,1,2,0,0,1,0,0,1}, {1,2,3,1,0,0,1,1,1}, {0,2,4,0,0,0,1,1,0}, {2,4,6,0,1,1,0,1,1},
		{2,3,6,0,1,1,1,0,0}, {3,6,7,1,1,0,0,1,0}, {1,3,5,1,0,1,1,0,0}, {3,5,7,1,1,0,0,0,1},
		{0,1,4,0,0,1,0,0,1}, {1,4,5,1,0,0,1,1,1}, {4,5,6,0,1,1,1,0,0}, {5,6,7,1,1,0,0,1,0}
	};
	vec3 norm[12] = {
		{0,1,0}, {0,1,0}, {1,0,0}, {1,0,0}, {0,0,-1}, {0,0,-1},
		{-1,0,0}, {-1,0,0}, {0,0,1}, {0,0,1}, {0,-1,0}, {0,-1,0}
	};

	//textured cube
	obj txt_cube = { //you should go to 3D.c to see the structure of obj
		{3,0,5}, {0}, {1,1,1}, //position, no rotation, scale is 1
		MallocMat4x4(), //we need to allocate a 3D matrix ourselves (but we dont use it)
		12, 8, //12 faces and 8 vertices
		vert, tri, norm, //vetice, face and normal arrays
		&texture //finally, add our texture here to signify that we dont do single color
	};

	color_face clr_tri[12] = {
		{0,1,2,RED}, {1,2,3,RED}, {0,2,4,GREEN}, {2,4,6,GREEN},
		{2,3,6,BLUE}, {3,6,7,BLUE}, {1,3,5,YELLOW}, {3,5,7,YELLOW},
		{0,1,4,CYAN}, {1,4,5,CYAN}, {4,5,6,WHITE}, {5,6,7,WHITE}
	};

	//untextured cube
	obj cube = {
		{-3,0,5}, {0}, {1,1,1}, //pos, rot, scale
		MallocMat4x4(), //matrix
		12, 8, //12 faces and 8 vertices
		vert, clr_tri, norm, //use color triangles here instead of UV ones
		NULL //indicate that this is an untextured object
	};

	MallocProjectedPointBuffer(8); //allocate vertices for projection per GrObject() call

	//we need a vector pointing to the object from the light source
	//down vector along with 80/20 base light and how much the light source direction matters
	//if you wanted the darkest shadows, you could make UV be 50/50 as in (0.5, 0.5)
	vertex lighting = {0, 1, 0, 0.8, 0.2};

	vec3 cam = MakeCamera(90, (F32)res.y/res.x, 100); //make camera parameters (FOV, aspect, view dist)
	vec3 pos = {0}, rot = {0}; //camera's position and rotation
	vec2 vFront = {0,1};
	F32 speed = 4, delta = 0;

	RenderLoop:
	Input(); //https://wiki.libsdl.org/SDL_Keycode
	if (kbd[SDL_SCANCODE_W]) pos.x += vFront.x*speed*delta, pos.z += vFront.y*speed*delta;
	if (kbd[SDL_SCANCODE_S]) pos.x -= vFront.x*speed*delta, pos.z -= vFront.y*speed*delta;
	if (kbd[SDL_SCANCODE_D]) pos.x += vFront.y*speed*delta, pos.z -= vFront.x*speed*delta;
	if (kbd[SDL_SCANCODE_A]) pos.x -= vFront.y*speed*delta, pos.z += vFront.x*speed*delta;
	if (kbd[SDL_SCANCODE_E]) pos.y += speed*delta;
	if (kbd[SDL_SCANCODE_Q]) pos.y -= speed*delta;
	if (kbd[SDL_SCANCODE_LEFTBRACKET] == KEY_PRESSED) speed *= 0.5;
	if (kbd[SDL_SCANCODE_RIGHTBRACKET] == KEY_PRESSED) speed *= 2;
	if (kbd[SDL_SCANCODE_LEFT]) rot.y = modF32(rot.y-delta*2, 2*M_PI), vFront = (vec2){sinf(rot.y), cosf(rot.y)};
	if (kbd[SDL_SCANCODE_RIGHT]) rot.y = fmodf(rot.y+delta*2, 2*M_PI), vFront = (vec2){sinf(rot.y), cosf(rot.y)};
	if (kbd[SDL_SCANCODE_UP]) rot.x = fmodf(rot.x-delta*2, 2*M_PI);
	if (kbd[SDL_SCANCODE_DOWN]) rot.x = modF32(rot.x+delta*2, 2*M_PI);

	GrObject(&Gr, depth, cube, pos, rot, lighting, cam);
	GrObject(&Gr, depth, txt_cube, pos, rot, lighting, cam);

	DrawFill(&Gr, BLACK);
	DepthReset(depth, res.x, res.y); //clear the depth buffer

	delta = FramerateHandler(60);
	goto RenderLoop;
}
