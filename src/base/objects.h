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

namespace neptune {

struct Attribute {}; // simple temp stuff

struct Color : Attribute {
    Color(unsigned int r, unsigned int g, unsigned int b) : r(r), g(g), b(b) {};
    std::string name = "color";
    unsigned char r, g, b;
};

class Object {
public:
    Object() = default;
    virtual ~Object() {}
    std::string name;
    std::vector<std::unique_ptr<Attribute>> attributes;
    glm::vec3 position;
    glm::vec3 size;
    std::vector<float> vertices;
    std::vector<float> indices;
    virtual void draw(GLFWwindow* window) const = 0;
};

class Box : public Object {
public:
    Box() : color(255, 255, 255) {
        vertices = {
            // Position (x, y, z)
            -0.5f, -0.5f, 0.0f, // Bottom left
            0.5f, -0.5f, 0.0f, // Bottom right
            0.5f,  0.5f, 0.0f, // Top right
            -0.5f,  0.5f, 0.0f  // Top left
        };
        indices = {
            0, 1, 2, // First triangle
            2, 3, 0  // Second triangle
        };
        attributes.emplace_back(std::make_unique<Color>(color.r, color.g, color.b));
    }
    void draw(GLFWwindow* window) const override;
private:
    Color color;
};

};