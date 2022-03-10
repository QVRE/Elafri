#ifndef _EGRMATH //Elafri's small collection of functions I wrote for 3D math
#define _EGRMATH //actual sin/cos function calls to keep it simpler. worth the performance loss
#include <stdlib.h>
#include <math.h>

typedef struct FVector4 {F32 x,y,z,w;} vec4;
typedef F32 *mat4;

vec3 Vec3RotX(vec3 v, F32 q) { //rotate Vector3 around X axis
	F32 sinq = sinf(q), cosq = cosf(q);
	return (vec3){v.x, v.y*cosq-v.z*sinq, v.z*cosq+v.y*sinq};
}
vec3 Vec3RotY(vec3 v, F32 q) { //rotate Vector3 around Y axis
	F32 sinq = sinf(q), cosq = cosf(q);
	return (vec3){v.x*cosq+v.z*sinq, v.y, v.z*cosq-v.x*sinq};
}
vec3 Vec3RotZ(vec3 v, F32 q) { //rotate Vector3 around Z axis
	F32 sinq = sinf(q), cosq = cosf(q);
	return (vec3){v.x*cosq-v.y*sinq, v.y*cosq+v.x*sinq, v.z};
}
vec3 Vec3Rot(vec3 v, vec3 r) { //rotate Vector3 around Z axis
	F32 sinx=sinf(r.x),cosx=cosf(r.x),siny=sinf(r.y),cosy=cosf(r.y),sinz=sinf(r.z),cosz=cosf(r.z);
	vec3 vx = {v.x, v.y*cosx-v.z*sinx, v.z*cosx+v.y*sinx};
	vec3 vy = {vx.x*cosy+vx.z*siny, vx.y, vx.z*cosy-vx.x*siny};
	return (vec3){vy.x*cosz-vy.y*sinz, vy.y*cosz+vy.x*sinz, vy.z};
}
vec3 GetTriangleNormal(vec3 A, vec3 B, vec3 C) {
	vec3 e1 = {A.x-B.x, A.y-B.y, A.z-B.z}; //edge AB
	vec3 e2 = {C.x-B.x, C.y-B.y, C.z-B.z}; //edge BC
	vec3 cr = {e1.y*e2.z-e1.z*e2.y, e1.z*e2.x-e1.x*e2.z, e1.x*e2.y-e1.y*e2.x}; //cross product
	F32 n = 1 / sqrtf(cr.x*cr.x + cr.y*cr.y + cr.z*cr.z); //get inverse length (for dividing)
	return (vec3){cr.x*n, cr.y*n, cr.z*n}; //normalize it and return
}

static inline mat4 MallocMat4x4() {
	return malloc(16*sizeof(F32));
}
mat4 Mat4x4() { //gives back an allocated matrix with all the values pre-set to be normal
	mat4 m = calloc(16,sizeof(F32));
	m[0] = 1, m[5] = 1, m[10] = 1, m[15] = 1;
	return m;
}

void Mat4x4Reset(mat4 m) { //sets them manually. hard on the eye but works well enough
	m[0] = 1, m[1] = 0, m[2] = 0, m[3] = 0, m[4] = 0, m[5] = 1, m[6] = 0, m[7] = 0,
	m[8] = 0, m[9] = 0, m[10] = 1, m[11] = 0, m[12] = 0, m[13] = 0, m[14] = 0, m[15] = 1;
}
void Mat4x3Reset(mat4 m) {
	m[0] = 1, m[1] = 0, m[2] = 0, m[3] = 0, m[4] = 0, m[5] = 1, m[6] = 0, m[7] = 0,
	m[8] = 0, m[9] = 0, m[10] = 1, m[11] = 0;
}
void Mat4x4Set(mat4 m, vec3 pos, vec3 rot, vec3 scale) {
	m[0] = scale.x, m[1] = 0, m[2] = 0, m[4] = 0, m[5] = scale.y, m[6] = 0,
	m[8] = 0, m[9] = 0, m[10] = scale.z, m[12] = 0, m[13] = 0, m[14] = 0, m[15] = 1;
	F32 sinx=sinf(rot.x), cosx=cosf(rot.x), siny=sinf(rot.y), cosy=cosf(rot.y),
		sinz=sinf(rot.z), cosz=cosf(rot.z), tmp;
	for (u32 i=0; i<3; i++) {
		tmp = m[i+4], m[i+4] = m[i+4]*cosx-m[i+8]*sinx, m[i+8] = m[i+8]*cosx+tmp*sinx;
		tmp = m[i], m[i] = m[i]*cosy+m[i+8]*siny, m[i+8] = m[i+8]*cosy-tmp*siny;
		tmp = m[i], m[i] = m[i]*cosz-m[i+4]*sinz, m[i+4] = m[i+4]*cosz+tmp*sinz;
	}
	m[3] = pos.x, m[7] = pos.y, m[11] = pos.z;
}

//multiplies matrices and the result goes into dest
//4x3 is an optimization I made. It's not actual 4x3
//you can use 4x3 and assume the 4th layer to be [0 0 0 1]
void Mat4x3MultMat4x3(mat4 a, mat4 b, mat4 dest) {
	u32 i,j;
	for (u32 k=0; k<12; k++)
		i=k&12, j=k&3, dest[k] = a[i]*b[j] + a[i+1]*b[j+4] + a[i+2]*b[j+8] + a[i+3]*(j==3);
}
void Mat4x4MultMat4x4(mat4 a, mat4 b, mat4 dest) {
	u32 i,j;
	for (u32 k=0; k<16; k++)
		i=k&12, j=k&3, dest[k] = a[i]*b[j] + a[i+1]*b[j+4] + a[i+2]*b[j+8] + a[i+3]*b[j+12];
}

vec3 Mat4x3MultVec3(mat4 m, vec3 v) { //multiply 4x3 matrix by a 3D vector
	return (vec3){m[0]*v.x+m[1]*v.y+m[2]*v.z+m[3], m[4]*v.x+m[5]*v.y+m[6]*v.z+m[7],
		m[8]*v.x+m[9]*v.y+m[10]*v.z+m[11]};
}
vec4 Mat4x4MultVec3(mat4 m, vec3 v) { //multiply 4x4 matrix by a 3D vector (assuming w=1)
	return (vec4){m[0]*v.x+m[1]*v.y+m[2]*v.z+m[3], m[4]*v.x+m[5]*v.y+m[6]*v.z+m[7],
		m[8]*v.x+m[9]*v.y+m[10]*v.z+m[11], m[12]*v.x+m[13]*v.y+m[14]*v.z+m[15]};
}
vec4 Mat4x4MultVec4(mat4 m, vec4 v) { //multiply 4x4 matrix by a 4D vector. Normal but tedious
	return (vec4){m[0]*v.x+m[1]*v.y+m[2]*v.z+m[3]*v.w, m[4]*v.x+m[5]*v.y+m[6]*v.z+m[7]*v.w,
		m[8]*v.x+m[9]*v.y+m[10]*v.z+m[11]*v.w, m[12]*v.x+m[13]*v.y+m[14]*v.z+m[15]*v.w};
}

void Mat4x4Move(mat4 m, vec3 pos) {
	m[3] = pos.x, m[7] = pos.y, m[11] = pos.z;
}
//Here I simplified the matrix multiplication to make it faster by removing the mults by zero
void Mat4x4RotX(mat4 m, const F32 q) {
	F32 tmp, sinq = sinf(q), cosq = cosf(q);
	for (u32 i=0; i<4; i++)
		tmp = m[i+4], m[i+4] = m[i+4]*cosq-m[i+8]*sinq, m[i+8] = m[i+8]*cosq+tmp*sinq;
}
void Mat4x4RotY(mat4 m, const F32 q) {
	F32 tmp, sinq = sinf(q), cosq = cosf(q);
	for (u32 i=0; i<4; i++)
		tmp = m[i], m[i] = m[i]*cosq+m[i+8]*sinq, m[i+8] = m[i+8]*cosq-tmp*sinq;
}
void Mat4x4RotZ(mat4 m, const F32 q) {
	F32 tmp, sinq = sinf(q), cosq = cosf(q);
	for (u32 i=0; i<4; i++)
		tmp = m[i], m[i] = m[i]*cosq-m[i+4]*sinq, m[i+4] = m[i+4]*cosq+tmp*sinq;
}
void Mat4x4Rot(mat4 m, const F32 x, const F32 y, const F32 z) {
	Mat4x4RotX(m,x);
	Mat4x4RotY(m,y);
	Mat4x4RotZ(m,z);
}
//scale matrices (multiply all components of them by s)
void Mat4x3Scale(mat4 m, F32 s) {
	for (u32 i=0; i<12; i++)
		m[i] *= s;
}
void Mat4x4Scale(mat4 m, F32 s) {
	for (u32 i=0; i<16; i++)
		m[i] *= s;
}
void Mat4x4ApplyView(mat4 m, vec3 camera_pos, vec3 camera_rot) { //faster than a matrix multiplication
	m[3] -= camera_pos.x, m[7] -= camera_pos.y, m[11] -= camera_pos.z;
	Mat4x4RotY(m, -camera_rot.y);
	Mat4x4RotX(m, -camera_rot.x);
	Mat4x4RotZ(m, -camera_rot.z);
}
#endif
