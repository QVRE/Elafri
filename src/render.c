#ifndef _ERENDER //3D graphics for the Elafri engine
#define _ERENDER
#include "graphics.c"
#include "grmath.c"

typedef struct TriangleFace3D {
	u32 a,b,c; color clr;
} face;
typedef struct Object3D {
	vec3 pos, rot, scale; mat4 mat;
	u32 facecount, vertcount; vec3 *vert, *norm; face *f;
} obj;

F32 *depth; //the depth buffer
vec3 *proj_pnt; //projected point buffer. Store point coords on-screen here

//This buffer is used for temporary on-screen coordinates for points per object
//You should pass the maximum num of vertices an object can have in your program
static inline void AllocProjectedPointBuffer(int max_vertice_count) {
	proj_pnt = malloc(max_vertice_count*sizeof(vec3));
}
static inline void AllocDepthBuffer(uvec2 resolution) {
	depth = malloc(resolution.x*(resolution.y+1)*sizeof(F32));
}
static inline void DepthReset(uvec2 resolution) { //1 is considered view distance
	for (int i=0; i<resolution.x*resolution.y; i++) depth[i] = 1;
}

void GrTriangle3D(gr *buf, vec3 Av, vec3 Bv, vec3 Cv, color clr) {
	F32 x_small, y_small, z_small, x_big, y_big; //smallest and biggest coordinates
	//discard triangle is outside the screen / view
	z_small = min(Av.z, Bv.z), z_small = min(z_small, Cv.z);
	if (z_small >= 1) return; //too far away
	x_small = min(Av.x, Bv.x), x_small = min(x_small, Cv.x);
	x_big = max(Av.x, Bv.x), x_big = max(x_big, Cv.x);
	if (x_big < -1 || x_small > 1) return; //not visible
	y_small = min(Av.y, Bv.y), y_small = min(y_small, Cv.y);
	y_big = max(Av.y, Bv.y), y_big = max(y_big, Cv.y);
	if (y_big < -1 || y_small > 1) return; //not visible
	int x_clipped = x_small < -1 || x_big >= 1; //a flag for optimized branches of code

	const F32 cx = buf->w*0.5, cy = buf->h*0.5; //center positions
	//for perspective correct depth interpolation, we linearly interpolate 1/z instead of z
	ivec2 A = {roundf(cx+Av.x*cx), roundf(cy+Av.y*cy)};
	ivec2 B = {roundf(cx+Bv.x*cx), roundf(cy+Bv.y*cy)};
	ivec2 C = {roundf(cx+Cv.x*cx), roundf(cy+Cv.y*cy)};
	F32 Az = 1. / Av.z, Bz = 1. / Bv.z, Cz = 1. / Cv.z;
	ivec2 swap; //swap conditionally to order from highest to lowest point
	F32 swapz;
	if (A.y > B.y)
		swap = A, A = B, B = swap, swapz = Az, Az = Bz, Bz = swapz;
	if (B.y > C.y)
		swap = B, B = C, C = swap, swapz = Bz, Bz = Cz, Cz = swapz;
	if (A.y > B.y)
		swap = A, A = B, B = swap, swapz = Az, Az = Bz, Bz = swapz;
	int i = A.y, dep_off = A.y*buf->w, roundl, roundr;
	F32 left, right, lstep, rstep, ldep, rdep, ldstep, rdstep; //sides, depth and steps per Δy
	F32 depstep, curdep; //depth step and current pixel's depth
	if (B.y > A.y) { //draw top section of triangle
		lstep = 1. / (B.y-A.y), rstep = 1. / (C.y-A.y); //put these here as temp
		ldstep = (Bz-Az)*lstep, rdstep = (Cz-Az)*rstep, lstep *= B.x-A.x, rstep *= C.x-A.x;
		if (lstep > rstep) //check if it's not left to right
			left = lstep, lstep = rstep, rstep = left, //swap position steps for sides
			left = ldstep, ldstep = rdstep, rdstep = left; //swap depth steps for sides
		left = A.x, right = A.x, ldep = Az, rdep = Az;
		if (B.y > 0) {
			if (i < -1) //if y clipping, skip clipping section
				i = -i-1, left+=lstep*i, right+=rstep*i, ldep+=ldstep*i, rdep+=rdstep*i,
				dep_off+=buf->w*i, i = -1;
			if (x_clipped) //optimization
				do { //Draw from Ay -> By for possible clipping
					left+=lstep, right+=rstep, ldep+=ldstep, rdep+=rdstep, dep_off+=buf->w, i++;
					roundl = roundf(left), roundr = roundf(right); //get left and right pixel
					depstep = roundr > roundl ? (rdep-ldep)/(roundr-roundl) : 0; //get step
					curdep = roundl >= 0 ? ldep : ldep-roundl*depstep; //current depth counter
					roundl = max(roundl, 0), roundr = min(roundr, (int)buf->w-1); //clip
					for (int j=roundl; j<=roundr; j++, curdep+=depstep) { //raster line
						const F32 d = 1. / curdep; //perspective correct depth
						if (d < depth[dep_off+j])
							buf->pal[dep_off+j] = clr, depth[dep_off+j] = d;
					}
				} while (i < min(B.y, buf->h));
			else
				do { //Draw from Ay -> By
					left+=lstep, right+=rstep, ldep+=ldstep, rdep+=rdstep, dep_off+=buf->w, i++;
					curdep = ldep, roundl = roundf(left), roundr = roundf(right);
					depstep = roundr > roundl ? (rdep-ldep)/(roundr-roundl) : 0;
					for (int j=roundl; j<=roundr; j++, curdep+=depstep) {
						const F32 d = 1. / curdep; //perspective correct depth
						if (d < depth[dep_off+j])
							buf->pal[dep_off+j] = clr, depth[dep_off+j] = d;
					}
				} while (i < min(B.y, buf->h));
		} else {
			i=B.y-i, left+=lstep*i, right+=rstep*i, ldep+=ldstep*i, rdep+=rdstep*i,
			dep_off+=buf->w*i, i=B.y; //we know that at least C.y >= 0
		}
	} else if (C.y > B.y) { //means Ay = By so we treat it as upside down triangle
		if (A.x < B.x) //left to right order
			lstep = 1./(C.y-A.y), rstep = 1./(C.y-B.y), left=A.x, right=B.x, ldep=Az, rdep=Bz,
			ldstep = (Cz-Az)*lstep, rdstep = (Cz-Bz)*rstep, lstep*=C.x-A.x, rstep*=C.x-B.x;
		else
			rstep = 1./(C.y-A.y), lstep = 1./(C.y-B.y), right=A.x, left=B.x, rdep=Az, ldep=Bz,
			rdstep = (Cz-Az)*rstep, ldstep = (Cz-Bz)*lstep, rstep*=C.x-A.x, lstep*=C.x-B.x;
		goto Draw2Cy;
	}
	if (C.y > B.y) { //draw bottom section of triangle
		lstep = 1. / (C.y-i), rstep = 1. / (C.y-i);
		ldstep = (Cz-ldep)*lstep, rdstep = (Cz-rdep)*rstep, lstep*=C.x-left, rstep*=C.x-right;
Draw2Cy:
		if (i < 0) //if y clipping, skip clipping section
			i = -i, left+=lstep*i, right+=rstep*i, ldep+=ldstep*i, rdep+=rdstep*i,
			dep_off+=buf->w*i, i=0;
		if (x_clipped) { //optimization
			do { //Draw from By -> Cy for possible clipping
				roundl = roundf(left), roundr = roundf(right);
				depstep = roundr > roundl ? (rdep-ldep)/(roundr-roundl) : 0;
				curdep = roundl < 0 ? ldep-roundl*depstep : ldep;
				roundl = max(roundl, 0), roundr = min(roundr, (int)buf->w-1);
				for (int j=roundl; j<=roundr; j++, curdep+=depstep) {
					const F32 d = 1. / curdep; //perspective correct depth
					if (d < depth[dep_off+j])
						buf->pal[dep_off+j] = clr, depth[dep_off+j] = d;
				}
				left+=lstep, right+=rstep, ldep+=ldstep, rdep+=rdstep, dep_off+=buf->w, i++, curdep=ldep;
			} while (i < min(C.y, buf->h));
		} else {
			curdep = ldep;
			do { //Draw from By -> Cy
				roundl = roundf(left), roundr = roundf(right);
				depstep = roundr > roundl ? (rdep-ldep)/(roundr-roundl) : 0;
				for (int j=roundl; j<=roundr; j++, curdep+=depstep) {
					const F32 d = 1. / curdep; //perspective correct depth
					if (d < depth[dep_off+j])
						buf->pal[dep_off+j] = clr, depth[dep_off+j] = d;
				}
				left+=lstep, right+=rstep, ldep+=ldstep, rdep+=rdstep, dep_off+=buf->w, i++, curdep=ldep;
			} while (i < min(C.y, buf->h));
		}
	}
}

static inline void ObjectMakeMatrix(obj object) {
	Mat4x4Set(object.mat, object.pos, object.rot, object.scale);
}

//FOV is in degrees 0-360, aspect ratio is h/w
vec3 GrMake3DParams(F32 fov, F32 aspect_ratio, F32 view_distance) {
	vec3 param;
	param.y = 1 / tanf(fov*pi/360); //trigonometry to turn degrees to plane tan(θ/2) -> radians
	param.x = aspect_ratio * param.y; //we also multiply x by aspect ratio
	param.z = view_distance;
	return param;
}

//params are gotten from the above function. light is a normal vector pointing TO the source
//pos and rot are camera variables. pos -> position, rot -> rotation (euler)
void GrObject(gr *buf, obj object, vec3 pos, vec3 rot, vec3 light, const vec3 param) {
	ObjectMakeMatrix(object);
	Mat4x4ApplyView(object.mat, pos, rot);
	F32 inv_dist = 1 / param.z; //inverse view distance (how far we can see)
	for (u32 i=0; i < object.vertcount; i++) { //transform all object points into perspective
		const vec3 tmpv = Mat4x3MultVec3(object.mat, object.vert[i]);
		F32 idiv = tmpv.z != 0 ? 1 / tmpv.z : 0;
		proj_pnt[i] = (vec3){tmpv.x*idiv*param.x, -tmpv.y*idiv*param.y, tmpv.z*inv_dist};
	}
	for (u32 i=0; i < object.facecount; i++) {
		const face f = object.f[i];
		const vec3 nor = Vec3Rot(object.norm[i], object.rot); //world normal
		color c = f.clr;
		F32 ls = 0.6+0.4*(nor.x*light.x + nor.y*light.y + nor.z*light.z); //lighting dot product
		c.r *= ls, c.g *= ls, c.b *= ls;
		const vec3 proj_a = proj_pnt[f.a], proj_b = proj_pnt[f.b], proj_c = proj_pnt[f.c];
		int a_inside = proj_a.z > 0, b_inside = proj_b.z > 0, c_inside = proj_c.z > 0;
		int cnt = a_inside + b_inside + c_inside; //point count
		if (cnt == 3)
			GrTriangle3D(buf, proj_a, proj_b, proj_c, c);
		else if (cnt) {
			cnt = 0; //refresh count
			vec3 points[4]; //we will at most see 4 points after a clip
			//Here, I sort of put the points back in view space, keeping aspect ratio
			//we will clip with the screen because points behind the camera are undefined
			const F32 prm = param.z, pa = proj_a.z*prm, pb = proj_b.z*prm, pc = proj_c.z*prm;
			const vec3 n_a = {proj_a.x*pa, proj_a.y*pa, pa},
				n_b = {proj_b.x*pb, proj_b.y*pb, pb},
				n_c = {proj_c.x*pc, proj_c.y*pc, pc};
			F32 dz; //delta Z
			if (a_inside) points[0] = proj_a, cnt++;
			if (b_inside) points[cnt] = proj_b, cnt++;
			if (c_inside) points[cnt] = proj_c, cnt++;
			if (b_inside ^ c_inside) //this is a line equation to get the point at z = 0
				dz = n_b.z / (n_c.z - n_b.z),
				points[cnt] = (vec3){(n_b.x-(n_c.x-n_b.x)*dz)*100, (n_b.y-(n_c.y-n_b.y)*dz)*100, 0.01*inv_dist},
				cnt++;
			if (a_inside ^ c_inside)
				dz = n_a.z / (n_c.z - n_a.z),
				points[cnt] = (vec3){(n_a.x-(n_c.x-n_a.x)*dz)*100, (n_a.y-(n_c.y-n_a.y)*dz)*100, 0.01*inv_dist},
				cnt++;
			if (a_inside ^ b_inside)
				dz = n_a.z / (n_b.z - n_a.z),
				points[cnt] = (vec3){(n_a.x-(n_b.x-n_a.x)*dz)*100, (n_a.y-(n_b.y-n_a.y)*dz)*100, 0.01*inv_dist},
				cnt++;

			GrTriangle3D(buf, points[0], points[1], points[2], c);
			if (cnt == 4) //if two points in view, two clippings with screen, thus quad
				GrTriangle3D(buf, points[0], points[2], points[3], c);
		}
	}
}
#endif
