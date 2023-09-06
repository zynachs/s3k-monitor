#include "../inc/aes128.h"
#include "../inc/code-auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define signature_size 16 
#define sectioninfo_size 8 
#define header_size 512

void main() {

	uint8_t *app_buffer = NULL;
	int bufsize;

    // open app binary for reading
    FILE* file_ptr = fopen("build/app.bin", "rb");
	// open file with sections info
	FILE* sectioninfo_ptr = fopen("scripts/sectionsinfo.txt","r");
	// create and open new file, the formated binary
	FILE* new_file_ptr = fopen("build/app_format.bin", "wb");

	if (file_ptr != 0){

/*		creating buffer containing header and app-binary		*/

    	// set bufsize to size of binary + size of header
    	if (fseek(file_ptr, 0L, SEEK_END) == 0) {
        	bufsize = ((ftell(file_ptr))+ header_size);
		}

		// move curser back to beginning of file
		fseek(file_ptr, 0L, SEEK_SET);

		// allocate memory for app-buffer 
		app_buffer = malloc(sizeof(uint8_t) * (bufsize + 1));

		// index to keep track of buffer pointer
		size_t buffer_index = 0;
		// expected max size of every line in sectionsinfo.txt
    	char line[18];

		// reading sections info into buffer
    	while (buffer_index < header_size && (fgets(line, sizeof(line), sectioninfo_ptr) != NULL)) {
			long converted_value = strtol(line, NULL, 16);
        	memcpy(app_buffer + buffer_index, &converted_value, sectioninfo_size);
        	buffer_index += sectioninfo_size;

	        puts("");
    	}
    	// Fill the remaining space in app_buffer with zero
    	while (buffer_index < header_size) {
        	app_buffer[buffer_index] = 0;
        	buffer_index++;
    	}

		// copy binary to app buffer
	    fread(app_buffer + buffer_index, sizeof(uint8_t), bufsize - buffer_index, file_ptr);

/*		generating signature of app-buffer		*/

		// Holder for generating MAC signature
		uint8_t signature[16];
		// calculate MAC signature of app_buffer (header+binary)
		calc_sig(app_buffer, bufsize, signature);


/*		Putting together the formatted file		*/

		// append signature to new file
		fwrite(signature, sizeof(uint8_t), signature_size, new_file_ptr);
		// move file cursor to end of header
		fseek (new_file_ptr, signature_size, SEEK_SET);
		// append header and binary to the new file bytewise
		fwrite(app_buffer, sizeof(uint8_t), bufsize, new_file_ptr);

/*		free buffer and close files		*/

		// free app buffer
		free(app_buffer);
		// close files
		fclose(file_ptr);
		fclose(new_file_ptr);

	} else{
		puts("file is empty");
	}

}