#ifndef RND
#define RND
#include <stdlib.h>
F32 rndz(F32 range) { //from 0 to range
	return rand() * (range * (1./RAND_MAX));
}
F32 rnd(F32 min, F32 max) {
	return min + rndz(max-min);
}
#endif
