#pragma once

class IBTParameter
{
public:

	IBTParameter();
	~IBTParameter();

	void Set(char * inName, char * inDescription, char * inUnits, int inSampleCount, int inLineOffset, int inType, char * inBuffer, int inLineLength);
	bool Set(char * inName, char * inDescription, char * inUnits, int inSampleCount);
	void SetMinMax(double inMin, double inMax);
	void Clear(void);

	char * GetName();
	char * GetMnemonic();
	char * GetGroup();
	char * GetUnits();
	char * GetFormat();
	char * GetDescription();
	double GetMinValue();
	double GetMaxValue();
	int GetLineOffset();
	int GetType();

	double GetSample(int inTickIndex);
	void SetSample(int inTickIndex, double inValue);
	int GetSampleCount();

private:

	void CalculateMinMax();

	static const int kMaxStringLength = 64;

	bool m_set;

	char m_name[kMaxStringLength];
	char m_mnemonic[kMaxStringLength];
	char m_group[kMaxStringLength];
	char m_units[kMaxStringLength];
	char m_description[kMaxStringLength];
	char m_format[kMaxStringLength];
	
	bool m_minMaxSet;
	double m_min;
	double m_max;

	int m_sampleCount;
	char * m_buffer;
	bool m_allocated;

	int m_lineOffset;
	int m_lineLength;
	int m_type;

};
