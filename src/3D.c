#ifndef _ERENDER //3D graphics for Elafri
#define _ERENDER
#include "graphics.c"
#include "grmath.c"

typedef struct { //5D vector
	F32 x,y,z,u,v;
} vertex;
typedef struct TriangleFace3D { //for textures, stores UV coords
	u32 a,b,c; F32 au, av, bu, bv, cu, cv;
} face;
typedef struct ColoredTriangleFace3D { //single color textureless
	u32 a,b,c; color clr;
} color_face;

typedef struct Object3D {
	vec3 pos, rot, scale; mat4 mat; //if you use GrObject(), you dont need to set the matrix
	u32 facecount, vertcount;
	vec3 *vert; void *tri; vec3 *norm; //tri's type is either face of color_face
	gr *texture; //set texture to NULL if you're using color faces
} obj;

vec3 *proj_pnt; //projected point buffer. Store point coords on-screen here

//This buffer is used for temporary on-screen coordinates for points per object
//You should pass the maximum number of vertices an object can have in your program
static inline void MallocProjectedPointBuffer(int max_vertice_count) {
	proj_pnt = malloc(max_vertice_count*sizeof(vec3));
}

F32* MallocDepthBuffer(u32 w, u32 h) {
	return malloc(w*(h+1)*sizeof(F32));
}
static inline void DepthReset(F32 *depth, u32 w, u32 h) { //1 is considered view distance
	for (int i=0; i<w*h; i++) depth[i] = 1;
}

static inline void ObjectMakeMatrix(obj object) {
	Mat4x4Set(object.mat, object.pos, object.rot, object.scale);
}

//returns a vector with the attributes of the camera
//FOV is in degrees 0-360, aspect ratio is h/w
vec3 MakeCamera(F32 fov, F32 aspect_ratio, F32 view_distance) {
	vec3 param;
	param.y = 1 / tanf(fov*M_PI/360); //trigonometry to turn degrees to plane tan(θ/2) -> radians
	param.x = aspect_ratio * param.y; //we also multiply x by aspect ratio
	param.z = view_distance;
	return param;
}

// Draw a 3D single color triangle on buffer and depth buffer array

void GrTriangle3D(gr *buf, F32 *depth, vec3 At, vec3 Bt, vec3 Ct, color clr) {
	F32 x_small, y_small, z_small, x_big, y_big; //smallest and biggest coordinates on screen
	//discard triangle if outside the screen / view
	z_small = min(At.z, Bt.z), z_small = min(z_small, Ct.z);
	if (z_small >= 1) return; //too far away
	x_small = min(At.x, Bt.x), x_small = min(x_small, Ct.x);
	x_big = max(At.x, Bt.x), x_big = max(x_big, Ct.x);
	if (x_big < -1 || x_small > 1) return; //not visible
	y_small = min(At.y, Bt.y), y_small = min(y_small, Ct.y);
	y_big = max(At.y, Bt.y), y_big = max(y_big, Ct.y);
	if (y_big < -1 || y_small > 1) return; //not visible
	const int x_clipped = x_small < -1 || (u32)roundf(x_big*buf->w) >= buf->w; //optimization flag

	const F32 cx = buf->w*0.5, cy = buf->h*0.5; //center positions
	//for perspective correct depth interpolation, we linearly interpolate 1/z instead of z
	ivec2 A = {roundf(cx+At.x*cx), roundf(cy+At.y*cy)};
	ivec2 B = {roundf(cx+Bt.x*cx), roundf(cy+Bt.y*cy)};
	ivec2 C = {roundf(cx+Ct.x*cx), roundf(cy+Ct.y*cy)};
	F32 Az = 1. / At.z, Bz = 1. / Bt.z, Cz = 1. / Ct.z; //get the 1/z

	ivec2 swap; //sort triangle points from highest to lowest Y (assuming Y=0 is highest)
	F32 swapf; //for swapping floats
	vec2 swapv; //for swapping 2D vectors
	if (A.y > B.y)
		swap = A, A = B, B = swap, swapf = Az, Az = Bz, Bz = swapf;
	if (B.y > C.y)
		swap = B, B = C, C = swap, swapf = Bz, Bz = Cz, Cz = swapf;
	if (A.y > B.y)
		swap = A, A = B, B = swap, swapf = Az, Az = Bz, Bz = swapf;

	vec2 left, right, left_step, right_step; //params of left and right sides (Y is depth)
	//assume temporarily that AB is the left side and sort later if necessary
	right.x = A.x, right.y = Az;
	//we will rasterize the triangle line-by-line so we calculate deltas per new line here
	const F32 rdiv = 1. / (C.y - A.y); //precalculate division
	right_step.x = (C.x - A.x) * rdiv, right_step.y = (Cz - Az) * rdiv; //make steps for AC

	if (B.y != A.y && B.y > 0) { //draw top section of triangle (between A and B without B)
		left = right;
		const F32 ldiv = 1. / (B.y - A.y);
		left_step.x = (B.x - A.x) * ldiv, left_step.y = (Bz - Az) * ldiv;
		int swap_flag = 0; //keep track which is AB
		if (left_step.x > right_step.x) //swap if sides are not ordered properly
			swapv = left_step, left_step = right_step, right_step = swapv, swap_flag = 1;
		if (A.y < 0)
			left.x -= left_step.x*A.y, left.y -= left_step.y*A.y,
			right.x -= right_step.x*A.y, right.y -= right_step.y*A.y, A.y = 0;

		if (B.y > buf->h) B.y = buf->h; //check if B is outside the screen
		for (int i=A.y, offset=i*buf->w; i < B.y; i++, offset+=buf->w) {
			int L = roundf(left.x), R = roundf(right.x); //get x coords of sides in pixels
			const F32 div = 1. / (R-L); //Due to the IEEE standard, it shouldn't crash if L = R
			F32 dep = left.y; //for interpolating accross the scanline
			F32 step = (right.y - left.y) * div;
			if (x_clipped) { //branch prediction should make this pretty fast
				if (L < 0) dep -= step*L, L = 0;
				if (R >= (int)buf->w) R = buf->w-1;
			}
			for (int j=L; j<=R; j++, dep+=step) {
				const int offs = offset + j;
				const F32 z = 1. / dep; //retrieve actual z at this point
				if (depth[offs] > z)
					depth[offs] = z, buf->dat[offs] = clr;
			}
			left.x += left_step.x, left.y += left_step.y, right.x += right_step.x, right.y += right_step.y;
		}

		const F32 bc_div = 1. / (C.y - B.y);
		if (swap_flag) //if we swapped sides, we know right is what we modify from AB to BC
			right_step.x = (C.x - B.x) * bc_div, right_step.y = (Cz - Bz) * bc_div;
		else left_step.x = (C.x - B.x) * bc_div, left_step.y = (Cz - Bz) * bc_div;
	} else { //skip AB section
		if (B.y != A.y) { //in this case, we need to interpolate AC until B's height
			const F32 y_diff = B.y - A.y; //add steps to bring AC to the height of B
			right.x += right_step.x * y_diff, right.y += right_step.y * y_diff;
		}
		left.x = B.x, left.y = Bz; //we will consider left to be B
		const F32 ldiv = 1. / (C.y - B.y); //precalculate division and multiply below
		left_step.x = (C.x - B.x) * ldiv, left_step.y = (Cz - Bz) * ldiv;

		if (left.x > right.x) //swap if not ordered left to right
			swapv = left, left = right, right = swapv,
			swapv = left_step, left_step = right_step, right_step = swapv;
		if (B.y < 0)
			left.x -= left_step.x*B.y, left.y -= left_step.y*B.y,
			right.x -= right_step.x*B.y, right.y -= right_step.y*B.y, B.y = 0;
	}
	if (C.y > buf->h) C.y = buf->h; //if this is true, we know that B.y will be equal to C.y
	for (int i=B.y, offset=i*buf->w; i <= C.y; i++, offset+=buf->w) { //draw bottom section
		int L = roundf(left.x), R = roundf(right.x); //get x coords of sides in pixels
		const F32 div = 1. / (R-L); //Due to the IEEE standard, it shouldn't crash if L = R
		F32 dep = left.y; //for interpolating accross the scanline
		F32 step = (right.y - left.y) * div;
		if (x_clipped) { //branch prediction should make this pretty fast
			if (L < 0) dep -= step*L, L = 0;
			if (R >= (int)buf->w) R = buf->w-1;
		}
		for (int j=L; j<=R; j++, dep+=step) {
			const int offs = offset + j;
			const F32 z = 1. / dep; //retrieve actual z at this point
			if (depth[offs] > z)
				depth[offs] = z, buf->dat[offs] = clr;
		}
		left.x += left_step.x, left.y += left_step.y, right.x += right_step.x, right.y += right_step.y;
	}
}

// For 3D textured Triangles

void GrTexturedPixel(color *dest, gr *texture, F32 u, F32 v, F32 light) { //writes to dest
	u = (u - (int)u) + 1, v = (v - (int)v) + 1;
	u -= (int)u, v -= (int)v;
	int tx = texture->w * u, ty = texture->h * v;
	color t = texture->dat[ty*texture->w+tx];
	dest->r = t.r*light, dest->g = t.g*light, dest->b = t.b*light, dest->a = t.a;
}

void GrTexturedTriangle3D(gr *buf, F32 *depth, vertex At, vertex Bt, vertex Ct, gr *texture, F32 light) {
	F32 x_small, y_small, z_small, x_big, y_big; //smallest and biggest coordinates on screen
	//discard triangle if outside the screen / view
	z_small = min(At.z, Bt.z), z_small = min(z_small, Ct.z);
	if (z_small >= 1) return; //too far away
	x_small = min(At.x, Bt.x), x_small = min(x_small, Ct.x);
	x_big = max(At.x, Bt.x), x_big = max(x_big, Ct.x);
	if (x_big < -1 || x_small > 1) return; //not visible
	y_small = min(At.y, Bt.y), y_small = min(y_small, Ct.y);
	y_big = max(At.y, Bt.y), y_big = max(y_big, Ct.y);
	if (y_big < -1 || y_small > 1) return; //not visible
	const int x_clipped = x_small < -1 || (u32)roundf(x_big*buf->w) >= buf->w; //optimization flag

	const F32 cx = buf->w*0.5, cy = buf->h*0.5; //center positions
	//for perspective correct depth interpolation, we linearly interpolate 1/z instead of z
	ivec2 A = {roundf(cx+At.x*cx), roundf(cy+At.y*cy)};
	ivec2 B = {roundf(cx+Bt.x*cx), roundf(cy+Bt.y*cy)};
	ivec2 C = {roundf(cx+Ct.x*cx), roundf(cy+Ct.y*cy)};
	F32 Az = 1. / At.z, Bz = 1. / Bt.z, Cz = 1. / Ct.z; //get the 1/z
	F32 Au = At.u * Az, Bu = Bt.u * Bz, Cu = Ct.u * Cz; //the u/z for UV interpolation
	F32 Av = At.v * Az, Bv = Bt.v * Bz, Cv = Ct.v * Cz; // v/z

	ivec2 swap; //sort triangle points from highest to lowest Y (assuming Y=0 is highest)
	F32 swapf; //for swapping floats
	vertex swapv; //for swapping vertices later in the code
	if (A.y > B.y)
		swap = A, A = B, B = swap, swapf = Az, Az = Bz, Bz = swapf,
		swapf = Au, Au = Bu, Bu = swapf, swapf = Av, Av = Bv, Bv = swapf;
	if (B.y > C.y)
		swap = B, B = C, C = swap, swapf = Bz, Bz = Cz, Cz = swapf,
		swapf = Bu, Bu = Cu, Cu = swapf, swapf = Bv, Bv = Cv, Cv = swapf;
	if (A.y > B.y)
		swap = A, A = B, B = swap, swapf = Az, Az = Bz, Bz = swapf,
		swapf = Au, Au = Bu, Bu = swapf, swapf = Av, Av = Bv, Bv = swapf;

	vertex left, right, left_step, right_step; //params of left and right sides (Y is unused)
	//assume temporarily that AB is the left side and sort later if necessary
	right.x = A.x, right.z = Az, right.u = Au, right.v = Av;
	//we will rasterize the triangle line-by-line so we calculate deltas per new line here
	const F32 rdiv = 1. / (C.y - A.y); //precalculate division
	right_step.x = (C.x - A.x) * rdiv, right_step.z = (Cz - Az) * rdiv, //make steps for AC
	right_step.u = (Cu - Au) * rdiv, right_step.v = (Cv - Av) * rdiv;

	if (B.y != A.y && B.y > 0) { //draw top section of triangle (between A and B without B)
		left = right;
		const F32 ldiv = 1. / (B.y - A.y);
		left_step.x = (B.x - A.x) * ldiv, left_step.z = (Bz - Az) * ldiv,
		left_step.u = (Bu - Au) * ldiv, left_step.v = (Bv - Av) * ldiv;
		int swap_flag = 0; //keep track which is AB
		if (left_step.x > right_step.x) //swap if sides are not ordered properly
			swapv = left_step, left_step = right_step, right_step = swapv, swap_flag = 1;
		if (A.y < 0)
			left.x -= left_step.x*A.y, left.z -= left_step.z*A.y, left.u -= left_step.u*A.y,
			left.v -= left_step.v*A.y, right.x -= right_step.x*A.y, right.z -= right_step.z*A.y,
			right.u -= right_step.u*A.y, right.v -= right_step.v*A.y, A.y = 0;

		if (B.y > buf->h) B.y = buf->h; //check if B is outside the screen
		for (int i=A.y, offset=i*buf->w; i < B.y; i++, offset+=buf->w) {
			int L = roundf(left.x), R = roundf(right.x); //get x coords of sides in pixels
			const F32 div = 1. / (R-L); //Due to the IEEE standard, it shouldn't crash if L = R
			vec3 param = {left.u, left.v, left.z}; //x & y represent UV
			vec3 step = {(right.u-left.u)*div, (right.v-left.v)*div, (right.z-left.z)*div};
			if (x_clipped) { //branch prediction should make this pretty fast
				if (L < 0) param.x -= step.x*L, param.y -= step.y*L, param.z -= step.z*L, L = 0;
				if (R >= (int)buf->w) R = buf->w-1;
			}
			for (int j=L; j<=R; j++, param.x+=step.x, param.y+=step.y, param.z+=step.z) {
				const int offs = offset + j;
				const F32 z = 1. / param.z;
				if (depth[offs] > z) {
					depth[offs] = z;
					GrTexturedPixel(buf->dat + offs, texture, param.x*z, param.y*z, light);
				}
			}
			left.x += left_step.x, left.z += left_step.z, left.u += left_step.u, left.v += left_step.v,
			right.x += right_step.x, right.z += right_step.z, right.u += right_step.u, right.v += right_step.v;
		}

		const F32 bc_div = 1. / (C.y - B.y);
		if (swap_flag) //if we swapped sides, we know right is what we modify from AB to BC
			right_step.x = (C.x - B.x) * bc_div, right_step.z = (Cz - Bz) * bc_div,
			right_step.u = (Cu - Bu) * bc_div, right_step.v = (Cv - Bv) * bc_div;
		else
			left_step.x = (C.x - B.x) * bc_div, left_step.z = (Cz - Bz) * bc_div,
			left_step.u = (Cu - Bu) * bc_div, left_step.v = (Cv - Bv) * bc_div;
	} else { //skip AB section
		if (B.y != A.y) { //in this case, we need to interpolate AC until B's height
			const F32 y_diff = B.y - A.y; //add steps to bring AC to the height of B
			right.x += right_step.x * y_diff, right.z += right_step.z * y_diff,
			right.u += right_step.u * y_diff, right.v += right_step.v * y_diff;
		}
		left.x = B.x, left.z = Bz, left.u = Bu, left.v = Bv; //we will consider left to be B
		const F32 ldiv = 1. / (C.y - B.y); //precalculate division and multiply below
		left_step.x = (C.x - B.x) * ldiv, left_step.z = (Cz - Bz) * ldiv,
		left_step.u = (Cu - Bu) * ldiv, left_step.v = (Cv - Bv) * ldiv;

		if (left.x > right.x) //swap if not ordered left to right
			swapv = left, left = right, right = swapv,
			swapv = left_step, left_step = right_step, right_step = swapv;
		if (B.y < 0)
			left.x -= left_step.x*B.y, left.z -= left_step.z*B.y, left.u -= left_step.u*B.y,
			left.v -= left_step.v*B.y, right.x -= right_step.x*B.y, right.z -= right_step.z*B.y,
			right.u -= right_step.u*B.y, right.v -= right_step.v*B.y, B.y = 0;
	}
	if (C.y > buf->h) C.y = buf->h; //if this is true, we know that B.y will be equal to C.y
	for (int i=B.y, offset=i*buf->w; i <= C.y; i++, offset+=buf->w) { //draw bottom section
		int L = roundf(left.x), R = roundf(right.x); //get x coords of sides in pixels
		const F32 div = 1. / (R-L); //Due to the IEEE standard, it shouldn't crash if L = R
		vec3 param = {left.u, left.v, left.z}; //x & y represent UV
		vec3 step = {(right.u-left.u)*div, (right.v-left.v)*div, (right.z-left.z)*div};
		if (x_clipped) { //branch prediction should make this pretty fast
			if (L < 0) param.x -= step.x*L, param.y -= step.y*L, param.z -= step.z*L, L = 0;
			if (R >= (int)buf->w) R = buf->w-1;
		}
		for (int j=L; j<=R; j++, param.x+=step.x, param.y+=step.y, param.z+=step.z) {
			const int offs = offset + j;
			const F32 z = 1. / param.z;
			if (depth[offs] > z) {
				depth[offs] = z;
				GrTexturedPixel(buf->dat + offs, texture, param.x*z, param.y*z, light);
			}
		}
		left.x += left_step.x, left.z += left_step.z, left.u += left_step.u, left.v += left_step.v,
		right.x += right_step.x, right.z += right_step.z, right.u += right_step.u, right.v += right_step.v;
	}
}

//pos and rot are camera variables ( pos -> position, rot -> rotation (euler) )
//params are gotten from MakeCamera() and light is a vertex with xyz and uv params
//light's xyz points TO the light source and uv is base light and how strong the source is (u >= v)
void GrObject(gr *buf, F32 *depth, obj object, vec3 pos, vec3 rot, vertex light, vec3 param) {
	ObjectMakeMatrix(object);
	Mat4x4ApplyView(object.mat, pos, rot);
	const F32 inv_dist = 1 / param.z; //inverse view distance (how far we can see)

	for (u32 i=0; i < object.vertcount; i++) { //transform all vertices to screen space
		const vec3 screen = Mat4x3MultVec3(object.mat, object.vert[i]);
		const F32 div = screen.z != 0 ? 1 / screen.z : 0;
		proj_pnt[i] = (vec3){screen.x*div*param.x, -screen.y*div*param.y, screen.z*inv_dist};
	}
	if (object.texture == NULL) { //if not using a texture
		for (u32 i=0; i < object.facecount; i++) { //render triangles
			const color_face tri = ((color_face*)object.tri)[i];
			const vec3 norm = Vec3Rot(object.norm[i], object.rot); //world space normal
			const vec3 proj_a = proj_pnt[tri.a], proj_b = proj_pnt[tri.b], proj_c = proj_pnt[tri.c];
			F32 lit = light.u + light.v*(norm.x*light.x + norm.y*light.y + norm.z*light.z);
			color clr = tri.clr;
			clr.r *= lit, clr.g *= lit, clr.b *= lit; //multiply the color by how lit up it is
			int a_inside = proj_a.z > 0, b_inside = proj_b.z > 0, c_inside = proj_c.z > 0;
			int cnt = a_inside + b_inside + c_inside; //point count inside the camera's view

			if (cnt == 3) { //if all points are in view
				GrTriangle3D(buf, depth, proj_a, proj_b, proj_c, clr);
			} else if (cnt) { //if at least one is in view, we have to clip and draw
				cnt = 0; //refresh count
				vec3 points[4]; //we will at most see 4 points after the clip
				//put the points back in view space to interpolate properly and find clipping
				//linear transformations such as aspect ratio and Y flip are preserved
				//we will clip with the near plane because points behind the camera are undefined
				const F32 prm = param.z, pa = proj_a.z*prm, pb = proj_b.z*prm, pc = proj_c.z*prm;
				const vec3 n_a = {proj_a.x*pa, proj_a.y*pa, pa},
					n_b = {proj_b.x*pb, proj_b.y*pb, pb},
					n_c = {proj_c.x*pc, proj_c.y*pc, pc};
				F32 t, t0; //for interpolating all the vertex attributes (x0 = 1 - x)
				if (a_inside) points[0] = proj_a, cnt++;
				if (b_inside) points[cnt] = proj_b, cnt++;
				if (c_inside) points[cnt] = proj_c, cnt++;
				if (b_inside ^ c_inside) //this is a simplified line equation where f(b)=0, f(c)=1
					t = (1. - 100*n_b.z) / (n_c.z - n_b.z), t0 = 100. - t,
					points[cnt] = (vec3){t*n_c.x + t0*n_b.x, t*n_c.y + t0*n_b.y, 0.01*inv_dist},
					cnt++;
				if (a_inside ^ c_inside) //we multiply things by 100 because we get z=0.01 attributes
					t = (1. - 100*n_a.z) / (n_c.z - n_a.z), t0 = 100. - t,
					points[cnt] = (vec3){t*n_c.x + t0*n_a.x, t*n_c.y + t0*n_a.y, 0.01*inv_dist},
					cnt++;
				if (a_inside ^ b_inside)
					t = (1. - 100*n_a.z) / (n_b.z - n_a.z), t0 = 100. - t,
					points[cnt] = (vec3){t*n_b.x + t0*n_a.x, t*n_b.y + t0*n_a.y, 0.01*inv_dist},
					cnt++;

				GrTriangle3D(buf, depth, points[0], points[1], points[2], clr);
				if (cnt == 4) //if two points in view, two clippings with screen, thus quad
					GrTriangle3D(buf, depth, points[0], points[2], points[3], clr);
			}
		}
	} else { //for textured triangles, same thing as above but with textures
		gr *texture = object.texture; //grab object's texture pointer
		for (u32 i=0; i < object.facecount; i++) {
			const face tri = ((face*)object.tri)[i];
			const vec3 norm = Vec3Rot(object.norm[i], object.rot); //world space normal
			const vec3 pnt_a = proj_pnt[tri.a], pnt_b = proj_pnt[tri.b], pnt_c = proj_pnt[tri.c];
			vertex proj_a, proj_b, proj_c; //projected points + UV
			*(vec3*)(&proj_a) = pnt_a, *(vec3*)(&proj_b) = pnt_b, *(vec3*)(&proj_c) = pnt_c;
			proj_a.u = tri.au, proj_a.v = tri.av, //copy over UV coords from face
			proj_b.u = tri.bu, proj_b.v = tri.bv, proj_c.u = tri.cu, proj_c.v = tri.cv;
			F32 lit = light.u + light.v*(norm.x*light.x + norm.y*light.y + norm.z*light.z);
			int a_inside = proj_a.z > 0, b_inside = proj_b.z > 0, c_inside = proj_c.z > 0;
			int cnt = a_inside + b_inside + c_inside; //point count inside the camera's view

			if (cnt == 3) { //if all points are in view
				GrTexturedTriangle3D(buf, depth, proj_a, proj_b, proj_c, texture, lit);
			} else if (cnt) { //if at least one is in view, we have clip and draw
				cnt = 0;
				vertex points[4];
				const F32 prm = param.z, pa = proj_a.z*prm, pb = proj_b.z*prm, pc = proj_c.z*prm;
				const vec3 n_a = {proj_a.x*pa, proj_a.y*pa, pa},
					n_b = {proj_b.x*pb, proj_b.y*pb, pb},
					n_c = {proj_c.x*pc, proj_c.y*pc, pc};
				F32 t, t0; //for interpolating all the vertex attributes (x0 = 1 - x)
				if (a_inside) points[0] = proj_a, cnt++;
				if (b_inside) points[cnt] = proj_b, cnt++;
				if (c_inside) points[cnt] = proj_c, cnt++;
				if (b_inside ^ c_inside) //this is a simplified line equation where f(b)=0, f(c)=1
					t = (0.01 - n_b.z) / (n_c.z - n_b.z), t0 = 1. - t,
					points[cnt] = (vertex){100*(t*n_c.x + t0*n_b.x), 100*(t*n_c.y + t0*n_b.y),
						0.01*inv_dist, t*proj_c.u + t0*proj_b.u, t*proj_c.v + t0*proj_b.v}, cnt++;
				if (a_inside ^ c_inside) //we multiply things by 100 because we get the z=0.01 attributes
					t = (0.01 - n_a.z) / (n_c.z - n_a.z), t0 = 1. - t,
					points[cnt] = (vertex){100*(t*n_c.x + t0*n_a.x), 100*(t*n_c.y + t0*n_a.y),
						0.01*inv_dist, t*proj_c.u + t0*proj_a.u, t*proj_c.v + t0*proj_a.v}, cnt++;
				if (a_inside ^ b_inside)
					t = (0.01 - n_a.z) / (n_b.z - n_a.z), t0 = 1. - t,
					points[cnt] = (vertex){100*(t*n_b.x + t0*n_a.x), 100*(t*n_b.y + t0*n_a.y),
						0.01*inv_dist, t*proj_b.u + t0*proj_a.u, t*proj_b.v + t0*proj_a.v}, cnt++;

				GrTexturedTriangle3D(buf, depth, points[0], points[1], points[2], texture, lit);
				if (cnt == 4) //if two points in view, two clippings with screen, thus quad
					GrTexturedTriangle3D(buf, depth, points[0], points[2], points[3], texture, lit);
			}
		}
	}
}
#endif
