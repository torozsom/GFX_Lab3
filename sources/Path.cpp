//
// Created by toron on 2025. 04. 11..
//

#include "Path.h"


/**
 * Converts geographic coordinates (latitude and longitude) to normalized map coordinates.
 *
 * This function maps geographic coordinates from the range of latitude [-85, 85] degrees
 * and longitude [-180, 180] degrees to normalized coordinates where:
 * - Latitude is scaled to [-1, 1] using a Mercator projection.
 * - Longitude is scaled to [-1, 1] based on its range.
 *
 * @param geo A vec2 object where `geo.x` represents latitude in degrees and `geo.y`
 *            represents longitude in degrees.
 *
 * @return A vec2 object representing the normalized map coordinates, where `x` is
 *         the normalized longitude and `y` is the normalized latitude.
 */
vec2 geoToNormalizedMap(const vec2 &geo) {
    float latitudeMinRad = -85.0f * M_PI / 180.0f;
    float latitudeMaxRad = 85.0f * M_PI / 180.0f;
    float latitudeRad = geo.x * M_PI / 180.0f;

    float mercatorProjectionY = logf(tanf(latitudeRad) + 1.0f / cosf(latitudeRad));
    float mercatorProjectionMinY = logf(tanf(latitudeMinRad) + 1.0f / cosf(latitudeMinRad));
    float mercatorProjectionMaxY = logf(tanf(latitudeMaxRad) + 1.0f / cosf(latitudeMaxRad));

    float normalizedMapY = -1.0f + 2.0f * (mercatorProjectionY - mercatorProjectionMinY) /
                           (mercatorProjectionMaxY - mercatorProjectionMinY);
    float normalizedMapX = geo.y / 180.0f;

    return vec2(normalizedMapX, normalizedMapY);
}


/**
 * Converts normalized map coordinates to geographic coordinates (latitude and longitude).
 *
 * This function maps normalized coordinates where:
 * - The normalized x-coordinate (-1 to 1) represents longitude scaled from [-180, 180] degrees.
 * - The normalized y-coordinate (-1 to 1) represents latitude scaled to fit within the
 *   Mercator projection range for latitude [-85, 85] degrees.
 *
 * @param normalizedMap A vec2 object where `normalizedMap.x` represents the normalized
 *                      longitude and `normalizedMap.y` represents the normalized latitude.
 *
 * @return A vec2 object representing geographic coordinates:
 *         - `x` is the latitude in degrees.
 *         - `y` is the longitude in degrees.
 */
vec2 mapCoordinatesToGeographic(const vec2 &normalizedMap) {
    float longitude = normalizedMap.x * 180.0f;

    float latitudeMinRadians = -85.0f * M_PI / 180.0f;
    float latitudeMaxRadians = 85.0f * M_PI / 180.0f;

    float yMin = logf(tanf(latitudeMinRadians) + 1.0f / cosf(latitudeMinRadians));
    float yMax = logf(tanf(latitudeMaxRadians) + 1.0f / cosf(latitudeMaxRadians));
    float y = yMin + (normalizedMap.y + 1.0f) / 2.0f * (yMax - yMin);

    float latitudeRadians = atanf(sinhf(y));
    float latitude = latitudeRadians * 180.0f / M_PI;

    return vec2(latitude, longitude);
}


/**
 * Converts geographic coordinates (latitude and longitude) to Cartesian coordinates.
 *
 * This function maps geographic coordinates, where:
 * - Latitude is in degrees, ranging from -90 to 90.
 * - Longitude is in degrees, ranging from -180 to 180.
 *
 * These coordinates are transformed into a 3D Cartesian vector (x, y, z) based on a spherical
 * representation of the Earth.
 *
 * @param geo A vec2 object where `geo.x` represents latitude in degrees and `geo.y`
 *            represents longitude in degrees.
 *
 * @return A vec3 object representing the Cartesian coordinates, where `x`, `y`, and `z`
 *         correspond to the computed 3D positions.
 */
vec3 geoToCartesian(const vec2 &geo) {
    float latitudeRad = geo.x * M_PI / 180.0f;
    float longitudeRad = geo.y * M_PI / 180.0f;

    return vec3(cosf(latitudeRad) * cosf(longitudeRad),
                cosf(latitudeRad) * sinf(longitudeRad),
                sinf(latitudeRad));
}


/**
 * Performs spherical linear interpolation (SLERP) between two vectors.
 *
 * This function computes a smooth interpolation between two 3D vectors over the surface
 * of a sphere. It is useful for animations, rotations, and other applications where
 * interpolation over a spherical surface is required. If the vectors are nearly
 * identical, linear interpolation is used as an approximation.
 *
 * @param startVector The starting 3D vector for the interpolation.
 * @param endVector The ending 3D vector for the interpolation.
 * @param interpolationFactor A factor in the range [0, 1] determining the
 *                            progression of interpolation:
 *                            - 0 corresponds to the startVector.
 *                            - 1 corresponds to the endVector.
 *
 * @return A 3D vector representing the interpolated result based on the spherical
 *         interpolation.
 */
vec3 sphericalLinearInterpolation(const vec3 &startVector, const vec3 &endVector, float interpolationFactor) {
    float dotProduct = dot(startVector, endVector);

    // If the vectors are almost identical, use linear interpolation
    if (dotProduct > 0.9995f)
        return normalize(mix(startVector, endVector, interpolationFactor));

    float angle = acosf(dotProduct);
    float sinAngle = sinf(angle);

    return (sinf((1.0f - interpolationFactor) * angle) / sinAngle) * startVector +
           (sinf(interpolationFactor * angle) / sinAngle) * endVector;
}


/**
 * Converts Cartesian 3D coordinates to geographic coordinates (latitude and longitude).
 *
 * This function calculates the geographic latitude and longitude in degrees from a
 * given 3D Cartesian coordinate vector.
 *
 * @param cartesianCoordinates A vec3 object representing the Cartesian coordinates
 *                             where `x`, `y`, and `z` correspond to the respective Cartesian axes.
 *
 * @return A vec2 object containing the geographic coordinates:
 *         - `x` represents the latitude in degrees.
 *         - `y` represents the longitude in degrees.
 */
vec2 cartesianToGeographic(const vec3 &cartesianCoordinates) {
    float latitude = asinf(cartesianCoordinates.z) * 180.0f / M_PI; // Calculate latitude in degrees
    float longitude = atan2f(cartesianCoordinates.y, cartesianCoordinates.x) * 180.0f / M_PI;
    // Calculate longitude in degrees
    return vec2(latitude, longitude); // Return the geographic coordinates
}


/**
 * Constructs a Path object connecting two geographic coordinates.
 *
 * This function generates a path between two points specified by their geographic
 * coordinates (latitude and longitude). The path consists of interpolated points
 * between the start and end coordinates, which are converted and stored as normalized
 * map coordinates. The object's vertex buffer is updated to include these points.
 *
 * @param start A vec2 object representing the starting point of the path, where
 *              `start.x` is the latitude in degrees and `start.y` is the longitude in degrees.
 * @param end   A vec2 object representing the ending point of the path, where
 *              `end.x` is the latitude in degrees and `end.y` is the longitude in degrees.
 */
Path::Path(const vec2 &start, const vec2 &end) {
    vec3 startCart = geoToCartesian(start);
    vec3 endCart = geoToCartesian(end);
    const int numPoints = 100;

    for (int i = 0; i <= numPoints; ++i) {
        float t = static_cast<float>(i) / numPoints;
        vec3 interpCart = sphericalLinearInterpolation(startCart, endCart, t);
        vec2 interpGeo = cartesianToGeographic(interpCart);
        Vtx().push_back(geoToNormalizedMap(interpGeo));
    }
    updateGPU();
}


/**
 * Renders a path using a GPU program with the specified color.
 *
 * This method draws a series of connected line segments representing the path
 * on the current OpenGL context. It uses the provided GPU program to set uniforms,
 * such as the color of the path and whether it is textured. The method ensures drawing
 * with a specific line width and activates the necessary vertex arrays and buffers.
 *
 * @param prog The GPUProgram instance used to set uniforms and render the path.
 * @param color A vec3 object specifying the RGB color of the path.
 */
void Path::DrawPath(GPUProgram *prog, vec3 color) {
    if (vtx.size() > 0) {
        prog->setUniform(color, "color"); // Szín beállítása (pl. sárga)
        prog->setUniform(false, "isTextured"); // Nem textúrázott rajzolás
        glLineWidth(3.0f); // Vonalvastagság 3 pixel
        Bind(); // VAO és VBO aktiválása
        glDrawArrays(GL_LINE_STRIP, 0, static_cast<int>(vtx.size())); // Vonal rajzolása
    }
}
