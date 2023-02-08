#pragma once
#include <stdlib.h>
#include "evar.h"

typedef struct FVector4 {F32 x,y,z,w;} vec4;
typedef F32 *mat4;

vec3 Vec3RotX(vec3 v, F32 q); //rotate Vector3 around X axis
vec3 Vec3RotY(vec3 v, F32 q); //rotate Vector3 around Y axis
vec3 Vec3RotZ(vec3 v, F32 q); //rotate Vector3 around Z axis
vec3 Vec3Rot(vec3 v, vec3 r); //rotate Vector3 around X -> Y -> Z axis
vec3 GetTriangleNormal(vec3 A, vec3 B, vec3 C);

static inline mat4 MallocMat4x4() {
	return malloc(16*sizeof(F32));
}
//returns an allocated and normalized matrix
mat4 Mat4x4();

void Mat4x4Reset(mat4 m);
void Mat4x3Reset(mat4 m);
void Mat4x4Set(mat4 m, vec3 pos, vec3 rot, vec3 scale);

//multiplies matrices and the result goes into dest
//4x3 is an optimization. It's not actually 4x3 but 4th layer is assumed to be [0 0 0 1]
void Mat4x3MultMat4x3(mat4 a, mat4 b, mat4 dest);
void Mat4x4MultMat4x4(mat4 a, mat4 b, mat4 dest);

vec3 Mat4x3MultVec3(mat4 m, vec3 v); //multiply 4x3 matrix by a 3D vector
vec4 Mat4x4MultVec3(mat4 m, vec3 v); //multiply 4x4 matrix by a 3D vector (assuming w=1)
vec4 Mat4x4MultVec4(mat4 m, vec4 v); //multiply 4x4 matrix by a 4D vector. Normal but tedious

void Mat4x4Move(mat4 m, vec3 pos); //moves matrix to specified position

//rotate matrices
void Mat4x4RotX(mat4 m, const F32 q);
void Mat4x4RotY(mat4 m, const F32 q);
void Mat4x4RotZ(mat4 m, const F32 q);
void Mat4x4Rot(mat4 m, const F32 x, const F32 y, const F32 z); //order is X -> Y -> Z

//scale matrices (multiply all components of them by s)
void Mat4x3Scale(mat4 m, F32 s);
void Mat4x4Scale(mat4 m, F32 s);

//rotate and move matrix
void Mat4x4ApplyView(mat4 m, vec3 camera_pos, vec3 camera_rot);
