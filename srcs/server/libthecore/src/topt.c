#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/base32.h"
#include "../include/hmac.h"
#include "../include/sha1.h"


int compute_code(const uint8_t *secret, int secretLen, unsigned long value) {
	uint8_t val[8];
	for (int i = 8; i--; value >>= 8) {
		val[i] = value;
	}
	uint8_t hash[SHA1_DIGEST_LENGTH];
	hmac_sha1(secret, secretLen, val, 8, hash, SHA1_DIGEST_LENGTH);
	memset(val, 0, sizeof(val));
	int offset = hash[SHA1_DIGEST_LENGTH - 1] & 0xF;
	unsigned int truncatedHash = 0;
	for (int i = 0; i < 4; ++i) {
		truncatedHash <<= 8;
		truncatedHash |= hash[offset + i];
	}
	memset(hash, 0, sizeof(hash));
	truncatedHash &= 0x7FFFFFFF;
	truncatedHash %= 1000000;
	return truncatedHash;
}

int verify_code(const uint8_t * secret, int secretLen, int code)
{
	const int tm = time(0) / 30;
	int window = 4;
	
	for (int i = -((window-1)/2); i <= window/2; ++i) {
		unsigned int hash = compute_code(secret, secretLen, tm + i);
		if (hash == (unsigned int)code) {
			return 1;
		}
	}
	
	return 0;
}