//
// Created by toron on 2025. 04. 11..
//

#ifndef MAP_H
#define MAP_H

#include "framework.h"
#include "Station.h"
#include "Path.h"


class Map final : public Geometry<vec2> {
    Texture *texture;
    unsigned int vboTex;

private:
    std::vector<vec3> decodeImage(const std::vector<unsigned char> &encodedData);

public:
    Map(const std::vector<unsigned char> &encodedData);

    void DrawMap(GPUProgram *prog);

    ~Map() override;
};


#endif //MAP_H
