#ifndef PATH_H
#define PATH_H

#include "Map.h"


vec2 geoToNormalizedMap(const vec2 &geo);

vec2 mapCoordinatesToGeographic(const vec2 &normalizedMap);

vec3 geoToCartesian(const vec2 &geo);

vec3 sphericalLinearInterpolation(const vec3 &startVector, const vec3 &endVector, float interpolationFactor);

vec2 cartesianToGeographic(const vec3 &cartesianCoordinates);


/**
 * @class Path
 * @brief A class representing a geographical path between two points, derived from the Geometry class.
 *
 * The Path class allows for creating a path between two geographical coordinates
 * and rendering it using a specific GPU program and color. Internally, the path
 * is represented as a series of points interpolated between the starting and ending
 * geographical coordinates.
 *
 * The class utilizes the `Geometry` class for its GPU vertex array and buffer management.
 */
class Path final : public Geometry<vec2> {
public:
    Path(const vec2 &start, const vec2 &end);

    void DrawPath(GPUProgram *prog, vec3 color);
};


#endif //PATH_H
