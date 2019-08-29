#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <stdio.h>
#include <tchar.h>
#include <assert.h>
#include <string.h>
#include <malloc.h>

#include "..\common\channel_data.h"
#include "..\common\csv_data.h"
#include "..\common\run_data.h"

void process_file(char * input_file_name)
{
    size_t len = strlen(input_file_name) + 7;
	char * output_file_name = (char *) malloc(len);

	strcpy_s(output_file_name, len, input_file_name);

	char * extension = strrchr(output_file_name, '.');

	if (extension != NULL && strcmp(extension,".run")==0)
	{
        size_t extension_len = len - (extension - output_file_name);
		sprintf_s(extension, extension_len, ".clean.run");

		c_run run;
		 
		run.open();
		run.read_from_file(input_file_name);
		run.offset();
		run.write_to_file(output_file_name);
		run.close();
	}

	free(output_file_name);
}

int _tmain(int argc, _TCHAR* argv[])
{
	for (int i=1; i < argc; i++)
	{
		char * input_file_name = argv[i];

		process_file(input_file_name);
	}

	return 0;
}
