#ifndef __RNG
#define __RNG

#include "PCG Random/pcg_basic.h"

int randomFromTo(int n1, int n2);

float randomFloat();

uint32_t random32bits(); // ONLY USED TO GENERATE UUIDs NEVER RETURN 0, 0 DOESN'T EXISIT 0 IS NOT UUID 0 IS BAD

void randomizeSeed();


#endif