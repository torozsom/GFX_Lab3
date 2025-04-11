//
// Created by toron on 2025. 04. 11..
//

#ifndef STATION_H
#define STATION_H


#include "Map.h"

/**
 * @class Station
 * @brief Represents a station on a map, inheriting from Geometry<vec2>.
 *
 * The Station class provides functionality for representing and rendering a station
 * on a map. It manages its position and rendering capabilities using OpenGL APIs.
 */
class Station final : public Geometry<vec2> {
public:
    Station(const vec2 &pos);

    void DrawStation(GPUProgram *prog, vec3 color);
};


#endif //STATION_H
