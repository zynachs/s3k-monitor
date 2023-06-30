#include "../inc/aes128.c"
#include "../inc/code-auth.c"
#include <stdio.h>
#include <stdlib.h>
#define signature_size 16 
#define sectioninfo_size 8 
#define header_size 512

void main() {

	uint8_t *app_bin = NULL;
	int bufsize;

    // open app binary for reading
    FILE* file_ptr = fopen("../app/app.bin", "rb");
	// open file with sections info
	FILE* sectioninfo_ptr = fopen("sectionsinfo.txt","r");
	// create and open new file, the formated binary
	FILE* new_file_ptr = fopen("../app/app_format.bin", "wb");

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
		// append signature to new file
		fwrite(signature, sizeof(uint8_t), signature_size, new_file_ptr);

		// append sections info
		char line[18];
		while(fgets(line, sizeof(line), sectioninfo_ptr)){
			printf("%lx", strtol(line, NULL, 16));
			long converted_value = strtol(line, NULL, 16);
			fwrite(&converted_value, sizeof(uint8_t), sectioninfo_size, new_file_ptr);
			puts("");
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