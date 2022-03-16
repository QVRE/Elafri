#ifndef SHASH
#define SHASH
#include "evar.c"

#define ROTR64(x,y) ( ((x)>>(y)) | ((x)<<(64-(y))) )
#define ROTL64(x,y) ( ((x)<<(y)) | ((x)>>(64-(y))) )

#define rnd_c1 0x479cb651e5065175
#define rnd_c2 0x313dcaeceb3c2ac0
#define rnd_c3 0x4c64ca47ecbae738
#define rnd_c4 0x93081b762dd2cc37

u64 Mix(u64 x, u64 y) {
    u64 mix[32];
    mix[0] = (x * y) ^ ROTR64(x,22);
    mix[1] = (x * y) ^ ROTR64(y,44);
    mix[2] = (x*rnd_c3) ^ rnd_c1;
    mix[3] = (y*rnd_c4) ^ rnd_c2;
    for (u64 i=4; i<32; i++) {
        const u64 A = ROTL64(mix[i-4],37)^(i*rnd_c3);
        const u64 B = ROTR64(mix[i-1],53)^((127-i)*rnd_c4);
        mix[i] = A^B;
    }
    return mix[31];
}

F64 U64toF64(u64 x) {
    return (x>>12) * (1.0/(~(u64)0>>12));
}

F64 Mix2F64(u64 x, u64 y) {
    return U64toF64(Mix(x,y));
}
u64 MixBetween(u64 min, u64 max, u64 x, u64 y) {
    const u64 delta = max-min;
    return round(min + delta * Mix2F64(x,y));
}
F64 MixBetweenF64(F64 min, F64 max, u64 x, u64 y) {
    const F64 delta = max-min;
    return min + delta * Mix2F64(x,y);
}
#endif
