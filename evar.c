#ifndef _EVAR //file made to keep Elafri addons distinct
#define _EVAR //make sure to compile with -flto to remove unnecessary garbage
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long
#define F32 float
#define F64 double

#define max(x,y) ((x) > (y) ? (x) : (y))
#define min(x,y) ((x) < (y) ? (x) : (y))
#define pi 3.1415926535897932384626433832795

#define InitTimer(t) struct timeval t ## _tstart, t ## _tend
#define StartTimer(t) gettimeofday(&t ## _tstart, NULL)
#define StopTimer(t) gettimeofday(&t ## _tend, NULL)
#define dt_sec(t) (t ## _tend.tv_sec-t ## _tstart.tv_sec)
#define dt_usec(t) (t ## _tend.tv_usec-t ## _tstart.tv_usec)
#define Sleep(time) select(1, NULL, NULL, NULL, &time)

#include <sys/select.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct FVector2 {F32 x,y;} vec2;
typedef struct UVector2 {u32 x,y;} uvec2;
typedef struct IVector2 {int x,y;} ivec2;

int s(int x) { //get sign of 32 bit integer
    return -(((u32)x>>31)*2-1);
}
int mod32(int x, int m) { //the way actual mod works
    return (x%m + m)%m;
}
long mod64(long x, long m) {
    return (x%m + m)%m;
}
F32 modF32(F32 x, F32 m) {
    return fmod(fmod(x,m)+m,m);
}
#endif
