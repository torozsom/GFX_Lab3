#include <iostream>

#include "Map.h"
#include <vector>


/**
 * The GLSL vertex shader source code used to define the vertex processing stage
 * in the rendering pipeline. This shader primarily handles the transformation of
 * vertex positions into clip space and passes texture coordinates to the fragment
 * shader for further processing.
 *
 * The shader performs the following operations:
 * - Position Transformation: Converts the input vertex position from object space
 *   to clip space using a predefined layout (location 0) for vertex attributes.
 *   The position is represented as a 2D `vec2`, with z and w components set to 0.0
 *   and 1.0, respectively, for compatibility with 3D rendering pipelines.
 * - Texture Coordinate Passing: Accepts 2D texture coordinates as input (layout location 1)
 *   and passes them to the fragment shader via the `vTexCoord` varying. This enables
 *   texturing in subsequent stages of the rendering pipeline.
 *
 * Inputs:
 * - position: A 2D vector (vec2) representing the vertex's position in object space,
 *   associated with layout location 0.
 * - texCoord: A 2D vector (vec2) representing the texture coordinate of the vertex,
 *   associated with layout location 1.
 *
 * Outputs:
 * - vTexCoord: A 2D vector (vec2) passed to the fragment shader, containing the
 *   interpolated texture coordinates for each fragment.
 */
const char *vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 position;
layout(location = 1) in vec2 texCoord;

out vec2 vTexCoord;

void main() {
    gl_Position = vec4(position, 0.0, 1.0);
    vTexCoord = texCoord;
}
)";


/**
 * The GLSL fragment shader source code used to render a textured or colored fragment,
 * simulating lighting effects based on solar illumination on a geographical coordinate
 * system. If a texture is applied using `tex`, the shader uses texture coordinates
 * to approximate geographic coordinates and compute lighting for day and night cycles.
 * Without a texture, the fragment color is set to a uniform color.
 *
 * The shader supports the following functionalities:
 * - Texturing: If `isTextured` is true, the shader samples a color from the provided
 *   texture (`tex`) using the texture coordinates (`vTexCoord`).
 * - Lighting: Computes solar illumination by calculating an angle between the
 *   fragment's surface normal and the sun direction, determined by its geographic
 *   position and a configurable offset (`hourOffset`). Simulates day and night
 *   conditions by dimming fragments in shadow.
 * - Geographic calculations: The shader internally calculates conversions between
 *   geographic coordinates (latitude and longitude) and Cartesian coordinates for
 *   lighting computations based on assumptions of Earth's tilt and solar position.
 *
 * Constants:
 * - PI: The mathematical constant for π (3.14159265359), used for radian conversions.
 * - earthTiltDeg: Earth's axial tilt in degrees (23.0°), used to represent the sun's
 *   latitude at summer solstice.
 *
 * Inputs:
 * - vTexCoord: 2D texture coordinates input from the vertex shader.
 *
 * Outputs:
 * - fragColor: The resulting RGBA color of the fragment.
 *
 * Uniforms:
 * - tex: Sampler used to fetch texels from the texture when `isTextured` is true.
 * - isTextured: Boolean indicating whether the shader operates in texturing mode.
 * - color: Uniform RGB color used when `isTextured` is false.
 * - hourOffset: Offset in hours, representing the sun's longitudinal location based on
 *   the given time zone (used for lighting calculations).
 */
const char *fragmentShaderSource = R"(
#version 330 core
in vec2 vTexCoord;
out vec4 fragColor;

uniform sampler2D tex;
uniform bool isTextured;
uniform vec3 color;
uniform float hourOffset;

const float PI = 3.14159265359;
const float earthTiltDeg = 23.0;

vec3 geoToCartesian(float lat, float lon) {
    float latRad = radians(lat);
    float lonRad = radians(lon);
    return vec3(
        cos(latRad) * cos(lonRad),
        cos(latRad) * sin(lonRad),
        sin(latRad)
    );
}

void main() {
    if (isTextured) {
        vec3 texColor = texture(tex, vTexCoord).rgb;

        // Convert texture coordinates to geographic coordinates
        float lon = vTexCoord.x * 360.0 - 180.0;

        float latitudeMinRad = radians(-85.0);
        float latitudeMaxRad = radians(85.0);
        float yMin = log(tan(latitudeMinRad) + 1.0 / cos(latitudeMinRad));
        float yMax = log(tan(latitudeMaxRad) + 1.0 / cos(latitudeMaxRad));
        float y = yMin + vTexCoord.y * (yMax - yMin);
        float lat = degrees(atan(sinh(y)));

        vec3 normal = geoToCartesian(lat, lon);

        // Sun position at summer solstice: fixed latitude +23°
        float sunLon = 180.0 - hourOffset * 15.0;
        float sunLat = earthTiltDeg;
        vec3 sunDir = geoToCartesian(sunLat, sunLon);

        // Calculate lighting (angle between surface normal and sun direction)
        float light = dot(normal, sunDir);

        if (light > 0)
            fragColor = vec4(texColor, 1.0);          // Day
        else
            fragColor = vec4(texColor * 0.5, 1.0);     // Night (50% dim)
    } else {
        fragColor = vec4(color, 1.0);
    }
}
)";


const std::vector<unsigned char> encodedData = {
    252, 252, 252, 252, 252, 252, 252, 252, 252, 0, 9, 80,
    1, 148, 13, 72, 13, 140, 25, 60, 21, 132, 41, 12, 1, 28,
    25, 128, 61, 0, 17, 4, 29, 124, 81, 8, 37, 116, 89, 0, 69,
    16, 5, 48, 97, 0, 77, 0, 25, 8, 1, 8, 253, 253, 253, 253,
    101, 10, 237, 14, 237, 14, 241, 10, 141, 2, 93, 14, 121, 2,
    5, 6, 93, 14, 49, 6, 57, 26, 89, 18, 41, 10, 57, 26, 89, 18,
    41, 14, 1, 2, 45, 26, 89, 26, 33, 18, 57, 14, 93, 26, 33, 18,
    57, 10, 93, 18, 5, 2, 33, 18, 41, 2, 5, 2, 5, 6, 89, 22, 29, 2,
    1, 22, 37, 2, 1, 6, 1, 2, 97, 22, 29, 38, 45, 2, 97, 10, 1, 2, 37,
    42, 17, 2, 13, 2, 5, 2, 89, 10, 49, 46, 25, 10, 101, 2, 5, 6, 37,
    50, 9, 30, 89, 10, 9, 2, 37, 50, 5, 38, 81, 26, 45, 22, 17, 54, 77,
    30, 41, 22, 17, 58, 1, 2, 61, 38, 65, 2, 9, 58, 69, 46, 37, 6, 1, 10,
    9, 62, 65, 38, 5, 2, 33, 102, 57, 54, 33, 102, 57, 30, 1, 14, 33, 2,
    9, 86, 9, 2, 21, 6, 13, 26, 5, 6, 53, 94, 29, 26, 1, 22, 29, 0, 29, 98,
    5, 14, 9, 46, 1, 2, 5, 6, 5, 2, 0, 13, 0, 13, 118, 1, 2, 1, 42, 1, 4, 5,
    6, 5, 2, 4, 33, 78, 1, 6, 1, 6, 1, 10, 5, 34, 1, 20, 2, 9, 2, 12, 25, 14,
    5, 30, 1, 54, 13, 6, 9, 2, 1, 32, 13, 8, 37, 2, 13, 2, 1, 70, 49, 28, 13,
    16, 53, 2, 1, 46, 1, 2, 1, 2, 53, 28, 17, 16, 57, 14, 1, 18, 1, 14, 1, 2,
    57, 24, 13, 20, 57, 0, 2, 1, 2, 17, 0, 17, 2, 61, 0, 5, 16, 1, 28, 25, 0,
    41, 2, 117, 56, 25, 0, 33, 2, 1, 2, 117, 52, 201, 48, 77, 0, 121, 40, 1, 0,
    205, 8, 1, 0, 1, 12, 213, 4, 13, 12, 253, 253, 253, 141
};



class MyApp : public glApp {

    Map *map;
    std::vector<Path *> paths;
    std::vector<Station *> stations;
    GPUProgram *prog;

    std::vector<vec2> stationGeoCoords;
    std::vector<float> distances;
    int hourOffset;

private:
    /**
     * Calculates the great-circle distance between two geographical coordinates
     * specified in degrees. The calculation uses cartesian conversions and the
     * haversine formula to compute the shortest path between the `start` and `end`
     * points on the surface of the Earth.
     *
     * @param start The starting geographical coordinate as a vec2 object, where
     *              x represents longitude and y represents latitude, in degrees.
     * @param end   The ending geographical coordinate as a vec2 object, where
     *              x represents longitude and y represents latitude, in degrees.
     * @return The great-circle distance between the start and end coordinates,
     *         measured in kilometers.
     */
    float calculateDistance(const vec2 &start, const vec2 &end) const {
        vec3 startCart = geoToCartesian(start);
        vec3 endCart = geoToCartesian(end);

        float dotProduct = dot(startCart, endCart);
        if (dotProduct > 0.9995f)
            dotProduct = 0.9995f;

        if (dotProduct < -0.9995f)
            dotProduct = -0.9995f;

        float angle = acosf(dotProduct);
        const float earthRadius = 40000.0f / (2.0f * M_PI);
        return angle * earthRadius;
    }


public:
    MyApp() : glApp(4, 5, 600, 600, "Grafika labor #3") { }


    /**
     * Initializes essential components required for the application, including:
     * - Setting up the map data using the provided encoded dataset.
     * - Creating and initializing a GPU program for rendering operations by compiling
     *   and linking vertex and fragment shaders.
     * - Initializing the hour offset used for time-based application logic.
     *
     * This method is overridden from the base class and designed to be called during
     * the application initialization phase. It ensures the necessary objects and
     * resources are prepared for rendering and other application-specific tasks.
     *
     * Actions performed in this method:
     * 1. Creates a new instance of the `Map` class using the `encodedData` to populate
     *    the geometry or other map-related structures.
     * 2. Allocates and initializes a new `GPUProgram` instance, using the provided
     *    `vertexShaderSource` and `fragmentShaderSource` strings for shader compilation.
     * 3. Sets the `hourOffset` variable to an initial value of 0, possibly for time or
     *    animation-related features.
     */
    void onInitialization() override {
        map = new Map(encodedData);
        prog = new GPUProgram();
        prog->create(vertexShaderSource, fragmentShaderSource);
        hourOffset = 0;
    }


    /**
     * Handles the rendering process for the application. This method is responsible for clearing
     * the screen, setting up the GPU program, and rendering the main map, paths, and stations
     * with appropriate attributes and visual properties.
     *
     * The rendering sequence includes:
     * - Clearing the color buffer of the screen using `glClear` to ensure fresh rendering for every frame.
     * - Activating the shader program through `prog->Use`, preparing it for drawing operations.
     * - Setting a uniform value for the "hourOffset" parameter in the shader program to influence
     *   time-based visual adjustments for the rendering.
     * - Drawing the main map via the `map->DrawMap` function, using the active shader program.
     * - Iterating over `paths` and rendering each path by calling `DrawPath` with a yellow (1.0, 1.0, 0.0)
     *   color for visualization.
     * - Iterating over `stations` and rendering each station by calling `DrawStation` with a red
     *   (1.0, 0.0, 0.0) color for visualization.
     *
     * Inputs:
     * - `hourOffset`: An integer offset that is converted to a floating-point value for use in the
     *   shader program. This value can influence the visual representation of time-related information.
     * - `prog`: A pointer to the `GPUProgram` object responsible for managing shader programs and
     *   handling uniform data.
     * - `map`: A pointer to the `Map` object that represents the primary map geometry to be rendered.
     * - `paths`: A collection of `Path` objects, each representing a graphical path to be rendered
     *   with a specific color.
     * - `stations`: A collection of `Station` objects, each representing a graphical station to be
     *   rendered with a specific color.
     *
     * Outputs:
     * - The frame is rendered to the screen with the updated representation of the map, paths, and
     *   stations according to their associated visual properties.
     */
    void onDisplay() override {
        glClear(GL_COLOR_BUFFER_BIT);
        prog->Use();

        prog->setUniform(static_cast<float>(hourOffset), "hourOffset");
        map->DrawMap(prog);

        for (auto *path: paths)
            path->DrawPath(prog, vec3(1.0f, 1.0f, 0.0f));

        for (auto *station: stations)
            station->DrawStation(prog, vec3(1.0f, 0.0f, 0.0f));
    }


    /**
     * Handles keyboard input events triggered by the user. Specifically, listens for the
     * 'n' or 'N' key presses to advance the hour offset and refresh the application screen
     * to reflect the updated state.
     *
     * @param key The integer representation of the key pressed. This can correspond to ASCII values,
     *            where 'n' or 'N' are used to indicate advancing the hour.
     */
    void onKeyboard(int key) override {
        if (key == 'n' || key == 'N') {
            hourOffset++;
            refreshScreen();
        }
    }


    /**
     * Handles the event when a mouse button is pressed. Specifically, it processes
     * left mouse button clicks to create new stations, compute geographic coordinates,
     * and, if multiple stations exist, adds a connecting path between the last two
     * created stations.
     *
     * When the left button is pressed, the method performs the following:
     * - Converts the screen coordinates of the click into normalized device coordinates (NDC).
     * - Maps the NDC to geographic coordinates on the map.
     * - Creates a new station at the geographic position and stores it.
     * - If there are at least two stations, generates a path between the last two stations,
     *   calculates the distance, and updates the list of distances.
     * - Displays the computed distance in kilometers.
     * - Triggers a screen refresh to render the updated elements.
     *
     * @param but The mouse button that was pressed. Expected to be `MOUSE_LEFT` for processing.
     * @param pX The x-coordinate of the mouse cursor at the time of the press, in screen coordinates.
     * @param pY The y-coordinate of the mouse cursor at the time of the press, in screen coordinates.
     */
    void onMousePressed(MouseButton but, int pX, int pY) override {
        if (but == MOUSE_LEFT) {
            float ndcX = (2.0f * pX / 600) - 1.0f;
            float ndcY = 1.0f - (2.0f * pY / 600);
            vec2 geoPos = mapCoordinatesToGeographic(vec2(ndcX, ndcY));
            stations.push_back(new Station(geoPos));
            stationGeoCoords.push_back(geoPos);
            if (stations.size() >= 2) {
                vec2 start = stationGeoCoords[stationGeoCoords.size() - 2];
                vec2 end = stationGeoCoords[stationGeoCoords.size() - 1];
                paths.push_back(new Path(start, end));
                float distance = calculateDistance(start, end);
                distances.push_back(distance);
                std::cout << "Distance: " << static_cast<int>(distance) << " km" << std::endl;
            }
            refreshScreen();
        }
    }


    /**
     * Destructor for the MyApp class.
     * Cleans up dynamically allocated memory and deallocates resources used by the application.
     * This ensures there is no memory leak when the application terminates.
     *
     * The destructor performs the following steps:
     * - Frees memory allocated for the map object.
     * - Frees memory allocated for the GPUProgram object.
     * - Iterates through and deletes all dynamically allocated Path objects stored in the `paths` vector.
     * - Iterates through and deletes all dynamically allocated Station objects stored in the `stations` vector.
     *
     * This process releases all resources associated with the application, preparing it for a proper cleanup.
     */
    ~MyApp() {
        delete map;
        delete prog;
        for (auto *path: paths) delete path;
        for (auto *station: stations) delete station;
    }

} app;
