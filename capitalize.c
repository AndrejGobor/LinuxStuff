#include "busybox.h"

int capitalize_main(int argc, char **argv)
{
	FILE* in_file;
	FILE* out_file;

	char* buff = NULL;
	int i = 0;
	int file_len = 0;
	int in_file_counter = 1;

	int retval = EXIT_SUCCESS;
	if (argc < 3) bb_show_usage();
	
	// Looping to the last argument(last file name)
	while (in_file_counter < argc - 1) {
		// Opening input file
		if ((in_file = fopen(argv[in_file_counter], "r")) != NULL)
		{
			// Checking file length
			fseek(in_file, 0, SEEK_END);
			file_len = ftell(in_file);
			
			// Reallocating memory for buffer
			buff = realloc(buff, file_len);
			if(!buff) 
			{
				retval = EXIT_FAILURE;
				break;
			}
			
			// Returning pointer to the start of file
			fseek(in_file, 0, SEEK_SET);
			
			// Reading chars to buffer
			file_len = fread(buff, sizeof(char), file_len, in_file);
			
			// Opening output file
			out_file = fopen(argv[argc-1], "a");
			if(!out_file) 
			{
				retval = EXIT_FAILURE;
				break;
			}
			
			// Switching chars to upper case and writing to file
			for (i = 0; i < file_len; i++)
			{
				fputc(toupper(buff[i]), out_file);			
			}
			
			// Closing files
			fclose(in_file);
			fclose(out_file);

			// Incrementing counter for argument(next file name)
			in_file_counter++;
			
			if (file_len >= 0) continue;
		}
		retval = EXIT_FAILURE;
	}
	return retval;
}
