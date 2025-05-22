# Interactive Map with Stations and Paths

This project is an interactive graphics application built using OpenGL, displaying a textured map where users can add stations and visualize paths between them. It calculates great-circle distances and simulates day-night cycles based on solar illumination. This README provides an overview, explains the underlying concepts, and guides you through the codebase.

## Table of Contents

- [Background Knowledge](#background-knowledge)
  - [OpenGL Basics](#opengl-basics)
  - [Geographic Coordinate Systems](#geographic-coordinate-systems)
  - [Physics Formulas](#physics-formulas)
- [Class Explanations](#class-explanations)
  - [Map](#map)
  - [Station](#station)
  - [Path](#path)
  - [MyApp](#myapp)
- [Shader Usage](#shader-usage)
  - [Vertex Shader](#vertex-shader)
  - [Fragment Shader](#fragment-shader)
- [Usage Instructions](#usage-instructions)
- [Contributing](#contributing)
- [License](#license)

---

## Background Knowledge

### OpenGL Basics

OpenGL (Open Graphics Library) is a powerful tool for rendering 2D and 3D graphics by communicating with your computer's GPU (Graphics Processing Unit). Think of it as a middleman between your code and the hardware that draws images on the screen. Here’s a breakdown of key OpenGL concepts for beginners:

- **Vertex**: A point in space, like a dot on a graph, defined by coordinates (e.g., x, y for 2D).
- **Primitive**: Basic shapes OpenGL can draw, such as points, lines, or triangles, made from vertices.
- **Shader**: A mini-program that runs on the GPU. There are two main types:
  - *Vertex Shader*: Processes each vertex (e.g., positioning it on the screen).
  - *Fragment Shader*: Colors each pixel (fragment) within a primitive.
- **Texture**: An image you can "stick" onto a shape, like wallpaper on a wall.
- **Buffer**: A chunk of memory on the GPU storing data, like vertex positions or texture coordinates.
- **Vertex Array Object (VAO)**: A container that organizes vertex data and how it’s fed to the GPU.
- **Uniform**: A value sent from your C++ code to the shader, staying constant for all vertices or fragments in a draw call.

**How OpenGL Works (Rendering Pipeline):**

1. **Vertex Specification**: You define vertex data (e.g., positions) in your code.
2. **Vertex Processing**: The vertex shader transforms these vertices (e.g., into screen coordinates).
3. **Primitive Assembly**: Vertices are grouped into shapes like lines or triangles.
4. **Rasterization**: These shapes are broken into fragments (pixels).
5. **Fragment Processing**: The fragment shader assigns colors to each fragment.
6. **Output Merging**: The final image is written to the screen.

This project uses OpenGL to draw a map, stations (as points), and paths (as lines), with shaders handling the visuals.

### Geographic Coordinate Systems

The Earth’s surface is mapped using geographic coordinates:
- **Latitude**: Angle from the equator (0°) to the poles (±90°), measuring north or south.
- **Longitude**: Angle from the Prime Meridian (0°) to ±180°, measuring east or west.

To display this on a flat screen, we use the **Mercator Projection**, which stretches the Earth’s surface into a rectangle. It preserves angles (good for navigation) but distorts sizes near the poles. This project maps these coordinates to a 2D plane for rendering.

### Physics Formulas

The project uses physics to calculate distances and interpolate paths on a sphere (Earth). Here are the key formulas:

#### Great-Circle Distance

The shortest distance between two points on a sphere, like flying between cities. We use this instead of straight-line distance because Earth is round.

Given two points with latitudes $\phi_1, \phi_2$ (in radians) and longitudes $\lambda_1, \lambda_2$ (in radians):
- Central angle $\theta$:
  
$$
\theta = \arccos(\sin \phi_1 \sin \phi_2 + \cos \phi_1 \cos \phi_2 \cos(\lambda_2 - \lambda_1))
$$

- Distance: $$d = r \theta$$

  where $r$ is Earth's radius (approximately 6371 km). In the code, we use $r = \frac{40000}{2\pi}$ km for simplicity.

#### Spherical Linear Interpolation (SLERP)

SLERP creates a smooth curve between two points on a sphere, used for drawing paths. For two 3D vectors $\mathbf{a}$ and $\mathbf{b}$ (converted from geographic coordinates), and an interpolation factor $t \in [0,1]$:

$$
\mathbf{p}(t) = \frac{\sin((1 - t) \theta)}{\sin \theta} \mathbf{a} + \frac{\sin(t \theta)}{\sin \theta} \mathbf{b}
$$

where $\theta = \arccos(\mathbf{a} \cdot \mathbf{b})$ is the angle between the vectors. When $t = 0$, you get $\mathbf{a}$; when $t = 1$, you get $\mathbf{b}$.

---

## Class Explanations

### Map

- **Purpose**: Displays the background map as a textured rectangle.
- **How It Works**: 
  - Takes run-length encoded image data, decodes it into RGB colors, and creates a 64x64 texture.
  - Sets up a VAO and two VBOs (Vertex Buffer Objects): one for vertex positions (a full-screen quad), another for texture coordinates.
  - The `DrawMap` method binds the texture and draws the quad using OpenGL’s `GL_TRIANGLE_FAN`.
- **Why It’s Needed**: Provides the visual foundation, showing the Earth’s surface for users to interact with.

### Station

- **Purpose**: Marks a user-defined point on the map.
- **How It Works**: 
  - Stores a geographic position (latitude, longitude), converts it to normalized map coordinates using the Mercator projection.
  - Updates a VBO with this position and draws it as a red point (size 10 pixels) via `DrawStation`.
- **Why It’s Needed**: Represents key locations (e.g., cities) users click to add, serving as endpoints for paths.

### Path

- **Purpose**: Connects two stations with a curved line.
- **How It Works**: 
  - Takes two geographic coordinates, converts them to Cartesian (3D) vectors, and uses SLERP to generate 100 intermediate points.
  - Converts these points back to normalized map coordinates, stores them in a VBO, and draws them as a yellow line strip (width 3 pixels) with `DrawPath`.
- **Why It’s Needed**: Visualizes routes between stations, showing realistic spherical paths (not straight lines).

### MyApp

- **Purpose**: The main class that runs the application and ties everything together.
- **How It Works**: 
  - Initializes the map, shaders, and OpenGL resources in `onInitialization`.
  - Handles rendering (`onDisplay`) by drawing the map, paths, and stations.
  - Responds to user input: 
    - Left-click (`onMousePressed`) adds stations and paths, calculating distances.
    - ‘n’/‘N’ key (`onKeyboard`) advances the hour for day-night simulation.
  - Cleans up memory in the destructor.
- **Why It’s Needed**: Acts as the controller, managing user interaction and rendering logic.

---

## Shader Usage

Shaders are written in GLSL (OpenGL Shading Language) and run on the GPU to process graphics.

### Vertex Shader

- **Purpose**: Positions vertices on the screen and passes texture coordinates.
- **How It Works**: 
  - Takes 2D vertex positions (e.g., map corners) and converts them to 4D clip space (adding z=0, w=1).
  - Passes texture coordinates (0 to 1) to the fragment shader for texture mapping.
- **Why It’s Needed**: Ensures the map and other elements are correctly placed on the screen.

### Fragment Shader

- **Purpose**: Colors each pixel, adding texture and lighting effects.
- **How It Works**: 
  - For the map (textured mode):
    - Samples the texture color using texture coordinates.
    - Converts coordinates to geographic latitude/longitude, then to a 3D normal vector.
    - Calculates lighting by comparing the normal to the sun’s position (based on `hourOffset`), dimming night areas by 50%.
  - For stations/paths (non-textured mode):
    - Uses a uniform color (red for stations, yellow for paths).
- **Why It’s Needed**: Adds visual realism with textures and a day-night cycle based on solar illumination.

---

## Usage Instructions

1. **Running the Application**:
   - Compile the C++ code with a compiler supporting OpenGL (e.g., g++ with GLFW and GLAD libraries).
   - Run the executable to open a 600x600 window showing the map.

2. **Adding Stations**:
   - Left-click anywhere on the map to place a station (red dot).
   - Each new station connects to the previous one with a path (yellow line).

3. **Advancing Time**:
   - Press ‘n’ or ‘N’ to increment the hour, updating the lighting to simulate day and night.

4. **Viewing Distances**:
   - After adding two or more stations, the console prints the great-circle distance between the last two in kilometers.

---

## Contributing

Feel free to fork this repository and submit pull requests with improvements or bug fixes. Suggestions for new features (e.g., zooming, custom textures) are welcome!
