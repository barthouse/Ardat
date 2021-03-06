#include <stdio.h>
#include <string.h>
#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

#include "..\common\channel_data.h"
#include "..\common\math_util.h"

#define kStartFinishLatitudeLeft 45.594780
#define kStartFinishLongitudeLeft -122.694674
#define kStartFinishAltitudeLeft 0.0

#define kStartFinishLatitudeRight 45.595319
#define kStartFinishLongitudeRight -122.694370
#define kStartFinishAltitudeRight 0.0


// Some helpers for converting GPS readings from the WGS84 geodetic system to a local North-East-Up cartesian axis.

// The implementation here is according to the paper:
// "Conversion of Geodetic coordinates to the Local Tangent Plane" Version 2.01.
// "The basic reference for this paper is J.Farrell & M.Barth 'The Global Positioning System & Inertial Navigation'"
// Also helpful is Wikipedia: http://en.wikipedia.org/wiki/Geodetic_datum
// Also helpful are the guidance notes here: http://www.epsg.org/Guidancenotes.aspx
class GpsUtils
{
public:

    // WGS-84 geodetic constants
    static const double a;         // WGS-84 Earth semimajor axis (m)

    static const double b;     // Derived Earth semiminor axis (m)
    static const double f;           // Ellipsoid Flatness
    static const double f_inv;       // Inverse flattening

    static const double a_sq;
    static const double b_sq;
    static const double e_sq;    // Square of Eccentricity

    // Converts WGS-84 Geodetic point (lat, lon, h) to the 
    // Earth-Centered Earth-Fixed (ECEF) coordinates (x, y, z).
    static void GeodeticToEcef(double lat, double lon, double h,
                               double & x, double & y, double & z)
    {
        // Convert to radians in notation consistent with the paper:
        double lambda = DegreesToRadians(lat);
        double phi = DegreesToRadians(lon);
        double s = sin(lambda);
        double N = a / sqrt(1 - e_sq * s * s);

        double sin_lambda = sin(lambda);
        double cos_lambda = cos(lambda);
        double cos_phi = cos(phi);
        double sin_phi = sin(phi);

        x = (h + N) * cos_lambda * cos_phi;
        y = (h + N) * cos_lambda * sin_phi;
        z = (h + (1 - e_sq) * N) * sin_lambda;
    }

    // Converts the Earth-Centered Earth-Fixed (ECEF) coordinates (x, y, z) to 
    // (WGS-84) Geodetic point (lat, lon, h).
    static void EcefToGeodetic(double x, double y, double z,
                               double & lat, double & lon, double & h)
    {
        double eps = e_sq / (1.0 - e_sq);
        double p = sqrt(x * x + y * y);
        double q = atan2((z * a), (p * b));
        double sin_q = sin(q);
        double cos_q = cos(q);
        double sin_q_3 = sin_q * sin_q * sin_q;
        double cos_q_3 = cos_q * cos_q * cos_q;
        double phi = atan2((z + eps * b * sin_q_3), (p - e_sq * a * cos_q_3));
        double lambda = atan2(y, x);
        double v = a / sqrt(1.0 - e_sq * sin(phi) * sin(phi));
        h = (p / cos(phi)) - v;

        lat = RadiansToDegrees(phi);
        lon = RadiansToDegrees(lambda);
    }

    // Converts the Earth-Centered Earth-Fixed (ECEF) coordinates (x, y, z) to 
    // East-North-Up coordinates in a Local Tangent Plane that is centered at the 
    // (WGS-84) Geodetic point (lat0, lon0, h0).
    static void EcefToEnu(double x, double y, double z,
                                    double lat0, double lon0, double h0,
                                    double & xEast, double & yNorth, double & zUp)
    {
        // Convert to radians in notation consistent with the paper:
        double lambda = DegreesToRadians(lat0);
        double phi = DegreesToRadians(lon0);
        double s = sin(lambda);
        double N = a / sqrt(1 - e_sq * s * s);

        double sin_lambda = sin(lambda);
        double cos_lambda = cos(lambda);
        double cos_phi = cos(phi);
        double sin_phi = sin(phi);

        double x0 = (h0 + N) * cos_lambda * cos_phi;
        double y0 = (h0 + N) * cos_lambda * sin_phi;
        double z0 = (h0 + (1 - e_sq) * N) * sin_lambda;

        double xd, yd, zd;
        xd = x - x0;
        yd = y - y0;
        zd = z - z0;

        // This is the matrix multiplication
        xEast = -sin_phi * xd + cos_phi * yd;
        yNorth = -cos_phi * sin_lambda * xd - sin_lambda * sin_phi * yd + cos_lambda * zd;
        zUp = cos_lambda * cos_phi * xd + cos_lambda * sin_phi * yd + sin_lambda * zd;
    }

    // Inverse of EcefToEnu. Converts East-North-Up coordinates (xEast, yNorth, zUp) in a
    // Local Tangent Plane that is centered at the (WGS-84) Geodetic point (lat0, lon0, h0)
    // to the Earth-Centered Earth-Fixed (ECEF) coordinates (x, y, z).
    static void EnuToEcef(double xEast, double yNorth, double zUp,
                                    double lat0, double lon0, double h0,
                                    double & x, double & y, double & z)
    {
        // Convert to radians in notation consistent with the paper:
        double lambda = DegreesToRadians(lat0);
        double phi = DegreesToRadians(lon0);
        double s = sin(lambda);
        double N = a / sqrt(1 - e_sq * s * s);

        double sin_lambda = sin(lambda);
        double cos_lambda = cos(lambda);
        double cos_phi = cos(phi);
        double sin_phi = sin(phi);

        double x0 = (h0 + N) * cos_lambda * cos_phi;
        double y0 = (h0 + N) * cos_lambda * sin_phi;
        double z0 = (h0 + (1 - e_sq) * N) * sin_lambda;

        double xd = -sin_phi * xEast - cos_phi * sin_lambda * yNorth + cos_lambda * cos_phi * zUp;
        double yd = cos_phi * xEast - sin_lambda * sin_phi * yNorth + cos_lambda * sin_phi * zUp;
        double zd = cos_lambda * yNorth + sin_lambda * zUp;

        x = xd + x0;
        y = yd + y0;
        z = zd + z0;
    }

    // Converts the geodetic WGS-84 coordinated (lat, lon, h) to 
    // East-North-Up coordinates in a Local Tangent Plane that is centered at the 
    // (WGS-84) Geodetic point (lat0, lon0, h0).
    static void GeodeticToEnu(double lat, double lon, double h,
                                        double lat0, double lon0, double h0,
                                        double & xEast, double & yNorth, double & zUp)
    {
        double x, y, z;
        GeodeticToEcef(lat, lon, h, x, y, z);
        EcefToEnu(x, y, z, lat0, lon0, h0, xEast, yNorth, zUp);
    }

    static bool AreClose(double x0, double x1)
    {
        double d = x1 - x0;
        return (d * d) < 0.1;
    }


    static double DegreesToRadians(double degrees)
    {
        return M_PI / 180.0 * degrees;
    }

    static double RadiansToDegrees(double radians)
    {
        return 180.0 / M_PI * radians;
    }
};

// WGS-84 geodetic constants
const double GpsUtils::a = 6378137.0;         // WGS-84 Earth semimajor axis (m)
const double GpsUtils::b = 6356752.314245;     // Derived Earth semiminor axis (m)
const double GpsUtils::f = (GpsUtils::a - GpsUtils::b) / GpsUtils::a;           // Ellipsoid Flatness
const double GpsUtils::f_inv = 1.0 / GpsUtils::f;       // Inverse flattening

const double GpsUtils::a_sq = GpsUtils::a * GpsUtils::a;
const double GpsUtils::b_sq = GpsUtils::b * GpsUtils::b;
const double GpsUtils::e_sq = GpsUtils::f * (2 - GpsUtils::f);    // Square of Eccentricity

void calculate_xyz(double longitude, double latitude, double altitude, 
                   double center_longitude, double center_latitude, double center_altitude,
                   double * x, double *y, double * z)
{
#if 0
    // https://stackoverflow.com/questions/16266809/convert-from-latitude-longitude-to-x-y
    // https://en.wikipedia.org/wiki/Equirectangular_projection
    double radius = 3959;   // miles
    double kDegreesToRadians = 2 * M_PI / 360.0;
    double longitude_delta = longitude - center_longitude;
    double latitude_delta = latitude - center_latitude;

    *x = radius * sin(longitude_delta * kDegreesToRadians) * cos(center_latitude * kDegreesToRadians);
    *y = radius * sin(latitude_delta * kDegreesToRadians);   
#else
    double east, north, height;
    GpsUtils::GeodeticToEnu(latitude, longitude, altitude, 
        kStartFinishLatitudeLeft, kStartFinishLongitudeLeft, kStartFinishAltitudeLeft,
        east, north, height);
    double feetPerMeter = 3.2808399;
    double milesPerFoot = 1.0f / 5280;

    *x = east * feetPerMeter * milesPerFoot;
    *y = north * feetPerMeter * milesPerFoot;
    *z = height * feetPerMeter * milesPerFoot;

#endif
}

void process_file(const char * input_file_name)
{
	char * output_file_name = _strdup(input_file_name);
	char * extension = strrchr(output_file_name, '.');

	if (extension != NULL && strcmp(extension,".txt")==0)
	{
		extension[1] = 'r';
		extension[2] = 'u';
		extension[3] = 'n';

		FILE * output_file = NULL;
       
		if(fopen_s(&output_file, output_file_name, "wb")==0)
		{
			c_txt_decoder txt_decoder;

			txt_decoder.begin(input_file_name);

            uint8_t header[8] = { 0x98, 0x1d, 0, 0, 0xc8, 0, 0, 0 };

            fwrite(header, 1, sizeof(header), output_file);

			c_channel_data * channel_data = txt_decoder.get_channel_data();

			while(channel_data!=NULL)
			{
                channel_data->write_run(output_file);

                delete channel_data;
				channel_data = txt_decoder.get_channel_data();
			}

			txt_decoder.end();

			fclose(output_file);
		}
	}
}

int main(int argc, char * argv[])
{
    process_file("test.txt");
    return 0;
}

