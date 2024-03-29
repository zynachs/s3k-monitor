#include "codeauth.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* config file for testing */
#include "../config.h"

#define signature_size 16 

void main(int argc, char * argv[]) {

	uint8_t *app_bin = NULL;
	int bufsize;

	if (argc < 2) {
		printf("Too few arguments provided. Expects one argument containing the input file name.\n");
		return;
	}

	char input_file[64]; 
	char filename[64];
	char extension[64];
	char output_file[64];

	strcpy(input_file, argv[1]);

	strcpy(filename, strtok(argv[1], "."));
	strcpy(extension, strtok(NULL, "."));

	/* creates name for output file */
	strcpy(output_file, filename);
	strcat(output_file, ".sig.");
	strcat(output_file, extension);

    // open app binary for reading
    FILE* file_ptr = fopen(input_file, "rb");

	// create and open new file, the formated binary
	FILE* new_file_ptr = fopen(output_file, "wb");

	if (file_ptr != 0){
    	// go to the end of binary
    	if (fseek(file_ptr, 0L, SEEK_END) == 0) {
        	// get the size of the binary
        	bufsize = ftell(file_ptr);
		}

		// allocate memory for app-buffer 
		uint8_t *app_bin = malloc(sizeof(uint8_t) * (bufsize + 1));
		// move curser to beginning of file
		fseek(file_ptr, 0L, SEEK_SET);
		// copy contents of file to app buffer
	    fread(app_bin, sizeof(uint8_t), bufsize, file_ptr);

		// Holder for generating MAC signature
		uint8_t signature[16];
		// calculate MAC signature of app binary
		calc_sig(app_bin, bufsize, signature);

#ifdef SIG_BROKEN
		// break signature
		signature[0] = 0xFF;
		signature[1] = 0xFF;
#endif

		// append signature to new file
		fwrite(signature, sizeof(uint8_t), signature_size, new_file_ptr);

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