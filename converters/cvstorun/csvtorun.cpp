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

void process_file(char * in_file_name)
{
	printf("processing %s\n", in_file_name);

	char * output_file_name = _strdup(in_file_name);

	if (output_file_name != NULL)
	{
		char * extension = strrchr(output_file_name, '.');

		if (extension != NULL && (strcmp(extension,".vbo")==0||strcmp(extension,".csv")==0))
		{
			extension[1] = 'r';
			extension[2] = 'u';
			extension[3] = 'n';

			FILE * output_file = NULL;

			printf("outputing %s\n", output_file_name);

			if(fopen_s(&output_file, output_file_name, "wb")==0)
			{
				// write header

				fputc(0x98, output_file);
				fputc(0x1D, output_file);
				fputc(0x00, output_file);
				fputc(0x00, output_file);
				fputc(0xC8, output_file);
				fputc(0x00, output_file);
				fputc(0x00, output_file);
				fputc(0x00, output_file);

				c_csv_data_decoder data_decoder;

				data_decoder.begin(in_file_name);

				while(1)
				{
					c_csv_data * csv_data = data_decoder.get_csv_data();

					if (csv_data==NULL)
						break;

					csv_data->write_run_data(output_file);

					delete csv_data;
				}

				data_decoder.end();

				fclose(output_file);

			}
			else
			{
				printf("unable to open %s\n", output_file_name);
			}
		}
		else
		{
			printf("input file name must end in .csv or .vbo\n");
		}

		free(output_file_name);
	}
	else
	{
		printf("unable to allocate output file string\n");
	}
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
