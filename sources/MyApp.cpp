#include <iostream>

#include "Map.h"
#include <vector>
#include <GLFW/glfw3.h>


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


const char *fragmentShaderSource = R"(
#version 330 core
in vec2 vTexCoord;
out vec4 fragColor;

uniform sampler2D tex;
uniform bool isTextured;
uniform vec3 color;
uniform float dayNightFactor;

void main() {
    if (isTextured) {
        vec3 texColor = texture(tex, vTexCoord).rgb;
        fragColor = vec4(texColor * dayNightFactor, 1.0);
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
    std::chrono::system_clock::time_point currentTime;
    int hourOffset;


private:
    /**
     * Calculates a day-night factor based on the given hour offset from the
     * adjusted summer solstice day and time. The returned factor ranges from
     * 0.0 to 1.0, where 0.0 represents complete night and 1.0 represents
     * full day, transitioning smoothly using a cosine function.
     *
     * @return The day-night factor as a float in the range [0.0, 1.0].
     */
    float calculateDayNightFactor() {
        std::tm summerSolstice = {};
        summerSolstice.tm_year = 2025 - 1900;
        summerSolstice.tm_mon = 5;
        summerSolstice.tm_mday = 21;
        summerSolstice.tm_hour = 0;
        summerSolstice.tm_min = 0;
        summerSolstice.tm_sec = 0;
        summerSolstice.tm_isdst = 0;

        auto solsticeTime = std::chrono::system_clock::from_time_t(std::mktime(&summerSolstice));
        auto adjustedTime = solsticeTime + std::chrono::hours(hourOffset);

        std::time_t tt = std::chrono::system_clock::to_time_t(adjustedTime);
        std::tm *local = std::localtime(&tt);

        float hour = local->tm_hour + local->tm_min / 60.0f;
        float factor = cosf(((hour - 12.0f) / 12.0f) * M_PI);
        return (factor + 1.0f) / 2.0f;
    }


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
    float calculateDistance(const vec2 &start, const vec2 &end) {
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


    void onInitialization() override {
        map = new Map(encodedData);
        prog = new GPUProgram();
        prog->create(vertexShaderSource, fragmentShaderSource);

        hourOffset = 0;
        std::tm summerSolstice = {};
        summerSolstice.tm_year = 2025 - 1900;
        summerSolstice.tm_mon = 5;
        summerSolstice.tm_mday = 21;
        summerSolstice.tm_hour = 0;
        summerSolstice.tm_min = 0;
        summerSolstice.tm_sec = 0;
        summerSolstice.tm_isdst = 0;
        currentTime = std::chrono::system_clock::from_time_t(std::mktime(&summerSolstice));
    }


    void onDisplay() override {
        glClear(GL_COLOR_BUFFER_BIT);
        prog->Use();
        float dayNightFactor = calculateDayNightFactor();

        prog->setUniform(dayNightFactor, "dayNightFactor");
        map->DrawMap(prog);

        for (size_t i = 0; i < paths.size(); ++i) {
            prog->setUniform(1.0f, "dayNightFactor");
            paths[i]->DrawPath(prog, vec3(1.0f, 1.0f, 0.0f));
        }

        for (auto *station: stations) {
            prog->setUniform(1.0f, "dayNightFactor");
            station->DrawStation(prog, vec3(1.0f, 0.0f, 0.0f));
        }
    }

    void onKeyboard(int key) override {
        if (key == 'n' || key == 'N') {
            hourOffset++;
            refreshScreen();
        }
    }


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


    ~MyApp() {
        delete map;
        delete prog;
        for (auto *path: paths) delete path;
        for (auto *station: stations) delete station;
    }

} app;
