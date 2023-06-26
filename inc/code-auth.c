#include <stdint.h>
#include "aes128.c"


// calculates signature and returns pointer to signature
uint8_t* calc_sig (uint8_t *buf, int len, uint8_t *mac){

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

	// generate CBC-MAC from buf
	aes128_cbc_mac(mac_rk, buf, mac, len);

	return mac;

}

// compares two signatures and returns 1 if they are equal
int comp_sig (uint8_t *sig1, uint8_t *sig2){
		
	// compare generated MAC with signature; match = 1, no match = 0
	int equal = 1;
	for (int i=0; i<128; i++){
		if (sig1[i]!=sig2[i]){
			equal=0;
		}
	}

	// for debugging
	/*
	if (equal == 0){
		alt_puts("MAC not approved");
	}
	if (equal == 1){
		alt_puts("MAC approved");
	}
	*/

	return equal;

}