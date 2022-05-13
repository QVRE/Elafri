#include "Elafri.c"

#define FPS 30

//program to visualize icosahedron subdivision into an icosphere
//this is used to get points around a circle that are equally spaced appart
//you can use the code inside DrawIt() for your own purposes, just remove
//the graphics code and return the icosphere instead of freeing it

typedef struct F64Vector3 {F64 x,y,z;} dvec3;
typedef struct UVector3 {u32 x,y,z;} uvec3;

//taken from Elafri's grmath.c
dvec3 DVec3RotY(dvec3 v, F64 q) { //rotate 64 bit vector around Y axis
	F64 sinq = sin(q), cosq = cos(q);
	return (dvec3){v.x*cosq+v.z*sinq, v.y, v.z*cosq-v.x*sinq};
}

void Icosphere_lerp(dvec3 *mesh, u32 a, u32 b, u32 offset, u32 n, F64 step) {
	F64 c = step;
	dvec3 start = mesh[a], end = mesh[b];
	for (u32 i=0; i<n; i++, c+=step) {
		const F64 c1 = 1 - c;
		const dvec3 v = {end.x*c + start.x*c1, end.y*c + start.y*c1, end.z*c + start.z*c1};
		const F64 norm = 1. / sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
		mesh[offset+i] = (dvec3){v.x*norm, v.y*norm, v.z*norm};
	}
}

//this is a bit of a complex method I came up with but it ensures no duplicate points
void DrawIt(gr *buf, const u32 div, uvec2 res, F64 size, F64 rot) {
	res.x >>= 1, res.y >>= 1;

	const F64 phi = (1 + sqrt(5)) * 0.5; //golden ratio
	const F64 norm = 1. / sqrt(phi*phi + 1); //normalizer

	//basic normalized icosahedron
	static const dvec3 base[12] = {
		{-1,phi,0}, {1,phi,0}, {0,1,phi}, {-phi,0,1},
		{-phi,0,-1}, {0,1,-phi}, {phi,0,1}, {0,-1,phi},
		{-1,-phi,0}, {0,-1,-phi}, {phi,0,-1}, {1,-phi,0}
	};
	u32 alloc_size = 12 + 10 * ((1 << (2*div)) - 1); //equation for point num
	dvec3 *mesh = calloc(alloc_size, sizeof(dvec3));
	for (u32 i=0; i<12; i++) //copy over the icosahedron points
		mesh[i] = (dvec3){base[i].x*norm, base[i].y*norm, base[i].z*norm};
	if (div == 0) goto Render; //no subdivisions, go directly to end

	//subdivision procedure
	F64 step = 1. / (1 << div);
	const u32 len = (1<<div)-1, tri_size = (1 << (2*div-1)) + 3*(1 << (div-1)) + 1 - (div == 0);
	u32 cur = 12;
	for (u32 i=1; i<=5; i++, cur += 6*len) { //interpolate between all edges
		Icosphere_lerp(mesh, 0, i, 			cur, len, step); //5 edges of our picked point
		Icosphere_lerp(mesh, i, i%5 + 1, 	cur+len, len, step); //that neighbor to the next
		Icosphere_lerp(mesh, i, i+5, 		cur+2*len, len, step); //to outer point
		Icosphere_lerp(mesh, i%5 + 1, i+5, 	cur+3*len, len, step); //outer
		Icosphere_lerp(mesh, i+5, i%5 + 6, 	cur+4*len, len, step); //next neighbor of outer
		Icosphere_lerp(mesh, 11, i+5, 		cur+5*len, len, step); //opposite point to outer
	}
	for (u32 i=0; i<5; i++) { //interpolate inside points of all icosahedron faces
		u32 a = 12 + i*6*len, b = 12 + ((i+1)%5)*6*len;
		for (u32 j=1; j<len; j++) {
			Icosphere_lerp(mesh, a+j, b+j, cur, j, 1. / (j+1));
			cur += j;
		}
		a += 2*len, b = 12 + i*6*len + 3*len;
		for (u32 j=1; j<len; j++) {
			Icosphere_lerp(mesh, a+len-j-1, b+len-j-1, cur, j, 1. / (j+1));
			cur += j;
		}
		a += len, b = 12 + ((i+1)%5)*6*len + 2*len;
		for (u32 j=1; j<len; j++) {
			Icosphere_lerp(mesh, a+j, b+j, cur, j, 1. / (j+1));
			cur += j;
		}
		a += 2*len, b += 3*len;
		for (u32 j=1; j<len; j++) {
			Icosphere_lerp(mesh, a+j, b+j, cur, j, 1. / (j+1));
			cur += j;
		}
	}

	Render:
	for (u32 i=0; i<alloc_size; i++) {
		const dvec3 p = DVec3RotY(mesh[i], rot);
		const ivec2 o = {(p.x*size*res.x) / (p.z+2) * res.y/res.x + res.x, (p.y*size*res.y) / (p.z+2) + res.y};
		GrCircle(buf, o, res.y*size/100, 1, GREEN);
	}
	free(mesh); //since we only use this function to render, we free instead of returning
}

int main() {
	ElafriInit("Icosphere Visualization", (uvec2){1280, 960});

	gr Gr = GrBuffer(res.x,res.y);

	u32 d = 0; //subdiv amount
	F64 r = 0; //rotation

	RenderLoop:

	Input(); //https://wiki.libsdl.org/SDL_Keycode
	if (kbd[SDL_SCANCODE_0]) d=0;
	if (mC & SDL_BUTTON_LMASK) { //a crude slider
		if (m.x >= 10 && m.x <= 260 && m.y < 30) d = (m.x-10) / 25;
	}

	GrLine(&Gr, (ivec2){10,15}, (ivec2){260,15}, WHITE); //draw slider
	GrCircleFilled(&Gr, (ivec2){10+d*25,15}, 7, RED);
	DrawIt(&Gr, d, res, 1, r); //draw icosahedron

	drawc(&Gr);
	r = fmod(r + FramerateHandler(FPS)*0.25, 2*pi); //rotate object

	goto RenderLoop;
}
