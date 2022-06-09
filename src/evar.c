#ifndef _EVAR //Elafri definitions for using addons by themselves
#define _EVAR
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <sys/select.h>
#include <sys/time.h>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define F32 float
#define F64 double

#define max(x,y) ((x) > (y) ? (x) : (y))
#define min(x,y) ((x) < (y) ? (x) : (y))

#define InitTimer(t) struct timeval t ## _tstart, t ## _tend
#define StartTimer(t) gettimeofday(&t ## _tstart, NULL)
#define StopTimer(t) gettimeofday(&t ## _tend, NULL)
#define dt_sec(t) (t ## _tend.tv_sec-t ## _tstart.tv_sec)
#define dt_usec(t) (t ## _tend.tv_usec-t ## _tstart.tv_usec)
#define Sleep(time) select(1, NULL, NULL, NULL, &time)

typedef struct FVector2 {F32 x,y;} vec2;
typedef struct FVector3 {F32 x,y,z;} vec3;
typedef struct UVector2 {u32 x,y;} uvec2;
typedef struct IVector2 {int x,y;} ivec2;

int mod32(int x, int m) { //actual modulo for negative numbers
	return (x%m + m)%m;
}
long mod64(long x, long m) {
	return (x%m + m)%m;
}
F32 modF32(F32 x, F32 m) {
	return fmodf(fmodf(x,m)+m,m);
}
#endif
