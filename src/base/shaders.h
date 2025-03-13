#include <string>
#include <vector>
#include <memory>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>
#include <GL/glew.h>
#include <iostream>

namespace neptune {

    const char* vertexShaderSource = R"(
        attribute vec4 vertexPosition;
        uniform mat4 modelMatrix;
        uniform mat4 viewMatrix;
        uniform mat4 projectionMatrix;

        void main() {
            gl_Position = projectionMatrix * viewMatrix * modelMatrix * vertexPosition;
        }
    )";

    const char* fragmentShaderSource = R"(
        uniform vec3 color;

        void main() {
            gl_FragColor = vec4(color, 1.0);
        }
    )";

    void checkCompileErrors(GLuint shader, const std::string& type);
    GLuint compileShaders();

};