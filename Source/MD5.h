#pragma once

#include <string>

typedef union uwb {
	unsigned w;
	unsigned char b[4];
} WBunion;

typedef unsigned Digest[4];

unsigned f0(unsigned abcd[]);
unsigned f1(unsigned abcd[]);
unsigned f2(unsigned abcd[]);
unsigned f3(unsigned abcd[]);

typedef unsigned(*DgstFctn)(unsigned a[]);

std::string  CreateUID(const std::string& name);
unsigned int* md5(const char *msg, int mlen);
unsigned rol(unsigned v, short amt);
unsigned *calcKs(unsigned *k);

