#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Primitives {
struct Material {
    glm::vec3 colour;
//    float ambient;
//    float diffuse;
//    float specular;
//    float shininess;
//    float reflective;
//    float transparency;
//    float refractiveIndex;
//
////        std::shared_ptr<Pattern> pattern;
//    bool shadow = true;
};

struct Shape {
    glm::mat4 inverseTransform;
    Material material;
    uint32_t typeEnum;
    uint32_t id;
};

struct Camera {
    glm::mat4 inverseTransform;
    float pixelSize;
    float halfWidth;
    float halfHeight;
};

Camera makeCamera(glm::vec4 position, glm::vec4 centre, glm::vec4 up, int hsize,
               int vsize, float fov)
{
    Camera camera {};
    
    glm::mat4 transform = glm::lookAt(glm::vec3(position),
                                glm::vec3(centre), glm::vec3(up));
    camera.inverseTransform = glm::affineInverse(transform);

    float halfView = glm::tan(fov / 2);
    float aspect = (float)hsize / (float)vsize;

    if (aspect >= 1)
    {
    camera.halfWidth = halfView;
    camera.halfHeight = halfView / aspect;
    }
    else
    {
      camera.halfWidth = halfView * aspect;
      camera.halfHeight = halfView;
    }
    camera.pixelSize = (camera.halfWidth * 2.0) / (float)hsize;
    
    return camera;
}

static int sNextId = 0;
uint32_t getNextId() { return ++sNextId; }

Shape makeSphere(Material& material, glm::mat4& transform) {
    Shape shape {};
    shape.id = getNextId();
    shape.typeEnum = 0;
    shape.inverseTransform = glm::affineInverse(transform);
    shape.material = material;
    
    return shape;
}
}
