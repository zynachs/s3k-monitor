#include <stdint.h>
#include "altio.h"
#include "aes128.h"


int code_auth (uint8_t *buf, uint8_t *sig, int len){

	alt_puts("inside code_auth");

	// Encryption key
	uint32_t mac_key[4] = {
		0x16157e2b,
		0xa6d2ae28,
		0x8815f7ab,
		0x3c4fcf09,
	};

	// Encryption round key
	uint32_t mac_rk[44];

	// generate round key (mac_rk) from key (mac_key)
	aes128_keyexpansion(mac_key, mac_rk);

	// Holder for generating MAC
	uint8_t mac[16];

	// generate CBC-MAC from buf
	aes128_cbc_mac(mac_rk, buf, mac, len);

	// compare generated MAC with signature; match = 1, no match = 0
	int equal = 1;
	for (int i=0; i<len; i++){
		if (sig[i]!=mac[i]){
			equal=0;
		}
	}

	// for debugging
	if (equal == 0){
		alt_puts("MAC not approved");
	}
	if (equal == 1){
		alt_puts("MAC approved");
	}

	return equal;
}