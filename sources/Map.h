#ifndef MAP_H
#define MAP_H

#include "framework.h"
#include "Station.h"
#include "Path.h"


/**
 * @class Map
 * @brief Represents a 2D textured map using OpenGL geometry.
 *
 * This class is responsible for rendering a map using a texture. The map's
 * texture is loaded and configured from encoded image data, and the class
 * handles OpenGL setup for rendering the map on the GPU. It inherits from
 * the Geometry template class with vec2 as the primary type.
 *
 * The class manages OpenGL resources such as vertex array objects (VAOs),
 * vertex buffer objects (VBOs), and textures, ensuring proper initialization
 * and cleanup.
 */
class Map final : public Geometry<vec2> {
    Texture *texture;
    unsigned int vboPos;
    unsigned int vboTex;
    unsigned int mapVao;

private:
    std::vector<vec3> decodeImage(const std::vector<unsigned char> &encodedData);

public:
    Map(const std::vector<unsigned char> &encodedData);

    void DrawMap(GPUProgram *prog) const;

    ~Map() override;
};


#endif //MAP_H
