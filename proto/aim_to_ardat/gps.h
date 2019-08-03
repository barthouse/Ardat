#pragma once

#include <stdio.h>
#include <string.h>

#define _USE_MATH_DEFINES
#include <math.h>

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
    // 
    // height in meters
    // east, north and up in meters
    static void GeodeticToEnu(double lat, double lon, double height,
                                        double lat0, double lon0, double height0,
                                        double & xEast, double & yNorth, double & zUp)
    {
        double x, y, z;
        GeodeticToEcef(lat, lon, height, x, y, z);
        EcefToEnu(x, y, z, lat0, lon0, height0, xEast, yNorth, zUp);
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
