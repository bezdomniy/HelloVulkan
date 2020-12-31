#pragma once

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace Primitives {
struct Material {
    glm::vec4 colour;
    float ambient;
    float diffuse;
    float specular;
    float shininess;
//    float reflective;
//    float transparency;
//    float refractiveIndex;
//
////        std::shared_ptr<Pattern> pattern;
//    bool shadow = true;
};

struct Shape {
    glm::mat4 inverseTransform;
    alignas(16) Material material;
    glm::vec4 data[6];
    uint32_t typeEnum;
    uint32_t id;
};

struct BVH {
    glm::mat4 inverseTransform;
    alignas(16) Material material;
    glm::vec4 nodes[];
};

struct Camera {
    glm::mat4 inverseTransform;
    float pixelSize;
    float halfWidth;
    float halfHeight;
    uint32_t width;
    uint32_t height;
};

Camera makeCamera(glm::vec4 position, glm::vec4 centre, glm::vec4 up, uint32_t hsize,
                  uint32_t vsize, float fov)
{
    Camera camera {};
    camera.width = hsize;
    camera.height = vsize;
    
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

Shape makePlane(Material& material, glm::mat4& transform) {
    Shape shape {};
    shape.id = getNextId();
    shape.typeEnum = 1;
    shape.inverseTransform = glm::affineInverse(transform);
    shape.material = material;
    
    return shape;
}

Shape makeTriangle(std::vector<glm::vec4>& params, Material& material, glm::mat4& transform) {
    Shape shape {};
    shape.id = getNextId();
    shape.typeEnum = 2;
    shape.inverseTransform = glm::affineInverse(transform);
    shape.material = material;
    
    for (int i = 0; i < 6; i++) {
        shape.data[i] = params.at(i);
    }
    
    return shape;
}

void parseObjFile(std::string const &path, std::vector<unsigned int> &vertexIndices,
                  //    std::vector<unsigned int> &uvIndices,
                      std::vector<unsigned int> &normalIndices,
                      std::vector<glm::vec4> &temp_vertices,
                      // std::vector<glm::dvec2> &temp_uvs,
                      std::vector<glm::vec4> &temp_normals) {
    std::string line;

    std::ifstream in(path, std::ios::in);

    if (!in)
    {
        std::cout << "Impossible to open the file: " << path << std::endl;
    }

    while (std::getline(in, line))
    {
        if (line.substr(0, 2) == "v ")
        {
            std::istringstream v(line.substr(2));
            glm::vec4 vertex;

            double x, y, z;
            v >> x;
            v >> y;
            v >> z;

            vertex = glm::vec4(x, y, z, 1.f);
            temp_vertices.push_back(vertex);
        }
        else if (line.substr(0, 2) == "vn")
        {
            std::istringstream vn(line.substr(3));
            glm::vec4 normal;

            double x, y, z;
            vn >> x;
            vn >> y;
            vn >> z;

            normal = glm::vec4(x, y, z, 0.f);
            temp_normals.push_back(normal);
        }
        else if (line.substr(0, 2) == "f ")
        {
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            std::istringstream f(line.substr(2));

            if (line.find('/') != std::string::npos)
            {
                for (int i = 0; i < 3; i++)
                {
                    std::string vStr, vtStr, vnStr;
                    std::getline(f, vStr, '/');
                    std::getline(f, vtStr, '/');
                    std::getline(f, vnStr, ' ');

                    vertexIndex[i] = atoi(vStr.c_str());
                    // uvIndex[i] = atoi(vtStr.c_str());
                    normalIndex[i] = atoi(vnStr.c_str());
                }
            }
            else
            {
                std::string vStr[3];
                std::getline(f, vStr[0], ' ');
                std::getline(f, vStr[1], ' ');
                std::getline(f, vStr[2], ' ');
                vertexIndex[0] = atoi(vStr[0].c_str());
                vertexIndex[1] = atoi(vStr[1].c_str());
                vertexIndex[2] = atoi(vStr[2].c_str());
            }

            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            // uvIndices.push_back(uvIndex[0]);
            // uvIndices.push_back(uvIndex[1]);
            // uvIndices.push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }
        else if (line.substr(0, 2) == "vt")
        {
            // std::istringstream vt(line.substr(3));
            // glm::dvec2 uv;
            // int U, V;
            // vt >> U;
            // vt >> V;
            // uv = glm::dvec2(U, V);
            // temp_uvs.push_back(uv);
        }
    }
}

BVH* makeBVH(std::string const &path, Material& material, glm::mat4& transform, uint32_t& size) {
    std::vector<unsigned int> vertexIndices;
//    std::vector<unsigned int> uvIndices;
    std::vector<unsigned int> normalIndices;
    std::vector<glm::vec4> temp_vertices;
    // std::vector<glm::dvec2> temp_uvs;
    std::vector<glm::vec4> temp_normals;

    parseObjFile(path, vertexIndices, normalIndices, temp_vertices, temp_normals);
    
    std::vector<glm::vec4> triangleParams;
    triangleParams.reserve((vertexIndices.size() / 3) * 6);

    for (unsigned int i = 0; i < vertexIndices.size(); i += 3) {
        triangleParams.push_back(temp_vertices[vertexIndices[i] - 1]);
        triangleParams.push_back(temp_vertices[vertexIndices[i + 1] - 1]);
        triangleParams.push_back(temp_vertices[vertexIndices[i + 2] - 1]);
        triangleParams.push_back(temp_normals[normalIndices[i] - 1]);
        triangleParams.push_back(temp_normals[normalIndices[i + 1] - 1]);
        triangleParams.push_back(temp_normals[normalIndices[i + 2] - 1]);
    }
    
    size = triangleParams.size() * sizeof(glm::vec4);
    
    BVH* bvh = (BVH*)malloc(sizeof(glm::mat4) + sizeof(Material) + size);
    
    bvh->inverseTransform =glm::affineInverse(transform);
    bvh->material = material;
    memcpy(bvh->nodes, triangleParams.data(), size);
    
    return bvh;
}

std::vector<Shape> makeModel(std::string const &path, Material& material, glm::mat4& transform)
{
    std::vector<unsigned int> vertexIndices;
//    std::vector<unsigned int> uvIndices;
    std::vector<unsigned int> normalIndices;
    std::vector<glm::vec4> temp_vertices;
    // std::vector<glm::dvec2> temp_uvs;
    std::vector<glm::vec4> temp_normals;

    parseObjFile(path, vertexIndices, normalIndices, temp_vertices, temp_normals);

    std::vector<Shape> triangles;
    triangles.reserve(vertexIndices.size() / 3);

    for (unsigned int i = 0; i < vertexIndices.size(); i += 3)
    {
        Shape nextTriangle;
        std::vector<glm::vec4> params;

        if (temp_normals.empty())
        {
            params = {temp_vertices[vertexIndices[i] - 1], temp_vertices[vertexIndices[i + 1] - 1], temp_vertices[vertexIndices[i + 2] - 1],glm::vec4(0.f),glm::vec4(0.f),glm::vec4(0.f)};
        }
        else
        {
            params = {temp_vertices[vertexIndices[i] - 1], temp_vertices[vertexIndices[i + 1] - 1], temp_vertices[vertexIndices[i + 2] - 1], temp_normals[normalIndices[i] - 1], temp_normals[normalIndices[i + 1] - 1], temp_normals[normalIndices[i + 2] - 1]};
        }
        nextTriangle = makeTriangle(params, material, transform);

        triangles.push_back(nextTriangle);
    }

    return triangles;
}
}
