#ifndef _EVAR //file made to keep Elafri addons more modular
#define _EVAR //make sure to compile with -flto to remove unnecessary garbage
#include <sys/select.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t
#define F32 float
#define F64 double

#define max(x,y) ((x) > (y) ? (x) : (y))
#define min(x,y) ((x) < (y) ? (x) : (y))
#define pi 3.1415926535897932384626433832795
#define ROT 1024 //accuracy of precomputes for sine/cosine

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

F32 sinebuf[ROT*2+ROT/4]; //just some precomputes
F32 *wsin;
F32 *wcos;

static inline void SineInit() {
	for (int i=0; i<ROT*2+ROT/4; i++) //Initialize sinewave precomputes
		sinebuf[i] = sinf((2*pi/ROT)*i);
	wsin = sinebuf+ROT;
	wcos = &wsin[ROT/4];
}

int s(int x) { //get sign of 32 bit integer
	return -(((u32)x>>31)*2-1);
}
int sF32(const F32 x) {
	return (x>0) - (x<0);
}
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
