#include "aes128.h"
#include "../inc/aes128.h"
#include "../inc/code-auth.c"
#include <stdio.h>
#include <stdlib.h>

#define signature_size 16 //16B

void main() {

	uint8_t *app_bin = NULL;
	int bufsize;

    // open app binary for reading and transfer content to buffer
    FILE* file_ptr = fopen("app.bin", "rb");

	if (file_ptr != 0){
    	// go to the end of binary
    	if (fseek(file_ptr, 0L, SEEK_END) == 0) {
        	// get the size of the binary
        	bufsize = ftell(file_ptr);
		}

		// allocate memory for app-buffer 
		uint8_t *app_bin = malloc (sizeof(uint8_t) * (bufsize + 1));
		// move curser to beginning of file
		fseek(file_ptr, 0L, SEEK_SET);
		// copy contents of file to app buffer
	    fread(app_bin, sizeof(uint8_t), bufsize, file_ptr);

		// Holder for generating MAC signature
		uint8_t mac[16];
		// calculate MAC signature of app binary
		uint8_t* ptr_signature = calc_sig(app_bin, bufsize, mac);



		// create and open new file, the formated binary
		FILE* new_file_ptr = fopen("app_format.bin", "wb");

		// append signature to new file
		fwrite(ptr_signature, sizeof(uint8_t), signature_size, new_file_ptr);

		// append .sections info
		// ...

		// append binary to the new file bytewise
		fwrite(app_bin, sizeof(uint8_t), bufsize, new_file_ptr);

		// free app buffer
		free(app_bin);

		// close files
		fclose(file_ptr);
		fclose(new_file_ptr);

	} else{
		puts("file is empty");
	}

}