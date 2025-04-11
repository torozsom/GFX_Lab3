//
// Created by toron on 2025. 04. 11..
//

#ifndef PATH_H
#define PATH_H

#include "Map.h"


vec2 geoToNormalizedMap(const vec2 &geo);

vec2 mapCoordinatesToGeographic(const vec2 &normalizedMap);

vec3 geoToCartesian(const vec2 &geo);

vec3 sphericalLinearInterpolation(const vec3 &startVector, const vec3 &endVector, float interpolationFactor);

vec2 cartesianToGeographic(const vec3 &cartesianCoordinates);


class Path final : public Geometry<vec2> {
public:
    Path(const vec2 &start, const vec2 &end);

    void DrawPath(GPUProgram *prog, vec3 color);
};


#endif //PATH_H
