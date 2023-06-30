#pragma once
#include <stdint.h>

// calculates signature and stores it in mac
void calc_sig (uint8_t *buf, int len, uint8_t *mac);

// compares two signatures and returns 1 if they are equal
int comp_sig (uint8_t *sig1, uint8_t *sig2);


