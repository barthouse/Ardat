#include <SDKDDKVer.h>
#include <stdio.h>
#include <tchar.h>

#include "csv.h"
#include "gps.h"


class ChannelData
{
public:

    double m_time;
};

class GpsChannelData : public ChannelData
{
public:

    double m_elevation;
    double m_longitude;
    double m_latitude;
    double m_x;
    double m_y;
    double m_z;
    double m_speed;
    double m_distance;
};

class Channel {
public:

    Channel(const char * name) :
        m_name(name)
    {
        // do nothing
    }

    void RemoveDuplicateData()
    {
        size_t sampleCount = m_data.size();

        if (sampleCount == 0)
            return;

        std::vector<ChannelData *> data;

        data.resize(sampleCount);

        size_t src = 0, dst = 0;
        data[dst++] = m_data[src++];

        while (src < sampleCount) {
            if (m_data[src]->m_time == m_data[src - 1]->m_time) {
                delete m_data[src];
                src++;
                continue;
            }
            data[dst++] = m_data[src++];
        }

        data.resize(dst);

        m_data = std::move(data);
    }

    std::string m_name;
    std::string m_unit;
    std::vector<ChannelData *> m_data;
};

class Ardat {
public:

    Ardat() : m_channel("Gps")
    {
        // do nothing
    }

    bool Import(Csv & csv)
    {
        std::string formatKey("Format");

        if (csv.m_header.m_map.count(formatKey) != 1)
            return false;

        std::string format = csv.m_header.m_map[formatKey][0];

        if (format.compare("AIM CSV File") != 0)
            return false;

        // Find the columns we exepect

        std::vector<std::vector<double>> & data = csv.m_data.m_data;

        int timeColumn;
        
        if (!csv.m_columns.Find("Time", timeColumn))
            return false;

        std::vector<double> & lapTimeData = data[timeColumn];

        int gpsElevationColumn;

        if (!csv.m_columns.Find("GPS_Elevation", gpsElevationColumn))
            return false;

        std::vector<double> & gpsElevationData = data[gpsElevationColumn];

        int gpsLatitudeColumn;

        if (!csv.m_columns.Find("GPS_Latitude", gpsLatitudeColumn))
            return false;
        
        std::vector<double> & gpsLatitudeData = data[gpsLatitudeColumn];

        int gpsLongitudeColumn;

        if (!csv.m_columns.Find("GPS_Longitude", gpsLongitudeColumn))
            return false;

        std::vector<double> & gpsLongitudeData = data[gpsLongitudeColumn];

        int gpsSpeedColumn;

        if (!csv.m_columns.Find("GPS_Speed", gpsSpeedColumn))
            return false;

        std::vector<double> & gpsSpeedData = data[gpsSpeedColumn];

        // Verify sample count is uniform across all data sources

        int sampleCount = lapTimeData.size();

        if (sampleCount != gpsElevationData.size() ||
            sampleCount != gpsLatitudeData.size() ||
            sampleCount != gpsLongitudeData.size())
            return false;

        // Build time data
        
        std::vector<double> timeData;

        timeData.resize(lapTimeData.size());

        double timeOffset = 0.0;
        for (size_t i = 0; i < lapTimeData.size(); i++) {
            if (i > 0 && lapTimeData[i] == 0.0f)
                timeOffset += lapTimeData[i - 1];

            timeData[i] = lapTimeData[i] + timeOffset;
        }

        // Build position data

        std::vector<double> xData;
        std::vector<double> yData;
        std::vector<double> zData;

        xData.resize(sampleCount);
        yData.resize(sampleCount);
        zData.resize(sampleCount);

        const double kStartFinishLatitudeLeft = 45.594780;
        const double kStartFinishLongitudeLeft = -122.694674;
        const double kStartFinishElevationLeft = 0.0;

        const double kStartFinishLatitudeRight = 45.595319;
        const double kStartFinishLongitudeRight = -122.694370;
        const double kStartFinishElevationRight = 0.0;

        double x, y, z;
        for (int i = 0; i < sampleCount; i++) {
            GpsUtils::GeodeticToEnu(gpsLatitudeData[i], gpsLongitudeData[i], gpsElevationData[i],
                kStartFinishLatitudeLeft, kStartFinishLongitudeLeft, kStartFinishElevationLeft,
                x, y, z);

            xData[i] = x;
            yData[i] = y;
            zData[i] = z;
        }

        // Build distance data

        std::vector<double> gpsDistanceData;

        gpsDistanceData.resize(sampleCount);

        double distance = 0.0;
        for (int i = 0; i < sampleCount; i++) {
            double dx = (i > 0 ? xData[i] - xData[i-1] : 0.0);
            double dy = (i > 0 ? yData[i] - yData[i-1] : 0.0);
            double dz = (i > 0 ? zData[i] - zData[i-1] : 0.0);
            double delta = sqrt((dx * dx) + (dy * dy) + (dz * dz));
            distance += delta;

            gpsDistanceData[i] = distance;
        }

        // Build channel

        m_channel.m_data.resize(sampleCount);

        for (int i = 0; i < sampleCount; i++) {
            GpsChannelData * data = new GpsChannelData();

            data->m_time = timeData[i];
            data->m_elevation = gpsElevationData[i];
            data->m_longitude = gpsLongitudeData[i];
            data->m_latitude = gpsLatitudeData[i];
            data->m_x = xData[i];
            data->m_y = yData[i];
            data->m_z = zData[i];
            data->m_speed = gpsSpeedData[i];
            data->m_distance = gpsDistanceData[i];

            m_channel.m_data[i] = data;
        }

        // Remove dupliate data

         m_channel.RemoveDuplicateData();

        return true;
    }

    Channel m_channel;
};

int main(int argc, char **argv)
{
    if (argc != 2) {
        std::cout << "ex: aim_to_ardat <aim-csv-file>";
        exit(1);
    }

    std::fstream fs;

    fs.open(argv[1], std::ios_base::in);

    if (!fs) {
        std::cout << "couldn't open'" << argv[1] << "'for reading.";
        exit(1);
    }

    Csv csv;
    if (!csv.Read(fs)) {
        std::cout << "Failed to read CSV data.\n";
        exit(1);
    }

    Ardat ardat;

    if (!ardat.Import(csv)) {
        std::cout << "Failed to import CSV into ARDAT.\n";
        exit(1);
    }

    auto & data = ardat.m_channel.m_data;
    for (size_t i = 0; i < data.size(); i++) {
        GpsChannelData * gpsData = (GpsChannelData *) data[i];
        std::cout << gpsData->m_time << ":" << gpsData->m_x << "," << gpsData->m_y << "\n";
    }

    std::cout << "Done.\n";

    return 0;
}

