//
// Created by toron on 2025. 04. 11..
//

#include "Station.h"


/**
 * Constructor for the Station class. Initializes a station with a given position
 * in geographic coordinates, converts it to a normalized map position, and updates
 * the GPU with the new vertex data.
 *
 * @param pos The geographic position (latitude and longitude) of the station.
 *            Represented as a 2D vector (vec2).
 */
Station::Station(const vec2 &pos) {
    Vtx().push_back(geoToNormalizedMap(pos));
    updateGPU();
}


/**
 * Draws the station on the screen. This method sets up the GPU program with the
 * specified color and rendering properties, adjusts the point size for visualization,
 * and renders the station based on its vertex data.
 *
 * @param prog The GPU program used to set uniforms and manage rendering shaders.
 * @param color The color used to display the station, represented as a 3D vector (vec3).
 */
void Station::DrawStation(GPUProgram *prog, vec3 color) {
    if (vtx.size() > 0) {
        prog->setUniform(color, "color");
        prog->setUniform(false, "isTextured");
        glPointSize(10.0f);
        Bind();
        glDrawArrays(GL_POINTS, 0, static_cast<int>(vtx.size()));
    }
}
