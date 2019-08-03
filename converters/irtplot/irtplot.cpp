#define _CRT_SECURE_NO_WARNINGS

#include <SDKDDKVer.h>

#include <stdio.h>
#include <tchar.h>

#include "..\common\irt.h"
#include "..\common\run.h"

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 2)
	{
		printf("usage: %s <input_irt>\n");
	}
	else
	{
		char filename[128];
		strcpy(filename, argv[1]);

		char * extension = strstr(filename, ".irt");
		if (extension != NULL) *extension = '\0';

		char irtFilename[128];
		sprintf(irtFilename, "%s.irt", filename);

		irt_samples_t irt_samples;

		irt_samples.load(irtFilename);

		run_samples_t run_samples;

		run_samples.load(&irt_samples);
		run_samples.correct_using_start_finish();
		run_samples.plot(filename, 0.001);

	}

	return 0;
}

