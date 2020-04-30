#include <stdio.h>

#define LED2_MASK 0b000000000000010 // using a bit mask for our desired result & bitwise operations to get the desired result

int main() {
	short status = 0b000000000000011; // short is 16 bits, which is enough to hold the info we need
	if(status & LED2_MASK) {
		printf("LED 2 is on!\n");
		printf("%d\n", status);
	}
	
	status = status | LED2_MASK; // OR-ing the mask we have to add bits to it (adds the 1s in place of 0s and keeps 1s)
	// used for reading permissions from st_mode struct from stat()
}

// 0b000000000000010
// &
// 0b000000000000011
// -----------------
// 0b000000000000010
