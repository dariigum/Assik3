# Assignment 3 - 3D Graphics Rendering

Computer Graphics Assignment implementing viewing, projection, and various shading techniques using OpenGL.

## Features Implemented

### Part 1: Viewing, Projection and Flat-Shading
- ✅ Load triangle mesh models from SMF files
- ✅ Calculate surface normals for each triangle
- ✅ Visualize normals as colors (flat shading)
- ✅ Rotating camera view using LookAt function
- ✅ Cylindrical camera motion (adjustable height, radius, and angle)
- ✅ Toggle between parallel (orthographic) and perspective projection

### Part 2: Lighting, Materials, Gouraud and Phong Shading
- ✅ Calculate vertex normals by averaging face normals
- ✅ Implement Phong shading model
- ✅ Two point lights (one in object space, one in camera space)
- ✅ Gouraud shading algorithm (per-vertex lighting)
- ✅ Phong shading algorithm (per-fragment lighting)
- ✅ Three distinct materials with different properties
- ✅ Interactive light positioning on cylinder
- ✅ Camera-attached light source

## Project Structure

```
Assignment3/
├── src/
│   ├── main.cpp          # Main program
│   └── glad.c            # GLAD OpenGL loader
├── include/
│   ├── glad/
│   │   └── glad.h
│   ├── GLFW/
│   │   └── glfw3.h
│   └── glm/              # GLM math library
├── models/               # SMF model files
│   └── bound-lo-sphere.smf
├── .vscode/              # VS Code configuration
│   ├── tasks.json
│   ├── launch.json
│   └── c_cpp_properties.json
├── CMakeLists.txt
└── README.md
```

## Requirements

### Libraries
- **GLAD**: OpenGL loader (already have)
- **GLFW**: Window and input management (already have)
- **GLM**: Mathematics library for graphics
- **OpenGL 3.3+**: Core profile

### System Requirements
- C++17 compiler (g++, clang++, or MSVC)
- CMake 3.10+ (optional, for CMake build)
- Linux/macOS/Windows

## Setup Instructions

### 1. Install GLM (if not installed)

**Ubuntu/Debian:**
```bash
sudo apt-get install libglm-dev
```

**macOS (with Homebrew):**
```bash
brew install glm
```

**Windows:**
Download from https://github.com/g-truc/glm/releases and extract to your include directory.

### 2. Project Setup

Create the following directory structure:

```bash
mkdir -p Assignment3/src Assignment3/include Assignment3/models Assignment3/.vscode
cd Assignment3
```

Copy the files:
- `main.cpp` → `src/main.cpp`
- Your `glad.c` → `src/glad.c`
- Your GLAD headers → `include/glad/`
- Your GLFW headers → `include/GLFW/`
- VS Code config files → `.vscode/`

### 3. Download SMF Models

Download SMF files from the course website and place them in the `models/` directory. For example:
- `bound-lo-sphere.smf`
- `al.smf`
- `bound-sprellipse.smf`

## Building the Project

### Option 1: Direct Compilation (Quick)

**Linux/macOS:**
```bash
g++ -std=c++17 src/main.cpp src/glad.c \
    -Iinclude \
    -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl \
    -o assignment3
```

**Windows (MinGW):**
```bash
g++ -std=c++17 src/main.cpp src/glad.c \
    -Iinclude \
    -lglfw3 -lopengl32 -lgdi32 \
    -o assignment3.exe
```

### Option 2: Using CMake

```bash
mkdir build
cd build
cmake ..
make
```

### Option 3: VS Code (Recommended)

1. Open the project folder in VS Code
2. Press `Ctrl+Shift+B` to build
3. Press `F5` to debug/run

## Running the Program

```bash
./assignment3 models/bound-lo-sphere.smf
```

Or from VS Code: Press `F5`

## Controls

### Camera Controls
| Key | Action |
|-----|--------|
| `A` | Rotate camera left (decrease angle) |
| `D` | Rotate camera right (increase angle) |
| `W` | Move camera up (increase height) |
| `S` | Move camera down (decrease height) |
| `Q` | Move camera closer (decrease radius) |
| `E` | Move camera farther (increase radius) |

### Light Controls (Light 1 - Object Space)
| Key | Action |
|-----|--------|
| `J` | Rotate light left (decrease angle) |
| `L` | Rotate light right (increase angle) |
| `I` | Move light up (increase height) |
| `K` | Move light down (decrease height) |
| `U` | Move light closer (decrease radius) |
| `O` | Move light farther (increase radius) |

### Rendering Controls
| Key | Action |
|-----|--------|
| `P` | Toggle Perspective/Parallel projection |
| `1` | Flat shading (Part 1) |
| `2` | Gouraud shading (Part 2) |
| `3` | Phong shading (Part 2) |
| `M` | Cycle through materials (Part 2) |
| `ESC` | Exit program |

## Material Properties

### Material 0: Bright Specular (Required)
- Ambient: (0.6, 0.2, 0.2)
- Diffuse: (0.9, 0.1, 0.1)
- Specular: (0.8, 0.8, 0.8)
- Shininess: 80.0
- Produces bright white specular highlights

### Material 1: Gold-like
- Ambient: (0.247, 0.199, 0.074)
- Diffuse: (0.751, 0.606, 0.226)
- Specular: (0.628, 0.555, 0.366)
- Shininess: 51.2
- Warm metallic appearance

### Material 2: Emerald-like
- Ambient: (0.021, 0.174, 0.021)
- Diffuse: (0.075, 0.614, 0.075)
- Specular: (0.633, 0.727, 0.633)
- Shininess: 76.8
- Green gemstone appearance

## Lighting Setup

### Light 1 (Object Space)
- Position: Cylindrical coordinates around model center
- User-controllable via J/L/I/K/U/O keys
- Stays fixed relative to the model

### Light 2 (Camera Space)
- Position: Near the camera eye point (offset by 0.5 in each axis)
- Moves with the camera automatically
- Simulates a "headlight" effect

### Light Properties (Both Lights)
- Ambient: (0.2, 0.2, 0.2)
- Diffuse: (0.6, 0.6, 0.6)
- Specular: (1.0, 1.0, 1.0)

## Technical Implementation Details

### Shading Modes

**Flat Shading:**
- One normal per triangle (face normal)
- Color represents absolute value of normalized normal
- Fast but faceted appearance

**Gouraud Shading:**
- Lighting calculated per vertex
- Colors interpolated across triangle
- Smooth but can miss specular highlights

**Phong Shading:**
- Normals interpolated across triangle
- Lighting calculated per fragment
- Highest quality, smooth specular highlights

### Normal Calculation

**Face Normals:**
```
normal = normalize(cross(edge1, edge2))
```

**Vertex Normals:**
```
vertexNormal = normalize(sum of adjacent face normals)
```

### Projection

**Perspective:**
- FOV: 45 degrees
- Aspect ratio: matches window
- Near/far planes: 0.1 to 100

**Parallel (Orthographic):**
- Ortho size: 2.0 units
- Maintains object proportions
- No perspective distortion

## Troubleshooting

### Program won't compile
- Ensure GLM is installed: `sudo apt-get install libglm-dev`
- Check that glad.c is in src/ directory
- Verify include paths in tasks.json

### Black screen or no rendering
- Check that SMF file path is correct
- Ensure model has at least 100 triangles
- Try adjusting camera with E key (move farther)

### Lighting looks wrong
- Press `2` or `3` to enable Gouraud/Phong shading
- Press `M` to cycle materials
- Adjust light position with J/L/I/K/U/O keys

### Window closes immediately
- Check console for error messages
- Ensure SMF file exists and is readable
- Run from terminal to see output

## Expected Output

### Part 1 (Flat Shading)
- Model rendered with colored triangles
- Colors represent surface normal directions
- Camera rotates smoothly around model
- Projection toggles between perspective and parallel

### Part 2 (Gouraud and Phong)
For `bound-lo-sphere.smf` with Material 0:
- **Gouraud**: Smooth shading with some color banding
- **Phong**: Very smooth with sharp specular highlights
- Specular highlights appear bright white
- Highlights move as light position changes

## Submission Checklist

- [ ] Source code (main.cpp)
- [ ] VS Code configuration files
- [ ] CMakeLists.txt or Makefile
- [ ] README with setup instructions
- [ ] Screenshots showing:
  - [ ] Flat shading
  - [ ] Gouraud shading with all 3 materials
  - [ ] Phong shading with all 3 materials
  - [ ] Different camera angles
  - [ ] Parallel vs perspective projection
- [ ] Video demonstration or YouTube link
- [ ] Report in PDF/Word format describing implementation

## Report Structure

1. **Introduction**: Brief overview of assignment goals
2. **Implementation**:
   - Part 1: Viewing and flat shading
   - Part 2: Lighting and shading algorithms
3. **Libraries Used**: GLAD, GLFW, GLM, OpenGL 3.3
4. **Code Snippets**: Key functions with explanations
5. **Results**: Screenshots with descriptions
6. **Challenges**: Problems encountered and solutions
7. **Conclusion**: What was learned

## Additional Notes

- The camera rotates around the Y-Z plane by default
- Model center is calculated as average of all vertices
- All vectors are normalized before use in lighting calculations
- Fragment shader implements standard Phong reflection model
- Vertex normals are smoothed for better appearance

## References

- OpenGL 3.3 Core Profile Specification
- Learn OpenGL (learnopengl.com)
- Phong Reflection Model
- SMF File Format Specification

## Author

Name_Surname_GroupNo

## License

Educational use only - Computer Graphics Course Assignment