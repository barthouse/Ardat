#include <Windows.h>

#include "MemoryMappedFile.h"

MemoryMappedFile::MemoryMappedFile()
{
	m_opened = false;
	m_fileHandle = INVALID_HANDLE_VALUE;
	m_fileMapping = INVALID_HANDLE_VALUE;
	m_length = 0;
	m_buffer = NULL;
}

MemoryMappedFile::~MemoryMappedFile()
{
	assert(!m_opened);
}

bool MemoryMappedFile::Open(const char * inFileName)
{
	bool success;

	success = (m_fileHandle = CreateFile(inFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL)) != INVALID_HANDLE_VALUE;

	if (success)
	{
		m_length = GetFileSize(m_fileHandle, NULL);

		success = (m_fileMapping = CreateFileMapping(m_fileHandle, NULL, PAGE_READONLY, 0, 0, NULL)) != INVALID_HANDLE_VALUE;
	}

	if (success)
	{
		success = (m_buffer = (char *) MapViewOfFile(m_fileMapping, FILE_MAP_READ, 0, 0, 0)) != NULL;
	}

	if (!success)
	{
		if (m_fileMapping)
		{
			CloseHandle(m_fileMapping);
			m_fileMapping = INVALID_HANDLE_VALUE;
		}

		if (m_fileHandle)
		{
			CloseHandle(m_fileHandle);
			m_fileHandle = INVALID_HANDLE_VALUE;
		}
	}
	else
	{
		m_opened = true;
	}

	return success;
}

void MemoryMappedFile::Close(void)
{
	if (m_opened)
	{
		if (m_buffer != NULL)
		{
			UnmapViewOfFile(m_buffer);
			m_buffer = NULL;
		}

		if(m_fileMapping != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_fileMapping);
			m_fileMapping = INVALID_HANDLE_VALUE;
		}
			
		if (m_fileHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_fileHandle);
			m_fileHandle = INVALID_HANDLE_VALUE;
		}
			
		m_length = 0;
		m_opened = false;
	}
}

int MemoryMappedFile::GetLength()
{
	assert(m_opened);
	return m_length;
}

char * MemoryMappedFile::GetBuffer()
{
	assert(m_opened);
	return m_buffer;
}

