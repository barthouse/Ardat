// directory_test.cpp : Defines the entry point for the console application.
//


#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#include <stdio.h>
#include <tchar.h>
#include <direct.h>

int _tmain(int argc, _TCHAR* argv[])
{
	int errno = _mkdir(".\\foodir");
	printf("errno = %d\n", errno);

	return 0;
}

