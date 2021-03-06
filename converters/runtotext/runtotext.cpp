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

	if (extension != NULL && strcmp(extension,".run")==0)
	{
		extension[1] = 't';
		extension[2] = 'x';
		extension[3] = 't';

		FILE * output_file = NULL;
        
        double start_finish_left_x = 0.0;
        double start_finish_left_y = 0.0;
        double start_finish_left_z = 0.0;
        double start_finish_right_x;
        double start_finish_right_y;
        double start_finish_right_z;

        calculate_xyz(kStartFinishLongitudeRight, kStartFinishLatitudeRight, 0.0,
            kStartFinishLongitudeLeft, kStartFinishLatitudeLeft, kStartFinishAltitudeLeft,
            &start_finish_right_x, &start_finish_right_y, &start_finish_right_z);

		Line2d start_finish(start_finish_left_x, start_finish_left_y, 
            start_finish_right_x, start_finish_right_y);

		if(fopen_s(&output_file, output_file_name, "w")==0)
		{
			c_channel_data_decoder data_decoder;

			data_decoder.begin(input_file_name);

			c_channel_data * channel_data = data_decoder.get_channel_data();

            double time = 0.0;
            double altitude = 0;        // meters
            double speed_mph = 0.0;
            int position_count = 0;
            double last_x = 0.0;
            double last_y = 0.0;
            double last_z = 0.0;
            double last_time;
            double long_accel = 0.0;
            double lat_accel = 0.0;
            double distance = 0.0;

			while(channel_data!=NULL)
			{
                channel_data->write_txt(output_file);

#if 0
                switch (channel_data->get_channel())
                {
                case e_channel_acceleration_data:
                    {
                        c_acceleration_data * acceleration = (c_acceleration_data *)channel_data;
                        long_accel = acceleration->m_longitudinal_acceleration;
                        lat_accel = acceleration->m_lateral_acceleration;
                        fprintf(output_file, "%f: long accel %lf lat accel %lf\n", time, long_accel, lat_accel);
                    }
                    break;
                case e_channel_gps_speed:
                    {   
                        c_gps_speed * gps_speed = (c_gps_speed *)channel_data;
                        speed_mph = gps_speed->m_gps_speed * (3.2808399 / 5280.0) * 3600.0;
                        fprintf(output_file, "%f: gps speed %lf\n", time, speed_mph);
                    }
                    break;
                case e_channel_gps_altitude:
                    {
                        c_gps_altitude * gps_altitude = (c_gps_altitude *)channel_data;
                        fprintf(output_file, "%f: alt %lf\n", time, gps_altitude->m_z);
                        altitude = gps_altitude->m_z / 1000.0;
                    }
                    break;
                case e_channel_gps_position:
                    {
                        c_gps_position * gps_position = (c_gps_position *)channel_data;

                        double x, y, z;

                        calculate_xyz(gps_position->m_longitude, gps_position->m_latitude, altitude,
                            kStartFinishLongitudeLeft, kStartFinishLatitudeLeft, kStartFinishAltitudeLeft,
                            &x, &y, &z);

                        if (position_count++ > 0)
                        {
                            double delta_x = (x - last_x);
                            double delta_y = (y - last_y);
                            double delta_z = (z - last_z);

                            double delta_distance = sqrt( (delta_x * delta_x) + (delta_y * delta_y) + (delta_z * delta_z));

                            Line2d track(last_x, last_y, x, y);
                            Point2d intersection_point;

                            if (track.Intersect(start_finish, intersection_point))
                            {
                                if (intersection_point.m_x >= std::min(last_x, x) &&
                                    intersection_point.m_y >= std::min(last_y, y) &&
                                    intersection_point.m_x <= std::max(last_x, x) &&
                                    intersection_point.m_y <= std::max(last_y, y) &&
                                    intersection_point.m_x >= std::min(start_finish_left_x, start_finish_right_x) &&
                                    intersection_point.m_y >= std::min(start_finish_left_y, start_finish_right_y) &&
                                    intersection_point.m_x <= std::max(start_finish_left_x, start_finish_right_x) &&
                                    intersection_point.m_y <= std::max(start_finish_left_y, start_finish_right_y))
                                {
                                    double delta_x = abs(last_x - x);
                                    double delta_y = abs(last_y - y);
                                    double delta_time = time - last_time;
                                    double lerp;
                                    if ( delta_x > delta_y )
                                    {
                                        lerp = abs(intersection_point.m_x - last_x) / delta_x;
                                    }
                                    else
                                    {
                                        lerp = abs(intersection_point.m_y - last_y) / delta_y;
                                    }

                                    double cross_time = last_time + (delta_time * lerp);

                                    fprintf(output_file, "cross start_finish %f (%lf, %lf)\n",
                                        cross_time,
                                        intersection_point.m_x, intersection_point.m_y);

                                    distance = 0.0;
                                    delta_distance *= (1.0 - lerp);
                                }
                            }

                            distance += delta_distance;
                        }

                        fprintf(output_file, "%f: dist %lf pos (%lf, %lf, %lf)\n", time, distance, x, y, z);

                        last_x = x;
                        last_y = y;
                        last_z = z;
                        last_time = time;
                    }
                    break;

                case e_channel_time_stamp:
                    {
                        c_time_stamp * time_stamp = (c_time_stamp *) channel_data;
                        time = time_stamp->m_time_stamp;
                    }
                    break;
                default:
                    // do nothing
                    break;
                }
#endif

                delete channel_data;
				channel_data = data_decoder.get_channel_data();
			}

			data_decoder.end();

			fclose(output_file);
		}
	}
}

int main(int argc, char * argv[])
{
    process_file("test.run");
    return 0;
}

