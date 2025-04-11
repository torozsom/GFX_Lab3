#include "Map.h"

#include <iostream>


/**
 * Decodes a run-length encoded image and generates a vector of pixel colors.
 *
 * The method interprets the input `encodedData` as a sequence of run-length
 * encoded bytes, where each byte specifies a run length and a corresponding
 * color index from a predefined palette. It produces a 64x64 (4096 pixels)
 * output image, with any remaining pixels at the end being filled with the
 * default color (black).
 *
 * @param encodedData A vector of unsigned char data representing run-length
 *                    encoded image information. Each byte encodes a run length
 *                    and a color index.
 *
 * @return A vector of vec3 objects representing the RGB color values of each
 *         pixel in the decoded 64x64 image.
 */
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


/**
 * Constructs a Map object, initializes and sets up the texture,
 * vertex data, and OpenGL buffers for rendering.
 *
 * The constructor decodes the provided run-length encoded image data into
 * pixel colors, creates a texture with the decoded data, and configures
 * OpenGL settings for rendering. It also initializes vertex and texture
 * coordinate buffers and their respective vertex attributes.
 *
 * @param encodedData A vector containing run-length encoded image data,
 *                    where each byte represents a run length and a
 *                    corresponding color index.
 */
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

    glGenVertexArrays(1, &mapVao);
    glBindVertexArray(mapVao);

    glGenBuffers(1, &vboPos);
    glBindBuffer(GL_ARRAY_BUFFER, vboPos);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vec2), &vertices[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glGenBuffers(1, &vboTex);
    glBindBuffer(GL_ARRAY_BUFFER, vboTex);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(vec2), &texCoords[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

    glBindVertexArray(0);
}


/**
 * Renders the map using the provided GPUProgram instance.
 *
 * This method binds the associated texture, configures necessary uniforms,
 * and draws the map geometry as a triangle fan on the GPU. The map will only
 * be rendered if its vertex data is populated.
 *
 * @param prog A pointer to the GPUProgram currently in use for rendering,
 *             which supplies the required shaders and allows uniform settings.
 */
void Map::DrawMap(GPUProgram *prog) const {
    if (vtx.size() > 0) {
        prog->setUniform(true, "isTextured");
        texture->Bind(0);
        prog->setUniform(0, "tex");
        glBindVertexArray(mapVao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<int>(vtx.size()));
        glBindVertexArray(0);
    }
}


/**
 * Destructor for the `Map` class.
 *
 * This method is responsible for cleaning up GPU resources and memory
 * associated with a `Map` object. It deletes the texture object,
 * vertex buffer objects (VBOs), and the vertex array object (VAO) that
 * were created during the lifetime of the `Map` object.
 *
 * Specifically:
 * - Deletes the associated `Texture` object to release texture memory.
 * - Deletes the position buffer (vboPos) and the texture coordinate buffer (vboTex).
 * - Deletes the vertex array object (mapVao) to release the associated OpenGL resources.
 */
Map::~Map() {
    delete texture;
    glDeleteBuffers(1, &vboPos);
    glDeleteBuffers(1, &vboTex);
    glDeleteVertexArrays(1, &mapVao);
}
