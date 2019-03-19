#include "Random.h"
#include <iostream>
#include <random>


int randomFromTo(int n1, int n2) {

	if (n1 >= n2)
		return 0;

	// Calculate random number
	int bound = n2 - n1;
	int random_number = pcg32_boundedrand(bound);
	return n1 + random_number;
}

float randomFloat(){
	return ldexp(pcg32_random(), -32);
}

uint32_t random32bits() {
	return pcg32_random() + 1;
}

void randomizeSeed() {
	// Get random 64 bit values
	std::random_device rd;
	std::default_random_engine generator(rd());
	std::uniform_int_distribution<uint16_t> distribution(0, 0xFFFFFFFFFFFFFFFF);
	//Randomize seed
	pcg32_srandom(distribution(generator), distribution(generator));
}
