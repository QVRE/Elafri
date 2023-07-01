#pragma once
#include <stdint.h>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define F32 float
#define F64 double

typedef struct {F32 x,y;}   vec2;
typedef struct {F32 x,y,z;} vec3;
typedef struct {F64 x,y;}   dvec2;
typedef struct {F64 x,y,z;} dvec3;
typedef struct {u32 x,y;}   uvec2;
typedef struct {u32 x,y,z;} uvec3;
typedef struct {int x,y;}   ivec2;
typedef struct {int x,y,z;} ivec3;
