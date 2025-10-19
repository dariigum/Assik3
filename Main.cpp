#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>



// Vertex structure
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

// Triangle structure
struct Triangle {
    unsigned int indices[3];
    glm::vec3 faceNormal;
};

// Model data
std::vector<Vertex> vertices;
std::vector<Triangle> triangles;
std::vector<glm::vec3> vertexPositions;
std::vector<glm::vec3> vertexNormals;

// Camera parameters
float cameraAngle = 45.0f;    // ← начальный угол
float cameraRadius = 5.0f;    // ← дальше от куба
float cameraHeight = 2.0f;
glm::vec3 modelCenter(0.0f);

// Light parameters
float lightAngle = 45.0f;
float lightRadius = 2.0f;
float lightHeight = 2.0f;

// Projection mode
bool usePerspective = true;

// Shading mode (0: flat, 1: Gouraud, 2: Phong)
int shadingMode = 2;  // ← начинаем с Phong shading

// Material index
int currentMaterial = 0;

// Window dimensions
int windowWidth = 1200;
int windowHeight = 800;

// Shader sources
const char* flatVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char* flatFragmentShaderSource = R"(
#version 330 core
in vec3 Normal;
out vec4 FragColor;

void main()
{
    vec3 color = abs(normalize(Normal));
    FragColor = vec4(color, 1.0);
}
)";

const char* gouraudVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightPos1;
uniform vec3 lightPos2;
uniform vec3 viewPos;

uniform vec4 material_ambient;
uniform vec4 material_diffuse;
uniform vec4 material_specular;
uniform float material_shininess;

uniform vec4 light_ambient;
uniform vec4 light_diffuse;
uniform vec4 light_specular;

out vec4 vertexColor;

void main()
{
    vec3 FragPos = vec3(model * vec4(aPos, 1.0));
    vec3 Normal = mat3(transpose(inverse(model))) * aNormal;
    Normal = normalize(Normal);
    
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Ambient
    vec4 ambient = light_ambient * material_ambient;
    
    vec4 totalDiffuse = vec4(0.0);
    vec4 totalSpecular = vec4(0.0);
    
    // Light 1 (object space)
    vec3 lightDir1 = normalize(lightPos1 - FragPos);
    float diff1 = max(dot(Normal, lightDir1), 0.0);
    vec4 diffuse1 = light_diffuse * (diff1 * material_diffuse);
    
    vec3 reflectDir1 = reflect(-lightDir1, Normal);
    float spec1 = pow(max(dot(viewDir, reflectDir1), 0.0), material_shininess);
    vec4 specular1 = light_specular * (spec1 * material_specular);
    
    // Light 2 (camera space)
    vec3 lightDir2 = normalize(lightPos2 - FragPos);
    float diff2 = max(dot(Normal, lightDir2), 0.0);
    vec4 diffuse2 = light_diffuse * (diff2 * material_diffuse);
    
    vec3 reflectDir2 = reflect(-lightDir2, Normal);
    float spec2 = pow(max(dot(viewDir, reflectDir2), 0.0), material_shininess);
    vec4 specular2 = light_specular * (spec2 * material_specular);
    
    totalDiffuse = diffuse1 + diffuse2;
    totalSpecular = specular1 + specular2;
    
    vertexColor = ambient + totalDiffuse + totalSpecular;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char* gouraudFragmentShaderSource = R"(
#version 330 core
in vec4 vertexColor;
out vec4 FragColor;

void main()
{
    FragColor = vertexColor;
}
)";

const char* phongVertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
)";

const char* phongFragmentShaderSource = R"(
#version 330 core
in vec3 FragPos;
in vec3 Normal;
out vec4 FragColor;

uniform vec3 lightPos1;
uniform vec3 lightPos2;
uniform vec3 viewPos;

uniform vec4 material_ambient;
uniform vec4 material_diffuse;
uniform vec4 material_specular;
uniform float material_shininess;

uniform vec4 light_ambient;
uniform vec4 light_diffuse;
uniform vec4 light_specular;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Ambient
    vec4 ambient = light_ambient * material_ambient;
    
    vec4 totalDiffuse = vec4(0.0);
    vec4 totalSpecular = vec4(0.0);
    
    // Light 1
    vec3 lightDir1 = normalize(lightPos1 - FragPos);
    float diff1 = max(dot(norm, lightDir1), 0.0);
    vec4 diffuse1 = light_diffuse * (diff1 * material_diffuse);
    
    vec3 reflectDir1 = reflect(-lightDir1, norm);
    float spec1 = pow(max(dot(viewDir, reflectDir1), 0.0), material_shininess);
    vec4 specular1 = light_specular * (spec1 * material_specular);
    
    // Light 2
    vec3 lightDir2 = normalize(lightPos2 - FragPos);
    float diff2 = max(dot(norm, lightDir2), 0.0);
    vec4 diffuse2 = light_diffuse * (diff2 * material_diffuse);
    
    vec3 reflectDir2 = reflect(-lightDir2, norm);
    float spec2 = pow(max(dot(viewDir, reflectDir2), 0.0), material_shininess);
    vec4 specular2 = light_specular * (spec2 * material_specular);
    
    totalDiffuse = diffuse1 + diffuse2;
    totalSpecular = specular1 + specular2;
    
    FragColor = ambient + totalDiffuse + totalSpecular;
}
)";

// Function to compile shader
unsigned int compileShader(const char* source, GLenum type) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }
    return shader;
}

// Function to create shader program
unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    unsigned int vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

// Load SMF file
bool loadSMF(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return false;
    }

    vertexPositions.clear();
    triangles.clear();

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        char type;
        iss >> type;

        if (type == 'v') {
            float x, y, z;
            iss >> x >> y >> z;
            vertexPositions.push_back(glm::vec3(x, y, z));
        }
        else if (type == 'f') {
            Triangle tri;
            iss >> tri.indices[0] >> tri.indices[1] >> tri.indices[2];
            tri.indices[0]--; tri.indices[1]--; tri.indices[2]--;
            triangles.push_back(tri);
        }
    }

    file.close();

    // Calculate model center
    modelCenter = glm::vec3(0.0f);
    for (const auto& pos : vertexPositions) {
        modelCenter += pos;
    }
    modelCenter /= vertexPositions.size();

    std::cout << "Loaded " << vertexPositions.size() << " vertices and "
        << triangles.size() << " triangles" << std::endl;

    return true;
}

// Calculate face normals
void calculateFaceNormals() {
    for (auto& tri : triangles) {
        glm::vec3 v0 = vertexPositions[tri.indices[0]];
        glm::vec3 v1 = vertexPositions[tri.indices[1]];
        glm::vec3 v2 = vertexPositions[tri.indices[2]];

        glm::vec3 edge1 = v1 - v0;
        glm::vec3 edge2 = v2 - v0;
        tri.faceNormal = glm::normalize(glm::cross(edge1, edge2));
    }
}

// Calculate vertex normals (average of adjacent face normals)
void calculateVertexNormals() {
    vertexNormals.resize(vertexPositions.size(), glm::vec3(0.0f));

    for (const auto& tri : triangles) {
        vertexNormals[tri.indices[0]] += tri.faceNormal;
        vertexNormals[tri.indices[1]] += tri.faceNormal;
        vertexNormals[tri.indices[2]] += tri.faceNormal;
    }

    for (auto& normal : vertexNormals) {
        normal = glm::normalize(normal);
    }
}

// Prepare vertex data
void prepareVertexData() {
    vertices.clear();

    if (shadingMode == 0) {
        // Flat shading - use face normals
        for (const auto& tri : triangles) {
            for (int i = 0; i < 3; i++) {
                Vertex v;
                v.position = vertexPositions[tri.indices[i]];
                v.normal = tri.faceNormal;
                vertices.push_back(v);
            }
        }
    }
    else {
        // Gouraud or Phong - use vertex normals
        for (const auto& tri : triangles) {
            for (int i = 0; i < 3; i++) {
                Vertex v;
                v.position = vertexPositions[tri.indices[i]];
                v.normal = vertexNormals[tri.indices[i]];
                vertices.push_back(v);
            }
        }
    }
}

// Keyboard callback
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, true);
            break;
        case GLFW_KEY_A:
            cameraAngle -= 5.0f;
            std::cout << "Camera angle: " << cameraAngle << std::endl;
            break;
        case GLFW_KEY_D:
            cameraAngle += 5.0f;
            std::cout << "Camera angle: " << cameraAngle << std::endl;
            break;
        case GLFW_KEY_W:
            cameraHeight += 0.1f;
            std::cout << "Camera height: " << cameraHeight << std::endl;
            break;
        case GLFW_KEY_S:
            cameraHeight -= 0.1f;
            std::cout << "Camera height: " << cameraHeight << std::endl;
            break;
        case GLFW_KEY_Q:
            cameraRadius -= 0.1f;
            if (cameraRadius < 0.5f) cameraRadius = 0.5f;
            std::cout << "Camera radius: " << cameraRadius << std::endl;
            break;
        case GLFW_KEY_E:
            cameraRadius += 0.1f;
            std::cout << "Camera radius: " << cameraRadius << std::endl;
            break;
        case GLFW_KEY_P:
            usePerspective = !usePerspective;
            std::cout << "Projection: " << (usePerspective ? "Perspective" : "Parallel") << std::endl;
            break;
        case GLFW_KEY_1:
            shadingMode = 0;
            std::cout << "Shading: Flat" << std::endl;
            break;
        case GLFW_KEY_2:
            shadingMode = 1;
            std::cout << "Shading: Gouraud" << std::endl;
            break;
        case GLFW_KEY_3:
            shadingMode = 2;
            std::cout << "Shading: Phong" << std::endl;
            break;
        case GLFW_KEY_M:
            currentMaterial = (currentMaterial + 1) % 3;
            std::cout << "Material: " << currentMaterial << std::endl;
            break;
        case GLFW_KEY_J:
            lightAngle -= 5.0f;
            break;
        case GLFW_KEY_L:
            lightAngle += 5.0f;
            break;
        case GLFW_KEY_I:
            lightHeight += 0.1f;
            break;
        case GLFW_KEY_K:
            lightHeight -= 0.1f;
            break;
        case GLFW_KEY_U:
            lightRadius -= 0.1f;
            if (lightRadius < 0.5f) lightRadius = 0.5f;
            break;
        case GLFW_KEY_O:
            lightRadius += 0.1f;
            break;
        }
    }
}

int main(int argc, char** argv) {
    std::string filename;
    
    // Если аргумент не передан, используем путь по умолчанию
    if (argc < 2) {
        filename = "../../models/cube.smf";
        std::cout << "No argument provided. Using default: " << filename << std::endl;
    } else {
        filename = argv[1];
        std::cout << "Loading model: " << filename << std::endl;
    }
    
    // Загрузка модели
    if (!loadSMF(filename)) {
        std::cerr << "ERROR: Failed to load model!" << std::endl;
        std::cin.get(); // Держим консоль открытой
        return -1;
    }
    
    std::cout << "SUCCESS! Loaded " << vertexPositions.size() 
              << " vertices and " << triangles.size() << " triangles" << std::endl;
    
    calculateFaceNormals();
    calculateVertexNormals();

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Assignment 3", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, windowWidth, windowHeight);

    // Create shader programs
    unsigned int flatShader = createShaderProgram(flatVertexShaderSource, flatFragmentShaderSource);
    unsigned int gouraudShader = createShaderProgram(gouraudVertexShaderSource, gouraudFragmentShaderSource);
    unsigned int phongShader = createShaderProgram(phongVertexShaderSource, phongFragmentShaderSource);

    // Create VAO and VBO
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    std::cout << "\nControls:" << std::endl;
    std::cout << "Camera: A/D (rotate), W/S (height), Q/E (radius)" << std::endl;
    std::cout << "Light: J/L (rotate), I/K (height), U/O (radius)" << std::endl;
    std::cout << "P: Toggle projection, 1/2/3: Flat/Gouraud/Phong, M: Change material" << std::endl;

    int prevShadingMode = -1;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update vertex data if shading mode changed
        if (shadingMode != prevShadingMode) {
            prepareVertexData();
            prevShadingMode = shadingMode;

            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
            glEnableVertexAttribArray(1);
        }

        // Calculate camera position
        float radAngle = glm::radians(cameraAngle);
        glm::vec3 cameraPos(
            modelCenter.x + cameraRadius * cos(radAngle),
            modelCenter.y + cameraRadius * sin(radAngle),
            modelCenter.z + cameraHeight
        );

        // Calculate light position
        float lightRadAngle = glm::radians(lightAngle);
        glm::vec3 lightPos1(
            modelCenter.x + lightRadius * cos(lightRadAngle),
            modelCenter.y + lightRadius * sin(lightRadAngle),
            modelCenter.z + lightHeight
        );

        // Light 2 near camera
        glm::vec3 lightPos2 = cameraPos + glm::vec3(0.5f, 0.5f, 0.5f);

        // Matrices
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::lookAt(cameraPos, modelCenter, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 projection;
        if (usePerspective) {
            projection = glm::perspective(glm::radians(45.0f), (float)windowWidth / windowHeight, 0.1f, 100.0f);
        }
        else {
            float orthoSize = 2.0f;
            projection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 100.0f);
        }

        // Select shader
        unsigned int currentShader;
        if (shadingMode == 0) {
            currentShader = flatShader;
        }
        else if (shadingMode == 1) {
            currentShader = gouraudShader;
        }
        else {
            currentShader = phongShader;
        }

        glUseProgram(currentShader);

        // Set uniforms
        glUniformMatrix4fv(glGetUniformLocation(currentShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(currentShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(currentShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        if (shadingMode > 0) {
            // Material properties
            glm::vec4 mat_ambient, mat_diffuse, mat_specular;
            float mat_shininess;

            if (currentMaterial == 0) {
                // Bright specular material (required)
                mat_ambient = glm::vec4(0.6f, 0.2f, 0.2f, 1.0f);
                mat_diffuse = glm::vec4(0.9f, 0.1f, 0.1f, 1.0f);
                mat_specular = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f);
                mat_shininess = 80.0f;
            }
            else if (currentMaterial == 1) {
                // Gold-like material
                mat_ambient = glm::vec4(0.24725f, 0.1995f, 0.0745f, 1.0f);
                mat_diffuse = glm::vec4(0.75164f, 0.60648f, 0.22648f, 1.0f);
                mat_specular = glm::vec4(0.628281f, 0.555802f, 0.366065f, 1.0f);
                mat_shininess = 51.2f;
            }
            else {
                // Emerald-like material
                mat_ambient = glm::vec4(0.0215f, 0.1745f, 0.0215f, 1.0f);
                mat_diffuse = glm::vec4(0.07568f, 0.61424f, 0.07568f, 1.0f);
                mat_specular = glm::vec4(0.633f, 0.727811f, 0.633f, 1.0f);
                mat_shininess = 76.8f;
            }

            glm::vec4 light_ambient(0.2f, 0.2f, 0.2f, 1.0f);
            glm::vec4 light_diffuse(0.6f, 0.6f, 0.6f, 1.0f);
            glm::vec4 light_specular(1.0f, 1.0f, 1.0f, 1.0f);

            glUniform3fv(glGetUniformLocation(currentShader, "lightPos1"), 1, glm::value_ptr(lightPos1));
            glUniform3fv(glGetUniformLocation(currentShader, "lightPos2"), 1, glm::value_ptr(lightPos2));
            glUniform3fv(glGetUniformLocation(currentShader, "viewPos"), 1, glm::value_ptr(cameraPos));

            glUniform4fv(glGetUniformLocation(currentShader, "material_ambient"), 1, glm::value_ptr(mat_ambient));
            glUniform4fv(glGetUniformLocation(currentShader, "material_diffuse"), 1, glm::value_ptr(mat_diffuse));
            glUniform4fv(glGetUniformLocation(currentShader, "material_specular"), 1, glm::value_ptr(mat_specular));
            glUniform1f(glGetUniformLocation(currentShader, "material_shininess"), mat_shininess);

            glUniform4fv(glGetUniformLocation(currentShader, "light_ambient"), 1, glm::value_ptr(light_ambient));
            glUniform4fv(glGetUniformLocation(currentShader, "light_diffuse"), 1, glm::value_ptr(light_diffuse));
            glUniform4fv(glGetUniformLocation(currentShader, "light_specular"), 1, glm::value_ptr(light_specular));
        }

        // Draw
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices.size());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(flatShader);
    glDeleteProgram(gouraudShader);
    glDeleteProgram(phongShader);

    glfwTerminate();
    return 0;
}