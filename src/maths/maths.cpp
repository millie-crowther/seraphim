#include "maths.h"

#include <cmath>

float 
maths::inverse_square_root(float x){
	float halfx = 0.5f * x;
	float y = x;
	long i = *(long*)&y;
	i = 0x5f3759df - (i>>1);
	y = *(float*)&i;
	y = y * (1.5f - (halfx * y * y));   // 1st iteration
//	y = y * (1.5f - (halfx * y * y));   // 2nd iteration, this can be removed
	return y; 
}