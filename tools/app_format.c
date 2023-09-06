#include "codeauth.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* config file for testing */
#include "../config.h"

#define signature_size 16 
#define sectioninfo_size 8 
#define header_size 512

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
	char info_file[64];

	strcpy(input_file, argv[1]);

	strcpy(filename, strtok(argv[1], "."));
	strcpy(extension, strtok(NULL, "."));

	/* creates name for output file */
	strcpy(output_file, filename);
	strcat(output_file, ".fmt.");
	strcat(output_file, extension);

	/* creates name for sectionsfile */
	strcpy(info_file, filename);
	strcat(info_file, ".txt");

    // open app binary for reading
    FILE* file_ptr = fopen(input_file, "rb");

	// open file with sections info
	FILE* sectioninfo_ptr = fopen(info_file,"r");

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

		// append sections info
		char line[18];
		while(fgets(line, sizeof(line), sectioninfo_ptr)){
			long converted_value = strtol(line, NULL, 16);
			fwrite(&converted_value, sizeof(uint8_t), sectioninfo_size, new_file_ptr);
		}

		// move file cursor to end of header
		fseek (new_file_ptr, header_size, SEEK_SET);

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