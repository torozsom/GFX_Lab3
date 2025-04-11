//
// Created by toron on 2025. 04. 11..
//

#include "Map.h"

#include <iostream>


std::vector<vec3> Map::decodeImage(const std::vector<unsigned char> &encodedData) {
    std::vector<vec3> pixels(4096);
    size_t pixelIndex = 0;
    vec3 colorPalette[4] = {
        vec3(1.0f, 1.0f, 1.0f),
        vec3(0.0f, 0.0f, 1.0f),
        vec3(0.0f, 1.0f, 0.0f),
        vec3(0.0f, 0.0f, 0.0f)
    };

    for (unsigned char encodedByte: encodedData) {
        int runLengthMinusOne = (encodedByte >> 2) & 0x3F;
        int colorIndex = encodedByte & 0x03;
        int runLength = runLengthMinusOne + 1;

        for (int i = 0; i < runLength && pixelIndex < 4096; ++i) {
            pixels[pixelIndex++] = colorPalette[colorIndex];
        }
    }

    while (pixelIndex < 4096) {
        pixels[pixelIndex++] = colorPalette[3];
    }

    return pixels;
}


Map::Map(const std::vector<unsigned char> &encodedData) {

    std::vector<vec3> pixels = decodeImage(encodedData);
    texture = new Texture(64, 64, pixels);


    texture->Bind(0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);


    std::vector<vec2> vertices = {
        vec2(-1.0f, -1.0f), vec2(1.0f, -1.0f), vec2(1.0f, 1.0f), vec2(-1.0f, 1.0f)
    };

    std::vector<vec2> texCoords = {
        vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(1.0f, 1.0f), vec2(0.0f, 1.0f)
    };


    for (const auto &vertex: vertices)
        Vtx().push_back(vertex);

    updateGPU();


    glGenBuffers(1, &vboTex);
    glBindBuffer(GL_ARRAY_BUFFER, vboTex);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(vec2), &texCoords[0], GL_STATIC_DRAW);


    Bind();
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glBindVertexArray(0);
}


void Map::DrawMap(GPUProgram *prog) {
    if (vtx.size() > 0) {
        prog->setUniform(true, "isTextured");
        texture->Bind(0);
        prog->setUniform(0, "tex");
        Bind();
        glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<int>(vtx.size()));
    }
}


Map::~Map() {
    delete texture;
    glDeleteBuffers(1, &vboTex);
}
