#ifndef RND
#define RND
#include <stdlib.h>
float rndz(float range) { //from 0 to range
	return rand() * (range * (1./RAND_MAX));
}
float rnd(float min, float max) {
	return min + rndz(max-min);
}
#endif
