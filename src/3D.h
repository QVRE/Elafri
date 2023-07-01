#pragma once
#include <stdlib.h>
#include "graphics.h"
#include "grmath.h"
#include "evar.h"

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
	vec3 *vert; void *tri; vec3 *norm; //tri's type is either face or color_face
	gr *texture; //set texture to NULL if you're using color faces
} obj;

extern vec3 *proj_pnt; //projected point buffer used while rendering an object
extern u32 proj_pnt_size; //size of allocated array

//this buffer is used to cache the on-screen projection of each vertex of an object
//call this function using the max amount of vertices each object can end up having
//objects with more vertices than the cache size will not be drawn
static inline void MallocProjectedPointBuffer(int max_vertice_count) {
	proj_pnt = malloc(max_vertice_count*sizeof(vec3));
	proj_pnt_size = max_vertice_count;
}

static inline void ObjectMakeMatrix(obj object) { //makes a matrix using the object's attributes
	Mat4x4Set(object.mat, object.pos, object.rot, object.scale);
}

static inline void DepthReset(F32 *depth, u32 w, u32 h) {
	for (int i=0; i<w*h; i++) depth[i] = 1; //1 is considered view distance
}

static inline F32* MallocDepthBuffer(u32 w, u32 h) {
	return malloc(w*(h+1)*sizeof(F32));
}

//returns a vector with the attributes of the camera
//FOV is in degrees 0-360, aspect ratio is height divided by width of screen
vec3 MakeCamera(F32 fov, F32 aspect_ratio, F32 view_distance);

//draws a 3D single color triangle on the graphics and depth buffers
void GrTriangle3D(gr *buf, F32 *depth, vec3 At, vec3 Bt, vec3 Ct, color clr);

//used by 3D textured triangle drawing
//takes a texture and UV coordinates as well as a light value used to shade the pixel
//will write the resulting color into the pixel pointed to by *dest
void GrTexturedPixel(color *dest, gr *texture, F32 u, F32 v, F32 light);

//draws a 3D textured triangle on the graphics and depth buffers
//light is a value from 0 to 1 used to shade the given texture when drawing
void GrTexturedTriangle3D(gr *buf, F32 *depth, vertex At, vertex Bt, vertex Ct, gr *texture, F32 light);

//pos and rot are camera variables ( pos -> position, rot -> rotation (euler) )
//params are gotten from MakeCamera() and light is a vertex with xyz and uv params
//light's xyz points TO the light source and uv is base light and how strong the source is (u >= v)
void GrObject(gr *buf, F32 *depth, obj object, vec3 pos, vec3 rot, vertex light, vec3 param);
