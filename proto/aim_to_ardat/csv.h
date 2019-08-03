#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <regex>
#include <vector>
#include <map>

/*

    AIM Exporting Guidelines
    1. Export as 'csv'
    2. Select Versus 'time'
    3. Select channels GPS Elevation, GPS Longitude and GPS Latitude

*/

class CsvHelper {
public:

    static bool ReadLine(std::fstream & fs, std::string & line)
    {
        if (fs.eof()) return false;

        char linebuf[4096];
        fs.getline(linebuf, sizeof(linebuf));

        line = linebuf;

        line.append("\n");

        return true;
    }

    static bool ReadString(std::string & line, std::string & s)
    {
        std::smatch matches;

        std::regex_search(line, matches, s_stringPattern);

        if (matches.size() != 2)
            return false;

        s = matches[1].str();

        line = line.substr(matches[0].length());

        return true;
    }

    static bool ReadDouble(std::string & line, double & s)
    {
        std::smatch matches;

        std::regex_search(line, matches, s_doublePattern);

        if (matches.size() != 2)
            return false;

        s = atof(matches[1].str().c_str());

        line = line.substr(matches[0].length());
        return true;
    }

    static bool ReadStrings(std::fstream & fs, std::vector<std::string> & values)
    {
        std::string line;
        if (!CsvHelper::ReadLine(fs, line))
            return false;

        CsvHelper::ReadStrings(line, values);
        return true;
    }

    static void ReadStrings(std::string line, std::vector<std::string> & values)
    {
        std::string value;
        while (CsvHelper::ReadString(line, value))
            values.push_back(value);
    }

    static bool ReadDoubles(std::fstream & fs, std::vector<double> & values)
    {
        std::string line;
        if (!CsvHelper::ReadLine(fs, line))
            return false;

        if (line.size() == 1)
            return false;

        CsvHelper::ReadDoubles(line, values);
        return true;
    }

    static void ReadDoubles(std::string line, std::vector<double> & values)
    {
        double value;
        while (CsvHelper::ReadDouble(line, value))
            values.push_back(value);
    }

private:

    static std::regex s_stringPattern;
    static std::regex s_doublePattern;

};

class CsvHeader {
public:

    bool Read(std::fstream & fs)
    {
        std::string line;

        while (CsvHelper::ReadLine(fs, line)) {

            if (line.length() == 1)
                return true;

            std::string key;
            std::vector<std::string> values;

            if (CsvHelper::ReadString(line, key)) {
                CsvHelper::ReadStrings(line, values);
                m_map[key] = values;
            }
        }

        // expected blank line before EOF
        return false;
    }

    std::map<std::string, std::vector<std::string>> m_map;
};

class CsvColumns
{
public:

    bool Read(std::fstream & fs)
    {
        std::string line;
        if (!CsvHelper::ReadStrings(fs, m_names) ||
            !CsvHelper::ReadStrings(fs, m_descriptions) ||
            !CsvHelper::ReadStrings(fs, m_units) ||
            !CsvHelper::ReadStrings(fs, m_numbers) ||
            !CsvHelper::ReadLine(fs, line) ||
            line.length() != 1)
            return false;

        m_count = m_names.size();

        if (m_count != m_descriptions.size() ||
            m_count != m_units.size() ||
            m_count != m_numbers.size())
            return false;

        return true;
    }

    bool Find(const char * name, int & index)
    {
        for(unsigned int i = 0; i < m_names.size(); i++)
            if (m_names[i].compare(name) == 0) {
                index = i;
                return true;
            }

        return false;
    }

    int m_count;
    std::vector<std::string> m_names;
    std::vector<std::string> m_descriptions;
    std::vector<std::string> m_units;
    std::vector<std::string> m_numbers;

};

class CsvData {
public:

    bool Read(std::fstream & fs, int columnCount)
    {
        std::vector<double> values;
        m_data.resize(columnCount);

        while (CsvHelper::ReadDoubles(fs, values)) {

            if (values.size() != columnCount)
                return false;

            for (int i = 0; i < columnCount; i++)
                m_data[i].push_back(values[i]);

            values.clear();
        }

        return true;
    }

    std::vector<std::vector<double>> m_data;
};

class Csv {
public:

    bool Read(std::fstream & fs)
    {
        if (m_header.Read(fs) &&
            m_columns.Read(fs) &&
            m_data.Read(fs, m_columns.m_count))
            return true;

        return false;
    }

    CsvHeader m_header;
    CsvColumns m_columns;
    CsvData m_data;

};

