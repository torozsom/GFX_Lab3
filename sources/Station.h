//
// Created by toron on 2025. 04. 11..
//

#ifndef STATION_H
#define STATION_H


#include "Map.h"

class Station final : public Geometry<vec2> {
public:
    Station(const vec2 &pos);

    void DrawStation(GPUProgram *prog, vec3 color);
};


#endif //STATION_H
