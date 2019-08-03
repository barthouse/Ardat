#pragma once

#include <Windows.h>

#include <assert.h>

class MemoryMappedFile
{
public:

	MemoryMappedFile();
	~MemoryMappedFile();

	bool Open(const char * inFileName);
	void Close(void);

	int GetLength();
	char * GetBuffer();

private:

	HANDLE	m_fileHandle;
	HANDLE  m_fileMapping;
	bool	m_opened;
	int		m_length;
	char *	m_buffer;

};
