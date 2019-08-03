#include <Windows.h>

#include <assert.h>
#include <memory.h>
#include <malloc.h>

#include "IBTParameter.h"
#include "irsdk_defines.h"

IBTParameter::IBTParameter()
{
	m_name[0] = 0;
	m_mnemonic[0] = 0;
	m_units[0] = 0;
	m_format[0] = 0;
	m_description[0] = 0;

	strcpy_s(m_group, "iRacing");
	strcpy_s(m_format, "%5.3f");

	m_min = 0;
	m_max = 0;
	m_minMaxSet = false;

	m_sampleCount = 0;
	m_buffer = NULL;
	m_allocated = false;

	m_set = false;
}

IBTParameter::~IBTParameter()
{
	assert(!m_set);
	assert(m_buffer==NULL);
}

void IBTParameter::Set(char * inName, char * inDescription, char * inUnits, int inSampleCount, int inLineOffset, int inType, char * inBuffer, int inLineLength)
{
	assert(!m_set);
	
	strcpy_s(m_name, inName);
	strcpy_s(m_mnemonic, inName);
	strcpy_s(m_description, inDescription);
	strcpy_s(m_units, inUnits);

	m_sampleCount = inSampleCount;
	m_lineOffset = inLineOffset;
	m_type = inType;
	m_buffer = inBuffer;
	m_allocated = false;
	m_lineLength = inLineLength;

	m_set = true;

}

bool IBTParameter::Set(char * inName, char * inDescription, char * inUnits, int inSampleCount)
{
	assert(!m_set);
	
	m_buffer = (char *) malloc(sizeof(double) * inSampleCount);

	if (m_buffer != NULL)
	{
		strcpy_s(m_name, inName);
		strcpy_s(m_mnemonic, inName);
		strcpy_s(m_description, inDescription);
		strcpy_s(m_units, inUnits);

		m_sampleCount = inSampleCount;
		m_lineOffset = 0;
		m_type = irsdk_double;
		m_allocated = true;
		m_lineLength = sizeof(double);

		m_set = true;
	}

	return m_set;
}

void IBTParameter::Clear(void)
{
	if (m_allocated)
	{
		free(m_buffer);
		m_allocated = false;
	}

	m_buffer = NULL;
	m_sampleCount = 0;

	m_set = false;
	m_minMaxSet = false;
}

char * IBTParameter::GetName()
{
	assert(m_set);
	return m_name;
}

char * IBTParameter::GetMnemonic() 
{ 
	assert(m_set);
	return m_mnemonic;
}

char * IBTParameter::GetGroup()
{ 
	assert(m_set); 
	return m_group;
}

char * IBTParameter::GetUnits() 
{ 
	assert(m_set); 
	return m_units;
}

char * IBTParameter::GetFormat() 
{ 
	assert(m_set); 
	return m_format;
}

char * IBTParameter::GetDescription() 
{ 
	assert(m_set); 
	return m_description; 
}

double IBTParameter::GetMinValue() 
{ 
	assert(m_set);
	CalculateMinMax();
	return m_min;
}

double IBTParameter::GetMaxValue() 
{ 
	assert(m_set);
	CalculateMinMax();
	return m_max; 
}

void IBTParameter::SetMinMax(double inMin, double inMax)
{
	m_min = inMin;
	m_max = inMax;
	m_minMaxSet = true;
}

int IBTParameter::GetLineOffset() 
{ 
	assert(m_set);
	return m_lineOffset;
}

int IBTParameter::GetType() 
{ 
	assert(m_set); 
	return m_type; 
}

void IBTParameter::SetSample(int inTickIndex, double inValue)
{
	assert(m_buffer!=NULL);
	assert(inTickIndex < m_sampleCount);
	assert(m_allocated);
	
	double * values = (double *) m_buffer;
	values[inTickIndex] = inValue;

}

double IBTParameter::GetSample(int inTickIndex) 
{ 
	assert(m_buffer!=NULL);
	assert(inTickIndex < m_sampleCount);

	int bufferOffset = (inTickIndex * m_lineLength) + m_lineOffset;
	double sample = 0;

    __try
	{
		switch(m_type)
		{
		case irsdk_char: sample = (double) *((char *) (m_buffer + bufferOffset)); break;
		case irsdk_bool: sample = (double) *((bool *) (m_buffer + bufferOffset)); break;
		case irsdk_int: sample = (double) *((int *) (m_buffer + bufferOffset)); break;
		case irsdk_bitField: sample = (double) *((unsigned int *) (m_buffer + bufferOffset)); break;
		case irsdk_float: sample = (double) *((float *) (m_buffer + bufferOffset)); break;
		case irsdk_double: sample = (double) *((double *) (m_buffer + bufferOffset)); break;
		}
	}
	__except(GetExceptionCode()==EXCEPTION_IN_PAGE_ERROR ?
					EXCEPTION_EXECUTE_HANDLER : 
					EXCEPTION_CONTINUE_SEARCH)
	{
		// Failed to read from the view.
	}

	return sample;
}

int IBTParameter::GetSampleCount()
{
	return m_sampleCount;
}

void IBTParameter::CalculateMinMax()
{
	assert(m_set);

	if (!m_minMaxSet)
	{
		if (m_sampleCount == 0)
		{
			m_min = m_max = 0;
		}
		else
		{
			m_min = m_max = GetSample(0);

			for(int i=1; i<this->m_sampleCount;i++)
			{
				double value = GetSample(i);
				m_min = (m_min > value ? value : m_min);
				m_max = (m_max < value ? value : m_max);
			}
		}

		m_minMaxSet = true;
	}
}
